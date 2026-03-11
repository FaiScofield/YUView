# YUView Project Notes

## TODO

- UI 方面
  - [x] 调整 `YuvCustomFormat` 窗口控件逻辑，允许 `planar` 和 `byte-packed` 共存
  - [ ] 增加 `rowPitch(widthStride) / heightStride` 输入框，用于设定虚宽和虚高（UI已完成，但取数逻辑未完成）
  - [x] 调整 `CustionFormat` 窗口为可停靠窗口，方便设置
- 图像格式方面
  - [x] 支持 NV15/NV20/NV30 等10bit packed 格式显示 （已完成 ，但放大后显示的像素值还有问题）
  - [x] 10bit unbytepacking 格式支持调整对齐 padding 的位置 （`getName()`用于比较像个像素是否相等，未引入paddingInfo，导致比较时新旧像素被判定为一致）
  - [ ] YUV422I 10bit 转到 SP 时崩溃， 打开 bytepacking 崩溃（`YUVPlaneToRGB_422()函数改进`）
  - [ ] YUV格式改 `Subsampling` 和 `ComponentLayout` 似乎会多次触发 `formatChanged` 信号，待分析解决
  - [x] `ComponentOrder` 存在重复的枚举值，导致解析名字时不对，待解决
  - [ ] `PixelFormatYUV` 兼容性重构，合并到开发分支
  - [ ] `DataLayout` 和 `ComponentLayout` 数据重复，可以合并
- [ ] 增加配置文件，用于自定义格式的取数方式
- [ ] 丰富文件名格式猜测功能
- [x] 增加 spdlog 作为日志库，替换 Qt 的日志系统
- [ ] 命令行参数增加日志等级参数
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

### 总结

以上三个场景涵盖了YUView中从文件拖放到显示、格式变更到缩放控制的核心流程。每个流程都涉及多个组件和函数的协作，共同实现了YUV文件的高效处理和显示。

- **拖放与加载**：通过文件系统操作和类型检测，实现了不同格式视频的自动识别和加载
- **格式管理**：提供了灵活的格式设置机制，支持动态调整视频参数
- **交互控制**：通过鼠标事件处理，实现了直观的缩放控制功能

这些流程的实现展示了YUView作为专业视频分析工具的设计思路和技术实现，为用户提供了便捷、高效的视频处理体验。
