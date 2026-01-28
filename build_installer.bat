@echo off
REM ========================================
REM ZephyrSense Installer Build Script
REM Version: 0.1.0
REM ========================================

echo.
echo ========================================
echo ZephyrSense Installer Build Script v0.1
echo ========================================
echo.

REM Step 1: Check if Release build exists
echo [1/5] Checking for Release build...
if not exist "build\Release\appZephyrSense.exe" (
    echo.
    echo ERROR: Release build not found!
    echo.
    echo Please build the Release version first:
    echo   1. cd build
    echo   2. cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
    echo   3. ninja
    echo.
    exit /b 1
)
echo       OK - Found appZephyrSense.exe

REM Step 2: Check if Qt dependencies are deployed
echo [2/5] Checking Qt deployment...
if not exist "build\Release\Qt6Core.dll" (
    echo.
    echo ERROR: Qt dependencies not deployed!
    echo.
    echo Please run windeployqt:
    echo   C:\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe --release --qmldir qml\ build\Release\appZephyrSense.exe
    echo.
    exit /b 1
)
echo       OK - Qt DLLs found

REM Step 3: Check if VC++ Runtime is present
echo [3/5] Checking VC++ Runtime...
if not exist "build\Release\vc_redist.x64.exe" (
    echo.
    echo WARNING: vc_redist.x64.exe not found in build\Release\
    echo.
    echo The installer will not be able to install VC++ Runtime automatically.
    echo Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
    echo Place it in build\Release\ directory
    echo.
    echo Press any key to continue anyway, or Ctrl+C to abort...
    pause >nul
) else (
    echo       OK - vc_redist.x64.exe found
)

REM Step 4: Check if Inno Setup is installed
echo [4/5] Checking Inno Setup installation...
if not exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    echo.
    echo ERROR: Inno Setup 6 not found!
    echo.
    echo Please install Inno Setup 6 from:
    echo   https://jrsoftware.org/isdl.php
    echo.
    echo Default installation path should be:
    echo   C:\Program Files ^(x86^)\Inno Setup 6\
    echo.
    exit /b 1
)
echo       OK - Inno Setup 6 found

REM Step 5: Compile the installer
echo [5/5] Building installer with Inno Setup...
echo.
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\installer.iss

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo SUCCESS!
    echo ========================================
    echo.
    echo Installer created successfully:
    echo   installer\Output\ZephyrSense-Setup-0.1.0.exe
    echo.

    REM Display file size
    for %%A in (installer\Output\ZephyrSense-Setup-0.1.0.exe) do (
        echo File size: %%~zA bytes
    )

    echo.
    echo You can now distribute this installer to end users.
    echo.
    echo ========================================
) else (
    echo.
    echo ========================================
    echo ERROR: Installer compilation failed!
    echo ========================================
    echo.
    echo Check the Inno Setup output above for error details.
    echo.
    exit /b 1
)

pause
