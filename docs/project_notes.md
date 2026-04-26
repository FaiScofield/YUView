# YUView Project Notes

[TOC]

## TODO

- UI 方面
  - [x] 调整 `YuvCustomFormat` 窗口控件逻辑，允许 `planar` 和 `byte-packed` 共存
  - [ ] 增加 `rowPitch(widthStride) / heightStride` 输入框，用于设定虚宽和虚高（UI已完成，但取数逻辑未完成）
  - [x] 调整 `CustionFormat` 窗口为可停靠窗口，方便设置
  - [x] RGB custom UI 控件调整，加入 interleaved, alphaChannel改为combox, 加入 `bytepacking` 和 `paddingInfo` 选项
  - [ ] NV15 等格式绘制出的像素值不是10bit, 宽度翻倍后放大要绘制像素时崩溃 （`videoHandlerYUV::getPixelValue()`）
  - [x] 增加一个按钮 "Ignore Alpha"，用于忽略 Alpha 通道，只显示 RGB 颜色
- 图像格式方面
  - 整体
    - [ ] 引入别名 `alias`来预设一些常用的格式
    - [ ] 丰富文件名格式猜测功能
    - [ ] 增加配置文件，用于自定义格式的取数方式
    - [ ] 放大后显示的像素黑/白色应该根据实际像素深度来判断
    - [x] 引入RGB格式成员变量，解决RGB文件加载后没有对应更新ui控件的问题
    - [ ] 搞清楚RGB文件加载失败后的处理是什么逻辑
  - YUV 图像格式
    - [x] 支持 NV15/NV20/NV30 等10bit packed 格式显示 （已完成 ，但放大后显示的像素值还有问题）
    - [x] 10bit unbytepacking 格式支持调整对齐 padding 的位置 （`getName()`用于比较像个像素是否相等，未引入`paddingInfo`，导致比较时新旧像素被判定为一致）
    - [x] YUV422I 10bit 转到 SP 时崩溃， 打开 bytepacking 崩溃（src_stride 计算错误导致取数越界）
    - [x] YUV格式改 `Subsampling` 和 `ComponentLayout` 会导致频繁更新 `ComponentOrder` 控件，进而导致频繁触发 `formatChanged` 信号，待调整
    - [x] `ComponentOrder` 存在重复的枚举值，导致解析名字时不对，待解决
    - [x] `PixelFormatYUV` 合并到开发分支
    - [x] `DataLayout` 和 `ComponentLayout` 数据重复，可以合并
    - [ ] 支持 YUV420I_LEGACY 8bit 格式
    - [ ] `PaddingInfo` 在 depth=8/16 时应该只能选 `NoPadding`, 否则只能 `PaddingInLsb/Msb` 二选一
    - [x] 修正 YUV400 不支持色彩空间选择的问题；YUV400 应该 disable 掉 componentOrder 控件
  - RGB 图像格式
    - [x] 引入 RGB332/RGB565/RGBA5551/RGBA1010102 等通道位宽不一致的像素格式支持
    - [x] 支持 rgb planar bytepacking 格式
    - [x] 修正 RGB332 等格式的放大像素显示错误
    - [x] 查看 setting 里 `RGB5651010102` 字符串是哪里来的（`PixelFormatRGB::getName()`输出错误）
    - [ ] 查看 RGBA5551/RGBA1010102 **invertAlpha 选项不生效的原因**
    - [ ] 修正 RGBA1010102 Alpha 通道的显示问题，A=3时应该映射到255
    - [x] RGBA5551/RGBA1010102 Alpha 可以改为 Padding
    - [ ] RGB332/RGB565/RGBA5551/RGBA1010102 支持选择 order (显示部分还未支持)
- 其他
  - [x] 增加 spdlog 作为日志库，替换 Qt 的日志系统
  - [x] 命令行参数增加日志等级参数 （日志等级未传递到 YUViewLib 中）
  - [x] 改为手动 UIC，ui没变的情况下避免每次编译都要重新编译很多文件 （正确做法是取消对每次编译都会变的变量进行`add_definitions()`）

## RGB 图像格式

### RGB 格式命名规则

`PixelFormatRGB::getName()` 函数根据像素格式属性生成格式名称，遵循以下规则：

#### 1. DiffCompDepth 类格式

DiffCompDepth 类格式代表 R/G/B/A 通道之间至少有一个通道的位宽不一致。 代表性格式为: **RGB332**、**RGB565**、**RGBA5551**、**RGBA1010102**。

特点：

- 通道之间至少有一个通道的bit位数不一致，且一定是 BytePacking 格式，一定是 interleaved 排列
- RGBA5551 / RGBA1010102 有 Alpha 通道，其他格式没有 Alpha 通道
- RGBA5551 / RGBA1010102 的 Alpha 通道数据用户可能不关心，那么它就属于填充（Padding）内容

命名格式：`<通道字母序列><位数序列>`

- **通道字母序列**：按 MSB 到 LSB 的顺序排列
- **位数序列**：与通道字母序列一一对应的位数，连续排列
- **Alpha/Padding**：
  - 在通道字母序列开头 = 位于 MSB 端
  - 在通道字母序列结尾 = 位于 LSB 端
  - `A` 表示 Alpha 通道，`X` 表示 Padding（无效位）

| 格式类型选项 | channelOrder | alphaMode/paddingInfo | 通道字母序列 | 位数序列 | 最终名字 |
|---------|-------------|---------------------|------------|---------|---------|
| BPP8_RGB332 | RGB | - | RGB | 332 | RGB332 |
| BPP8_RGB332 | GBR | - | GBR | 323 | GBR323 |
| BPP16_RGB565 | BGR | - | BGR | 565 | BGR565 |
| BPP16_RGBA5551 | RGB | InLsb | RGBA | 5551 | RGBA5551 |
| BPP16_RGBA5551 | RGB | InMsb | ARGB | 1555 | ARGB1555 |
| BPP16_RGBA5551 | BGR | PaddingInLSB | BGRX | 5551 | BGRX5551 |
| BPP16_RGBA5551 | BGR | PaddingInMSB | XBGR | 1555 | XBGR1555 |
| BPP32_RGBA1010102 | RGB | InLsb | RGBA | 1010102 | RGBA1010102 |
| BPP32_RGBA1010102 | RGB | InMsb | ARGB | 2101010 | ARGB2101010 |

#### 2. 其他 BytePacking 格式

R/G/B/A 所有通道位宽一致且按bit紧凑排列

特点：

- 通道之间位宽一致，按bit紧凑排列
- 不存在填充数据
- 可以是 interleaved 排列，也可以是 planar 排列
- 可以是小端序存储，也可以是大端序存储

命名格式：`[A/X]<order>[A/X] <bits>bit bytepacking [planar] [BE]`

- **通道顺序**：从 MSB 到 LSB
- **Alpha/Padding 位置**：
  - 在开头 = 位于 MSB 端
  - 在结尾 = 位于 LSB 端
- 后跟 `<bits>bit bytepacking`
- 可选 `planar` 格式，将通道数据按每个通道一个平面存储
- 可选 `BE` 格式，大端序存储

示例：

- `BGRA 10bit bytepacking` - Alpha 在 LSB 端
- `ABGR 10bit bytepacking planar` - Alpha 在 MSB 端， 且每个通道一个平面
- `BGRX 10bit bytepacking` - Padding 在 LSB 端
- 不会存在 `BGRX 10bit bytepacking planar` 格式，因为 Padding 通道属于不关心的内容，不会单独放到一个平面存储

#### 3. 普通格式

非 BytePacking 的普通格式

特点：

- 通道之间位宽一致
- 在 `depth % 8 != 0` 时必然存在填充数据
- 可以是 interleaved 排列，也可以是 planar 排列
- 可以是小端序存储，也可以是大端序存储

格式：`[A]<order>[A] <bits>bit [paddingInfo] [planar] [BE]`

- **通道顺序**：直接使用 channelOrder（LSB 到 MSB）
- **Alpha/Padding 位置**：
  - `AlphaMode::First` / `PaddingInLSB` = 在开头
  - `AlphaMode::Last` / `PaddingInMSB` = 在结尾
- 后跟 `<bits>bit`，可选 `planar`、`paddingInfo` 和 `BE`

示例：

- `RGB 8bit` - 交织存储，从低位到高位分别是 R、G、B 通道，每个通道8bit
- `ARGB 16bit` - 交织存储，Alpha 在低位（First），每个通道16bit
- `RGBA 10bit paddingInLsb planar` - 4通道按平面存储, Alpha 在最后一个平面（Last），`paddingInLsb` 表示每一个word里10bit有效数据在高位，低位6bit为填充
- `RGB 10bit paddingInMsb` - 10bit RGB， 交织存储，`paddingInMsb` 表示每一个word里10bit有效数据在低位，高位6bit为填充

### RGB 格式控件调整逻辑

#### `Ui::CustomRGBFormatDialog` 控件列表

| 控件对象 | 对应 PixelFormatRGB 的成员变量 | 取值范围 | 和其他控件的联动关系 |
|---------|-------------------------------|---------|-------------------|
| `rgbOrderComboBox` | `channelOrder` | `RGB`, `RBG`, `GRB`, `GBR`, `BRG`, `BGR` | 始终启用 |
| `bitDepthSpinBox` | `bitsPerSample` | 1 - 32 | DiffCompDepth 格式时禁用，根据 `comboBoxDiffType` 自动设置；<br>其他格式时始终启用 |
| `comboBoxEndianness` | `endianness` | `Big Endian`, `Little Endian` | DiffCompDepth 格式时仅当类型为非 `BPP8_RGB332` 时启用(bpp>8)；<br>其他格式时仅当 `bitsPerSample > 8` 时启用 |
| `comboBoxAlphaPos` | `alphaMode` | `NoAlpha`, `First (InLsb)`, `Last (InMsb)` | DiffCompDepth 格式时仅 `RGBA5551/RGBA1010102` 类型启用，选择非 `NoAlpha` 时会禁用 `comboBoxPaddingPos`;<br> 其他格式时始终启用 |
| `comboBoxPaddingPos` | `paddingInfo` | `NoPadding`, `PaddingOnMsb`, `PaddingOnLsb` | DiffCompDepth 格式时仅 `RGBA5551/RGBA1010102` 类型且 `comboBoxAlphaPos == NoAlpha` 时启用，选择非 `NoPadding` 时会禁用 `comboBoxAlphaPos`；<br> 普通格式时仅当 `bitsPerSample % 8 != 0` 时启用；勾选 BytePacking 时始终禁用并设为 `NoPadding` |
| `planarCheckBox` | `dataLayout` | `true` (Planar), `false` (Interleaved) | DiffCompDepth 格式时禁用并强制为 false；<br>其他格式时启用，勾选时会禁用 `groupBoxDiffCompDepth` |
| `checkBoxBytePacking` | `bytePacking` | `true` (启用), `false` (禁用) | DiffCompDepth 格式时禁用并强制为 true；<br>其他格式时启用，仅当 `bitsPerSample % 8 != 0` 时可选 |
| `groupBoxDiffCompDepth` | `diffCompType` (是否启用) | `true` (启用), `false` (禁用) | 启用时会禁用 `planarCheckBox` 和 `checkBoxBytePacking`，`planarCheckBox` 勾选时会禁用该控件 |
| `comboBoxDiffType` | `diffCompType` | `BPP8_RGB332`, `BPP16_RGB565`, `BPP16_RGBA5551`, `BPP32_RGBA1010102` | 仅在 `groupBoxDiffCompDepth` 启用时可用，选择不同类型会影响 `bitDepthSpinBox` 和 `comboBoxEndianness` 的启用状态 |
| `labelRgbFmtName` | - (仅显示) | - | 显示当前 PixelFormatRGB 的 `getName()` 返回值，跟随其他控件变化更新 |

#### 三种格式类型的控件状态

| 控件对象 | 1. DiffCompDepth 类格式 | 2. 其他 BytePacking 格式 | 3. 普通格式 |
|---------|------------------------|-------------------------|-----------|
| **触发条件** | `groupBoxDiffCompDepth` 被勾选 | `groupBoxDiffCompDepth` 未勾选且 `checkBoxBytePacking` 勾选 | `groupBoxDiffCompDepth` 和 `checkBoxBytePacking` 都未勾选 |
| `groupBoxDiffCompDepth` | 勾选 | 未勾选 | 未勾选 |
| `comboBoxDiffType` | 启用，选择类型 | 禁用 | 禁用 |
| `bitDepthSpinBox` | 禁用，根据 `comboBoxDiffType` 自动设置 | 启用 | 启用 |
| `rgbOrderComboBox` | 启用 | 启用 | 启用 |
| `comboBoxEndianness` | `comboBoxDiffType == BPP8_RGB332` 时禁用，其他启用 | 启用 | `bitsPerSample > 8` 时启用，否则禁用 |
| `planarCheckBox` | 禁用，强制设置为 false | 启用 | 启用 |
| `checkBoxBytePacking` | 禁用，强制设置为 true | 勾选 | 未勾选 |
| `comboBoxAlphaPos` | `RGBA5551/RGBA1010102` 类型仅当 `comboBoxPaddingPos == NoPadding` 时启用，否则禁用并设为 `NoAlpha`(此时和`comboBoxPaddingPos` 互斥)；其他类型禁用并设为 `NoAlpha`； | 启用 | 启用 |
| `comboBoxPaddingPos` | `RGBA5551/RGBA1010102` 类型仅当 `comboBoxAlphaPos == NoAlpha` 时启用，否则禁用并设为 `NoPadding`(此时和`comboBoxAlphaPos` 互斥)；其他类型禁用并设为 `NoPadding` | 禁用，强制设置为 `NoPadding` | `bitsPerSample % 8 != 0` 时启用，否则禁用 |

## UML 类图

### UML 类图关系

```Mermaid
classDiagram
class playlistItem {
   <<QObject, QTreeWidgetItem>>
   +Properties prop
   +QString infoText
   #SafeUi~Ui::playlistItem~ ui
   +playlistItem(itemNameOrFileName, type)
   +~playlistItem()
   +properties() Properties
   +setName(name)
   +drawItem(painter, frameIdx, zoomFactor, drawRawValues)
   +getSize() QSize
   +needsLoading(frameIdx, loadRawData) ItemLoadingState
   +loadFrame(frameIdx, playing, loadRawData, emitSignals)
   +getFrameHandler() FrameHandler*
   +createPlaylistItemControls() QLayout*
   -slotVideoControlChanged()
}

class playlistItemWithVideo {
   #std::unique_ptr~videoHandler~ video
   #video::RawFormat rawFormat
   #bool isFrameLoading
   #bool isFrameLoadingDoubleBuffer
   #bool unresolvableError
   +playlistItemWithVideo(itemNameOrFileName)
   +drawItem(painter, frameIdx, zoomFactor, drawRawValues)
   +getSize() QSize
   +getFrameHandler() FrameHandler*
   +needsLoading(frameIdx, loadRawData) ItemLoadingState
   +loadFrame(frameIdx, playing, loadRawData, emitSignals)
   +isLoading() bool
   +isCachable() bool
   +cacheFrame(idx, testMode)
   +removeFrameFromCache(frameIdx)
   +removeAllFramesFromCache()
   +connectVideo()
   #slotVideoHandlerChanged(redrawNeeded, recache)
}

class playlistItemRawFile {
   -FileSource dataSource
   -bool isY4MFile
   -QList~uint64_t~ y4mFrameIndices
   -QString pixelFormatAfterLoading
   +playlistItemRawFile(rawFilePath, frameSize, sourcePixelFormat, fmt)
   +savePlaylist(root, playlistDir)
   +getInfo() InfoData
   +newplaylistItemRawFile(root, playlistFilePath) playlistItemRawFile*
   +canBeUsedInProcessing() bool
   +getPixelValues(pixelPos, frameIdx) ValuePairListSets
   +isSourceChanged() bool
   +reloadItemSource()
   +updateSettings()
   +cacheFrame(idx, testMode)
   #slotLoadRawData(frameIdx)
   #slotVideoPropertiesChanged()
   -setFormatFromFileName()
   -getNumberFrames() int
   -createPropertiesWidget()
   -parseY4MFile() bool
}

class FrameHandler {
   <<QObject>>
   #QImage currentImage // 图像
   #Size frameSize      // 分辨率
   #QSettings settings  // 用户配置
   #SafeUi~Ui::FrameHandler~ ui // 分辨率相关控件
   +FrameHandler()
   +getFrameSize() Size
   +getImageBitDepth() int
   +drawFrame(painter, zoomFactor, drawRawValues) // 根据当前缩放参数（zoomFactor）绘制当前图像（currentImage）
   +setFrameSize(size)
   +getPixelValues(pixelPos, frameIdx, item2, frameIdx1) QStringPairList // 获取指定像素位置（pixelPos）在当前帧（frameIdx）的RGB像素值
   +isPixelDark(pixelPos) bool
   +isFormatValid() bool
   +getFormatAsString() QString
   +setFormatFromString(format) bool
   +calculateDifference(item2, frameIdxItem0, frameIdxItem1, ...) QImage
   +createFrameHandlerControls(isSizeFixed) QLayout*
   +drawPixelValues(painter, frameIdx, videoRect, zoomFactor, ...) //
   +getCurrentFrameAsImage() QImage
   +loadCurrentImageFromFile(filePath) bool // 从文件加载图像数据到QImage
   +savePlaylist(root)
   +loadPlaylist(root)
   #slotVideoControlChanged()
   -getNewSizeFromControls() Size
   -getPixelVal(x, y) QRgb
   -static presetFrameSizes frameSizePresetList // 预设的几组分辨率
   +signalHandlerChanged(bool, recacheIndicator)
}

class videoHandler {
   <<FrameHandler>>
   +QImage requestedFrame
   +int requestedFrame_idx
   +QByteArray rawData
   +int rawData_frameIndex
   +videoHandler()
   +drawFrame(painter, frameIndex, zoomFactor, drawRawValues)
   +getNrFramesCached() int
   +cacheFrame(frameIndex, testMode)
   +getCachingFrameSize() unsigned
   +getCachedFrames() QList~int~
   +getNumberCachedFrames() int
   +isInCache(idx) bool
   +removeFrameFromCache(frameIndex)
   +removeAllFrameFromCache()
   +getBytesPerFrame() int64_t
   +setFrameSize(size)
   +calculateDifference(item2, ...) QImage
   +setFormatFromCorrelation(data, fileSize)
   +guessAndSetPixelFormat(frameFormat, fileInfo)
   +invalidateAllBuffers()
   +needsLoading(frameIndex, loadRawValues) ItemLoadingState
   +loadFrame(frameIndex, loadToDoubleBuffer)
   +getCurrentImageIndex() int
   +activateDoubleBuffer()
   +createVideoHandlerControls(isSizeFixed) QLayout*
   +needsLoadingRawValues(frameIndex) ItemLoadingState
   #int currentImageIndex
   #QMutex currentImageSetMutex
   #QImage doubleBufferImage
   #int doubleBufferImageFrameIndex
   #QByteArray currentFrameRawData
   #int currentFrameRawData_frameIndex
   #QMutex imageCacheAccess
   #QMap~int, QImage~ imageCache
   #bool cacheValid
   #slotVideoControlChanged()
   +signalRequestFrame(int)
   +signalRequestRawData(int)
}

class videoHandlerRGB {
   <<videoHandler>>
   -QMutex rgbFormatMutex
   -PixelFormatRGB srcPixelFormat
   -ComponentDisplayMode componentDisplayMode
   -double displayRScale, displayGScale, displayBScale, displayAScale
   -bool displayRInvert, displayGInvert, displayBInvert, displayAInvert
   -bool displayLimitedRange
   -videoHandlerRGBCustomFormatDialog* customFormatWidget
   +videoHandlerRGB()
   +~videoHandlerRGB()
   +isFormatValid() bool
   +getCachingFrameSize() unsigned
   +getPixelValues(pixelPos, frameIdx, item2, frameIdx1) QStringPairList
   +getFormatAsString() QString
   +setFormatFromString(format) bool
   +createVideoHandlerControls(isSizeFixed) QLayout*
   +updateControlsForNewPixelFormat()
   +getRawRGBPixelFormatName() QString
   +setRGBPixelFormat(format, emitSignal)
   +setRGBPixelFormatByName(name, emitSignal)
   +guessAndSetPixelFormat(frameFormat, fileInfo)
   +drawPixelValues(painter, frameIdx, videoRect, zoomFactor, ...)
   +calculateDifference(item2, frameIdxItem0, ...) QImage
   +setSrcPixelFormat(format)
   +loadRawRGBData(frameIndex)
   +getPixelValue(pixelPos) rgba_t
   +slotDisplayOptionsChanged()
   +slotRGBFormatControlChanged(int)
   +slotCustomFormatChanged()
   -convertRGBToQRgb(sourceBuffer, targetBuffer, ...)
}

class videoHandlerYUV {
   <<videoHandler>>
   -PixelFormatYUV srcPixelFormat
   -ComponentDisplayMode componentDisplayMode
   -ColorConversion colorConversion
   -ConversionSettings conversionSettings
   -double lumaScale, chromaScale
   -int lumaOffset, chromaOffset
   -bool lumaInvert, chromaInvert
   -InterpolationMode interpolationMode
   -bool showPixelValuesAsDiff
   -QByteArray currentFrameRawYUVData
   -videoHandlerYUVCustomFormatDialog* customFormatWidget
   +videoHandlerYUV()
   +~videoHandlerYUV()
   +getCachingFrameSize() unsigned
   +isFormatValid() bool
   +getFormatAsString() QString
   +setFormatFromString(format) bool
   +createVideoHandlerControls(isSizeFixed) QLayout*
   +updateControlsForNewPixelFormat()
   +getRawPixelFormatYUVName() QString
   +setPixelFormatYUV(fmt, emitSignal)
   +setPixelFormatYUVByName(name, emitSignal)
   +setYUVColorConversion(conversion)
   +loadValues(frameSize, sourcePixelFormat)
   +drawPixelValues(painter, frameIdx, videoRect, zoomFactor, ...)
   +loadFrame(frameIndex, loadToDoubleBuffer)
   +guessAndSetPixelFormat(frameFormat, fileInfo)
   +convertYUVToQRgb(sourceBuffer, targetBuffer, frameSize, sourceBufferFormat)
   +getPixelValue(pixelPos, frameIdx) yuv_t
   +calculateDifference(item2, ...) QImage
   -static formatPresetList std::vector~PixelFormatYUV~
   #SafeUi~Ui::videoHandlerYUV~ ui
   #bool diffReady
   #QByteArray diffYUV
   #PixelFormatYUV diffYUVFormat
   #QGroupBox* yuvControlsGroupBox
   #QGroupBox* customFormatGroupBox
   #slotYUVControlChanged()
   #slotYUVFormatControlChanged(idx)
   #slotCustomFormatChanged()
}

class PixelFormatRGB {
   -unsigned bitsPerSample
   -DataLayout dataLayout
   -ChannelOrder channelOrder
   -AlphaMode alphaMode
   -Endianness endianness
   -PaddingInfo paddingInfo
   -bool bytePacking
   -unsigned bitsPerPixel
   -DiffCompDepthType diffCompType
   +PixelFormatRGB()
   +PixelFormatRGB(name)
   +PixelFormatRGB(bitsPerSample, dataLayout, channelOrder, alphaMode, endianness, paddingInfo, bytePacking, diffType)
   +PixelFormatRGB(diffType, channelOrder, alphaMode, endianness)
   +isValid() bool
   +isDiffCompDepth() bool
   +nrChannels() unsigned
   +hasAlpha() bool
   +hasPadding() bool
   +getName() string
   +getBitsPerSample() unsigned
   +getDataLayout() DataLayout
   +getChannelOrder() ChannelOrder
   +getAlphaMode() AlphaMode
   +getEndianess() Endianness
   +getPaddingInfo() PaddingInfo
   +isBytePacking() bool
   +getDiffCompType() DiffCompDepthType
   +getBitsPerPixel() unsigned
   +setBitsPerSample(bitsPerSample)
   +setDataLayout(dataLayout)
   +setChannelOrder(channelOrder)
   +setAlphaMode(alphaMode)
}

class PixelFormatYUV {
   -std::optional~PredefinedPixelFormat~ predefinedPixelFormat
   -Subsampling subsampling
   -unsigned bitsPerSample
   -bool bigEndian
   -Offset chromaOffset
   -DataLayout dataLayout
   -ComponentOrder componentOrder
   -PaddingInfo paddingInfo
   -bool bytePacking
   +PixelFormatYUV()
   +PixelFormatYUV(name)
   +PixelFormatYUV(subsampling, bitsPerSample, dataLayout, componentOrder, bigEndian, chromaOffset, bytePacking, paddingInfo)
   +PixelFormatYUV(predefinedPixelFormat)
   +getPredefinedFormat() std::optional~PredefinedPixelFormat~
   +isValid() bool
   +canConvertToRGB(frameSize, whyNot) bool
   +bytesPerFrame(frameSize) int64_t
   +getName() string
   +getNrPlanes() unsigned
   +setDefaultChromaOffset()
   +getSubsampling() Subsampling
   +getSubsamplingHor() int
   +getSubsamplingVer() int
   +isChromaSubsampled() bool
   +getBitsPerSample() unsigned
   +isBigEndian() bool
   +isPlanar() bool
   +isInterleaved() bool
   +isSemiPlanar() bool
   +isUVInterleaved() bool
   +hasAlpha() bool
   +isBytePacking() bool
   +getChromaOffset() Offset
   +getDataLayout() DataLayout
   +getComponentOrder() ComponentOrder
   +getPlaneOrder() PlaneOrder
   +getPackingOrder() PackingOrder
   +getPaddingInfo() PaddingInfo
   +operator==(a) bool
   +operator!=(a) bool
   +operator bool() bool
}

class videoHandlerRGBCustomFormatDialog {
   <<QWidget>>
   -Ui::CustomRGBFormatDialog ui
   +videoHandlerRGBCustomFormatDialog(rgbFormat, parent)
   +getSelectedRGBFormat() PixelFormatRGB
   -updateControlsEnabledState()
   -updateAlphaPosComboBox()
   -updatePaddingPosComboBox()
   -updateDiffTypeComboBox()
   -on_bitDepthSpinBox_valueChanged(value)
   -on_planarCheckBox_stateChanged(state)
   -on_groupBoxDiffCompDepth_toggled(checked)
   -on_comboBoxDiffType_currentIndexChanged(index)
   -on_comboBoxAlphaPos_currentIndexChanged(index)
   -on_comboBoxPaddingPos_currentIndexChanged(index)
   +formatChanged()
}

class videoHandlerYUVCustomFormatDialog {
   <<QWidget>>
   -Ui::CustomYUVFormatDialog ui
   +videoHandlerYUVCustomFormatDialog() = delete
   +videoHandlerYUVCustomFormatDialog(yuvFormat, parent)
   +getSelectedYUVFormat() PixelFormatYUV
   -updateComponentOrderComboBox()
   -on_comboBoxChromaSubsampling_currentIndexChanged(idx)
   -on_comboBoxBitDepth_currentIndexChanged(idx)
   +formatChanged()
}

%% 继承关系
playlistItem <|-- playlistItemWithVideo : 继承
playlistItemWithVideo <|-- playlistItemRawFile : 继承
FrameHandler <|-- videoHandler : 继承
videoHandler <|-- videoHandlerRGB : 继承
videoHandler <|-- videoHandlerYUV : 继承

%% 关联关系
playlistItemWithVideo --> videoHandler : 持有 video*
playlistItemRawFile --> FileSource : 持有 dataSource
videoHandlerRGB --> PixelFormatRGB : 持有 srcPixelFormat
videoHandlerRGB --> videoHandlerRGBCustomFormatDialog : 持有 customFormatWidget
videoHandlerYUV --> PixelFormatYUV : 持有 srcPixelFormat
videoHandlerYUV --> videoHandlerYUVCustomFormatDialog : 持有 customFormatWidget
```

### 类继承层次

```Mermaid
classDiagram
direction TB

class QObject
class QTreeWidgetItem
class QWidget

QObject <|-- playlistItem
QTreeWidgetItem <|-- playlistItem
playlistItem <|-- playlistItemWithVideo
playlistItemWithVideo <|-- playlistItemRawFile

QObject <|-- FrameHandler
FrameHandler <|-- videoHandler

videoHandler <|-- videoHandlerRGB
videoHandler <|-- videoHandlerYUV

videoHandlerRGB --> PixelFormatRGB
videoHandlerRGB --> videoHandlerRGBCustomFormatDialog

videoHandlerYUV --> PixelFormatYUV
videoHandlerYUV --> videoHandlerYUVCustomFormatDialog

QWidget <|-- videoHandlerRGBCustomFormatDialog
QWidget <|-- videoHandlerYUVCustomFormatDialog

```

### 信号-槽调用关系图

```Mermaid
sequenceDiagram
autonumber
participant User as 用户
participant RGB_Dialog as videoHandlerRGBCustomFormatDialog
participant YUV_Dialog as videoHandlerYUVCustomFormatDialog
participant RGB as videoHandlerRGB
participant YUV as videoHandlerYUV
participant VH as videoHandler
participant PIV as playlistItemWithVideo
participant PLI as playlistItemRawFile
participant Tree as PlaylistTreeWidget
participant Main as MainWindow

par RGB 和 YUV 分支
   User->>RGB_Dialog: 修改RGB格式参数
   RGB_Dialog->>RGB: formatChanged() 信号
   RGB->>RGB: slotCustomFormatChanged()
and
   User->>YUV_Dialog: 修改YUV格式参数
   YUV_Dialog->>YUV: formatChanged() 信号
   YUV->>YUV: slotCustomFormatChanged()
end

par RGB 处理流程
   RGB->>RGB: setSrcPixelFormat(newFormat)
   RGB->>RGB: slotDisplayOptionsChanged()
   RGB->>VH: setCacheInvalid()
   VH-->>RGB: cacheValid = false
   RGB->>PIV: signalHandlerChanged(true, RECACHE_CLEAR)
and YUV 处理流程
   YUV->>YUV: setPixelFormatYUV(newFormat)
   YUV->>YUV: slotYUVControlChanged()
   YUV->>VH: setCacheInvalid()
   VH-->>YUV: cacheValid = false
   YUV->>PIV: signalHandlerChanged(true, RECACHE_CLEAR)
end

PIV->>PIV: slotVideoHandlerChanged()
PIV->>PIV: updateStartEndRange()
PIV->>PLI: SignalItemChanged(redrawNeeded, RECACHE_CLEAR)
PLI-->>PIV:
PIV->>Tree: SignalItemChanged
Tree->>Tree: slotItemChanged()
Tree->>Tree: selectedItemChanged(redraw)
Tree->>Tree: signalItemRecache(item, RECACHE)
Tree->>Main: selectedItemChanged
Main->>Main: fileInfoAdapter()
Main->>PLI: getInfo()
PLI->>PLI: slotVideoPropertiesChanged()

```

## YUView 几个关键场景的函数调用链分析

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
   - `videoHandler::setFrameSize` - 更新帧分辨率大小
   - `videoHandler::clearFrameCache` - 清除旧格式的帧缓存
   - `emit SignalItemChanged -> PlaylistTreeWidget::slotItemChanged` - 发出项目变更信号，触发重新加载cache
      - `-> PlaylistTreeWidget::signalItemRecache -> VideoCache::itemNeedsRecache`

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

### 场景四：RGB 格式变化

- 使用`videoHandler::currentFrameRawData()`前先检查`videoHandler::currentFrameRawData_frameIndex`是否正确，如果错误则调用`videoHandler::loadFrame()`加载帧。
- `videoHandler::signalRequestRawData() -> playlistItemRawFile::loadRawData()`，更新`videoHandler::rawData`和`videoHandler::rawData_frameIndex`
- `videoHandlerRGB::loadRawRGBData(int frameIndex)`：
   1. 先检查`frameIndex == currentFrameRawData_frameIndex && cacheValid`，如果通过则说明不用更新；
   2. 再检查`frameIndex == rawData_frameIndex`，如果通过则赋值`currentFrameRawData = rawData; currentFrameRawData_frameIndex = frameIndex;`
   3. 以上都不匹配，发出信号`signalRequestRawData()`，请求加载指定帧的RGB数据，等加载完毕后再进行第2步检查

加载指定帧的RGB数据到`currentFrameRawData`中。

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

### 场景五：放大显示像素值

当用户放大视频帧到一定比例时，YUView 会在每个像素上显示其 RGB 像素值。这个功能通过以下步骤实现：

1. **触发条件**：当 `drawRawValues` 为 true 且缩放因子 `zoomFactor >= SPLITVIEW_DRAW_VALUES_ZOOMFACTOR` 时，会启用像素值显示功能。

2. **实现位置**：
   - 在 `videoHandler.cpp` 的 `drawFrame` 函数中（第206-210行），当满足条件时调用 `drawPixelValues` 函数。
   - 实际的绘制逻辑在 `FrameHandler.cpp` 的 `drawPixelValues` 函数中（第347-437行）。

3. **实现原理**：
   - 首先计算可见区域内的像素范围，只处理可见的像素以提高性能。
   - 遍历可见区域内的每个像素，计算其在屏幕上的位置。
   - 获取每个像素的 RGB 值，可以是单个帧的像素值或两个帧的差值。
   - 根据像素的亮度自动选择文本颜色（黑色或白色），以确保文本在不同亮度的像素上都清晰可见。
   - 在每个像素的中心绘制 RGB 值，格式为十六进制或十进制，取决于用户设置。

4. **关键功能**：
   - 支持显示单个帧的像素值
   - 支持显示两个帧之间的像素差值
   - 自动适应不同亮度的像素背景
   - 只处理可见区域，提高渲染性能

5. **相关设置**：
   - 用户可以通过设置 `ShowPixelValuesHex` 来选择使用十六进制或十进制显示像素值。

### 总结

以上三个场景涵盖了YUView中从文件拖放到显示、格式变更到缩放控制的核心流程。每个流程都涉及多个组件和函数的协作，共同实现了YUV文件的高效处理和显示。

- **拖放与加载**：通过文件系统操作和类型检测，实现了不同格式视频的自动识别和加载
- **格式管理**：提供了灵活的格式设置机制，支持动态调整视频参数
- **交互控制**：通过鼠标事件处理，实现了直观的缩放控制功能

这些流程的实现展示了YUView作为专业视频分析工具的设计思路和技术实现，为用户提供了便捷、高效的视频处理体验。

## 其他信息

`QSetting` 对应的配置设置位于注册表`\HKEY_CURRENT_USER\SOFTWARE\Institut für Nachrichtentechnik, RWTH Aachen University\YUView v3.0.0\` 下，

- 其中记录的历史文件和格式位于此路径下`itemMemory`子文件夹内
- 路径要注意版本号
