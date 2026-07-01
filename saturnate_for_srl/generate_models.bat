@ECHO Off
SET COMPILER_DIR=..\..\Compiler
SET PATH=%COMPILER_DIR%\Other Utilities;%PATH%
SET PATH=%COMPILER_DIR%\msys2\usr\bin;%PATH%
SET PATH=%COMPILER_DIR%\sh2eb-elf\bin;%PATH%
SET PATH=%COMPILER_DIR%\WINDOWS\Other Utilities;%COMPILER_DIR%\WINDOWS\bin;%PATH%;C:\Program Files\Blender Foundation\Blender 5.1

where blender >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERREUR] Please install blender and add it to PATH to use this command..
    pause
    exit /b 1
)

make assets