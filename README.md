# Presence For Plex

A lightweight C++ application that displays your Plex media activity in Discord's Rich Presence.

## Features

-   Current Plex activity in your Discord status.
-   Includes show titles, episode information, and progress, Bitrate, Quality, Bluray tags if the filename includes "Remux, or Bluray"
-   Runs in the system tray (Windows only).
-   Shows Music quality tag from file info.
-   Preferences box to decide what type of activity and how to display it. 

## Installation

1.  Download the latest release from the [Releases](https://github.com/gunny-62/DiscordPlex/releases) page.
2.  Run the installer.
3.  Connect your Plex account when prompted.

## Building from Source

### Requirements

-   C++17 compatible compiler
-   CMake 3.25+
-   vcpkg

### Build Instructions

```bash
git clone https://github.com/gunny-62/DiscordPlex.git
cd DiscordPlex
./build.bat
