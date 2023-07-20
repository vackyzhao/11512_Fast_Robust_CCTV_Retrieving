# 11512_Fast_Robust_CCTV_Retrieving

## 简介

本代码包含了两个专为硬件开发板设计的应用，旨在实现实时事件检测与上传功能。其中，Hi3516开发板应用主要通过板载摄像头捕获视频流，并进行人物活动的检测、目标跟踪与事件分类。得到的分类标签可以通过串口传输给Hi3861开发板。而3861开发板应用则通过UART从Hi3516处理器接收事件检测数据，并在OLED屏幕上直观显示给用户，同时通过MQTT协议将检测到的事件上传至云端服务器，实现实时数据传输与监控。
功能特点
### Hi3516开发板应用 - 人物活动检测与事件分类

实现了针对人物活动的检测与目标跟踪。
进行事件分类，得到分类标签。
实时将目标检测与跟踪的结果显示在板载MIPI显示屏上。
通过RTSP协议在设备所在局域网中广播捕获的视频流。

### 3861开发板应用 - 事件检测与上传

通过UART与Hi3516处理器进行连接，实时接收事件检测数据。
在OLED显示屏上直观展示检测结果，为用户提供实时的监控反馈。
利用MQTT协议，将检测到的事件上传至云端服务器，实现数据实时同步与远程监控。

## 使用指南
### Hi3516开发板应用 - 人物活动检测与事件分类

在Hi3516开发板上部署本代码，并确保板载摄像头可用。
运行应用程序，开始捕获视频流进行人物活动检测和目标跟踪。
获取分类标签，并通过串口传输给Hi3861开发板。
实时观察目标检测与跟踪结果在板载MIPI显示屏上的展示。
在设备所在局域网中，通过RTSP协议广播捕获的视频流。

### 3861开发板应用 - 事件检测与上传

在代码中修改MQTT服务器的地址、用户名、密码，以及Wi-Fi网络的名称和密码，确保与您的云端服务器和局域网连接正确。
使用适当的编译工具，编译本代码，并将生成的可执行文件烧录到3861开发板中。
启动开发板，系统将自动连接到指定Wi-Fi网络，并通过UART接收来自Hi3516的事件检测数据。
监控数据将实时在OLED显示屏上显示，用户可以直接观察检测结果。
检测到的事件将通过MQTT协议上传至您配置的云端服务器，您可以在云端进行数据分析和远程监控。

## 注意事项
### Hi3516开发板应用 - 人物活动检测与事件分类

本代码适用于Hi3516开发板，并需要可用的板载摄像头。
需要配置好相应的串口通信和RTSP协议设置，确保正常传输数据和视频流。

### 3861开发板应用 - 事件检测与上传

请确保您已经正确配置代码中的MQTT服务器和Wi-Fi网络信息，以确保数据传输的正确性和安全性。
本代码适用于3861开发板，并通过UART与Hi3516通信。如需在其他硬件平台上运行，请做相应适配。

## 文件结构

以下是代码仓库的文件结构：

```
    |-- hi3516 
    |   |-- smp
    |   |   |-- sample_rtsp.c 
    |   |-- README.md 
    |-- hi3861 
    |   |-- wifi
    |   |   |-- wifi_connect.c 
    |   |   |-- wifi_connect.h 
    |   |-- README.md
    |-- ResNet50 
    |   |-- checkpoint
    |   |   |-- ResNet50_model_0.pth 
    |   |-- save
    |   |   |-- analysis.csv 
    |   |-- utils 
    |   |   |-- preprocess.py 
    |   |-- __pycache__
    |   |-- dataset.py 
    |   |-- label_LUT.json 
    |   |-- main.py 
    |   |-- preprocess.py 
    |   |-- README.md 
    |   |-- resnet.caffemodel 
    |   |-- resnet.prototxt

```

## 版权信息

本开源项目遵循开源协议，您可以根据需要自由使用、修改和分发代码。详细的版权信息请查阅项目中的LICENSE文件。

欢迎参与项目的贡献和改进！如有任何问题或建议，请在GitHub上提交issue或联系项目维护者。

感谢您对本开源项目的支持与关注！
