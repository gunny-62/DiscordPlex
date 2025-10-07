# Presence For Plex

A lightweight C++ application that displays your Plex media activity in Discord's Rich Presence. Uses around 3-4 MB of RAM

## Features

-   Current Plex activity in your Discord status.
-   Includes show titles, episode information, and progress, Bitrate, Quality, Bluray tags if the filename includes "Remux, or Bluray" as well as client type
-   Runs in the system tray (Windows only).
-   Shows Music quality tag from file info.
-   Preferences panel to decide what type of activity and how to display it. 

## Installation

1.  Download the latest release from the [Releases](https://github.com/gunny-62/DiscordPlex/releases) page.
2.  Run the installer.
3.  Connect your Plex account when prompted.

## Building from Source

### Requirements

-   C++17 compatible compiler
-   CMake 3.25+
-   vcpkg
-   Set `VCPKG_ROOT` environment variable

### Build Instructions

#### Using Warp Terminal (Recommended)

1. Clone the repository using Warp's clone feature
2. Ask the Warp AI to assist with building and making changes

#### Manual Build

```bash
git clone https://github.com/gunny-62/DiscordPlex.git
cd DiscordPlex

# Windows
./build.ps1  # PowerShell script (recommended)
# or
./self-contained-windows-build.bat  # Alternative batch script

# macOS/Linux  
./build.sh
```

For detailed Windows build instructions, see [WINDOWS-BUILD-INSTRUCTIONS.md](WINDOWS-BUILD-INSTRUCTIONS.md).
