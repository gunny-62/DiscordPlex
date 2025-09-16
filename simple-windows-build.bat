@echo off

REM --- Configuration ---
set GITHUB_REPO=gunny-62/DiscordPlex
set BUILD_SCRIPT=build.bat
set RELEASE_FOLDER=release

echo === Simple Windows Build and Upload Script ===
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
    pause
    exit /b 1
)

REM --- Check for vcpkg ---
if "%VCPKG_ROOT%"=="" (
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
if exist %RELEASE_FOLDER% rmdir /s /q %RELEASE_FOLDER%

REM --- Run the Build Script ---
echo.
echo --- Running build script... ---
call %BUILD_SCRIPT%
if errorlevel 1 (
    echo ERROR: Build script failed. Aborting.
    pause
    exit /b 1
)

REM --- Find and upload the installer file ---
echo.
echo Looking for Windows installer...
if exist "%RELEASE_FOLDER%\PresenceForPlex-%VERSION_TAG%-win64.exe" (
    set INSTALLER_PATH=%RELEASE_FOLDER%\PresenceForPlex-%VERSION_TAG%-win64.exe
    goto :upload
)

REM Try wildcard pattern
for %%f in ("%RELEASE_FOLDER%\PresenceForPlex-*-win64.exe") do (
    set INSTALLER_PATH=%%f
    goto :upload
)

echo ERROR: Could not find Windows installer in %RELEASE_FOLDER%
echo Available files:
dir "%RELEASE_FOLDER%" /b
pause
exit /b 1

:upload
echo Found installer: %INSTALLER_PATH%
echo.
echo --- Uploading to GitHub release %VERSION_TAG%... ---
gh release upload "%VERSION_TAG%" --repo "%GITHUB_REPO%" "%INSTALLER_PATH%" --clobber
if errorlevel 1 (
    echo ERROR: GitHub release upload failed.
    pause
    exit /b 1
)

echo.
echo --- SUCCESS! Windows build uploaded! ---
echo The Windows installer has been added to release %VERSION_TAG%
echo Release URL: https://github.com/%GITHUB_REPO%/releases/tag/%VERSION_TAG%
pause