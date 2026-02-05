@echo off
chcp 65001 > nul

echo 正在生成 MyQtUiProj 安装程序...

:: 获取脚本所在目录的父目录（项目根目录）
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..

:: 从CMakeLists.txt获取版本号
for /f %%i in ('powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%\get_version.ps1"') do set VERSION=%%i
echo 检测到当前版本号: v%VERSION%

:: 检查是否安装了Inno Setup
set ISCC_PATH="D:\InnoSetup6\ISCC.exe"
if exist %ISCC_PATH% (
    echo 找到 Inno Setup，正在编译安装程序...
    %ISCC_PATH% "%SCRIPT_DIR%\installer_script.iss" /DMyAppVersion=%VERSION%
) else (
    echo 错误: 未找到 Inno Setup 编译器
    echo 请安装 Inno Setup 6 或更高版本，然后重新运行此脚本
    echo 下载地址: https://jrsoftware.org/isinfo.php
    echo.
    echo 或者您可以手动运行以下命令:
    echo "D:\InnoSetup6\ISCC.exe" "%SCRIPT_DIR%\installer_script.iss" /DMyAppVersion=%VERSION%
    pause
    exit /b 1
)

echo.
echo 安装程序生成完成!
echo 输出文件位于: %PROJECT_ROOT%\release\MyQtUiProj_Setup_v%VERSION%.exe
echo Done.