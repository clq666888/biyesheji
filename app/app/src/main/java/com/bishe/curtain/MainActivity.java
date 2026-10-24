package com.bishe.curtain;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.webkit.JavascriptInterface;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicInteger;
import org.json.JSONObject;

public class MainActivity extends Activity {
    private static final int REQUEST_BT_CONNECT = 41;
    private static final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private WebView webView;
    private BluetoothSocket socket;
    private volatile BluetoothSocket pendingSocket;
    private OutputStream output;
    private volatile boolean connected;
    private volatile boolean stopped;
    private final Object writeLock = new Object();
    private final AtomicInteger connectGeneration = new AtomicInteger();

    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        webView = new WebView(this);
        setContentView(webView);
        webView.getSettings().setJavaScriptEnabled(true);
        webView.getSettings().setDomStorageEnabled(true);
        webView.setWebChromeClient(new WebChromeClient());
        webView.setWebViewClient(new WebViewClient());
        webView.addJavascriptInterface(new Bridge(), "AndroidBridge");
        webView.loadUrl("file:///android_asset/index.html");
        DiagnosticLogger.startup(this);
    }

    private boolean hasConnectPermission() {
        return Build.VERSION.SDK_INT < 31 || checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED;
    }

    private void notifyJs(String js) {
        runOnUiThread(() -> { if (webView != null) webView.evaluateJavascript(js, null); });
    }

    private void setStatus(String message, boolean isConnected) {
        connected = isConnected;
        notifyJs("window.nativeConnectionChanged && window.nativeConnectionChanged(" + JSONObject.quote(message) + "," + isConnected + ")");
    }

    private void chooseDevice() {
        recordDiagnostic("打开已配对设备列表");
        if (Build.VERSION.SDK_INT >= 31 && !hasConnectPermission()) {
            recordDiagnostic("缺少 BLUETOOTH_CONNECT 权限，发起权限请求");
            requestPermissions(new String[]{Manifest.permission.BLUETOOTH_CONNECT}, REQUEST_BT_CONNECT);
            return;
        }
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter == null) { recordDiagnostic("系统未提供 BluetoothAdapter"); toast("此手机不支持蓝牙"); return; }
        if (!adapter.isEnabled()) { recordDiagnostic("手机蓝牙当前未开启"); toast("请先在手机设置中打开蓝牙"); return; }
        Set<BluetoothDevice> bonded = adapter.getBondedDevices();
        if (bonded == null || bonded.isEmpty()) { recordDiagnostic("系统返回的已配对设备列表为空"); toast("请先在手机蓝牙设置中配对 SPP 模块"); return; }
        recordDiagnostic("系统返回已配对设备数量：" + bonded.size());
        List<BluetoothDevice> devices = new ArrayList<>(bonded);
        String[] labels = new String[devices.size()];
        for (int i = 0; i < devices.size(); i++) {
            BluetoothDevice d = devices.get(i);
            labels[i] = (d.getName() == null ? "Bluetooth device" : d.getName()) + "\n" + d.getAddress();
        }
        new AlertDialog.Builder(this).setTitle("选择已配对设备")
                .setItems(labels, (dialog, which) -> connect(devices.get(which)))
                .setNegativeButton("取消", null).show();
    }

    private void connect(BluetoothDevice device) {
        disconnect();
        int generation = connectGeneration.incrementAndGet();
        stopped = false;
        String deviceName;
        try { deviceName = device.getName(); }
        catch (SecurityException e) { deviceName = null; }
        final String selectedName = deviceName == null ? "SPP 设备" : deviceName;
        recordDiagnostic("选择设备：" + selectedName + "；bondState=" + device.getBondState()
                + "；type=" + device.getType() + "；开始安全 SPP/RFCOMM 连接");
        setStatus("正在连接 " + selectedName + "…", false);
        new Thread(() -> {
            BluetoothSocket candidate = null;
            Exception secureFailure;
            boolean usedFallback = false;
            try {
                candidate = device.createRfcommSocketToServiceRecord(SPP_UUID);
                pendingSocket = candidate;
                candidate.connect();
                recordDiagnostic("安全 SPP/RFCOMM 连接成功");
            } catch (Exception e) {
                secureFailure = e;
                recordDiagnostic("安全 SPP/RFCOMM 失败：" + describeException(e));
                closeQuietly(candidate);
                pendingSocket = null;
                if (generation != connectGeneration.get() || stopped) return;
                setStatus("安全 SPP 连接失败，正在尝试兼容连接…", false);

                candidate = null;
                try {
                    candidate = device.createInsecureRfcommSocketToServiceRecord(SPP_UUID);
                    pendingSocket = candidate;
                    candidate.connect();
                    usedFallback = true;
                    recordDiagnostic("兼容 SPP/RFCOMM 连接成功");
                } catch (Exception fallbackFailure) {
                    recordDiagnostic("兼容 SPP/RFCOMM 失败：" + describeException(fallbackFailure));
                    closeQuietly(candidate);
                    pendingSocket = null;
                    if (generation != connectGeneration.get() || stopped) return;
                    setStatus("连接失败；安全连接：" + describeException(secureFailure)
                            + "；兼容连接：" + describeException(fallbackFailure), false);
                    return;
                }
            }

            if (generation != connectGeneration.get() || stopped) {
                closeQuietly(candidate);
                pendingSocket = null;
                return;
            }
            try {
                socket = candidate;
                pendingSocket = null;
                output = candidate.getOutputStream();
                InputStream input = candidate.getInputStream();
                recordDiagnostic("BluetoothSocket 已连接；输入/输出流创建成功；方式="
                        + (usedFallback ? "兼容 RFCOMM（非安全）" : "安全 RFCOMM"));
                setStatus(usedFallback ? selectedName + "（兼容 RFCOMM）" : selectedName, true);
                readLoop(input, candidate);
            } catch (Exception e) {
                closeQuietly(candidate);
                if (socket == candidate) {
                    socket = null;
                    output = null;
                }
                if (generation == connectGeneration.get() && !stopped) {
                    recordDiagnostic("连接后初始化或接收异常：" + describeException(e));
                    setStatus("连接中断：" + describeException(e), false);
                }
            }
        }, "spp-connect").start();
    }

    private String describeException(Exception e) {
        String message = e.getMessage();
        String name = e.getClass().getSimpleName();
        return name + (message == null || message.trim().isEmpty() ? "" : ": " + message.trim());
    }

    private void recordDiagnostic(String message) {
        DiagnosticLogger.record(message);
    }

    private void readLoop(InputStream input, BluetoothSocket activeSocket) throws IOException {
        byte[] chunk = new byte[128];
        ArrayList<Integer> bytes = new ArrayList<>();
        while (!stopped && activeSocket.isConnected()) {
            int count = input.read(chunk);
            if (count < 0) throw new IOException("Bluetooth stream closed");
            for (int i = 0; i < count; i++) bytes.add(chunk[i] & 0xff);
            parseTelemetry(bytes);
        }
        if (!stopped) {
            recordDiagnostic("蓝牙流结束；socket.isConnected=" + activeSocket.isConnected());
            setStatus("蓝牙连接已断开", false);
        }
    }

    // Telemetry frame is fixed at 7 bytes: E2 80 weather temp humidity lightLo lightHi.
    private void parseTelemetry(ArrayList<Integer> bytes) {
        while (bytes.size() >= 2) {
            int start = -1;
            for (int i = 0; i + 1 < bytes.size(); i++) {
                if (bytes.get(i) == 0xE2 && bytes.get(i + 1) == 0x80) { start = i; break; }
            }
            if (start < 0) {
                int keep = bytes.get(bytes.size() - 1) == 0xE2 ? 1 : 0;
                while (bytes.size() > keep) bytes.remove(0);
                return;
            }
            for (int i = 0; i < start; i++) bytes.remove(0);
            if (bytes.size() < 7) return;
            int weather = bytes.get(2), temp = bytes.get(3), humidity = bytes.get(4);
            int light = bytes.get(5) | (bytes.get(6) << 8);
            for (int i = 0; i < 7; i++) bytes.remove(0);
            notifyJs("window.updateDeviceTelemetry && window.updateDeviceTelemetry(" + weather + "," + temp + "," + humidity + "," + light + ")");
        }
    }

    private boolean sendHex(String hex) {
        if (!connected || output == null) return false;
        String[] parts = hex.trim().split("\\s+");
        byte[] data = new byte[parts.length];
        try {
            for (int i = 0; i < parts.length; i++) data[i] = (byte) Integer.parseInt(parts[i], 16);
            synchronized (writeLock) { output.write(data); output.flush(); }
            recordDiagnostic("发送成功；帧长度=" + data.length + " 字节（内容未记录）");
            return true;
        } catch (Exception e) {
            recordDiagnostic("发送失败：" + describeException(e));
            setStatus("发送失败，蓝牙连接可能已断开", false);
            disconnect();
            return false;
        }
    }

    private void disconnect() {
        if (connected || pendingSocket != null) recordDiagnostic("应用主动关闭蓝牙连接");
        connectGeneration.incrementAndGet();
        stopped = true;
        connected = false;
        BluetoothSocket old = socket;
        BluetoothSocket pending = pendingSocket;
        socket = null;
        pendingSocket = null;
        output = null;
        closeQuietly(old);
        if (pending != old) closeQuietly(pending);
    }

    private void closeQuietly(BluetoothSocket s) { if (s != null) try { s.close(); } catch (IOException ignored) { } }
    private void toast(String s) { runOnUiThread(() -> Toast.makeText(this, s, Toast.LENGTH_LONG).show()); }

    @Override public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(requestCode, permissions, results);
        if (requestCode == REQUEST_BT_CONNECT) {
            if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) {
                recordDiagnostic("用户已授予 BLUETOOTH_CONNECT 权限");
                chooseDevice();
            } else {
                recordDiagnostic("用户拒绝 BLUETOOTH_CONNECT 权限");
                toast("需要“附近设备”权限才能连接蓝牙模块");
            }
        }
    }

    @Override protected void onDestroy() { disconnect(); if (webView != null) webView.destroy(); super.onDestroy(); }

    private final class Bridge {
        @JavascriptInterface public void chooseDevice() { runOnUiThread(MainActivity.this::chooseDevice); }
        @JavascriptInterface public boolean sendFrame(String hex) { return sendHex(hex); }
        @JavascriptInterface public boolean isConnected() { return connected; }
        @JavascriptInterface public void disconnect() { runOnUiThread(() -> { MainActivity.this.disconnect(); setStatus("已断开", false); }); }
    }
}



