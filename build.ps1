# PowerShell build script for DiscordPlex

# 1) Get version
$currentVersion = ""
if (Test-Path "version.txt") {
    $currentVersion = Get-Content "version.txt"
}

Write-Host "Enter release version (current $currentVersion): " -NoNewline
$version = Read-Host
if ([string]::IsNullOrWhiteSpace($version)) {
    Write-Host "[ERROR] Version is required."
    exit 1
}

# Update version.txt
$version | Set-Content "version.txt"
$tag = "v$version"

# 2) Initialize VS environment
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
    Write-Host "[ERROR] vswhere.exe not found at $vsWhere"
    Write-Host "Install Visual Studio 2022 with C++ workload."
    exit 1
}

$vsInstall = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsInstall) {
    Write-Host "[ERROR] Could not find a Visual Studio installation with C++ tools."
    exit 1
}

$vsDevCmd = Join-Path $vsInstall "Common7\Tools\VsDevCmd.bat"
if (-not (Test-Path $vsDevCmd)) {
    Write-Host "[ERROR] VsDevCmd not found: $vsDevCmd"
    exit 1
}

# Call VsDevCmd and import its environment
$output = & "${env:COMSPEC}" /c "`"$vsDevCmd`" -arch=x64 -host_arch=x64 && set"
foreach ($line in $output) {
    if ($line -match '^([^=]+)=(.*)$') {
        $varName = $matches[1]
        $varValue = $matches[2]
        Set-Item "env:$varName" $varValue
    }
}

# 3) Ensure vcpkg exists and is bootstrapped
$vcpkgDir = Join-Path $PWD "vcpkg"
if (-not (Test-Path (Join-Path $vcpkgDir "vcpkg.exe"))) {
    if (-not (Test-Path $vcpkgDir)) {
        Write-Host "[INFO] Cloning vcpkg locally"
        git clone https://github.com/Microsoft/vcpkg.git $vcpkgDir
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Failed to clone vcpkg."
            exit 1
        }
    }
    
    Write-Host "[INFO] Bootstrapping vcpkg"
    Push-Location $vcpkgDir
    & .\bootstrap-vcpkg.bat
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] vcpkg bootstrap failed."
        Pop-Location
        exit 1
    }
    Pop-Location
}

$env:VCPKG_ROOT = $vcpkgDir
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows-static"

# 4) Clean previous artifacts
Remove-Item -Force -Recurse -ErrorAction SilentlyContinue build,release
Remove-Item -Force -ErrorAction SilentlyContinue include\version.h
New-Item -ItemType Directory -Force build | Out-Null

# 5) Configure with CMake
Write-Host "[INFO] Configuring static linkage"
$cmakeArgs = @(
    "-S", "."
    "-B", "build"
    "-G", "Visual Studio 17 2022"
    "-DCMAKE_TOOLCHAIN_FILE=`"$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`""
    "-DVCPKG_TARGET_TRIPLET=$env:VCPKG_DEFAULT_TRIPLET"
    "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW"
)

$cmakeResult = & cmake $cmakeArgs 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[WARN] Static configuration failed. Retrying with dynamic x64-windows"
    $env:VCPKG_DEFAULT_TRIPLET = "x64-windows"
    $cmakeResult = & cmake $cmakeArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] CMake configuration failed."
        exit $LASTEXITCODE
    }
}

# 6) Build
Write-Host "[INFO] Building Release"
& cmake --build build --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed."
    exit $LASTEXITCODE
}

# 7) Package
Write-Host "[INFO] Packaging installer"
& cpack -C Release --config build/CPackConfig.cmake
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Packaging failed."
    exit $LASTEXITCODE
}

# 8) Move artifact to release folder
New-Item -ItemType Directory -Force release | Out-Null
$artifact = Get-ChildItem -Path . -Filter "PresenceForPlex-*-win64.exe" | Select-Object -First 1
if (-not $artifact) {
    $artifact = Get-ChildItem -Path build -Filter "PresenceForPlex-*-win64.exe" | Select-Object -First 1
}
if (-not $artifact) {
    Write-Host "[ERROR] Could not find packaged installer (PresenceForPlex-*-win64.exe)."
    exit 1
}
Move-Item -Force $artifact.FullName release\
$artifact = Get-ChildItem -Path release -Filter "PresenceForPlex-*-win64.exe" | Select-Object -First 1
if (-not $artifact) {
    Write-Host "[ERROR] Failed to move artifact to release folder."
    exit 1
}

# 9) Git operations and GitHub release
Write-Host "[INFO] Preparing Git operations and GitHub release"

# Check if gh is logged in and properly configured
$ghStatus = gh auth status 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] GitHub CLI not authenticated. Please run: gh auth login"
    exit 1
}

$ghRepo = gh repo view --json nameWithOwner -q .nameWithOwner 2>&1
if ($ghRepo -ne "gunny-62/DiscordPlex") {
    Write-Host "[INFO] Setting up GitHub repository"
    gh repo set-default gunny-62/DiscordPlex
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Failed to set GitHub repository. Please run: gh repo set-default gunny-62/DiscordPlex"
        exit 1
    }
}

# Ensure main branch
Write-Host "[INFO] Ensuring we're on master branch"
git checkout master 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Failed to switch to main branch"
    exit 1
}

# Pull latest changes
Write-Host "[INFO] Pulling latest changes"
git pull 2>&1 | Out-Null

# Stage version change
Write-Host "[INFO] Committing version bump and tagging $tag"
git add version.txt
git commit -m "Release $tag" 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "[WARN] Nothing to commit (maybe version unchanged)."
}

# Create and push tag
git tag -f $tag 2>&1 | Out-Null

# Push changes
Write-Host "[INFO] Pushing changes to GitHub"
git push 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Failed to push branch changes to GitHub"
    exit 1
}

# Push tag
Write-Host "[INFO] Pushing tag to GitHub"
git push -f origin $tag 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Failed to push tag to GitHub"
    exit 1
}

# Create GitHub release
Write-Host "[INFO] Creating GitHub release $tag"

# Generate changelog by comparing with previous release
$prevTag = (git describe --abbrev=0 --tags $(git rev-list --tags --skip=1 --max-count=1) 2>$null)
if ($prevTag) {
    $changelog = git log --pretty=format:"- %s" "$prevTag..HEAD" | Where-Object { $_ -notlike "*Release v*" }
    if (-not $changelog) {
        $changelog = @("- Bug fixes and improvements")
    }
} else {
    $changelog = @("- Initial release")
}

$releaseNotes = @"
Presence For Plex $tag

Changelog:
$($changelog -join "`n")
"@

# Check if release already exists
$releaseExists = gh release view $tag 2>$null
if ($releaseExists) {
    Write-Host "[INFO] Updating existing release $tag"
    gh release delete $tag --yes 2>&1 | Out-Null
}

# Create new release
Write-Host "[INFO] Uploading release to GitHub"
gh release create $tag $artifact.FullName --title "Presence For Plex $tag" --notes $releaseNotes --draft=false
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Failed to create GitHub release"
    exit 1
}

Write-Host "[INFO] Successfully created GitHub release $tag"

# 10) GitHub Release
Write-Host "[INFO] Uploading release asset via gh CLI"
$ghExists = Get-Command gh -ErrorAction SilentlyContinue
if (-not $ghExists) {
    Write-Host "[WARN] GitHub CLI (gh) not found. Skipping release upload."
    Write-Host "Install from https://cli.github.com/ and run: gh auth login"
} else {
    $releaseExists = gh release view $tag 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[INFO] Uploading to existing release $tag"
        gh release upload $tag $artifact.FullName --clobber 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[WARN] Failed to upload release asset."
        }
    } else {
        Write-Host "[INFO] Creating new release $tag"
        gh release create $tag $artifact.FullName --title $tag --notes "Automated build" 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[WARN] Failed to create release."
        }
    }
}

# Done!
Write-Host "`n=============================================="
Write-Host "Build successful!"
Write-Host "Version: $version"
Write-Host "Artifact: $($artifact.FullName)"
Write-Host "Uploaded tag: $tag"
Write-Host "=============================================="`n