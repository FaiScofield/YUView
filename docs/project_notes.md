# YUView Project Notes

## TODO

- UI 方面
  - [x] 调整 `YuvCustomFormat` 窗口控件逻辑，允许 `planar` 和 `byte-packed` 共存
  - [ ] 增加 `rowPitch(widthStride) / heightStride` 输入框，用于设定虚宽和虚高（UI已完成，但取数逻辑未完成）
  - [x] 调整 `CustionFormat` 窗口为可停靠窗口，方便设置
  - [x] RGB custom UI 控件调整，加入 interleaved, alphaChannel改为combox, 加入 `bytepacking` 和 `paddingInfo` 选项
- YUV 图像格式方面
  - [x] 支持 NV15/NV20/NV30 等10bit packed 格式显示 （已完成 ，但放大后显示的像素值还有问题）
  - [x] 10bit unbytepacking 格式支持调整对齐 padding 的位置 （`getName()`用于比较像个像素是否相等，未引入`paddingInfo`，导致比较时新旧像素被判定为一致）
  - [x] YUV422I 10bit 转到 SP 时崩溃， 打开 bytepacking 崩溃（src_stride 计算错误导致取数越界）
  - [ ] YUV格式改 `Subsampling` 和 `ComponentLayout` 会导致频繁更新 `ComponentOrder` 控件，进而导致频繁触发 `formatChanged` 信号，待调整
  - [x] `ComponentOrder` 存在重复的枚举值，导致解析名字时不对，待解决
  - [x] `PixelFormatYUV` 合并到开发分支
  - [x] `DataLayout` 和 `ComponentLayout` 数据重复，可以合并
  - [ ] 支持 YUV420I_LEGACY 8bit 格式
  - [ ] `PaddingInfo` 在 depth=8/16 时应该只能选 `NoPadding`, 否则只能 `PaddingInLsb/Msb` 二选一
  - [ ] 引入别名 `alias`来预设一些常用的格式
- RGB 图像格式方面
  - [x] 引入 RGB332/RGB565/RGBA5551/RGBA1010102 等通道位宽不一致的像素格式支持
  - [x] 支持 rgb  planar bytepacking 格式
  - [ ] 修正 RGB332 等格式的放大像素显示错误
- [ ] 增加配置文件，用于自定义格式的取数方式
- [ ] 丰富文件名格式猜测功能
- [x] 增加 spdlog 作为日志库，替换 Qt 的日志系统
- [ ] 命令行参数增加日志等级参数 （日志等级未传递到 YUViewLib 中）
- [x] 改为手动 UIC，ui没变的情况下避免每次编译都要重新编译很多文件 （正确做法是取消对每次编译都会变的变量进行`add_definitions()`）

## YUView 三个关键场景的函数调用链分析

### 场景一：raw yuv 文件从被拖到播放列表到显示窗口显示对应的图像

1. **文件拖放处理**
   - `PlaylistTreeWidget::dropEvent` - 处理文件拖放事件，提取文件路径
   - `PlaylistTreeWidget::loadFiles` - 加载拖放的文件

2. **文件类型检测与播放列表项创建**
   - `playlistItems::createPlaylistItemFromFile` - 根据文件类型创建对应的播放列表项
   - `playlistItems:guessFileTypeFromFileAndCreatePlaylistItem` - 尝试根据文件扩展名自动检测文件类型
   - `new playlistItemRawFile` - 创建raw yuv文件的播放列表项实例

3. **YUV文件初始化与格式检测**
   - `playlistItemRawFile::playlistItemRawFile` - 构造函数，初始化文件源
   - `setFormatFromFileName` - 尝试从文件名中提取格式信息
   - `videoHandler::setFrameSize` - 设置视频帧大小
   - `videoHandler::guessAndSetPixelFormat` - 猜测并设置像素格式

4. **视频加载与渲染**
   - `playlistItemWithVideo::loadItem` - 加载视频项
   - `videoHandler::loadFrame` - 加载视频帧
   - `playlistItemWithVideo::drawItem` - 绘制视频项
   - `videoHandler::drawFrame` - 绘制视频帧到显示窗口

### 场景二：文件对应的图像格式被改变

1. **格式设置触发**
   - `playlistItemRawFile::propertiesWidget` - 格式设置界面的属性变更
   - `videoHandlerYUV::setPixelFormatYUVByName` - 根据名称设置YUV像素格式

2. **格式更新与缓存处理**
   - `FrameHandler::setFormatFromString` - 从字符串设置格式
   - `videoHandler::setFrameSize` - 更新帧大小
   - `videoHandler::clearFrameCache` - 清除旧格式的帧缓存
   - `emit SignalItemChanged` - 发出项目变更信号

3. **重新加载与渲染**
   - `PlaylistTreeWidget::slotItemChanged` - 处理项目变更信号
   - `playlistItem::loadItem` - 重新加载项目
   - `videoHandler::loadFrame` - 以新格式加载帧
   - `playlistItem::drawItem` - 以新格式绘制项目

### 场景三：鼠标滚轮在显示窗口的改变图像的显示缩放倍率

1. **鼠标滚轮事件处理**
   - `MoveAndZoomableView::wheelEvent` - 处理鼠标滚轮事件
   - `MoveAndZoomableView::zoom` - 执行缩放操作

2. **缩放计算与应用**
   - `MoveAndZoomableView::setZoomFactor` - 设置新的缩放因子
   - `splitViewWidget::setZoomFactor` - 应用缩放因子到分割视图

3. **重新绘制**
   - `splitViewWidget::update` - 触发视图更新
   - `splitViewWidget::paintEvent` - 处理绘制事件
   - `playlistItem::drawItem` - 以新缩放因子绘制项目
   - `videoHandler::drawFrame` - 以新缩放因子绘制视频帧

### 场景四： RGB 格式变化

```c++

playlistItemRawFile::loadRawData()
videoHandlerRGB::loadRawRGBData()

/// videoHandlerRGB.cpp:
// `R/G/B/AInvertCheckBox, R/G/B/AScaleSpinBox, limitedRangeCheckBox, colorComponentsComboBox`变化，触发信号 `slotDisplayOptionsChanged`
videoHandler::setCacheInvalid(); // videoHandler::cacheValid = false
emit videoHandler::signalHandlerChanged(bool redrawNeeded=true, recacheIndicator recache=RECACHE_CLEAR); // 该信号由 PlaylistItemXXX 绑定，会跳到对应的类型文件内槽函数

// 触发以下槽函数
playlistItemWithVideo::slotVideoHandlerChanged(); // 调用如下函数
   playlistItemWithVideo::updateStartEndRange();
   emit playlistItem::SignalItemChanged(redrawNeeded, recache);

// 触发以下槽函数
PlaylistTreeWidget::slotItemChanged
   emit PlaylistTreeWidget::selectedItemChanged(redraw); // 触发该信号
   emit PlaylistTreeWidget::signalItemRecache(senderItem, recache);

// 触发以下槽函数
MainWindow::fileInfoAdapter // 由 PlaylistTreeWidget::selectionRangeChanged / PlaylistTreeWidget::selectedItemChanged 触发
   setInfo()
playlistItemRawFile::getInfo()
playlistItemRawFile::slotVideoPropertiesChanged // 调用如下函数
   videoHandlerRGB::getFormatAsString(); // 用于对比新旧格式是否相同
   itemMemoryHandler::itemMemoryAddFormat(); // 如果不同则


playlistItemWithVideo::loadFrame();
videoHandlerRGB::loadRawRGBData(int frameIndex)

videoHandlerRGB::loadFrame();
videoHandlerRGB::convertRGBToImage();
videoHandlerRGB::convertSourceToRGBA32Bit();
```

### 总结

以上三个场景涵盖了YUView中从文件拖放到显示、格式变更到缩放控制的核心流程。每个流程都涉及多个组件和函数的协作，共同实现了YUV文件的高效处理和显示。

- **拖放与加载**：通过文件系统操作和类型检测，实现了不同格式视频的自动识别和加载
- **格式管理**：提供了灵活的格式设置机制，支持动态调整视频参数
- **交互控制**：通过鼠标事件处理，实现了直观的缩放控制功能

这些流程的实现展示了YUView作为专业视频分析工具的设计思路和技术实现，为用户提供了便捷、高效的视频处理体验。
