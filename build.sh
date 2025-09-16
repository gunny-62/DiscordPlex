#!/bin/bash
set -e

echo "=== macOS Build Script for PresenceForPlex ==="

# Check if VCPKG_ROOT is set
if [ -z "$VCPKG_ROOT" ]; then
    echo "Error: VCPKG_ROOT environment variable is not set"
    echo "Please set VCPKG_ROOT to your vcpkg installation directory"
    exit 1
fi

# Clean up previous build
echo "Cleaning previous build..."
if [ -d "build" ]; then
    rm -rf build
fi
if [ -d "release" ]; then
    rm -rf release
fi
if [ -f "include/version.h" ]; then
    rm include/version.h
fi

# Configure the project for macOS
echo "Configuring project..."
cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "CMake configuration failed."
    exit 1
fi

# Build the project
echo "Building project..."
cmake --build build --config Release

if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

# Package the application
echo "Packaging application..."
cd build
cpack

if [ $? -ne 0 ]; then
    echo "Packaging failed."
    exit 1
fi

cd ..

# Create release directory and move the package
echo "Organizing release artifacts..."
mkdir -p release

# Move the created package to release directory
for package in build/PresenceForPlex-*.dmg build/PresenceForPlex-*.tar.gz; do
    if [ -f "$package" ]; then
        mv "$package" release/
        echo "Moved $(basename "$package") to release directory"
    fi
done

echo ""
echo "Build successful!"
echo "The application package has been placed in the 'release' directory."

# Make the executable runnable if it exists
if [ -f "build/PresenceForPlex" ]; then
    chmod +x build/PresenceForPlex
    echo "Executable: ./build/PresenceForPlex"
fi