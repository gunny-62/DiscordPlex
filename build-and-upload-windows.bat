@echo off
setlocal enabledelayedexpansion

REM --- Configuration ---
set "GITHUB_REPO=gunny-62/DiscordPlex"
set "VERSION_TAG=6.0.11"
set "BUILD_SCRIPT=build.bat"
set "RELEASE_FOLDER=release"

echo === Windows Build and Upload Script for PresenceForPlex ===
echo Target version: %VERSION_TAG%
echo.

REM --- Check for GitHub CLI ---
gh --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: GitHub CLI (gh) is not installed or not in PATH.
    echo Please install from: https://cli.github.com/
    pause
    exit /b 1
)

REM --- Check for vcpkg ---
if not defined VCPKG_ROOT (
    echo ERROR: VCPKG_ROOT environment variable is not set
    echo Please set VCPKG_ROOT to your vcpkg installation directory
    pause
    exit /b 1
)

REM --- Write version to file ---
echo %VERSION_TAG% > version.txt

REM --- Clean build directory ---
echo Cleaning previous build...
if exist build rmdir /s /q build
if exist "%RELEASE_FOLDER%" rmdir /s /q "%RELEASE_FOLDER%"

REM --- Run the Build Script ---
echo.
echo --- Running build script... ---
call "%BUILD_SCRIPT%"
if %errorlevel% neq 0 (
    echo ERROR: Build script failed. Aborting.
    pause
    exit /b %errorlevel%
)

REM --- Find the installer file ---
echo.
echo Looking for Windows installer...
for %%f in ("%RELEASE_FOLDER%\PresenceForPlex-*-win64.exe") do (
    set "INSTALLER_PATH=%%f"
    echo Found installer: !INSTALLER_PATH!
)

if not defined INSTALLER_PATH (
    echo ERROR: Could not find the Windows installer .exe file in %RELEASE_FOLDER%\
    echo Available files:
    dir "%RELEASE_FOLDER%\" /b
    pause
    exit /b 1
)

REM --- Upload to existing GitHub release ---
echo.
echo --- Uploading Windows installer to GitHub release %VERSION_TAG%... ---
gh release upload "%VERSION_TAG%" --repo "%GITHUB_REPO%" "!INSTALLER_PATH!" --clobber
if %errorlevel% neq 0 (
    echo ERROR: GitHub release upload failed.
    pause
    exit /b %errorlevel%
)

echo.
echo --- Windows build uploaded successfully! ---
echo The Windows installer has been added to release %VERSION_TAG%
echo Release URL: https://github.com/%GITHUB_REPO%/releases/tag/%VERSION_TAG%
pause
endlocal