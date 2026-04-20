@ECHO Off
SET COMPILER_DIR=..\..\Compiler
SET PATH=%COMPILER_DIR%\WINDOWS\Other Utilities;%COMPILER_DIR%\WINDOWS\bin;%PATH%;C:\Program Files\Blender Foundation\Blender 5.1
SET CDDIR=cd
SET CDASSETDIR=%CDDIR%\ASSETS

where blender >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERREUR] Please install blender and add it to PATH to use this command..
    pause
    exit /b 1
)

if not exist "%CDASSETDIR%" mkdir "%CDASSETDIR%"

make assets