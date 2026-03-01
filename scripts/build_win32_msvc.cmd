
@echo off
chcp 65001 > nul

echo Usage^: %~n0 -t [Debug^|Release] [--clean] [--deploy] [--export]
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

set BUILD_DIR=%PROJECT_ROOT%\build\build_msvc

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

:: Setup VS environment variables. NOTE: cmd too long, might need to use short path name
if not defined VCINSTALLDIR (
    ::call "C:\PROGRA~2\MICROS~4\2020\COMMUN~1\VC\Auxiliary\Build\vcvars64.bat"
    call "C:\Program Files (x86)\Microsoft Visual Studio\2020\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
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

cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j6 --

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
    if not exist "%BUILD_DIR%\YUViewApp\%BUILD_TYPE%\Qt5Cored.dll" (
        call %SCRIPT_DIR%\collect_dependencies.bat msvc %BUILD_TYPE% %BUILD_DIR%\YUViewApp\%BUILD_TYPE%
    )
)

echo Done.
