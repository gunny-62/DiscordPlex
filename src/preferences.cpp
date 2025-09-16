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
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_CLIENT, config.getShowClient() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_MUSIC, config.getShowMusic() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_GATEKEEP_MUSIC, config.getGatekeepMusic() ? BST_CHECKED : BST_UNCHECKED);
        SetDlgItemText(hDlg, IDC_EDIT_GATEKEEP_MUSIC_TITLE, config.getGatekeepMusicTitle().c_str());
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GATEKEEP_MUSIC_TITLE), config.getGatekeepMusic());
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_FLAC_AS_CD, config.getShowFlacAsCD() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_MOVIES, config.getShowMovies() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_MOVIE_BITRATE, config.getShowMovieBitrate() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_MOVIE_QUALITY, config.getShowMovieQuality() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_TVSHOWS, config.getShowTVShows() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_TVSHOW_BITRATE, config.getShowTVShowBitrate() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SHOW_TVSHOW_QUALITY, config.getShowTVShowQuality() ? BST_CHECKED : BST_UNCHECKED);
        // Episode Format ComboBox
        HWND hEpisodeCombo = GetDlgItem(hDlg, IDC_EDIT_EPISODE_FORMAT);
        SendMessage(hEpisodeCombo, CB_ADDSTRING, 0, (LPARAM) "E{episode_num}");
        SendMessage(hEpisodeCombo, CB_ADDSTRING, 0, (LPARAM) "Episode {episode_num}");
        SendMessage(hEpisodeCombo, CB_SELECTSTRING, -1, (LPARAM)config.getEpisodeFormat().c_str());

        // Season Format ComboBox
        HWND hSeasonCombo = GetDlgItem(hDlg, IDC_EDIT_SEASON_FORMAT);
        SendMessage(hSeasonCombo, CB_ADDSTRING, 0, (LPARAM) "Season {season_num}");
        SendMessage(hSeasonCombo, CB_ADDSTRING, 0, (LPARAM) "S{season_num}");
        SendMessage(hSeasonCombo, CB_SELECTSTRING, -1, (LPARAM)config.getSeasonFormat().c_str());

        HWND hMusicCombo = GetDlgItem(hDlg, IDC_COMBO_MUSIC_FORMAT);
        SendMessage(hMusicCombo, CB_ADDSTRING, 0, (LPARAM) "{title} - {artist} - {album}");
        SendMessage(hMusicCombo, CB_ADDSTRING, 0, (LPARAM) "{title} - {artist}");
        SendMessage(hMusicCombo, CB_ADDSTRING, 0, (LPARAM) "{title}");
        SendMessage(hMusicCombo, CB_SELECTSTRING, -1, (LPARAM)config.getMusicFormat().c_str());

        HWND hTVCombo = GetDlgItem(hDlg, IDC_COMBO_TV_FORMAT);
        SendMessage(hTVCombo, CB_ADDSTRING, 0, (LPARAM) "{show_title} - {season_episode} - {episode_title}");
        SendMessage(hTVCombo, CB_ADDSTRING, 0, (LPARAM) "{episode_title}");
        SendMessage(hTVCombo, CB_ADDSTRING, 0, (LPARAM) "{season} - {episode_title}");
        SendMessage(hTVCombo, CB_ADDSTRING, 0, (LPARAM) "{season} - {episode_number}");
        SendMessage(hTVCombo, CB_ADDSTRING, 0, (LPARAM) "{season} {episode_number} - {episode_title}");
        SendMessage(hTVCombo, CB_SELECTSTRING, -1, (LPARAM)config.getTVShowFormat().c_str());

        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_CHECK_GATEKEEP_MUSIC:
        {
            BOOL isChecked = IsDlgButtonChecked(hDlg, IDC_CHECK_GATEKEEP_MUSIC);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GATEKEEP_MUSIC_TITLE), isChecked);
            break;
        }
        case IDC_BUTTON_SAVE:
        {
            Config &config = Config::getInstance();
            config.setShowMusic(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_MUSIC) == BST_CHECKED);
            config.setGatekeepMusic(IsDlgButtonChecked(hDlg, IDC_CHECK_GATEKEEP_MUSIC) == BST_CHECKED);
            char buffer[256];
            GetDlgItemText(hDlg, IDC_EDIT_GATEKEEP_MUSIC_TITLE, buffer, 256);
            config.setGatekeepMusicTitle(buffer);
            config.setShowFlacAsCD(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_FLAC_AS_CD) == BST_CHECKED);
            config.setShowMovies(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_MOVIES) == BST_CHECKED);
            config.setShowMovieBitrate(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_MOVIE_BITRATE) == BST_CHECKED);
            config.setShowMovieQuality(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_MOVIE_QUALITY) == BST_CHECKED);
            config.setShowTVShows(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_TVSHOWS) == BST_CHECKED);
            config.setShowClient(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_CLIENT) == BST_CHECKED);
            config.setShowTVShowBitrate(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_TVSHOW_BITRATE) == BST_CHECKED);
            config.setShowTVShowQuality(IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_TVSHOW_QUALITY) == BST_CHECKED);

            HWND hEpisodeCombo = GetDlgItem(hDlg, IDC_EDIT_EPISODE_FORMAT);
            int episodeIndex = SendMessage(hEpisodeCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hEpisodeCombo, CB_GETLBTEXT, episodeIndex, (LPARAM)buffer);
            config.setEpisodeFormat(buffer);

            HWND hSeasonCombo = GetDlgItem(hDlg, IDC_EDIT_SEASON_FORMAT);
            int seasonIndex = SendMessage(hSeasonCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hSeasonCombo, CB_GETLBTEXT, seasonIndex, (LPARAM)buffer);
            config.setSeasonFormat(buffer);

            HWND hMusicCombo = GetDlgItem(hDlg, IDC_COMBO_MUSIC_FORMAT);
            int musicIndex = SendMessage(hMusicCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hMusicCombo, CB_GETLBTEXT, musicIndex, (LPARAM)buffer);
            config.setMusicFormat(buffer);

            HWND hTVCombo = GetDlgItem(hDlg, IDC_COMBO_TV_FORMAT);
            int tvIndex = SendMessage(hTVCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hTVCombo, CB_GETLBTEXT, tvIndex, (LPARAM)buffer);
            config.setTVShowFormat(buffer);

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
