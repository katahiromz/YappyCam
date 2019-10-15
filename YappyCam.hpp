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

#define MIN_FPS 0.1
#define MAX_FPS 7.0
#define DEFAULT_FPS 4.0

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
    INT m_nWindow1X;
    INT m_nWindow1Y;
    INT m_nWindow2X;
    INT m_nWindow2Y;
    INT m_nWindow3X;
    INT m_nWindow3Y;
    INT m_nWindow1CX;
    INT m_nWindow1CY;
    INT m_nWindow2CX;
    INT m_nWindow2CY;
    INT m_nWindow3CX;
    INT m_nWindow3CY;
    INT m_nSoundDlgX;
    INT m_nSoundDlgY;
    INT m_nPicDlgX;
    INT m_nPicDlgY;
    UINT m_nFPSx100;
    BOOL m_bDrawCursor;
    BOOL m_bNoSound;
    INT m_nMonitorID;
    INT m_nCameraID;
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
    void update(HWND hwnd);
    void fix_size(HWND hwnd);
    void fix_size0(HWND hwnd);

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
void DoStartStopTimers(HWND hwnd, BOOL bStart);

#endif  // ndef YAPPYCAM_HPP_
