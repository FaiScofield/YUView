
@echo off
chcp 65001 > nul

echo Usage: %~n0 [Release^|Debug] [clean_flag]
echo ==================================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..
set GENERATOR="Visual Studio 17 2022"
@REM set GENERATOR=Ninja
set BUILD_DIR=%PROJECT_ROOT%\build\build_msvc
set BUILD_TYPE=Release
set QT_PATH=D:/Qt/5.15.2/msvc2019_64/bin/
set QT_VERSION=5
set DO_CLEAN=0

:: parse command line arguments
if /i "%~1" == "debug" (
    set BUILD_TYPE=Debug
    set BUILD_DIR=%PROJECT_ROOT%\build\build_msvc
)
if /i "%~2" == "1" (
    set DO_CLEAN=1
    echo.
    echo 正在执行 CMake 清理...
    cd %BUILD_DIR%
    ninja clean
    cd %CD%
)

@REM if not exist "%PROJECT_ROOT%\uic" (
@REM echo.
@REM echo 正在更新 UI 代码...
@REM call %SCRIPT_DIR%\update_ui_codes.cmd
@REM )

echo.
echo 正在执行 CMake 配置...
echo.

:: setup VS environment variables. NOTE: cmd too long, might need to use short path name
if not defined VCINSTALLDIR (
    ::call "C:\PROGRA~2\MICROS~4\2020\COMMUN~1\VC\Auxiliary\Build\vcvars64.bat"
    call "C:\Program Files (x86)\Microsoft Visual Studio\2020\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

cmake -G %GENERATOR% ^
    -H%PROJECT_ROOT% ^
    -B%BUILD_DIR% ^
    -DCMAKE_VERBOSE_MAKEFILE=OFF ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DQT_PATH=%QT_PATH% ^
    -DCMAKE_C_COMPILER=clang-cl.exe ^
    -DCMAKE_CXX_COMPILER=clang-cl.exe ^
    -DENABLE_ASAN=OFF ^
    -DENABLE_CONSOLE=ON ^
    -DENABLE_SPDLOG=ON

if errorlevel 1 (
    echo CMake 配置失败，请检查编译选项
    exit /b 1
) else (
    echo.
    echo CMake 配置成功，正在编译...
    echo.
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j6 --

if errorlevel 1 (
    echo 编译失败！
    exit /b 1
) else (
    echo.
    echo 编译成功，正在安装...
    echo.
)

:: install
cmake --install %BUILD_DIR% --config %BUILD_TYPE%
if errorlevel 1 (
    echo 安装失败！
)

:: copy compile_commands.json to .vscode folder
if exist "%BUILD_DIR%\compile_commands.json" (
    cp %BUILD_DIR%\compile_commands.json %PROJECT_ROOT%\.vscode\
) else (
    echo do msvc compile_commands.json generation...
    powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%clang-build.ps1" -dir "%BUILD_DIR%" -export-jsondb
    copy /y "%BUILD_DIR%\compile_commands.json" "%PROJECT_ROOT%\.vscode\"
)

:: collect dependencies qt libraries
if not exist "%BUILD_DIR%\YUViewApp\%BUILD_TYPE%\Qt5Cored.dll" (
    call %SCRIPT_DIR%\collect_dependencies.bat msvc %BUILD_TYPE% %BUILD_DIR%\YUViewApp\%BUILD_TYPE%
)

echo Done.
