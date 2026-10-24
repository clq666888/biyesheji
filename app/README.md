# 智能窗帘蓝牙上位机 Android 项目

本项目使用 Android WebView 承载界面原型，并由 Java 通过经典蓝牙 SPP 与已配对的 HC 系列串口蓝牙模块通信。

## 当前实现范围

- 选择系统设置中已经配对的蓝牙模块并建立 SPP 连接。
- 发送窗帘开关、运行模式、光照/湿度阈值、手机时间同步数据帧。
- 接收 `E2 80` 七字节状态帧，更新天气、温度、湿度和光照显示。
- 蓝牙连接异常时显示断开；状态帧超过约 3.5 秒未到达时提示数据超时。
- 中英文界面沿用现有 HTML 原型。

## 构建

需要 Android Studio、Android SDK 35 和 JDK 17。用 Android Studio 打开本目录，等待 Gradle 同步后构建 APK。此开发环境目前未检测到 Android SDK 或 Gradle，因此暂时只能交付源码，尚未生成 APK。

## APK 体积估算

这是不带大型第三方库的 Java + WebView 应用。粗估：debug APK 约 3–6 MB，release APK 约 2–4 MB；最终大小以实际构建结果为准。
