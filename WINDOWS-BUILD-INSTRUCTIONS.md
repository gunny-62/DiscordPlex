# Windows Build Instructions

## Quick Start

1. **Clone the repository** (or pull latest changes):
   ```batch
   git clone https://github.com/gunny-62/DiscordPlex.git
   cd DiscordPlex
   ```
   OR
   ```batch
   git pull origin master
   ```

2. **Run the self-contained build script**:
   ```batch
   self-contained-windows-build.bat
   ```

3. **Enter the version** when prompted (e.g., `6.0.11`)

## What the script does:

✅ Downloads and sets up vcpkg automatically  
✅ Configures the project with CMake  
✅ Builds the Windows x64 executable  
✅ Packages it as an installer (.exe)  
✅ Uploads it to the GitHub release  

## Prerequisites:

- **Visual Studio 2022** with C++ workload
- **CMake** (usually comes with Visual Studio)
- **Git** (to clone vcpkg)
- **GitHub CLI** - Download from: https://cli.github.com/

## Troubleshooting:

### "gh is not recognized"
Install GitHub CLI from https://cli.github.com/ and make sure you're authenticated:
```batch
gh auth login
```

### "CMake is not recognized" 
Make sure Visual Studio 2022 is installed with the C++ workload, or install CMake separately.

### "Git is not recognized"
Install Git from https://git-scm.com/

## Alternative Scripts:

If the self-contained script doesn't work, try:

1. **simple-windows-build.bat** - Requires pre-installed vcpkg
2. **release.bat** - Original release script
3. **build.bat** - Just builds (doesn't upload)

## Manual Process:

If all scripts fail, you can build manually:

```batch
# Set up vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
cd ..

# Set environment
set VCPKG_ROOT=%CD%\vcpkg

# Build
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release
cd build
cpack -C Release
```

Then upload manually using GitHub CLI or the web interface.