# YUView 打包工具使用说明

## 快速开始

运行 `scripts\package_app.bat` 脚本，按照提示选择操作：

```shell
.\scripts\package_app.bat msvc   # 或
.\scripts\package_app.bat mingw
```

或分步手动构建：

```shell
.\scripts\build_msvc_ninja.cmd -t release --clean --deploy
.\scripts\build_win32_msvc.cmd  -t release --clean --deploy
.\scripts\build_win32_mingw.cmd -t release --clean --deploy
```

## 打包脚本结构

```shell
scripts/
├── build_msvc_ninja.cmd     # Ninja + MSVC 编译（推荐）
├── build_win32_msvc.cmd     # Visual Studio 生成器编译
├── build_win32_mingw.cmd    # MinGW 编译
├── get_version.ps1          # 从 CMakeLists.txt 提取版本号
├── collect_dependencies.bat # 收集运行时依赖（windeployqt），支持 debug 和 release
├── build_installer.bat      # 构建安装程序（Inno Setup）
├── installer_script.iss     # Inno Setup 安装配置
├── package_app.bat          # 一键编译 + 打包脚本
├── local_build_config.cmd   # 本机路径配置（可选，已 git 忽略，参考 local_build_config.cmd.example）
└── PackagingGuide.md        # 本文档
```

## 使用方法

### 方法一：一键打包（推荐）

```shell
.\scripts\package_app.bat msvc
```

脚本会自动依次执行：编译（Release）→ 收集依赖 → 生成安装程序。

### 方法二：分步执行

```shell
# 1. 编译应用程序：
cd scripts
build_msvc_ninja.cmd -t release [--clean]

# 2. 收集依赖：
collect_dependencies.bat msvc release

# 3. 生成安装程序：
build_installer.bat
```

### 本机路径配置（local_build_config.cmd）

不同电脑上 Qt / Visual Studio / Inno Setup 的安装路径可能不同，无需修改脚本本身：

1. 复制 `scripts\local_build_config.cmd.example` 为 `scripts\local_build_config.cmd`
2. 按本机实际路径取消注释并修改对应变量（QT_MSVC_ROOT、QT_MINGW_ROOT、GENERATOR、VS_VARSALL_BAT、ISCC_PATH）
3. 该文件已加入 `.gitignore`，不会污染仓库

## 版本同步

所有打包脚本会自动从 `CMakeLists.txt` 中读取版本号（当前为 `project(YUView VERSION ...)` 定义），无需手动维护多个地方的版本号。

## 输出文件

- 发布包：`release/v{VERSION}/`（Debug 为 `release/v{VERSION}d/`）
- 安装程序：`release/YUView_Setup_v{VERSION}.exe`

## 注意事项

1. 确保已安装 Inno Setup 6 或更高版本
   - 如果提示找不到 Inno Setup，请从 <https://jrsoftware.org/isinfo.php> 下载并安装，或在 `local_build_config.cmd` 中设置 `ISCC_PATH`。
2. 确保已正确安装 Qt 5.15.2
   - 如果提示找不到 windeployqt，请确认：
     - Qt 已正确安装
     - Qt 路径设置正确（`local_build_config.cmd` 中的 `QT_MSVC_ROOT` / `QT_MINGW_ROOT`）
3. 确保应用程序已在 Release 模式下成功编译
