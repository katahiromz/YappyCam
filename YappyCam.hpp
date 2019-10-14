#ifndef YAPPYCAM_HPP_
#define YAPPYCAM_HPP_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <string>
#include <opencv2/opencv.hpp>
#include "Sound.hpp"
#include "resource.h"
#include "MRegKey.hpp"

enum DisplayMode
{
    DM_CAPFRAME,
    DM_BITMAP
};

enum PictureType
{
    PT_BLACK,
    PT_WHITE,
    PT_SCREENCAP,
    PT_VIDEOCAP,
    PT_STATUSTEXT
};

struct Settings
{
    DisplayMode m_nDisplayMode;
    PictureType m_nPictureType;
    INT m_nWidth;
    INT m_nHeight;
    INT m_nMovieID;
    INT m_iSoundDev;
    INT m_iWaveFormat;
    INT m_xCap;
    INT m_yCap;
    INT m_cxCap;
    INT m_cyCap;
    UINT m_nFPSx100;
    std::wstring m_strDir;
    std::wstring m_strMovieDir;
    std::wstring m_strImageFileName;
    std::wstring m_strMovieFileName;
    std::wstring m_strStatusText;

    Settings()
    {
        init();
    }

    void init();
    bool load(HWND hwnd);
    bool save(HWND hwnd) const;
    bool create_dirs() const;

    DisplayMode GetDisplayMode() const
    {
        return m_nDisplayMode;
    }
    PictureType GetPictureType() const
    {
        return m_nPictureType;
    }
    BOOL SetPictureType(HWND hwnd, PictureType type);

protected:
    void SetDisplayMode(DisplayMode mode)
    {
        m_nDisplayMode = mode;
    }
};
extern Settings g_settings;

extern Sound m_sound;
extern HWND g_hMainWnd;
extern HWND g_hwndSoundInput;
extern HWND g_hwndPictureInput;

typedef std::vector<CComPtr<IMMDevice> > sound_devices_t;
extern sound_devices_t m_sound_devices;
extern std::vector<WAVE_FORMAT_INFO> m_wave_formats;

void ErrorBoxDx(HWND hwnd, LPCTSTR pszText);
LPTSTR LoadStringDx(INT nID);
LPSTR ansi_from_wide(LPCWSTR pszWide);

BOOL DoSoundInputDialogBox(HWND hwndParent);
BOOL DoPictureInputDialogBox(HWND hwndParent);

#endif  // ndef YAPPYCAM_HPP_
