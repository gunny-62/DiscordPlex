@echo off
setlocal enabledelayedexpansion

REM --- Configuration ---
set "GITHUB_REPO=gunny-62/DiscordPlex"
set "BUILD_SCRIPT=build.bat"
set "RELEASE_FOLDER=release"

REM --- Get New Version Tag ---
echo.
set "VERSION_TAG="
set /p VERSION_TAG="Enter the new version tag (e.g., v0.3.8): "
if not defined VERSION_TAG (
    echo ERROR: No version tag entered. Aborting.
    pause
    exit /b 1
)

REM --- Write version to file ---
echo !VERSION_TAG! > version.txt

REM --- Clean build directory ---
if exist build rmdir /s /q build

REM --- Run the Build Script ---
echo.
echo --- Running build script... ---
call "%BUILD_SCRIPT%"
if %errorlevel% neq 0 (
    echo ERROR: Build script failed. Aborting release.
    pause
    exit /b %errorlevel%
)

REM --- Git Commit and Push ---
echo.
echo --- Committing and pushing changes to GitHub... ---
git add .
git commit -m "Release %VERSION_TAG%"
git tag "%VERSION_TAG%" >nul 2>&1
if errorlevel 1 (
    echo "Tag %VERSION_TAG% already exists, skipping."
)
git push
git push --tags
if %errorlevel% neq 0 (
    echo ERROR: Git push failed. Aborting release.
    pause
    exit /b %errorlevel%
)

REM --- Find the installer file ---
set "INSTALLER_PATH=release\PresenceForPlex-!VERSION_TAG!-win64.exe"
if not exist "!INSTALLER_PATH!" (
    echo ERROR: Could not find the installer .exe.
    pause
    exit /b 1
)
echo Found installer: !INSTALLER_PATH!

REM --- Generate Changelog ---
echo.
echo --- Generating changelog... ---
python generate_changelog.py
if %errorlevel% neq 0 (
    echo ERROR: Changelog generation failed. Aborting release.
    pause
    exit /b %errorlevel%
)

set "CHANGELOG_PATH=release_notes\changelog_!VERSION_TAG!.md"

REM --- Create GitHub Release and Upload Asset ---
echo.
echo --- Creating GitHub release and uploading installer... ---
gh release create "%VERSION_TAG%" --repo "%GITHUB_REPO%" --title "Release %VERSION_TAG%" --notes-file "!CHANGELOG_PATH!"
if %errorlevel% neq 0 (
    echo ERROR: GitHub release creation failed.
    pause
    exit /b %errorlevel%
)

gh release upload "%VERSION_TAG%" --repo "%GITHUB_REPO%" "!INSTALLER_PATH!" --clobber
if %errorlevel% neq 0 (
    echo ERROR: GitHub release upload failed.
    pause
    exit /b %errorlevel%
)

echo.
echo --- Release %VERSION_TAG% created successfully! ---
echo The installer has been uploaded to your GitHub repository.
pause
endlocal
