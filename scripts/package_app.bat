@echo off
chcp 65001 > nul

echo Usage: %~n0 [msvc^|mingw]
echo ==================================================

echo.
echo MyQtUiProj 自动打包脚本

:: 获取脚本所在目录的父目录（项目根目录）
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..

:: 从CMakeLists.txt获取版本号
for /f %%i in ('powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%get_version.ps1"') do set VERSION=%%i
echo 检测到当前版本号: v%VERSION%

echo.
echo 步骤 1: 编译应用程序...
echo.

if /i "%~1" == "mingw" (
    echo 选择用 mingw 编译...
    call %SCRIPT_DIR%\build_win32_mingw.cmd Release 1
) else (
    echo 选择用 msvc 编译...
    call %SCRIPT_DIR%\build_win32_msvc.cmd Release 1
)

if errorlevel 1 (
    echo 编译失败，停止打包过程
    exit /b 1
)

echo.
echo 步骤 2: 收集运行时依赖...
echo.

call %SCRIPT_DIR%\collect_dependencies.bat %~1 Release
if errorlevel 1 (
    echo 依赖收集失败，停止打包过程
    exit /b 1
)

echo.
echo 步骤 3: 生成安装程序...
echo.

call %SCRIPT_DIR%\build_installer.bat
if errorlevel 1 (
    echo 安装程序生成失败
    exit /b 1
)

echo.
echo 打包完成!
echo 安装程序位于: %PROJECT_ROOT%\release\MyQtUiProj_Setup_v%VERSION%.exe
echo.