# Discord Artwork Fix

## Problem
After recent Discord updates, Rich Presence images are showing as question marks instead of the actual media artwork.

## Root Cause
Discord has implemented stricter requirements for Rich Presence images:
1. **Aggressive caching** - Discord caches images indefinitely
2. **HTTPS requirement** - HTTP URLs may be blocked
3. **URL validation** - Certain URL patterns may be restricted

## Solution Implemented

### Changes Made
1. **Cache-busting timestamps** - Added `&cb=<timestamp>` to force Discord to fetch fresh images
2. **HTTPS enforcement** - Automatically converts HTTP Plex server URLs to HTTPS
3. **Discord-friendly parameters** - Added `format=webp` and `upscale=1` for better compatibility
4. **Proper URL encoding** - Ensures thumb paths are properly encoded

### Files Modified
- `src/plex.cpp` - Updated `buildArtworkUrl()` and `fetchTMDBArtwork()` functions
- `WARP.md` - Added troubleshooting section for Discord artwork issues

## Testing the Fix

### 1. Build and Test
```bash
# Build the application
./build.sh

# Run the executable
./build/PresenceForPlex
```

### 2. Monitor Logs
Look for log messages like:
- `"Converting HTTP to HTTPS for Discord compatibility"`
- `"Built Discord-compatible artwork URL: ..."`

### 3. Test Artwork URLs
Use the diagnostic script to test artwork URLs:
```bash
python3 diagnose_artwork.py "your-generated-artwork-url"
```

### 4. Verify in Discord
1. Start playing media in Plex
2. Check your Discord profile shows the Rich Presence
3. Verify artwork displays correctly (not question marks)

## Troubleshooting

### If Images Still Don't Show
1. **Clear Discord cache**: Close Discord completely and restart
2. **Check Plex external access**: Ensure your Plex server is accessible from outside your network
3. **Verify HTTPS**: Make sure your Plex server supports HTTPS
4. **Test manually**: Copy the artwork URL and paste it in a browser to verify it loads

### Common Issues
- **Local Plex servers**: If your Plex server only uses local IPs (192.168.x.x), Discord won't be able to access the images
- **Self-signed certificates**: Discord may reject HTTPS URLs with invalid certificates
- **Firewall blocking**: Ensure Discord's servers can reach your Plex server

## Commit Message
```
fix: Update artwork URLs for Discord Rich Presence compatibility

- Add cache-busting timestamps to bypass Discord's aggressive caching
- Force HTTPS for Discord compatibility 
- Add Discord-friendly parameters (format=webp, upscale=1)
- Properly URL encode thumb paths
- Update TMDB artwork generation to use same improvements

Fixes issue where Discord Rich Presence shows question marks
instead of media artwork after recent Discord updates.
```

## Release Notes
When releasing this fix, include:

```markdown
### Bug Fixes
- **Discord Rich Presence**: Fixed artwork not displaying after Discord updates
  - Images now use cache-busting to bypass Discord's aggressive caching
  - Automatic HTTPS conversion for better Discord compatibility
  - Added Discord-friendly image parameters
```