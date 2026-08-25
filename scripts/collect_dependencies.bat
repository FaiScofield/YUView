@echo off
setlocal
chcp 65001 > nul

echo Usage: %~n0 [msvc^|mingw] [release^|debug] [exe_dir]
echo -------------------------------


set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..
set PROJECT_NAME=YUView

:: Qt 安装根目录（用于 windeployqt；注意与 build 脚本的 QT_PATH 语义不同），可在 local_build_config.cmd 中覆盖
set QT_MSVC_ROOT=D:\Qt\5.15.2\msvc2019_64
set QT_MINGW_ROOT=D:\Qt\5.15.2\mingw81_64

:: 加载本机配置(local_build_config.cmd 已 git 忽略)，用于覆盖不同电脑上的路径差异
if exist "%SCRIPT_DIR%\local_build_config.cmd" (
    echo Load local build config file: "%SCRIPT_DIR%\local_build_config.cmd"
    call "%SCRIPT_DIR%\local_build_config.cmd"
)

:: 从CMakeLists.txt获取版本号
for /f %%i in ('powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%\get_version.ps1"') do set VERSION=%%i

:: 解析命令行参数
if /i "%~1" == "mingw" (
    set QT_DIR=%QT_MINGW_ROOT%
) else (
    set QT_DIR=%QT_MSVC_ROOT%
)

if /i "%~2" == "debug" (
    set BUILD_TYPE=Debug
    set INSTALLER_DIR=%PROJECT_ROOT%\release\v%VERSION%d
) else (
    set BUILD_TYPE=Release
    set INSTALLER_DIR=%PROJECT_ROOT%\release\v%VERSION%
)

if "%~3" neq "" (
    set INSTALLER_DIR=%~f3
)

echo Qt 路径: %QT_DIR%
echo 编译类型: %BUILD_TYPE%
echo 安装路径: %INSTALLER_DIR%

:: 检查可执行文件是否存在
set TARGET_FILE=%INSTALLER_DIR%\%PROJECT_NAME%.exe
if not exist %TARGET_FILE% (
    echo 错误: 找不到可执行文件 %TARGET_FILE%
    echo 请确保已成功编译并生成应用程序到目标路径
    pause
    exit /b 1
)

:: 运行windeployqt工具收集依赖（需要Qt安装路径）
set WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe
if exist "%WINDEPLOYQT%" (
    echo.
    echo 正在使用 "%WINDEPLOYQT%" 收集Qt运行时依赖...
    :: 净化 PATH，避免其它环境（如 anaconda/conda）的 Qt DLL 干扰 windeployqt；
    :: 脚本结束时 setlocal 会自动恢复原始 PATH
    set "PATH=%QT_DIR%\bin;%SystemRoot%\System32;%SystemRoot%"
    "%WINDEPLOYQT%" "%TARGET_FILE%" --dir "%INSTALLER_DIR%" --no-compiler-runtime --no-translations --force

    if errorlevel 1 (
        echo 收集Qt运行时依赖配置失败!
        exit /b 1
    )
) else (
    echo 警告: 找不到 windeployqt 工具，请手动复制Qt运行时依赖文件!
    echo 需要复制的文件通常包括:
    echo - Qt5Core.dll
    echo - Qt5Gui.dll
    echo - Qt5Widgets.dll
    echo - platforms\qwindows.dll
    echo - iconengines\*
    echo - imageformats\*
    echo - styles\*
)

@REM echo 复制许可证文件...
@REM copy "%PROJECT_ROOT%\LICENSE.GPL3" "%INSTALLER_DIR%\LICENSE.GPL3"

echo.
echo 依赖收集完成！发布包位于: %INSTALLER_DIR%
echo Done.

if "%BUILD_TYPE%"=="Release" (
    echo 现在可以使用以下脚本生成安装程序^:
    echo %SCRIPT_DIR%\build_installer.bat
)
