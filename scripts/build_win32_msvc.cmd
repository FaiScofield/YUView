
@echo off
setlocal enabledelayedexpansion
chcp 65001 > nul

echo Usage^: %~n0 -t [Debug^|Release] [-c] [-d] [-e] [-h]
echo ==================================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..

set QT_PATH=D:/Qt/5.15.2/msvc2019_64/
set GENERATOR="Visual Studio 17 2022"
set VS_VARSALL_BAT=

set BUILD_DIR=%PROJECT_ROOT%\build\build_msvc
set BUILD_TYPE=Debug
set DO_CLEAN=0
set DO_DEPLOY=0
set DO_EXPORT=0

:: 加载本机配置(local_build_config.cmd 已 git 忽略)，用于覆盖不同电脑上的路径差异
if exist "%SCRIPT_DIR%\local_build_config.cmd" (
    echo Load local build config file: "%SCRIPT_DIR%\local_build_config.cmd"
    call "%SCRIPT_DIR%\local_build_config.cmd"
)

:: 本机配置若提供了 QT_MSVC_ROOT，则用它统一 QT_PATH（msvc 编译专用，避免误用 mingw 的 Qt）
if defined QT_MSVC_ROOT set "QT_PATH=%QT_MSVC_ROOT%/"

:: Parse command line arguments
:ParseLoop
if "%~1"=="" goto :RunBuild

if /i "%~1"=="-h" goto :ShowHelp
if /i "%~1"=="--help" goto :ShowHelp

if /i "%~1"=="-t" (
    :: check if next argument is valid
    if /i "%~2"=="debug" (
        set BUILD_TYPE=Debug
    ) else if /i "%~2"=="release" (
        set BUILD_TYPE=Release
    ) else (
        echo Warning: unknown type "%~2", use default %BUILD_TYPE%
    )
    shift
) else if /i "%~1"=="-c" (
    set DO_CLEAN=1
) else if /i "%~1"=="--clean" (
    set DO_CLEAN=1
) else if /i "%~1"=="-d" (
    set DO_DEPLOY=1
) else if /i "%~1"=="--deploy" (
    set DO_DEPLOY=1
) else if /i "%~1"=="-e" (
    set DO_EXPORT=1
) else if /i "%~1"=="--export" (
    set DO_EXPORT=1
) else (
    echo Warning: unknown argument "%~1"
)

:: Shift to next argument
shift
goto :ParseLoop

:: --- Help ---
:ShowHelp
echo.
echo Usage: %~n0 -t [Debug^|Release] [options]
echo.
echo Options:
echo   -t, [Debug^|Release]      Set build type (default: Release^).
echo   -c, --clean              Remove old CMake cache before configuring.
echo   -d, --deploy             Collect Qt dependencies and build installer (for Release^).
echo   -e, --export             Generate compile_commands.json and copy to .vscode.
echo   -h, --help               Show this help message and exit.
echo.
echo Example: %~n0 -t Release -c -d
echo.
exit /b 0

:: --- Main program execution area ---
:RunBuild

set BUILD_DIR=%PROJECT_ROOT%\build\build_msvc
echo .
echo Generator: %GENERATOR%
echo Build type: %BUILD_TYPE%
echo Build dir: %BUILD_DIR%
echo do Clean: %DO_CLEAN%
echo do Deploy: %DO_DEPLOY%
echo do Export: %DO_EXPORT%

:: Cmake clean
if exist "%BUILD_DIR%" if "%DO_CLEAN%"=="1" (
    echo.
    echo Clean the old cmake cache...
    set "USER_CONFIRM="
    set /p USER_CONFIRM=Continue? ^(Y/N^):
    if /i not "!USER_CONFIRM!"=="y" (
        echo clean cancel, skip...
        goto :SkipClean
    )

    if exist "%BUILD_DIR%\CMakeCache.txt" del "%BUILD_DIR%\CMakeCache.txt"
    if exist "%BUILD_DIR%\YUViewApp" rmdir /s /q "%BUILD_DIR%\YUViewApp"
    if exist "%BUILD_DIR%\YUViewLib" rmdir /s /q "%BUILD_DIR%\YUViewLib"
    echo clean success.
    goto :SkipClean
)

:SkipClean
mkdir "%BUILD_DIR%" 2>nul

:: Setup VS environment variables. NOTE: cmd too long, might need to use short path name
if not defined VCINSTALLDIR (
    set "VS_VARSALL_BAT_FOUND="
    if defined VS_VARSALL_BAT (
        set "VS_VARSALL_BAT_FOUND=!VS_VARSALL_BAT!"
    ) else (
        :: 用 vswhere 自动探测最新 Visual Studio 安装位置
        set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
        if exist "!VSWHERE!" (
            set "VS_INSTALL_DIR="
            for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -property installationPath`) do set "VS_INSTALL_DIR=%%i"
            if defined VS_INSTALL_DIR set "VS_VARSALL_BAT_FOUND=!VS_INSTALL_DIR!\VC\Auxiliary\Build\vcvarsall.bat"
        )
    )
    if exist "!VS_VARSALL_BAT_FOUND!" (
        call "!VS_VARSALL_BAT_FOUND!" x64
    ) else (
        echo Error: vcvarsall.bat not found: "!VS_VARSALL_BAT_FOUND!"
        echo Please set VS_VARSALL_BAT in local_build_config.cmd.
        exit /b 1
    )
)

:: Do CMake Configure
echo ========================================
echo Do CMake Configure...
echo ========================================

cmake -G%GENERATOR% ^
    -H"%PROJECT_ROOT%" ^
    -B"%BUILD_DIR%" ^
    -DCMAKE_VERBOSE_MAKEFILE=OFF ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DQT_PATH=%QT_PATH% ^
    -DCMAKE_C_COMPILER=clang-cl.exe ^
    -DCMAKE_CXX_COMPILER=clang-cl.exe ^
    -DENABLE_CONSOLE=ON ^
    -DENABLE_SPDLOG=ON

:: Check config result
if %errorlevel% neq 0 (
    echo CMake config failed, Please have a check!
    exit /b 1
)

:: Build project
echo ========================================
echo CMake config success, continue to build...
echo ========================================

cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j4 --

if %errorlevel% neq 0 (
    echo Cmake build failed!
    exit /b 1
)

:: install
echo ========================================
echo Cmake build success, continue to install...
echo ========================================

cmake --install %BUILD_DIR% --config %BUILD_TYPE%
if %errorlevel% neq 0 (
    echo Cmake install failed!
)

echo ========================================

:: Copy compile_commands.json to .vscode folder
if "%DO_EXPORT%"=="1" (
    if exist "%BUILD_DIR%\compile_commands.json" (
        echo 'compile_commands.json' exists, just copy to .vscode folder...
        cp %BUILD_DIR%\compile_commands.json %PROJECT_ROOT%\.vscode\
    ) else (
        echo Do msvc compile_commands.json generation...
        powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%clang-build.ps1" -dir "%BUILD_DIR%" -export-jsondb
        copy /y "%BUILD_DIR%\compile_commands.json" "%PROJECT_ROOT%\.vscode\"
    )
)

:: collect dependencies qt libraries
if "%DO_DEPLOY%"=="1" (
    @REM call %SCRIPT_DIR%\collect_dependencies.bat msvc %BUILD_TYPE% %BUILD_DIR%\YUViewApp\%BUILD_TYPE%
    call %SCRIPT_DIR%\collect_dependencies.bat msvc %BUILD_TYPE%

    if /i "%BUILD_TYPE%"=="release" (
        call %SCRIPT_DIR%\build_installer.bat Release
    )
)

echo Done.
