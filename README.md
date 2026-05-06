# YUView <img align="right" src="https://raw.githubusercontent.com/IENT/YUView/develop/YUViewLib/images/IENT-YUView-256.png">

YUView is a QT based, cross-platform YUV player with an advanced analytic toolset.

## Change Log

### v3.1.0

1. 增加虚宽（指行步距，单位：byte）虚高（单位：pixel）支持
2. 文件右键增加“Change File Type”选项用于切换文件类型，并修正原代码错误

### v3.0.3

1. YUV 格式增强：增加更多 YUV 格式支持，添加预设的 VU30 格式支持，修正多种 YUV 格式的显示问题，修复 VU30 格式的属性赋值和 UI 更新问题
2. 缓存机制改进：实现基于版本号的缓存机制和分辨率验证机制，避免程序崩溃
3. Padding 支持：增强对 paddingInfo 的支持，修复放大后 paddingInfo 改变像素值却不改变的问题
4. UI 改进：重构 YUV 自定义格式对话框

### v3.0.2

1. RGB 功能增强：实现 RGB bitpacking 格式的单通道显示功能，RGB 放大后显示像素值支持不同的 order，RGB 格式通道顺序支持及 Alpha 通道修复，取消 RGBA 预乘显示
2. Bug 修复：修复控件状态联动和互斥逻辑，修复 RGBA5551/RGBA1010102 格式中 alpha 和 padding 互斥逻辑
3. 命令行参数：程序增加更多命令行参数
4. 文档：添加 YUV 图像格式详细说明文档

### v3.0.1

1. RGB 功能增强：添加完整的 RGB bytepacking 格式支持（RGB332/RGB565/RGBA5551/RGBA1010102 等），支持位深度 1-32 位，新增 "Ignore Alpha" 显示选项，RGB 自定义格式对话框重构为可停靠控件，实时响应格式变化
2. Bug 修复：修复 YUV400 格式下的组件顺序和布局问题，修复 RGB 像素格式解析和显示问题，修复构建脚本和日志级别显示问题
3. 文档与工具：完善 Doxygen 文档配置和注释，更新项目文档和待办事项列表，调整 FrameHandler 界面布局

---

### v3.0.0 (from official 18b1c69e)

1. 构建系统改进：完成 CMakeLists.txt 构建支持，新增打包工具脚本支持创建 Windows 安装包，版本号改为通过头文件自动生成以提升编译效率
2. 日志系统重构：集成 spdlog v1.16.0 作为统一调试日志系统，替换原有的自定义调试宏，简化代码并提高可维护性
3. YUV 格式增强：添加 NV30/NV20/NV15 预置格式支持，完善 YUV 10bit 422 bytepacking 格式解包逻辑，支持 padding 位置变化显示，UI 控件支持选择 YUV420I 格式
4. UI 与功能改进：主窗口显示版本号，移除自动更新提示，重构像素格式数据布局枚举和组件布局管理逻辑，运行启动程序支持参数设置日志等级和日志文件路径


## Build Status

![CI build](https://github.com/IENT/YUView/workflows/CI%20build/badge.svg?branch=develop)

## Description

At its core, YUView is a YUV player and analysis tool. However, it can do so much more:
* simple navigation/zooming in the video
* support for a wide variety of YUV formats using various subsamplings and bit depts
* support for raw RGB files, image files and image sequences
* direct decoding of raw h.265/HEVC bitstreams with visualization of internals like prediction modes and motion vectors and many more
* interface with visualization for the reference software decoders HM and JEM
* support for opening almost any file using FFmpeg
* image comparison using side-by-side and comparison view
* calculation and display of differences (in YUV or RGB colorspace)
* save and load playlists
* overlay the video with statistics data
* ... and many more

Further details of the features can be found either [here](http://ient.github.io/YUView) or
in the [wiki](https://github.com/IENT/YUView/wiki).

Screenshot of YUView:

![YUView Main Window](https://raw.githubusercontent.com/IENT/YUView/gh-pages/images/Overview.png)

## Download

You can download precompiled binaries for Windows and MAC from the [release site](https://github.com/IENT/YUView/releases) which are all compiled on Github Actions. On the release page you can find:

 - Windows installer
 - Windows zip
 - Mac OS Application
 - Linux Appimage

On MacOS, just extract the zip file to your Application folder and remove it from quarantine:

> xattr -d com.apple.quarantine /Applications/YUView.app

If you have Ubuntu 22.04 or newer, you can get YUView from the official repo: `sudo apt install yuview`. For other Linux based platforms we are also on [flathub](https://flathub.org/apps/details/de.rwth_aachen.ient.YUView). More information on YUView on Linux can be found in out wiki page ["YUView on Linux"](https://github.com/IENT/YUView/wiki/YUView-on-Linux).

If none of these apply to you, you can easily [build YUView yourself](https://github.com/IENT/YUView/wiki/Compile-YUView).

## Building

Compiling YUView from source is easy! We use qmake for the project so on all supported platforms you just have to install qt and run `qmake` and `make` to build YUView. There are no further dependent libraries. Alternatively, you can use the QTCreator if you prefer a GUI. More help on building YUView can be found in the [wiki](https://github.com/IENT/YUView/wiki/Compile-YUView).
