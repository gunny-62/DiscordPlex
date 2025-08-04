@echo off
setlocal

REM Clean up previous build
if exist build rmdir /s /q build
if exist release rmdir /s /q release

REM Configure the project
call cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

REM Build the project
call cmake --build build --config Release
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

REM Package the installer
call cpack --config build/CPackConfig.cmake
if %errorlevel% neq 0 (
    echo Packaging failed.
    exit /b %errorlevel%
)

REM Create release directory and move the installer
mkdir release
for %%F in (PresenceForPlex-*-win64.exe) do (
    move "%%F" release\
)

echo.
echo Build successful!
echo The installer has been placed in the 'release' directory.

endlocal
