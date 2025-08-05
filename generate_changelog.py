import os
import sys
import re
import subprocess

def get_version_from_file():
    with open("version.txt", "r") as f:
        return f.read().strip()

def get_git_log(previous_tag):
    try:
        if previous_tag:
            command = ['git', 'log', f'{previous_tag}..HEAD', '--pretty=format:%s']
        else:
            command = ['git', 'log', '--pretty=format:%s']
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        return result.stdout.splitlines()
    except subprocess.CalledProcessError as e:
        print(f"Error getting git log: {e}", file=sys.stderr)
        return []

def get_tags():
    try:
        command = ['git', 'tag', '--sort=-v:refname']
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        tags = result.stdout.splitlines()
        return tags
    except subprocess.CalledProcessError as e:
        print(f"Error getting git tags: {e}", file=sys.stderr)
        return []

def delete_old_changelogs():
    if os.path.exists("release_notes"):
        for filename in os.listdir("release_notes"):
            if filename.startswith("changelog_") and filename.endswith(".md"):
                os.remove(os.path.join("release_notes", filename))
                print(f"Deleted old changelog: {filename}")

def main():
    version = get_version_from_file()
    changelog_path = os.path.join("release_notes", f"changelog_{version}.md")

    if not os.path.exists("release_notes"):
        os.makedirs("release_notes")
    
    delete_old_changelogs()

    tags = get_tags()
    if len(tags) < 2:
        previous_tag = None
    else:
        previous_tag = tags[1]

    commits = get_git_log(previous_tag)


    features = []
    fixes = []

    for commit in commits:
        if commit.lower().startswith('feat:'):
            features.append(commit[5:].strip())
        elif commit.lower().startswith('fix:'):
            fixes.append(commit[4:].strip())

    with open(changelog_path, "w") as f:
        f.write(f"# Changelog for version {version}\n\n")
        if features:
            f.write("## New Features\n\n")
            for feature in features:
                f.write(f"- {feature}\n")
            f.write("\n")

        if fixes:
            f.write("## Bug Fixes\n\n")
            for fix in fixes:
                f.write(f"- {fix}\n")
            f.write("\n")

    print(f"Changelog generated at {changelog_path}")

if __name__ == "__main__":
    main()
