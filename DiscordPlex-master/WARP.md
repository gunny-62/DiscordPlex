# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

This is a C++17 application that displays Plex media activity in Discord's Rich Presence. The app connects to Plex servers via server-sent events (SSE) to monitor playback activity and communicates with Discord via IPC to update status. It runs as a lightweight system tray application (~3-4 MB RAM usage).

## Development Commands

### Prerequisites Setup
- C++17 compatible compiler
- CMake 3.25+
- vcpkg package manager
- Set `VCPKG_ROOT` environment variable

### Building

#### Windows Development Build
```bash
# Clean build
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Debug
```

#### macOS Development Build
```bash
# Clean build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

#### Using CMake Presets (Recommended)
```bash
# Debug build
cmake --preset debug
cmake --build --preset debug

# Release build
cmake --preset release
cmake --build --preset release
```

#### Full Build & Package
```bash
# Windows
./build.bat

# macOS
./build.sh
```

### Release Management
```bash
# Full release process (builds, commits, tags, creates GitHub release)
# Windows
./release.bat

# macOS
./release.sh

# Generate changelog only
python generate_changelog.py
```

### Development Workflow
```bash
# Quick debug build
cmake --preset debug && cmake --build --preset debug

# Run executable
./build/debug/Debug/PresenceForPlex.exe  # Windows Debug
./build/release/Release/PresenceForPlex.exe  # Windows Release
./build/PresenceForPlex  # macOS (after build)
```

## Architecture Overview

### Core Components

1. **Application** (`application.cpp/.h`) - Main orchestrator
   - Manages lifecycle of Plex and Discord components
   - Handles system tray integration (Windows)
   - Processes playback state changes and updates
   - Coordinates automatic update checking

2. **Plex** (`plex.cpp/.h`) - Plex Media Server integration
   - Authenticates with Plex using PIN-based OAuth flow
   - Maintains persistent SSE connections to multiple servers
   - Caches media metadata, artwork, and external API data (TMDB, MAL)
   - Processes real-time session state notifications
   - Handles different media types (movies, TV shows, music)

3. **Discord** (`discord.cpp/.h`) - Discord Rich Presence integration
   - Manages IPC connection to Discord client via named pipes
   - Implements rate limiting and connection retry with exponential backoff
   - Queues presence updates to prevent spam
   - Handles connection state callbacks

4. **DiscordIPC** (`discord_ipc.cpp/.h`) - Low-level Discord IPC
   - Platform-specific named pipe communication
   - Handshake and message protocol implementation
   - Connection health monitoring

### Data Flow

1. **Initialization**: App authenticates with Plex, establishes server connections
2. **Monitoring**: SSE connections listen for playback state changes
3. **Processing**: Session notifications trigger media info fetching and caching
4. **Display**: Processed data sent to Discord via rate-limited IPC
5. **Persistence**: Configs and caches stored in user directories

### Key Data Structures

- **MediaInfo** - Complete media metadata including playback state, media details, and user info
- **PlexServer** - Server connection state with HttpClient and SSE management
- **PlaybackState** - Enum for current playback status (Playing, Paused, Stopped, etc.)
- **MediaType** - Enum for content type (Movie, TVShow, Music)

### Configuration System

- Uses YAML files for user preferences (`config.cpp/.h`)
- Stores authentication tokens securely in user profile
- Caches frequently accessed data to minimize API calls
- Supports multiple Plex servers simultaneously

### Threading Model

- Main UI thread handles application lifecycle and tray events
- Separate background threads for:
  - Each Plex server SSE connection
  - Discord IPC connection management
  - Automatic update checking
- Thread-safe caching with mutexes for shared data structures

### External Dependencies

- **CURL** - HTTP/HTTPS requests and SSE connections
- **nlohmann/json** - JSON parsing and generation
- **yaml-cpp** - Configuration file handling
- **vcpkg** - Package management for consistent builds

### Build System

- **CMake** with presets for different build configurations
- **vcpkg integration** for dependency management
- **CPack** for installer generation (NSIS on Windows, DMG on macOS, TGZ on Linux)
- Platform-specific resource inclusion (icons, version info)

### Commit Convention

Use conventional commit format for automatic changelog generation:
- `feat:` - New features
- `fix:` - Bug fixes
- Other prefixes ignored in changelog

### Version Management

- Version stored in `version.txt` file
- CMake generates `version.h` from `version.h.in` template
- Release script handles version tagging and changelog generation

### Discord Artwork Issues

**Common Problem**: Discord Rich Presence artwork not displaying after Discord updates.

**Root Cause**: Discord has strict requirements for image URLs in Rich Presence:
- Images must be accessible via HTTPS
- Plex transcoder URLs (`/photo/:/transcode`) may not always work with Discord
- Discord caches images aggressively and may not refresh broken URLs

**Investigation Steps**:
1. Check logs for artwork URL generation in `buildArtworkUrl()` function
2. Verify Plex server URLs are accessible externally (not localhost)
3. Test artwork URLs directly in browser to ensure they resolve
4. Consider TMDB fallback artwork is working properly

**Current Implementation**:
- Primary: Plex transcoded images (`serverUri + /photo/:/transcode?width=256&height=256...`)
- Fallback: TMDB API images for movies/TV shows with external IDs
- Gatekeeping: Random local images for music when privacy mode enabled
