# YUView Project Notes

\[TOC]

## TODO

- 整体
  - [ ] [FEAT] 增加 `rowPitch(widthStride) / heightStride` 输入框，用于设定虚宽和虚高（UI已完成，但取数逻辑未完成）
  - [ ] [FEAT] 增加 combox 用于强制切换 rgb/yuv 格式
  - [ ] [FEAT] 引入别名 `alias`来预设一些常用的格式
  - [ ] [FEAT] 丰富文件名格式猜测功能
  - [ ] [FEAT] 增加配置文件，用于自定义格式的取数方式
  - [x] [FEAT] 引入RGB格式成员变量，解决RGB文件加载后没有对应更新ui控件的问题
  - [x] [DOC]  搞清楚RGB文件加载失败后的处理是什么逻辑 (加载失败直接返回，不更新ui控件)
- YUV 图像格式
  - [x] [BUG]  修正 YUV400 不支持色彩空间选择的问题；YUV400 应该 disable 掉 componentOrder 控件
  - [ ] [BUG]  LimitedRangeToFullRange 映射没有舍入； limited Y转RGB前只减了偏移没有缩放回8bit尺度；UV没有L2F映射。 （还未完全解决）
  - [x] [FEAT] 调整 `YuvCustomFormat` 窗口控件逻辑，允许 `planar` 和 `byte-packed` 共存
  - [x] [FEAT] 支持 NV15/NV20/NV30 等10bit packed 格式显示 （已完成 ，但放大后显示的像素值还有问题）
  - [x] [FEAT] 10bit unbytepacking 格式支持调整对齐 padding 的位置 （`getName()`用于比较像个像素是否相等，未引入`paddingInfo`，导致比较时新旧像素被判定为一致）
  - [ ] [FEAT] 支持 YUV420I\_LEGACY 8bit 格式
  - [x] [FEAT] 增加对 VU30 格式的支持 （采用内置预设格式实现）
  - [x] [FIX]  YUV422I 10bit 转到 SP 时崩溃， 打开 bytepacking 崩溃（src\_stride 计算错误导致取数越界）
  - [x] [FIX]  YUV格式改 `Subsampling` 和 `ComponentLayout` 会导致频繁更新 `ComponentOrder` 控件，进而导致频繁触发 `formatChanged` 信号，待调整
  - [x] [FIX]  `ComponentOrder` 存在重复的枚举值，导致解析名字时不对，待解决
  - [ ] [FIX]  修正 NV15/NV20/NV30 等格式的放大像素显示错误
  - [ ] [FIX]  修正 P010/P012/VU24/YUV4xxX10l 等格式的显示错误 （目前要按16bit深度显示，调PaddingInfo没反应）
  - [ ] [FIX]  NV15 等格式绘制出的像素值不是10bit, 宽度翻倍后放大要绘制像素时崩溃 （`videoHandlerYUV::getPixelValue()`）
  - [ ] [FIX]  NV20 加载后在设为 bytepacking 前（被解析为YUV422SP10l时）放大像素会导致取数越界崩溃，好像没有对`sourceBufferSize`进行检测和保护步骤，应该在取数前先判断buffer大小和像素格式是否匹配，不匹配的话`drawPixelValue()`应该显示错误信息
  - [x] [FIX] `VideoCache.cpp`会崩溃问题解决（没有进行缓存有效性检查，没有对`nrFramesCachable`返回值进行检查，已解决）
  - [x] [REFCTOR] `PixelFormatYUV` 合并到开发分支
  - [x] [REFCTOR] `DataLayout` 和 `ComponentLayout` 数据重复，可以合并
- RGB 图像格式
  - [x] [BUG]  RGBA 选择显示 RGB 时， combox 不会变化，已解决
  - [x] [BUG]  放大后显示的像素黑/白色应该根据实际像素深度来判断（RGB先做r2y再进行阈值判断）
  - [x] [FEAT] RGB custom UI 控件改为停靠窗口，加入 interleaved, alphaChannel改为combox, 加入 `bytepacking` 和 `paddingInfo` 选项
  - [x] [FEAT] 增加一个按钮 "Ignore Alpha"，用于忽略 Alpha 通道，只显示 RGB 颜色
  - [x] [FEAT] 引入 RGB332/RGB565/RGBA5551/RGBA1010102 等通道位宽不一致的像素格式支持
  - [x] [FEAT] 支持 rgb planar bytepacking 格式
  - [x] [FEAT] RGBA5551/RGBA1010102 Alpha 可以改为 Padding
  - [x] [FEAT] RGB332/RGB565/RGBA5551/RGBA1010102 支持选择 order
  - [x] [FEAT] RGB332/RGB565/RGBA5551/RGBA1010102 等系列格式单通道显示支持 （还加上了大端支持）
  - [x] [FIX]  修正 RGB332 等格式的放大像素显示错误 （v3.0.1-2 之后支持不同的channelOrder了，放大后显示的像素值还跟channelOrder有关，待进一步支持）
  - [x] [FIX]  查看 setting 里 `RGB5651010102` 字符串是哪里来的（`PixelFormatRGB::getName()`输出错误）
  - [x] [FIX]  查看 RGBA5551/RGBA1010102 **invertAlpha 选项不生效的原因**
  - [x] [FIX]  修正 RGBA1010102 Alpha 通道的显示问题，A=3时应该映射到255
  - [x] [FIX]  选择 RGBA5551/RGBA1010102 时不会默认选择 alpha，应该在没有 padding 时候默认选择 alphaInLsb
  - [x] [FIX]  取消RGBA预乘显示，避免Alpha为0是不能正确显示图像
  - [ ] [FIX]  只显示一个通道时，像素值的渲染颜色是否只要考虑当前显示的通道像素值？
- 其他
  - [x] [FEAT] 增加 spdlog 作为日志库，替换 Qt 的日志系统
  - [x] [FEAT] 命令行参数增加日志等级参数 （日志等级未传递到 YUViewLib 中）
  - [x] [PERF] 改为手动 UIC，ui没变的情况下避免每次编译都要重新编译很多文件 （正确做法是取消对每次编译都会变的变量进行`add_definitions()`）
  - [x] [FEAT] 通过 `--console` 来开启控制台模式，平时隐藏控制台窗口； `-h/--help`可查看帮助信息

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

| 格式类型选项             | channelOrder | alphaMode/paddingInfo | 通道字母序列 | 位数序列    | 最终名字        |
| ------------------ | ------------ | --------------------- | ------ | ------- | ----------- |
| BPP8\_RGB332       | RGB          | -                     | RGB    | 332     | RGB332      |
| BPP8\_RGB332       | GBR          | -                     | GBR    | 323     | GBR323      |
| BPP16\_RGB565      | BGR          | -                     | BGR    | 565     | BGR565      |
| BPP16\_RGBA5551    | RGB          | InLsb                 | RGBA   | 5551    | RGBA5551    |
| BPP16\_RGBA5551    | RGB          | InMsb                 | ARGB   | 1555    | ARGB1555    |
| BPP16\_RGBA5551    | BGR          | PaddingInLSB          | BGRX   | 5551    | BGRX5551    |
| BPP16\_RGBA5551    | BGR          | PaddingInMSB          | XBGR   | 1555    | XBGR1555    |
| BPP32\_RGBA1010102 | RGB          | InLsb                 | RGBA   | 1010102 | RGBA1010102 |
| BPP32\_RGBA1010102 | RGB          | InMsb                 | ARGB   | 2101010 | ARGB2101010 |

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

| 控件对象                    | 对应 PixelFormatRGB 的成员变量 | 取值范围                                                                 | 和其他控件的联动关系                                                                                                                                                                                        |
| ----------------------- | ----------------------- | -------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `rgbOrderComboBox`      | `channelOrder`          | `RGB`, `RBG`, `GRB`, `GBR`, `BRG`, `BGR`                             | 始终启用                                                                                                                                                                                              |
| `bitDepthSpinBox`       | `bitsPerSample`         | 1 - 32                                                               | DiffCompDepth 格式时禁用，根据 `comboBoxDiffType` 自动设置；其他格式时始终启用                                                                                                                                          |
| `comboBoxEndianness`    | `endianness`            | `Big Endian`, `Little Endian`                                        | DiffCompDepth 格式时仅当类型为非 `BPP8_RGB332` 时启用(bpp>8)；其他格式时仅当 `bitsPerSample > 8` 时启用                                                                                                                  |
| `comboBoxAlphaPos`      | `alphaMode`             | `NoAlpha`, `First (InLsb)`, `Last (InMsb)`                           | DiffCompDepth 格式时仅 `RGBA5551/RGBA1010102` 类型启用，选择非 `NoAlpha` 时会禁用 `comboBoxPaddingPos`; 其他格式时始终启用                                                                                                 |
| `comboBoxPaddingPos`    | `paddingInfo`           | `NoPadding`, `PaddingOnMsb`, `PaddingOnLsb`                          | DiffCompDepth 格式时仅 `RGBA5551/RGBA1010102` 类型且 `comboBoxAlphaPos == NoAlpha` 时启用，选择非 `NoPadding` 时会禁用 `comboBoxAlphaPos`； 普通格式时仅当 `bitsPerSample % 8 != 0` 时启用；勾选 BytePacking 时始终禁用并设为 `NoPadding` |
| `planarCheckBox`        | `dataLayout`            | `true` (Planar), `false` (Interleaved)                               | DiffCompDepth 格式时禁用并强制为 false；其他格式时启用，勾选时会禁用 `groupBoxDiffCompDepth`                                                                                                                              |
| `checkBoxBytePacking`   | `bytePacking`           | `true` (启用), `false` (禁用)                                            | DiffCompDepth 格式时禁用并强制为 true；其他格式时启用，仅当 `bitsPerSample % 8 != 0` 时可选                                                                                                                              |
| `groupBoxDiffCompDepth` | `diffCompType` (是否启用)   | `true` (启用), `false` (禁用)                                            | 启用时会禁用 `planarCheckBox` 和 `checkBoxBytePacking`，`planarCheckBox` 勾选时会禁用该控件                                                                                                                        |
| `comboBoxDiffType`      | `diffCompType`          | `BPP8_RGB332`, `BPP16_RGB565`, `BPP16_RGBA5551`, `BPP32_RGBA1010102` | 仅在 `groupBoxDiffCompDepth` 启用时可用，选择不同类型会影响 `bitDepthSpinBox` 和 `comboBoxEndianness` 的启用状态                                                                                                         |
| `labelRgbFmtName`       | - (仅显示)                 | -                                                                    | 显示当前 PixelFormatRGB 的 `getName()` 返回值，跟随其他控件变化更新                                                                                                                                                  |

#### 三种格式类型的控件状态

| 控件对象                    | 1. DiffCompDepth 类格式                                                                                                                 | 2. 其他 BytePacking 格式                                  | 3. 普通格式                                              |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------- | ---------------------------------------------------- |
| **触发条件**                | `groupBoxDiffCompDepth` 被勾选                                                                                                          | `groupBoxDiffCompDepth` 未勾选且 `checkBoxBytePacking` 勾选 | `groupBoxDiffCompDepth` 和 `checkBoxBytePacking` 都未勾选 |
| `groupBoxDiffCompDepth` | 勾选                                                                                                                                   | 未勾选                                                   | 未勾选                                                  |
| `comboBoxDiffType`      | 启用，选择类型                                                                                                                              | 禁用                                                    | 禁用                                                   |
| `bitDepthSpinBox`       | 禁用，根据 `comboBoxDiffType` 自动设置                                                                                                        | 启用                                                    | 启用                                                   |
| `rgbOrderComboBox`      | 启用                                                                                                                                   | 启用                                                    | 启用                                                   |
| `comboBoxEndianness`    | `comboBoxDiffType == BPP8_RGB332` 时禁用，其他启用                                                                                           | 启用                                                    | `bitsPerSample > 8` 时启用，否则禁用                         |
| `planarCheckBox`        | 禁用，强制设置为 false                                                                                                                       | 启用                                                    | 启用                                                   |
| `checkBoxBytePacking`   | 禁用，强制设置为 true                                                                                                                        | 勾选                                                    | 未勾选                                                  |
| `comboBoxAlphaPos`      | `RGBA5551/RGBA1010102` 类型仅当 `comboBoxPaddingPos == NoPadding` 时启用，否则禁用并设为 `NoAlpha`(此时和`comboBoxPaddingPos` 互斥)；其他类型禁用并设为 `NoAlpha`； | 启用                                                    | 启用                                                   |
| `comboBoxPaddingPos`    | `RGBA5551/RGBA1010102` 类型仅当 `comboBoxAlphaPos == NoAlpha` 时启用，否则禁用并设为 `NoPadding`(此时和`comboBoxAlphaPos` 互斥)；其他类型禁用并设为 `NoPadding`    | 禁用，强制设置为 `NoPadding`                                  | `bitsPerSample % 8 != 0` 时启用，否则禁用                    |

## YUV 图像格式

### YUV 格式命名规则

`PixelFormatYUV::getName()` 函数根据像素格式属性生成格式名称，遵循以下规则：

命名格式：`<order><subsampling><layout> <bitsPerSample>bit[ endianness][ bytePacking][ paddingInfo][ chromaOffset]`

- **order**：通道顺序，如 `YUV`, `YVU`, `UYVY`, `YUYV` 等
- **subsampling**：色度子采样格式，如 `444`, `422`, `420`, `411`, `410`, `440`, `400`
- **layout**：数据布局
  - `P` = Planar（平面），Y/U/V 分别存储在不同平面
  - `SP` = SemiPlanar（半平面），Y 单独一个平面，UV 交错存储
  - `I` = Interleaved（交错），YUV 数据交错存储
  - `YUV_400` 无子采样，layout 后缀为空
- **bitsPerSample**：每个样本的位深度（8-32）
- **endianness**：字节序，位深度 > 8 时显示 `BE`（大端），否则均为小端
- **bytePacking**：是否使用字节打包
- **paddingInfo**：填充信息，当 `bitsPerSample % 8 != 0` 时显示 `PaddingInLSB` 或 `PaddingInMSB`
- **chromaOffset**：色度偏移，非默认值时显示 `Cx<n>` 和/或 `Cy<n>`

示例：

- `YUV420P 8bit` - 标准 YUV420p 格式
- `YUV422SP 10bit BE BytePacking` - NV16 风格半平面 10bit 大端字节打包格式
- `UYVY422I 8bit` - YUV422 交错格式（通道排列顺序从低位到高位为 UYVY）
- `YUV420P 10bit PaddingInLSB` - 10bit 非字节打包，填充在低位，高位数据有效
- `YUV420P 8bit Cx1` - 色度水平偏移 1/2 像素的 YUV420 planar 格式

### 预定义格式

系统内置了一些常用 YUV 格式的别名：

| 别名 | 对应格式参数 |
|------|-------------|
| `NV12` | YUV_420, 8bit, SemiPlanar, YUV 顺序 |
| `NV16` | YUV_422, 8bit, SemiPlanar, YUV 顺序 |
| `NV24` | YUV_444, 8bit, SemiPlanar, YUV 顺序 |
| `NV21` | YUV_420, 8bit, SemiPlanar, YVU 顺序 |
| `NV61` | YUV_422, 8bit, SemiPlanar, YVU 顺序 |
| `NV42` | YUV_444, 8bit, SemiPlanar, YVU 顺序 |
| `NV15` | YUV_420, 10bit, SemiPlanar, YUV, BytePacking |
| `NV20` | YUV_422, 10bit, SemiPlanar, YUV, BytePacking |
| `NV30` | YUV_444, 10bit, SemiPlanar, YUV, BytePacking |
| `V210` | 预定义 422 10bit 格式（Apple 标准） |

### 色度子采样格式

| 子采样格式 | 水平采样 | 垂直采样 | 说明 |
|-----------|---------|---------|------|
| YUV_444 | 1:1 | 1:1 | 无色度子采样，全分辨率 |
| YUV_422 | 2:1 | 1:1 | 水平方向色度减半 |
| YUV_420 | 2:1 | 2:1 | 水平和垂直方向色度都减半 |
| YUV_440 | 1:1 | 2:1 | 仅垂直方向色度减半 |
| YUV_411 | 4:1 | 1:1 | 水平方向色度减为 1/4 |
| YUV_410 | 4:1 | 4:1 | 水平和垂直方向色度都减为 1/4 |
| YUV_400 | - | - | 仅亮度，无色度分量 |

### 数据布局（DataLayout）

#### 1. Planar（平面布局）

Y、U、V 三个分量分别存储在独立的内存平面中：

- Y 平面：完整分辨率
- U 平面：根据子采样格式可能为半分辨率
- V 平面：根据子采样格式可能为半分辨率
- Alpha 平面（如有）：完整分辨率，单独平面

支持的 ComponentOrder：`YUV`, `YVU`, `AYUV`, `VUYA`, `YUVA`, `YVUA` (从LSB到MSB)

#### 2. SemiPlanar（半平面布局）

Y 分量单独存储，UV 分量交错存储：

- Y 平面：完整分辨率
- UV 平面：U 和 V 交错存储（如 NV12: UVUVUV...）

支持的子采样：420, 422, 440, 444, 410, 411

支持的 ComponentOrder：`YUV`, `YVU`（仅影响 UV 平面的顺序） (从LSB到MSB)

不支持 Alpha 通道

#### 3. Interleaved（交错布局）

YUV 数据按像素交错存储，仅支持 422 和 444 子采样：

支持的 ComponentOrder： (从LSB到MSB)
- YUV422 专用：`UYVY`, `VYUY`, `YUYV`, `YVYU`
- YUV444 通用：`YUV`, `YVU`, `AYUV`, `VUYA`, `YUVA`, `YVUA`

### 色度偏移（Chroma Offset）

色度分量相对于亮度分量的位置偏移，用于处理不同的采样相位：

- **Cx0/Cy0**：无色度偏移（默认）
- **Cx1**：水平偏移 1/2 像素
- **Cx2**：水平偏移 1 像素
- **Cx3**：水平偏移 3/2 像素
- **Cy1**：垂直偏移 1/2 像素
- **Cy2**：垂直偏移 1 像素
- **Cy3**：垂直偏移 3/2 像素

不同子采样格式支持的最大偏移值不同：

| 子采样格式 | 最大水平偏移 | 最大垂直偏移 |
|-----------|------------|------------|
| YUV_444 | 0 | 0 |
| YUV_422 | 3 | 1 |
| YUV_420 | 3 | 3 |
| YUV_440 | 1 | 3 |
| YUV_411 | 7 | 1 |
| YUV_410 | 7 | 7 |
| YUV_400 | - | - |

### YUV 格式控件调整逻辑

#### `videoHandlerYUVCustomFormatDialog` 控件列表

| 控件对象 | 对应 PixelFormatYUV 成员变量 | 取值范围 | 联动关系 |
|---------|---------------------------|---------|---------|
| `comboBoxChromaSubsampling` | `subsampling` | `444`, `422`, `420`, `440`, `411`, `410`, `400` | 改变时更新色度偏移选项和布局按钮状态 |
| `comboBoxBitDepth` | `bitsPerSample` | 8, 9, 10, 12, 14, 16, 24, 32 | 改变时启用/禁用字节序（>8bit时启用）、字节打包选项（%8!=0时启用）和填充信息选项（%8!=0时启用） |
| `comboBoxEndianness` | `bigEndian` | `Big Endian`, `Little Endian` | 位深度 > 8 时启用 |
| `comboBoxElemOrder` | `componentOrder` | 根据布局动态变化 | Interleaved 422 时仅显示 422 专用顺序， 400时禁用 |
| `comboBoxPaddingInfo` | `paddingInfo` | `NoPadding`, `PaddingInMSB`, `PaddingInLSB` | 位深度 % 8 != 0 时启用 |
| `comboBoxChromaOffsetX` | `chromaOffset.x` | 根据子采样动态变化 | YUV_400 时禁用 |
| `comboBoxChromaOffsetY` | `chromaOffset.y` | 根据子采样动态变化 | YUV_400 时禁用 |
| `radioButtonPlanar` | `dataLayout` | Planar | YUV_400 时强制选中 |
| `radioButtonSemiPlanar` | `dataLayout` | SemiPlanar | YUV_400 时禁用 |
| `radioButtonInterleaved` | `dataLayout` | Interleaved | 仅 422/444 子采样时启用，其他采样禁用 |
| `checkBoxBytePacking` | `bytePacking` | true/false | 位深度 % 8 != 0 时启用 |

#### 控件状态与数据布局的关系

| 控件对象 | Planar | SemiPlanar | Interleaved |
|---------|--------|-----------|-------------|
| **触发条件** | `radioButtonPlanar` 选中 | `radioButtonSemiPlanar` 选中 | `radioButtonInterleaved` 选中 |
| `radioButtonPlanar` | 选中 | 未选中 | 未选中（422/444 可用，其他禁用） |
| `radioButtonSemiPlanar` | 未选中 | 选中（非400） | 未选中 |
| `radioButtonInterleaved` | 未选中 | 未选中 | 选中（非400） |
| `comboBoxElemOrder`（400时禁用） | 显示所有 Planar 顺序 | 仅 `YUV`/`YVU` | 422 时显示 4 种 422 专用顺序，444 时显示所有 |
| `checkBoxBytePacking` | 根据位深度启用 | 根据位深度启用 | 根据位深度启用 |
| `comboBoxChromaOffsetX/Y` | 根据子采样启用 | 根据子采样启用 | 禁用（Interleaved 无偏移） |

#### 特殊格式处理

**YUV_400（灰度）**：
- 禁用 `radioButtonSemiPlanar`（无 UV 分量）
- 强制选中 `radioButtonPlanar`
- 禁用 `comboBoxChromaOffsetX/Y`
- `comboBoxElemOrder` 禁用并设为第一个选项

**Interleaved 布局限制**：
- 仅支持 YUV_422 和 YUV_444 子采样
- 不支持色度偏移
- 422 时仅支持 4 种专用交错顺序（UYVY/VYUY/YUYV/YVYU）

**SemiPlanar 限制**：
- 不支持 Alpha 通道
- ComponentOrder 仅支持 `YUV` 和 `YVU`

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

`QSetting` 对应的配置设置位于注册表`\HKEY_CURRENT_USER\SOFTWARE\Institut für Nachrichtentechnik, RWTH Aachen University\YUView xxx\` 下，

- 其中记录的历史文件和格式位于此路径下`itemMemory`子文件夹内
- 路径要注意版本号
