# MyQtUiProj 打包工具使用说明

## 快速开始

从项目根目录运行 `package.bat` 脚本，按照提示选择操作：

```cmd
package.bat
```

## 打包脚本结构

```
package/
├── get_version.ps1          # 从CMakeLists.txt提取版本号
├── collect_dependencies.bat # 收集运行时依赖
├── build_installer.bat      # 构建安装程序
├── installer_script.iss     # Inno Setup安装脚本
├── package_app.bat          # 一键打包脚本
├── PackagingGuide.md        # 详细打包说明文档
```

## 使用方法

### 方法一：一键打包（推荐）
1. 从项目根目录运行 `package.bat`
2. 选择选项1（运行完整打包流程）

### 方法二：分步执行
1. 编译应用程序：
   ```cmd
   cd build
   build_win32_msvc.cmd Release
   ```

2. 收集依赖：
   ```cmd
   cd package
   collect_dependencies.bat
   ```

3. 生成安装程序：
   ```cmd
   cd package
   build_installer.bat
   ```

## 版本同步

所有打包脚本会自动从 CMakeLists.txt 中读取版本号，无需手动维护多个地方的版本号。

## 输出文件

- 发布包：`release/v{VERSION}/`
- 安装程序：`release/MyQtUiProj_Setup_v{VERSION}.exe`

## 注意事项

1. 确保已安装 Inno Setup 6 或更高版本
2. 确保已正确安装 Qt 5.15.2
3. 确保应用程序已在 Release 模式下成功编译

---

# MyQtUiProj 应用程序打包说明

## 概述
本文档描述了如何为 MyQtUiProj 应用程序创建可分发的 Windows 安装程序。

## 打包脚本位置

所有打包相关的脚本和文件位于项目的 `package` 目录下：
- `package/get_version.ps1` - 版本号提取脚本
- `package/collect_dependencies.bat` - 收集依赖脚本
- `package/build_installer.bat` - 构建安装程序脚本
- `package/installer_script.iss` - Inno Setup安装脚本
- `package/package_app.bat` - 主打包脚本
- `package/PackagingGuide.md` - 本文档

## 打包前准备

### 1. 编译应用程序
确保应用程序已在 Release 模式下成功编译：
```cmd
cd build
build_win32_msvc.cmd Release
```

### 2. 收集运行时依赖
使用 windeployqt 工具收集所有必需的 Qt 运行时库：
```cmd
"D:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe" "build\build_win32_msvc\Release\MyQtUiProj.exe" --dir "release_\v{VERSION}" --release --no-pdb --no-compiler-runtime
```

## 打包工具

### Inno Setup
我们使用 Inno Setup 来创建 Windows 安装程序，因为它：
- 免费且开源
- 支持复杂的安装逻辑
- 提供良好的用户界面
- 支持多语言
- 包含卸载程序

### 所需工具
- Inno Setup 6 或更高版本
- windeployqt（包含在 Qt SDK 中）

## 打包步骤

### 方法一：一键打包（推荐）
运行 package_app.bat 脚本自动执行所有打包步骤：
```cmd
cd package
package_app.bat
```

### 方法二：分步执行

#### 1. 准备发布目录
运行 collect_dependencies.bat 脚本来收集所有必需的文件到发布目录：
```cmd
cd package
collect_dependencies.bat
```
该脚本会自动从 CMakeLists.txt 中读取版本号并创建相应版本的发布目录。

#### 2. 编译安装程序
运行 build_installer.bat 脚本来创建最终的安装程序：
```cmd
cd package
build_installer.bat
```
该脚本会自动从 CMakeLists.txt 中读取版本号并生成相应版本的安装程序。

## 版本同步机制

### 自动版本检测
- 通过 get_version.ps1 脚本从 CMakeLists.txt 的 `project()` 命令中提取版本号
- 所有打包脚本和安装程序都会使用相同的版本号
- 版本号格式：主版本号.次版本号.修订号 (例如：0.0.1)

### 版本号来源
- **源文件**：CMakeLists.txt
- **版本声明**：project(MyQtUiProj VERSION 0.0.1 LANGUAGES CXX)
- **自动提取**：通过正则表达式匹配提取版本号

## 安装程序特性

### 自动配置
- 将应用程序安装到 Program Files 目录
- 创建开始菜单快捷方式
- 可选创建桌面快捷方式
- 添加到 Windows 添加/删除程序列表

### 依赖管理
- 自动包含所有 Qt 运行时库
- 包含平台特定插件
- 包含样式和其他必要组件

### 注册表项
- 在 HKEY_CURRENT_USER\Software\MyQtUiProj 下创建安装路径注册表项
- 添加卸载信息到 Windows 控制面板

## 文件结构

### 发布包内容
- MyQtUiProj.exe - 主应用程序
- Qt5Core.dll - Qt核心库
- Qt5Gui.dll - Qt GUI库
- Qt5Widgets.dll - Qt控件库
- platforms\qwindows.dll - 平台插件
- styles\qwindowsvistastyle.dll - 样式插件
- LICENSE.txt - 许可证文件
- 启动MyQtUiProj.bat - 启动脚本（备用）

### 安装后目录结构
安装到 {pf}\MyQtUiProj 目录，包含上述所有文件。

## 版本管理

安装程序文件名格式：MyQtUiProj_Setup_v{version}.exe
- 当前版本：从 CMakeLists.txt 自动获取
- 输出目录：release

## 验证步骤

### 安装验证
1. 运行生成的安装程序
2. 验证应用程序能够正确安装
3. 验证快捷方式正常工作
4. 验证应用程序能够正常启动

### 卸载验证
1. 通过控制面板或开始菜单卸载应用程序
2. 验证所有文件和注册表项被正确删除
3. 确认快捷方式被移除

## 故障排除

### windeployqt 未找到
如果提示找不到 windeployqt，请确认：
- Qt 已正确安装
- Qt 路径设置正确
- Qt bin 目录在系统 PATH 中

### Inno Setup 未安装
如果提示找不到 Inno Setup，请从 https://jrsoftware.org/isinfo.php 下载并安装。

### 版本号读取失败
如果版本号无法从 CMakeLists.txt 中读取：
- 确认 CMakeLists.txt 中存在 `project(...VERSION x.x.x...)` 行
- 确认版本号格式为数字.数字.数字形式
- 检查 get_version.ps1 脚本权限（需要允许PowerShell脚本执行）

## 自定义选项

可以通过修改 package/installer_script.iss 文件来自定义安装程序：
- 修改安装路径
- 添加更多文件或组件
- 自定义安装界面
- 添加额外的安装步骤
注意：不要修改与版本号相关的宏定义，它们由脚本自动处理