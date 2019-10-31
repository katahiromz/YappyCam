#ifndef YAPPYCAM_HPP_
#define YAPPYCAM_HPP_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <string>
#include <mutex>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "sound.hpp"
#include "resource.h"
#include "mregkey.hpp"
#include "ring.hpp"
#include "misc.hpp"
#include "PluginFramework.h"

#define MIN_FPS 0.1
#define MAX_FPS 20.0
#define DEFAULT_FPS 6
#define TIME_PERIOD 1

enum DisplayMode
{
    DM_CAPFRAME,
    DM_BITMAP,
    DM_TEXT,
    DM_IMAGEFILE
};

enum PictureType
{
    PT_BLACK,
    PT_WHITE,
    PT_SCREENCAP,
    PT_VIDEOCAP,
    PT_FINALIZING,
    PT_IMAGEFILE
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
    INT m_nSaveToDlgX;
    INT m_nSaveToDlgY;
    INT m_nHotKeysDlgX;
    INT m_nHotKeysDlgY;
    INT m_nPluginsDlgX;
    INT m_nPluginsDlgY;
    UINT m_nFPSx100;
    BOOL m_bDrawCursor;
    BOOL m_bNoSound;
    INT m_nMonitorID;
    INT m_nCameraID;
    BOOL m_bFollowChange;
    INT m_nBrightness;
    INT m_nContrast;
    DWORD m_dwFOURCC;
    UINT m_nHotKey[7];
    std::wstring m_strDir;
    std::wstring m_strMovieDir;
    std::wstring m_strImageFileName;
    std::wstring m_strMovieFileName;
    std::wstring m_strMovieTempFileName;
    std::wstring m_strSoundFileName;
    std::wstring m_strSoundTempFileName;
    std::wstring m_strShotFileName;
    std::wstring m_strInputFileName;
    std::wstring m_strStatusText;
    std::vector<std::wstring> m_strvecPluginNames;
    std::vector<BOOL> m_bvecPluginEnabled;

    Settings()
    {
        init();
    }

    void init();
    bool load(HWND hwnd);
    bool save(HWND hwnd) const;
    bool create_dirs() const;
    void change_dirs(const WCHAR *dir);
    void update(HWND hwnd);
    void update(HWND hwnd, PictureType type);
    void fix_size(HWND hwnd);
    void fix_size0(HWND hwnd);
    void follow_display_change(HWND hwnd);

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
    void recreate_bitmap(HWND hwnd);
};
extern Settings g_settings;

extern Sound g_sound;

extern HWND g_hMainWnd;
extern HWND g_hwndSoundInput;
extern HWND g_hwndPictureInput;
extern HWND g_hwndSaveTo;
extern HWND g_hwndHotKeys;
extern HWND g_hwndPlugins;

typedef std::vector<CComPtr<IMMDevice> > sound_devices_t;
extern sound_devices_t g_sound_devices;
extern std::vector<WAVE_FORMAT_INFO> m_wave_formats;

// dialogs
BOOL DoSoundInputDialogBox(HWND hwndParent);
BOOL DoPictureInputDialogBox(HWND hwndParent);
BOOL DoSaveToDialogBox(HWND hwndParent);
BOOL DoHotKeysDialogBox(HWND hwndParent);
BOOL DoPluginsDialogBox(HWND hwndParent);

void DoStartStopTimers(HWND hwnd, BOOL bStart);

#define HOTKEY_0_ID  0x1000
#define HOTKEY_1_ID  0x1001
#define HOTKEY_2_ID  0x1002
#define HOTKEY_3_ID  0x1003
#define HOTKEY_4_ID  0x1004
#define HOTKEY_5_ID  0x1005
#define HOTKEY_6_ID  0x1006
BOOL DoSetupHotkeys(HWND hwnd, BOOL bSetup);

typedef std::unordered_map<DWORD, DWORD> RESO_MAP;
DWORD DoMultiResoDialogBox(HWND hwndParent, const RESO_MAP& map);


inline void dprint(const char *fmt, ...)
{
    static char buf[1024];
    va_list va;
    va_start(va, fmt);
    StringCbVPrintfA(buf, sizeof(buf), fmt, va);
    StringCbCatA(buf, sizeof(buf), "\n");
#ifdef __GNUC__
    fputs(buf, stdout);
    fflush(stdout);
#else
    OutputDebugStringA(buf);
#endif
    va_end(va);
}

#undef DPRINT

#ifdef NDEBUG
    #define DPRINT(...) /* empty */
#else
    #define DPRINT(...) dprint(__VA_ARGS__)
#endif

class mutex_debug : public std::mutex
{
    const char *m_name;
public:
    mutex_debug(const char *name) noexcept
        : std::mutex()
        , m_name(name)
    {
    }
    void lock(int line)
    {
        DPRINT("lock: %s, Line %d", m_name, line);
        std::mutex::lock();
    }
    void unlock(int line)
    {
        DPRINT("unlock: %s, Line %d", m_name, line);
        std::mutex::unlock();
    }
};

#endif  // ndef YAPPYCAM_HPP_
