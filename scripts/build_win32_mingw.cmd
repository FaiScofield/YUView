
@echo off
chcp 65001 > nul

echo Usage^: %~n0 -t [Debug^|Release] [--clean] [--deploy] [--export]
echo ==================================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..
set GENERATOR="MinGW Makefiles"
set BUILD_DIR=%PROJECT_ROOT%\build\build_win32_mingw
set BUILD_TYPE=Release
set QT_PATH=D:/Qt/5.15.2/mingw81_64/
set DO_CLEAN=0
set DO_DEPLOY=0
set DO_EXPORT=0

:: Parse command line arguments
:ParseLoop
if "%~1"=="" goto :RunBuild

if /i "%~1"=="-t" (
    :: check if next argument is valid
    if /i "%~2"=="debug" (
        set BUILD_TYPE=Debug
        shift
    ) else (
        if /i not "%~2"=="release" (
            echo Error: -t argument must be Debug or Release^: %~2
            cd %CD%
            exit /b 1
        )
        shift
    )
) else if /i "%~1"=="--clean" (
    set DO_CLEAN=1
) else if /i "%~1"=="--deploy" (
    set DO_DEPLOY=1
) else if /i "%~1"=="--export" (
    set DO_EXPORT=1
) else (
    echo Warning: unknown argument "%~1"
)

:: Shift to next argument
shift
goto :ParseLoop

:: --- Main program execution area ---
:RunBuild

echo Build type: %BUILD_TYPE%
echo Build dir: %BUILD_DIR%
echo do Clean: %DO_CLEAN%
echo do Deploy: %DO_DEPLOY%
echo do Export: %DO_EXPORT%

:: Cmake clean
if exist "%BUILD_DIR%" if "%DO_CLEAN%"=="1" (
    echo.
    echo Clean the old cmake cache...
    set /p USER_CONFIRM=Continue? ^(Y/N^):
    if /i not "%USER_CONFIRM%"=="Y" (
        echo clean cancel, skip...
        goto :SkipClean
    )

    del "%BUILD_DIR%\CMakeCache.txt"
    rmdir /s /q "%BUILD_DIR%\YUViewApp"
    rmdir /s /q "%BUILD_DIR%\YUViewLib"
)

:SkipClean
mkdir "%BUILD_DIR%" 2>nul

cmake -G %GENERATOR% ^
    -H%PROJECT_ROOT% -B%BUILD_DIR% ^
    -DCMAKE_VERBOSE_MAKEFILE=OFF ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DQT_PATH=%QT_PATH% ^
    -DCMAKE_C_COMPILER=gcc.exe ^
    -DCMAKE_CXX_COMPILER=g++.exe

if errorlevel 1 (
    echo CMake 配置失败，请检查编译选项
    exit /b 1
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j4 --

if errorlevel 1 (
    echo Cmake build failed!
    exit /b 1
)

cmake --install %BUILD_DIR%
if errorlevel 1 (
    echo Cmake install failed!
)

if "%DO_EXPORT%"=="1" (
    if exist "%BUILD_DIR%\compile_commands.json" (
        echo 'compile_commands.json' exists, just copy to .vscode folder...
        cp %BUILD_DIR%\compile_commands.json %PROJECT_ROOT%\.vscode\
    ) else (
        echo WARNING: compile_commands.json NOT found in %BUILD_DIR%
    )
)

if "%DO_DEPLOY%"=="1" (
    call %SCRIPT_DIR%\collect_dependencies.bat mingw %BUILD_TYPE% %BUILD_DIR%\YUViewApp\%BUILD_TYPE%

    if /i "%BUILD_TYPE%"=="release" (
        call %SCRIPT_DIR%\build_installer.bat release
    )
)

echo Done.
