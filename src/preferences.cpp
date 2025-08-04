#include "preferences.h"
#include "config.h"
#include "resources.h"
#include <windows.h>

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
        // Episode Format ComboBox
        HWND hEpisodeCombo = GetDlgItem(hDlg, IDC_EDIT_EPISODE_FORMAT);
        SendMessage(hEpisodeCombo, CB_ADDSTRING, 0, (LPARAM) "S{season_num}E{episode_num}");
        SendMessage(hEpisodeCombo, CB_ADDSTRING, 0, (LPARAM) "Season {season_num} Episode {episode_num}");
        SendMessage(hEpisodeCombo, CB_SETCURSEL, (WPARAM)0, 0);

        // Season Format ComboBox
        HWND hSeasonCombo = GetDlgItem(hDlg, IDC_EDIT_SEASON_FORMAT);
        SendMessage(hSeasonCombo, CB_ADDSTRING, 0, (LPARAM) "Season {season_num}");
        SendMessage(hSeasonCombo, CB_ADDSTRING, 0, (LPARAM) "S{season_num}");
        SendMessage(hSeasonCombo, CB_SETCURSEL, (WPARAM)0, 0);

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
            HWND hEpisodeCombo = GetDlgItem(hDlg, IDC_EDIT_EPISODE_FORMAT);
            int episodeIndex = SendMessage(hEpisodeCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hEpisodeCombo, CB_GETLBTEXT, episodeIndex, (LPARAM)buffer);
            config.setEpisodeFormat(buffer);

            HWND hSeasonCombo = GetDlgItem(hDlg, IDC_EDIT_SEASON_FORMAT);
            int seasonIndex = SendMessage(hSeasonCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hSeasonCombo, CB_GETLBTEXT, seasonIndex, (LPARAM)buffer);
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
