@echo off
chcp 65001 > nul

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..

echo generating %PROJECT_ROOT%\ui codes with uic tool ...

if not exist %PROJECT_ROOT%\uic (
    mkdir %PROJECT_ROOT%\uic
)

set UIC_EXE=uic.exe
@REM set UIC_EXE=D:\Qt\5.15.2\msvc2019_64\bin\uic.exe

@REM %UIC_EXE% %PROJECT_ROOT%\ui\demoMainWindow.ui -o %PROJECT_ROOT%\uic\demoMainWindowUi.h
@REM %UIC_EXE% %PROJECT_ROOT%\ui\demoPageBoxes.ui -o %PROJECT_ROOT%\uic\demoPageBoxesUi.h
@REM %UIC_EXE% %PROJECT_ROOT%\ui\demoPageButtons.ui -o %PROJECT_ROOT%\uic\demoPageButtonsUi.h
@REM %UIC_EXE% %PROJECT_ROOT%\ui\demoPageTextEdit.ui -o %PROJECT_ROOT%\uic\demoPageTextEditUi.h

echo Done.