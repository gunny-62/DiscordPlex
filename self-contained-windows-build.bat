@echo off

echo === Self-Contained Windows Build Script ===
echo This script will set up vcpkg and build the Windows executable
echo.

REM --- Get Version Tag ---
set /p VERSION_TAG="Enter the version tag (e.g., 6.0.11): "
if "%VERSION_TAG%"=="" (
    echo ERROR: No version tag entered. Aborting.
    pause
    exit /b 1
)

echo Target version: %VERSION_TAG%
echo.

REM --- Check for GitHub CLI ---
gh --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: GitHub CLI (gh) is not installed or not in PATH.
    echo Please install from: https://cli.github.com/
    echo You can download it from: https://cli.github.com/
    pause
    exit /b 1
)

REM --- Write version to file ---
echo %VERSION_TAG% > version.txt

REM --- Clean previous builds ---
echo Cleaning previous builds...
if exist build rmdir /s /q build
if exist release rmdir /s /q release
if exist vcpkg rmdir /s /q vcpkg

REM --- Set up vcpkg ---
echo.
echo --- Setting up vcpkg... ---
git clone https://github.com/Microsoft/vcpkg.git
if errorlevel 1 (
    echo ERROR: Failed to clone vcpkg repository
    pause
    exit /b 1
)

cd vcpkg
call bootstrap-vcpkg.bat
if errorlevel 1 (
    echo ERROR: Failed to bootstrap vcpkg
    pause
    exit /b 1
)
cd ..

REM --- Set vcpkg environment ---
set VCPKG_ROOT=%CD%\vcpkg
echo Using vcpkg at: %VCPKG_ROOT%

REM --- Configure and build ---
echo.
echo --- Configuring project... ---
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo --- Building project... ---
cmake --build build --config Release
if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo --- Packaging installer... ---
cd build
cpack -C Release
if errorlevel 1 (
    echo ERROR: Packaging failed
    pause
    exit /b 1
)
cd ..

REM --- Move installer to release folder ---
mkdir release
for %%f in ("build\PresenceForPlex-*-win64.exe") do (
    move "%%f" "release\"
    set INSTALLER_PATH=release\%%~nf%%~xf
    goto :upload
)

echo ERROR: Could not find generated installer
dir "build\" /b
pause
exit /b 1

:upload
echo.
echo Found installer: %INSTALLER_PATH%
echo --- Uploading to GitHub release %VERSION_TAG%... ---
gh release upload "%VERSION_TAG%" --repo "gunny-62/DiscordPlex" "%INSTALLER_PATH%" --clobber
if errorlevel 1 (
    echo ERROR: GitHub release upload failed
    echo Make sure you're authenticated with: gh auth login
    pause
    exit /b 1
)

echo.
echo === SUCCESS! ===
echo Windows installer has been built and uploaded to GitHub release %VERSION_TAG%
echo Release URL: https://github.com/gunny-62/DiscordPlex/releases/tag/%VERSION_TAG%
echo.
echo The installer is: %INSTALLER_PATH%
pause