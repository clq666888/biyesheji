package com.bishe.curtain;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.os.Build;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

final class DiagnosticLogger {
    private static final String TAG = "CurtainBluetooth";

    private DiagnosticLogger() { }

    static void startup(Context context) {
        String version = "unknown";
        try {
            version = context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionName;
        } catch (Exception ignored) { }
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        String bluetoothState = "unsupported";
        if (adapter != null) {
            try { bluetoothState = adapter.isEnabled() ? "on" : "off"; }
            catch (SecurityException e) { bluetoothState = "permission blocked"; }
        }
        record("应用启动；应用版本=" + version + "；设备=" + Build.MANUFACTURER + "/" + Build.MODEL
                + "；Android=" + Build.VERSION.RELEASE + " (SDK " + Build.VERSION.SDK_INT + ")"
                + "；Bluetooth=" + bluetoothState + "；构建=DEBUG");
    }

    static void record(String message) {
        String timestamp = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.US).format(new Date());
        Log.i(TAG, timestamp + "  " + message);
    }
}
