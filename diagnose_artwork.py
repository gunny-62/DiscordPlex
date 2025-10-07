#!/usr/bin/env python3
"""
Discord Artwork Diagnostic Script
Helps diagnose Discord Rich Presence image display issues after Discord updates.
"""

import requests
import sys
import json
import re
from urllib.parse import urlparse, parse_qs

def test_url_accessibility(url):
    """Test if a URL is accessible and returns valid image data."""
    print(f"Testing URL: {url}")
    
    try:
        response = requests.get(url, timeout=10, allow_redirects=True)
        print(f"  Status Code: {response.status_code}")
        print(f"  Content-Type: {response.headers.get('content-type', 'Unknown')}")
        print(f"  Content-Length: {response.headers.get('content-length', 'Unknown')}")
        
        # Check if it's actually an image
        content_type = response.headers.get('content-type', '').lower()
        if 'image' not in content_type:
            print(f"  ⚠️  WARNING: Content-Type is not an image: {content_type}")
            return False
            
        if response.status_code == 200:
            print(f"  ✅ URL is accessible and returns image data")
            return True
        else:
            print(f"  ❌ URL returned non-200 status code")
            return False
            
    except requests.exceptions.RequestException as e:
        print(f"  ❌ Error accessing URL: {e}")
        return False

def analyze_plex_url(url):
    """Analyze a Plex transcoder URL for potential Discord compatibility issues."""
    print(f"\nAnalyzing Plex URL structure:")
    
    parsed = urlparse(url)
    print(f"  Scheme: {parsed.scheme}")
    print(f"  Host: {parsed.netloc}")
    print(f"  Path: {parsed.path}")
    
    # Check for localhost/local IP
    if parsed.hostname in ['localhost', '127.0.0.1'] or parsed.hostname.startswith('192.168.') or parsed.hostname.startswith('10.'):
        print(f"  ⚠️  WARNING: Using local/private IP - Discord needs external access")
        return False
    
    # Check HTTPS requirement
    if parsed.scheme != 'https':
        print(f"  ⚠️  WARNING: Discord may require HTTPS for images")
        
    # Parse query parameters
    if parsed.query:
        params = parse_qs(parsed.query)
        print(f"  Query Parameters:")
        for key, values in params.items():
            print(f"    {key}: {values[0] if values else 'None'}")
    
    return True

def test_discord_image_requirements(url):
    """Test if URL meets Discord's image requirements."""
    print(f"\nTesting Discord Image Requirements:")
    
    # Test basic accessibility
    accessible = test_url_accessibility(url)
    if not accessible:
        return False
    
    # Test with Discord-like User-Agent
    try:
        headers = {
            'User-Agent': 'Mozilla/5.0 (compatible; Discordbot/2.0; +https://discordapp.com)'
        }
        response = requests.get(url, headers=headers, timeout=10)
        print(f"  Discord Bot Access: {response.status_code}")
        
        if response.status_code != 200:
            print(f"  ❌ Discord bot cannot access the image")
            return False
            
    except Exception as e:
        print(f"  ❌ Error testing Discord bot access: {e}")
        return False
    
    return True

def suggest_fixes(url):
    """Suggest potential fixes for Discord image issues."""
    print(f"\n🔧 Suggested Fixes:")
    
    parsed = urlparse(url)
    
    # Check if it's a Plex transcoder URL
    if '/photo/:/transcode' in url:
        print("1. This is a Plex transcoder URL. Discord may have blocked this pattern.")
        print("   Consider switching to direct image URLs or TMDB fallback.")
        
        # Suggest HTTPS if not present
        if parsed.scheme != 'https':
            https_url = url.replace('http://', 'https://')
            print(f"2. Try HTTPS version: {https_url}")
            
        # Suggest adding Discord-friendly parameters
        if 'format=webp' not in url:
            print("3. Try adding format=webp parameter for better Discord compatibility")
            
    # Check for local URLs
    if parsed.hostname in ['localhost', '127.0.0.1'] or (parsed.hostname and parsed.hostname.startswith('192.168.')):
        print("4. Local URLs won't work with Discord. Ensure Plex is accessible externally.")
        
    print("5. Consider implementing TMDB image fallback for more reliable artwork.")
    print("6. Add cache-busting parameters to bypass Discord's aggressive caching.")

def main():
    print("Discord Artwork Diagnostic Tool")
    print("=" * 50)
    
    if len(sys.argv) < 2:
        print("Usage: python3 diagnose_artwork.py <image_url>")
        print("\nExample Plex URL:")
        print("python3 diagnose_artwork.py 'https://your-plex-server.com/photo/:/transcode?width=256&height=256&url=/library/metadata/123/thumb/456&X-Plex-Token=abc123'")
        sys.exit(1)
    
    url = sys.argv[1]
    
    print(f"Testing URL: {url}\n")
    
    # Run diagnostics
    analyze_plex_url(url)
    discord_compatible = test_discord_image_requirements(url)
    
    if not discord_compatible:
        suggest_fixes(url)
    else:
        print("\n✅ URL appears to be Discord-compatible!")
        print("If images still don't show, try clearing Discord's cache or adding cache-busting parameters.")

if __name__ == "__main__":
    main()