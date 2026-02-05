
@echo off
chcp 65001 > nul

echo Usage: %~n0 [Release^|Debug] [clean_flag]
echo ==================================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..
set GENERATOR="MinGW Makefiles"
set BUILD_DIR=%PROJECT_ROOT%\build\build_win32_mingw
set BUILD_TYPE=Release
set QT_PATH=D:/Qt/5.15.2/mingw81_64/
set QT_VERSION=5
set DO_CLEAN=0

:: parse command line arguments
if /i "%~1" == "debug" (
    set BUILD_TYPE=Debug
    set BUILD_DIR=%PROJECT_ROOT%\build\build_msvc_debug
)
if /i "%~2" == "1" (
    set DO_CLEAN=1
    echo.
    echo 正在执行 CMake 清理...
    cmake --build %BUILD_DIR% --target clean
)

:: update submodules & ui_codes
if not exist "%PROJECT_ROOT%\3rd\qspdlog" (
    echo 警告: 未找到 qspdlog 子模块，将自动拉取...
    git submodule update --init --recursive
)
if not exist "%PROJECT_ROOT%\3rd\spdlog" (
    echo 警告: 未找到 spdlog 子模块，将自动拉取...
    git submodule update --init --recursive
)
@REM if not exist "%PROJECT_ROOT%\uic" (
echo.
echo 正在更新 UI 代码...
call %SCRIPT_DIR%\update_ui_codes.cmd
@REM )

echo.
echo 正在执行 CMake 配置...
echo.

:: setup VS environment variables. NOTE: cmd too long, might need to use short path name
@REM if not defined VCINSTALLDIR (
@REM     ::call "C:\PROGRA~2\MICROS~4\2020\COMMUN~1\VC\Auxiliary\Build\vcvars64.bat"
@REM     call "C:\Program Files (x86)\Microsoft Visual Studio\2020\Community\VC\Auxiliary\Build\vcvars64.bat"
@REM )

cmake -G %GENERATOR% ^
    -H%PROJECT_ROOT% -B%BUILD_DIR% ^
    -DCMAKE_VERBOSE_MAKEFILE=OFF ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DQT_PATH=%QT_PATH% ^
    -DQT_VERSION=%QT_VERSION% ^
    -DCMAKE_C_COMPILER=gcc.exe ^
    -DCMAKE_CXX_COMPILER=g++.exe

if errorlevel 1 (
    echo CMake 配置失败，请检查编译选项
    exit /b 1
) else (
    echo.
    echo CMake 配置成功，正在编译...
    echo.
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j4 --

if errorlevel 1 (
    echo 编译失败！
    exit /b 1
) else (
    echo.
    echo 编译成功，正在安装...
    echo.
)

:: copy compile_commands.json to .vscode folder
if exist "%BUILD_DIR%\compile_commands.json" (
    cp %BUILD_DIR%\compile_commands.json ../.vscode
) else (
    echo WARNING: compile_commands.json NOT found in %BUILD_DIR%
)

cmake --install %BUILD_DIR%
if errorlevel 1 (
    echo 安装失败！
    exit /b 1
)
echo Done.
