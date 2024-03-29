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

enum ASPECT_MODE
{
    ASPECT_IGNORE = 0,
    ASPECT_CUT,
    ASPECT_EXTEND_BLACK,
    ASPECT_EXTEND_WHITE
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
    INT m_nWindowX;
    INT m_nWindowY;
    INT m_nWindowCX;
    INT m_nWindowCY;
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
    INT m_nFacesDlgX;
    INT m_nFacesDlgY;
    INT m_nArea;
    BOOL m_bUseFaces;
    BOOL m_bUsePass1;
    BOOL m_bUsePass2;
    BOOL m_bUseBroadcast;
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
    ASPECT_MODE m_nAspectMode;
    std::wstring m_strDir;
    std::wstring m_strMovieDir;
    std::wstring m_strImageFileName;
    std::wstring m_strMovieFileName;
    std::wstring m_strMovieTempFileName;
    std::wstring m_strSoundFileName;
    std::wstring m_strSoundTempFileName;
    std::wstring m_strShotFileName;
    std::wstring m_strInputFileName;
    std::string m_strInputFileNameA;
    std::wstring m_strCascadeClassifierW;
    std::string m_strCascadeClassifierA;
    std::wstring m_strStatusText;
    std::vector<std::wstring> m_strvecPluginNames;
    std::vector<BOOL> m_bvecPluginEnabled;
    std::vector<DWORD> m_dwvecPluginState;

    Settings()
    {
    }

    void init(HWND hwnd);
    bool load(HWND hwnd);
    bool save(HWND hwnd) const;
    bool create_dirs() const;
    void change_dirs(const WCHAR *dir);
    void update(HWND hwnd);
    void update(HWND hwnd, PictureType type);
    void fix_size(HWND hwnd, INT nArea);
    void follow_display_change(HWND hwnd);

    DisplayMode GetDisplayMode() const
    {
        return m_nDisplayMode;
    }
    PictureType GetPictureType() const
    {
        return m_nPictureType;
    }
    BOOL SetPictureType(HWND hwnd, PictureType type, INT nArea = 0);
    BOOL SetPictureType(HWND hwnd)
    {
        return SetPictureType(hwnd, m_nPictureType, m_nArea);
    }

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
extern HWND g_hwndFaces;

typedef std::vector<CComPtr<IMMDevice> > sound_devices_t;
extern sound_devices_t g_sound_devices;
extern std::vector<WAVE_FORMAT_INFO> m_wave_formats;

// facial recognition
extern cv::CascadeClassifier g_cascade;
extern std::vector<cv::Rect> g_faces;

// plugins
extern std::vector<PLUGIN> s_plugins;

// dialogs
BOOL DoSoundInputDialogBox(HWND hwndParent);
BOOL DoPictureInputDialogBox(HWND hwndParent);
BOOL DoSaveToDialogBox(HWND hwndParent);
BOOL DoHotKeysDialogBox(HWND hwndParent);
BOOL DoPluginsDialogBox(HWND hwndParent);
BOOL DoFacesDialogBox(HWND hwndParent);

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

extern mutex_debug g_face_lock;

#endif  // ndef YAPPYCAM_HPP_
