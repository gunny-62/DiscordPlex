#pragma once

// Standard library headers
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

// Project headers
#include "http_client.h"

// Forward declarations
class HttpClient;

// Define the PlexServer struct
struct PlexServer
{
    std::string name;
    std::string clientIdentifier;
    std::string localUri;
    std::string publicUri;
    std::string accessToken;
    std::chrono::system_clock::time_point lastUpdated;
    std::unique_ptr<HttpClient> httpClient;
    std::atomic<bool> running;
    bool owned = false;
};

enum class PlaybackState
{
    Stopped,       // No active session
    Playing,       // Media is playing
    Paused,        // Media is paused
    Buffering,     // Media is buffering
    BadToken,      // Server configuration issue
    NotInitialized // Server not initialized
};

enum class MediaType
{
    Movie,
    TVShow,
    Music,
    Unknown
};

enum class LinkType
{
    IMDB,
    MAL,
    TMDB,
    TVDB,
    Unknown
};

// Playback information
struct MediaInfo
{
    // General
    std::string title;               // Title of the media
    std::string originalTitle;       // Original title (original language)
    MediaType type;                  // Type of media (movie, TV show)
    std::string artPath;             // Path to art on the server (cover image)
    std::string thumbPath;           // Path to the thumbnail on the server
    int year;                        // Year of release
    std::string summary;             // Summary of the media
    std::vector<std::string> genres; // List of genres
    std::string imdbId;              // IMDB ID (if applicable)
    std::string tmdbId;              // TMDB ID (if applicable)
    std::string tvdbId;              // TVDB ID (if applicable)
    std::string malId;               // MyAnimeList ID (if applicable)

    // TV Show specific
    std::string grandparentTitle; // Parent title (tv show name)
    std::string grandparentArt;   // Parent art URL (tv show cover image)
    std::string grandparentKey;   // Parent ID (tv show ID)
    int season;                   // Season number
    int episode;                  // Episode number

    // Music specific
    std::string album;  // Album title
    std::string artist; // Artist name
    int audioBitDepth;
    int audioSamplingRate;

    // Playback info
    std::string username; // Username of the person watching
    std::string videoResolution;
    int bitrate;
    PlaybackState state;  // Current playback state
    double progress;      // Current progress in seconds
    double duration;      // Total duration in seconds
    time_t startTime;     // When the playback started

    // Misc
    std::string sessionKey; // Plex session key
    std::string serverId;   // ID of the server hosting this content
    std::string filename;   // Filename of the media

    MediaInfo() : state(PlaybackState::Stopped),
                  progress(0),
                  duration(0),
                  startTime(0),
                  type(MediaType::Unknown),
                  bitrate(0),
                  audioBitDepth(0),
                  audioSamplingRate(0) {}
};
