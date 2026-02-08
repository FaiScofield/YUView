@echo off
chcp 65001 > nul

echo Usage: %~n0 [msvc^|mingw] [release^|debug]
echo -------------------------------

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..
set PROJECT_NAME=YUView

:: 从CMakeLists.txt获取版本号
for /f %%i in ('powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%\get_version.ps1"') do set VERSION=%%i

:: 解析命令行参数
if /i "%~1" == "mingw" (
    set APP_PATH=%PROJECT_ROOT%\build\build_win32_mingw\src\%PROJECT_NAME%.exe
    set QT_DIR=D:\Qt\5.15.2\mingw81_64
) else (
    :: msvc + ninja
    set APP_PATH=%PROJECT_ROOT%\build\build_win32_msvc\src\%PROJECT_NAME%.exe
    set QT_DIR=D:\Qt\5.15.2\msvc2019_64
)

if /i "%~2" == "debug" (
    set BUILD_TYPE=Debug
    set INSTALLER_DIR=%PROJECT_ROOT%\release\v%VERSION%d
) else (
    set BUILD_TYPE=Release
    set INSTALLER_DIR=%PROJECT_ROOT%\release\v%VERSION%
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

echo 复制许可证文件...
copy "%PROJECT_ROOT%\LICENSE" "%INSTALLER_DIR%\LICENSE.txt"

echo.
echo 依赖收集完成！发布包位于: %INSTALLER_DIR%
echo Done.

if "%BUILD_TYPE%"=="Release" (
    echo 现在可以使用以下脚本生成安装程序:
    echo %SCRIPT_DIR%\build_installer.bat
)
