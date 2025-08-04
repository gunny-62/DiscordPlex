import os
import sys
import re

def get_version_from_file():
    with open("version.txt", "r") as f:
        return f.read().strip()

def main():
    version = get_version_from_file()
    changelog_path = os.path.join("release_notes", f"changelog_{version}.md")

    if not os.path.exists("release_notes"):
        os.makedirs("release_notes")

    with open(changelog_path, "w") as f:
        f.write(f"# Changelog for version {version}\n\n")
        f.write("## New Features\n\n")
        f.write("- Restructured preferences dialog for improved clarity.\n")
        f.write("- Added a 'Customize Formatting' section to the preferences.\n")
        f.write("- Added a dropdown menu for music display formats.\n")
        f.write("- The 'General' section has been renamed to 'Tags'.\n")
        f.write("- The 'Show FLAC as CD' option has been moved to the 'Tags' section.\n\n")
        f.write("## Bug Fixes\n\n")
        f.write("- Fixed a build failure due to incorrect syntax.\n")
        f.write("- Resolved an issue where the FLAC quality was not displayed.\n")
        f.write("- Corrected the width of dropdowns in the preferences dialog.\n")

    print(f"Changelog generated at {changelog_path}")

if __name__ == "__main__":
    main()
