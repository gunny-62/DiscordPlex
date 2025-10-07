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
                LOG_INFO("Plex", "Public Plex URL not accessible, falling back to TMDB");
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