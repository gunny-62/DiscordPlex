#include "preferences.h"
#include "config.h"
#include "resources.h"
#include <windows.h>

// Control IDs
#define IDC_GROUP_MUSIC 101
#define IDC_CHECK_SHOW_MUSIC 102
#define IDC_GROUP_MOVIES 103
#define IDC_CHECK_SHOW_MOVIES 104
#define IDC_GROUP_TVSHOWS 105
#define IDC_CHECK_SHOW_TVSHOWS 106
#define IDC_GROUP_GENERAL 107
#define IDC_CHECK_SHOW_BITRATE 108
#define IDC_CHECK_SHOW_QUALITY 109
#define IDC_EDIT_EPISODE_FORMAT 110
#define IDC_EDIT_SEASON_FORMAT 111
#define IDC_BUTTON_SAVE 112
#define IDC_BUTTON_CANCEL 113

// Global variables for this file
static HWND g_hDlg = NULL;

// Forward declarations
INT_PTR CALLBACK PreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

Preferences::Preferences()
{
}

Preferences::~Preferences()
{
}

void Preferences::show()
{
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PREFERENCES), NULL, PreferencesDlgProc);
}

INT_PTR CALLBACK PreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        g_hDlg = hDlg;

        // Load current settings
        Config &config = Config::getInstance();
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_MUSIC, config.getShowMusic() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_MOVIES, config.getShowMovies() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_TVSHOWS, config.getShowTVShows() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_BITRATE, config.getShowBitrate() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_QUALITY, config.getShowQuality() ? BST_CHECKED : BST_UNCHECKED);
        SetDlgItemText(hDlg, IDC_EDIT_EPISODE_FORMAT, config.getEpisodeFormat().c_str());
        SetDlgItemText(hDlg, IDC_EDIT_SEASON_FORMAT, config.getSeasonFormat().c_str());

        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_SAVE:
        {
            Config &config = Config::getInstance();
            config.setShowMusic(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_MUSIC) == BST_CHECKED);
            config.setShowMovies(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_MOVIES) == BST_CHECKED);
            config.setShowTVShows(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_TVSHOWS) == BST_CHECKED);
            config.setShowBitrate(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_BITRATE) == BST_CHECKED);
            config.setShowQuality(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_QUALITY) == BST_CHECKED);

            char buffer[256];
            GetDlgItemText(hDlg, IDC_EDIT_EPISODE_FORMAT, buffer, 256);
            config.setEpisodeFormat(buffer);
            GetDlgItemText(hDlg, IDC_EDIT_SEASON_FORMAT, buffer, 256);
            config.setSeasonFormat(buffer);

            config.saveConfig();
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_CANCEL:
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        }
        break;
    }
    }
    return (INT_PTR)FALSE;
}
