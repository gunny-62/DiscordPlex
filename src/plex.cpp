#include "plex.h"
#include "utils.h"
#include <ctime>
#include <Windows.h>
#include <nlohmann/json.hpp>

Plex::Plex() : m_initialized(false), m_shuttingDown(false) {}

Plex::~Plex() {
    stop();
}

bool Plex::init() {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    m_shuttingDown = false;

    return true;
}

MediaInfo Plex::getCurrentPlayback() {
    MediaInfo info;
    // TODO: Implement actual playback status fetching
    return info;
}

void Plex::stop() {
    if (!m_initialized) {
        return;
    }

    m_shuttingDown = true;
    m_initialized = false;
}

std::string Plex::getClientIdentifier() {
    // Return a fixed client identifier or generate one if not already stored
    static std::string clientId = "presence_for_plex_client_v1";
    return clientId;
}

std::map<std::string, std::string> Plex::getStandardHeaders(const std::string &token) {
    std::map<std::string, std::string> headers;
    headers["Accept"] = "application/json";
    headers["X-Plex-Client-Identifier"] = getClientIdentifier();
    headers["X-Plex-Product"] = "Plex Discord Rich Presence";
    headers["X-Plex-Version"] = "1.0.0";
    
    if (!token.empty()) {
        headers["X-Plex-Token"] = token;
    }
    
    return headers;
}

// API endpoints and constants
namespace
{
    constexpr const char *PLEX_TV_API_URL = "https://plex.tv/api/v2";
    constexpr const char *PLEX_PIN_URL = "https://plex.tv/api/v2/pins";
    constexpr const char *PLEX_AUTH_URL = "https://app.plex.tv/auth#";
    constexpr const char *PLEX_USER_URL = "https://plex.tv/api/v2/user";
    constexpr const char *PLEX_RESOURCES_URL = "https://plex.tv/api/v2/resources?includeHttps=1";
    constexpr const char *JIKAN_API_URL = "https://api.jikan.moe/v4/anime";
    constexpr const char *TMDB_IMAGE_BASE_URL = "https://image.tmdb.org/t/p/w400";
    constexpr const char *SSE_NOTIFICATIONS_ENDPOINT = "/:/eventsource/notifications?filters=playing";
    constexpr const char *SESSION_ENDPOINT = "/status/sessions";

    // Cache timeouts (in seconds)
    constexpr const int TMDB_CACHE_TIMEOUT = 86400;  // 24 hours
    constexpr const int MAL_CACHE_TIMEOUT = 86400;   // 24 hours
    constexpr const int MEDIA_CACHE_TIMEOUT = 3600;  // 1 hour
    constexpr const int SESSION_CACHE_TIMEOUT = 300; // 5 minutes
}

// Cache structures
struct TimedCacheEntry
{
    time_t timestamp;
    bool valid() const { return (std::time(nullptr) - timestamp) < MEDIA_CACHE_TIMEOUT; }
};

struct TMDBCacheEntry : TimedCacheEntry
{
    std::string artPath;
    bool valid() const { return (std::time(nullptr) - timestamp) < TMDB_CACHE_TIMEOUT; }
};

struct MALCacheEntry : TimedCacheEntry
{
    std::string malId;
    bool valid() const { return (std::time(nullptr) - timestamp) < MAL_CACHE_TIMEOUT; }
};

struct MediaCacheEntry : TimedCacheEntry
{
    MediaInfo info;
};

struct SessionUserCacheEntry : TimedCacheEntry
{
    std::string username;
    bool valid() const { return (std::time(nullptr) - timestamp) < SESSION_CACHE_TIMEOUT; }
};

struct ServerUriCacheEntry : TimedCacheEntry
{
    std::string uri;
    bool valid() const { return (std::time(nullptr) - timestamp) < SESSION_CACHE_TIMEOUT; } // Reuse session timeout
};

void Plex::buildArtworkUrl(MediaInfo &info, const std::string &serverUri, const std::string &accessToken)
{
    if (info.thumbPath.empty() || serverUri.empty() || accessToken.empty()) {
        return;
    }

    // Generate cache-busting timestamp
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    // Try to use public URL first
    auto servers = Config::getInstance().getPlexServers();
    for (const auto& [id, server] : servers) {
        if (!server->publicUri.empty()) {
            std::string baseUri = server->publicUri;
            if (baseUri.substr(0, 7) == "http://") {
                baseUri = "https://" + baseUri.substr(7);
            }

            std::string publicUrl = baseUri + "/photo/:/transcode?width=256&height=256&minSize=1&upscale=1&format=webp&url=" 
                                  + utils::urlEncode(info.thumbPath) + "&X-Plex-Token=" + accessToken 
                                  + "&cb=" + std::to_string(timestamp);

            // Test if the public URL works
            HttpClient testClient;
            std::string response;
            if (testClient.get(publicUrl, getStandardHeaders(accessToken), response)) {
                info.artPath = publicUrl;
                LOG_INFO("Plex", "Using public Plex URL for artwork: " + publicUrl);
                return;
            }
            else {
                LOG_INFO("Plex", "Public Plex URL not accessible, trying TMDB");
            }
        }
    }

    // If public URL didn't work, try TMDB
    if (!info.tmdbId.empty()) {
        std::string oldArtPath = info.artPath;  // Save current art path
        fetchTMDBArtwork(info.tmdbId, info, serverUri, accessToken);
        if (info.artPath.find("image.tmdb.org") != std::string::npos) {
            LOG_INFO("Plex", "Using TMDB artwork URL: " + info.artPath);
            return;
        }
        info.artPath = oldArtPath;  // Restore art path if TMDB failed
    }

    // As a last resort, use the provided server URI
    std::string baseUri = serverUri;
    if (baseUri.substr(0, 7) == "http://") {
        baseUri = "https://" + baseUri.substr(7);
        LOG_INFO("Plex", "Converting HTTP to HTTPS for Discord compatibility");
    }
    
    info.artPath = baseUri + "/photo/:/transcode?width=256&height=256&minSize=1&upscale=1&format=webp&url=" 
                 + utils::urlEncode(info.thumbPath) + "&X-Plex-Token=" + accessToken 
                 + "&cb=" + std::to_string(timestamp);
    
    LOG_INFO("Plex", "Using local Plex artwork URL as fallback: " + info.artPath);
}

void Plex::fetchTMDBArtwork(const std::string &tmdbId, MediaInfo &info, const std::string &serverUri, const std::string &plexAccessToken)
{
    LOG_DEBUG("Plex", "Fetching TMDB artwork for ID: " + tmdbId);
    
    LOG_INFO("Plex", "Attempting TMDB artwork fetch (preferred for Discord compatibility)");

    // TMDB API requires an access token - get it from config
    std::string accessToken = Config::getInstance().getTMDBAccessToken();

    if (accessToken.empty())
    {
        LOG_INFO("Plex", "No TMDB access token available, falling back to Plex artwork");
        // Only fall back to Plex transcoder if TMDB is not available
        if (!info.thumbPath.empty() && !serverUri.empty() && !plexAccessToken.empty())
        {
            buildArtworkUrl(info, serverUri, plexAccessToken);
        }
        return;
    }

    // Create HTTP client
    HttpClient client;
    std::string url;

    // Construct proper endpoint URL based on media type
    if (info.type == MediaType::Movie)
    {
        url = "https://api.themoviedb.org/3/movie/" + tmdbId + "/images";
    }
    else
    {
        url = "https://api.themoviedb.org/3/tv/" + tmdbId + "/images";
    }

    // Set up headers with Bearer token for v4 authentication
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + accessToken},
        {"Content-Type", "application/json;charset=utf-8"}};

    // Make the request
    std::string response;
    if (!client.get(url, headers, response))
    {
        LOG_ERROR("Plex", "Failed to fetch TMDB images");
        return;
    }

    try
    {
        auto json = nlohmann::json::parse(response);

        // First try to get a poster
        if (json.contains("posters") && !json["posters"].empty())
        {
            std::string posterPath = json["posters"][0]["file_path"];
            info.artPath = std::string(TMDB_IMAGE_BASE_URL) + posterPath;
            LOG_INFO("Plex", "Found TMDB poster: " + info.artPath);
            return;
        }
        // Fallback to backdrops
        else if (json.contains("backdrops") && !json["backdrops"].empty())
        {
            std::string backdropPath = json["backdrops"][0]["file_path"];
            info.artPath = std::string(TMDB_IMAGE_BASE_URL) + backdropPath;
            LOG_INFO("Plex", "Found TMDB backdrop: " + info.artPath);
            return;
        }
        else
        {
            LOG_INFO("Plex", "No TMDB images found, falling back to Plex artwork");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Plex", "Error parsing TMDB response: " + std::string(e.what()));
    }
}