#!/bin/bash
set -e

# --- Configuration ---
GITHUB_REPO="gunny-62/DiscordPlex"
BUILD_SCRIPT="./build.sh"
RELEASE_FOLDER="release"

echo "=== macOS Release Script for PresenceForPlex ==="

# Check dependencies
if ! command -v git &> /dev/null; then
    echo "Error: git is required but not installed."
    exit 1
fi

if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is required but not installed."
    echo "Install with: brew install gh"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "Error: python3 is required but not installed."
    exit 1
fi

if [ -z "$VCPKG_ROOT" ]; then
    echo "Error: VCPKG_ROOT environment variable is not set"
    echo "Please set VCPKG_ROOT to your vcpkg installation directory"
    exit 1
fi

# --- Get New Version Tag ---
echo ""
read -p "Enter the new version tag (e.g., v0.3.8): " VERSION_TAG

if [ -z "$VERSION_TAG" ]; then
    echo "Error: No version tag entered. Aborting."
    exit 1
fi

# --- Write version to file ---
echo "$VERSION_TAG" > version.txt

# --- Clean build directory ---
if [ -d "build" ]; then
    rm -rf build
fi

# --- Run the Build Script ---
echo ""
echo "--- Running build script... ---"
if ! $BUILD_SCRIPT; then
    echo "Error: Build script failed. Aborting release."
    exit 1
fi

# --- Git Commit and Push ---
echo ""
echo "--- Committing and pushing changes to GitHub... ---"
git add .
git commit -m "Release $VERSION_TAG"

# Create tag (ignore if it already exists)
if git tag "$VERSION_TAG" 2>/dev/null; then
    echo "Created tag $VERSION_TAG"
else
    echo "Tag $VERSION_TAG already exists, skipping."
fi

git push
git push --tags

if [ $? -ne 0 ]; then
    echo "Error: Git push failed. Aborting release."
    exit 1
fi

# --- Find the installer/package file ---
INSTALLER_PATH=""

# Look for DMG files first (macOS preferred format)
if [ -f "$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG-Darwin.dmg" ]; then
    INSTALLER_PATH="$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG-Darwin.dmg"
elif [ -f "$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG.dmg" ]; then
    INSTALLER_PATH="$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG.dmg"
# Fallback to tar.gz
elif [ -f "$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG-Darwin.tar.gz" ]; then
    INSTALLER_PATH="$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG-Darwin.tar.gz"
elif [ -f "$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG.tar.gz" ]; then
    INSTALLER_PATH="$RELEASE_FOLDER/PresenceForPlex-$VERSION_TAG.tar.gz"
else
    # Look for any package file in the release directory
    INSTALLER_PATH=$(find "$RELEASE_FOLDER" -name "PresenceForPlex-*" -type f | head -n 1)
fi

if [ -z "$INSTALLER_PATH" ] || [ ! -f "$INSTALLER_PATH" ]; then
    echo "Error: Could not find the installer/package file in $RELEASE_FOLDER/"
    echo "Available files:"
    ls -la "$RELEASE_FOLDER/" || echo "No release folder found"
    exit 1
fi

echo "Found package: $INSTALLER_PATH"

# --- Generate Changelog ---
echo ""
echo "--- Generating changelog... ---"
python3 generate_changelog.py

if [ $? -ne 0 ]; then
    echo "Error: Changelog generation failed. Aborting release."
    exit 1
fi

CHANGELOG_PATH="release_notes/changelog_$VERSION_TAG.md"

if [ ! -f "$CHANGELOG_PATH" ]; then
    echo "Error: Could not find the changelog file at $CHANGELOG_PATH"
    exit 1
fi

# --- Create GitHub Release and Upload Asset ---
echo ""
echo "--- Creating GitHub release and uploading package... ---"
gh release create "$VERSION_TAG" --repo "$GITHUB_REPO" --title "Release $VERSION_TAG" --notes-file "$CHANGELOG_PATH"

if [ $? -ne 0 ]; then
    echo "Error: GitHub release creation failed."
    exit 1
fi

gh release upload "$VERSION_TAG" --repo "$GITHUB_REPO" "$INSTALLER_PATH" --clobber

if [ $? -ne 0 ]; then
    echo "Error: GitHub release upload failed."
    exit 1
fi

echo ""
echo "--- Release $VERSION_TAG created successfully! ---"
echo "The package has been uploaded to your GitHub repository."
echo "Package uploaded: $INSTALLER_PATH"