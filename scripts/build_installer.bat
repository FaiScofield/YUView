@echo off
chcp 65001 > nul

echo Usage: %~n0 [release^|debug]
echo -------------------------------

set PROJECT_NAME=YUView
echo 正在生成 %PROJECT_NAME% 安装程序...

:: 获取脚本所在目录的父目录（项目根目录）
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..

:: Inno Setup 编译器路径（默认值，可在 local_build_config.cmd 中覆盖）
set ISCC_PATH="D:\InnoSetup6\ISCC.exe"

:: 加载本机配置(local_build_config.cmd 已 git 忽略)，用于覆盖不同电脑上的路径差异
if exist "%SCRIPT_DIR%\local_build_config.cmd" (
    echo Load local build config file: "%SCRIPT_DIR%\local_build_config.cmd"
    call "%SCRIPT_DIR%\local_build_config.cmd"
)

:: 从CMakeLists.txt获取版本号
:: 注意: 必须加 -NoProfile,否则在 VS Code 集成终端里运行时,
:: VS Code 的 Shell Integration 会向 PowerShell 子进程输出注入 OSC 控制序列
:: (\x1b]633;P;IsWindows=True\x07),该序列会被 for /f 捕获进 VERSION,
:: 导致 ISCC 报 "You may not specify more than one script filename"
for /f %%i in ('powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%\get_version.ps1"') do set VERSION=%%i
if /i "%~1" == "debug" (
    set VERSION=%VERSION%d
)
echo 检测到当前版本号: v%VERSION%


:: 检查是否安装了Inno Setup
if exist %ISCC_PATH% (
    echo 找到 Inno Setup，正在编译安装程序...
    %ISCC_PATH% "%SCRIPT_DIR%\installer_script.iss" /DMyAppVersion=%VERSION%
) else (
    echo 错误: 未找到 Inno Setup 编译器
    echo 请安装 Inno Setup 6 或更高版本，然后重新运行此脚本
    echo 下载地址: https://jrsoftware.org/isinfo.php
    echo.
    echo 或者您可以手动运行以下命令:
    echo %ISCC_PATH% "%SCRIPT_DIR%\installer_script.iss" /DMyAppVersion=%VERSION%
    pause
    exit /b 1
)

echo.
echo 安装程序生成完成!
echo 输出文件位于: %PROJECT_ROOT%\release\%PROJECT_NAME%_Setup_v%VERSION%.exe
echo Done.