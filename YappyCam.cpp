// KeepAspect.cpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#include "YappyCam.hpp"

// timer IDs
#define SOUND_TIMER_ID  999
#define CAP_TIMER_ID    888

// for layout of the main window
static INT s_button_width = 0;
static INT s_progress_width = 0;

// for button images
static HBITMAP s_hbmRec = NULL;
static HBITMAP s_hbmPause = NULL;
static HBITMAP s_hbmStop = NULL;
static HBITMAP s_hbmDots = NULL;

// the settings
Settings g_settings;

HINSTANCE g_hInst = NULL;

// the main window handle
HWND g_hMainWnd = NULL;

// the icon
HICON g_hMainIcon = NULL;
HICON g_hMainIconSmall = NULL;

// for sound recording
typedef std::vector<CComPtr<IMMDevice> > sound_devices_t;
sound_devices_t m_sound_devices;
Sound m_sound;
std::vector<WAVE_FORMAT_INFO> m_wave_formats;

// for camera capture
cv::VideoCapture g_camera;

// for video writing
cv::VideoWriter g_writer;
static BOOL s_bWriting = FALSE;
static BOOL s_bWatching = FALSE;

CRITICAL_SECTION g_lock;    // mutex

HDC g_hdcScreen = NULL;     // screen DC

// frame info
static cv::Mat s_frame;
static INT s_nFrames = 0;
HBITMAP g_hbm = NULL;
BITMAPINFO g_bi =
{
    {
        sizeof(BITMAPINFOHEADER), 0, 0, 1
    }
};

void ErrorBoxDx(HWND hwnd, LPCTSTR pszText)
{
    MessageBox(hwnd, pszText, NULL, MB_ICONERROR);
}

LPTSTR LoadStringDx(INT nID)
{
    static UINT s_index = 0;
    const UINT cchBuffMax = 1024;
    static TCHAR s_sz[2][cchBuffMax];

    TCHAR *pszBuff = s_sz[s_index];
    s_index = (s_index + 1) % ARRAYSIZE(s_sz);
    pszBuff[0] = 0;
    if (!::LoadString(NULL, nID, pszBuff, cchBuffMax))
        assert(0);
    return pszBuff;
}

LPSTR ansi_from_wide(LPCWSTR pszWide)
{
    static char s_buf[256];
    WideCharToMultiByte(CP_ACP, 0, pszWide, -1, s_buf, ARRAYSIZE(s_buf), NULL, NULL);
    return s_buf;
}

void Settings::init()
{
    m_nDisplayMode = DM_BITMAP;
    m_nPictureType = PT_SCREENCAP;
    m_nWidth = 640;
    m_nHeight = 480;
    m_nMovieID = 0;

    m_iSoundDev = 0;
    m_iWaveFormat = 0;

    m_xCap = 0;
    m_yCap = 0;
    m_cxCap = GetSystemMetrics(SM_CXSCREEN);
    m_cyCap = GetSystemMetrics(SM_CYSCREEN);

    m_nWindow1X = m_nWindow1Y = CW_USEDEFAULT;
    m_nWindow2X = m_nWindow2Y = CW_USEDEFAULT;
    m_nWindow3X = m_nWindow3Y = CW_USEDEFAULT;

    m_nWindow1CX = 250;
    m_nWindow1CY = 160;
    m_nWindow2CX = 250;
    m_nWindow2CY = 160;
    m_nWindow3CX = 250;
    m_nWindow3CY = 160;

    m_nSoundDlgX = m_nSoundDlgY = CW_USEDEFAULT;
    m_nPicDlgX = m_nPicDlgY = CW_USEDEFAULT;
    m_nSaveToDlgX = m_nSaveToDlgY = CW_USEDEFAULT;

    m_nFPSx100 = UINT(DEFAULT_FPS * 100);
    m_bDrawCursor = TRUE;
    m_bNoSound = FALSE;
    m_nMonitorID = 0;
    m_nCameraID = 0;
    m_nBrightness = 0;
    m_nContrast = 100;
    m_dwFOURCC = 0x7634706d; // mp4v

    TCHAR szPath[MAX_PATH];

    SHGetSpecialFolderPath(NULL, szPath, CSIDL_MYVIDEO, TRUE);
    PathAppend(szPath, TEXT("YappyCam"));
    m_strDir = szPath;

    PathAppend(szPath, TEXT("movie-%03u"));
    m_strMovieDir = szPath;

    m_strImageFileName = TEXT("img-%04u.png");

    SHGetSpecialFolderPath(NULL, szPath, CSIDL_MYVIDEO, FALSE);
    PathAppend(szPath, TEXT("YappyCam"));
    PathAppend(szPath, TEXT("movie-%03u.avi"));
    m_strMovieFileName = szPath;

    SHGetSpecialFolderPath(NULL, szPath, CSIDL_MYVIDEO, FALSE);
    PathAppend(szPath, TEXT("YappyCam"));
    PathAppend(szPath, TEXT("movie-%03u-tmp.avi"));
    m_strMovieTempFileName = szPath;

    m_strSoundFileName = TEXT("Sound.wav");

    m_strStatusText = TEXT("No image");
}

bool Settings::load(HWND hwnd)
{
    init();

    MRegKey author_key(HKEY_CURRENT_USER, L"Software\\Katayama Hirofumi MZ", FALSE);
    if (!author_key)
        return false;

    MRegKey app_key(author_key, L"YappyCam", FALSE);
    if (!app_key)
        return false;

    PictureType type;
    app_key.QueryDword(L"PicType", (DWORD&)type);

    app_key.QueryDword(L"Width", (DWORD&)m_nWidth);
    app_key.QueryDword(L"Height", (DWORD&)m_nHeight);
    app_key.QueryDword(L"MovieID", (DWORD&)m_nMovieID);
    app_key.QueryDword(L"SoundDevice", (DWORD&)m_iSoundDev);
    app_key.QueryDword(L"WaveFormat", (DWORD&)m_iWaveFormat);

    app_key.QueryDword(L"xCap", (DWORD&)m_xCap);
    app_key.QueryDword(L"yCap", (DWORD&)m_yCap);
    app_key.QueryDword(L"cxCap", (DWORD&)m_cxCap);
    app_key.QueryDword(L"cyCap", (DWORD&)m_cyCap);

    app_key.QueryDword(L"Window1X", (DWORD&)m_nWindow1X);
    app_key.QueryDword(L"Window1Y", (DWORD&)m_nWindow1Y);
    app_key.QueryDword(L"Window2X", (DWORD&)m_nWindow2X);
    app_key.QueryDword(L"Window2Y", (DWORD&)m_nWindow2Y);
    app_key.QueryDword(L"Window3X", (DWORD&)m_nWindow3X);
    app_key.QueryDword(L"Window3Y", (DWORD&)m_nWindow3Y);

    app_key.QueryDword(L"Window1CX", (DWORD&)m_nWindow1CX);
    app_key.QueryDword(L"Window1CY", (DWORD&)m_nWindow1CY);
    app_key.QueryDword(L"Window2CX", (DWORD&)m_nWindow2CX);
    app_key.QueryDword(L"Window2CY", (DWORD&)m_nWindow2CY);
    app_key.QueryDword(L"Window3CX", (DWORD&)m_nWindow3CX);
    app_key.QueryDword(L"Window3CY", (DWORD&)m_nWindow3CY);

    app_key.QueryDword(L"SoundDlgX", (DWORD&)m_nSoundDlgX);
    app_key.QueryDword(L"SoundDlgY", (DWORD&)m_nSoundDlgY);
    app_key.QueryDword(L"PicDlgX", (DWORD&)m_nPicDlgX);
    app_key.QueryDword(L"PicDlgY", (DWORD&)m_nPicDlgY);
    app_key.QueryDword(L"SaveToDlgX", (DWORD&)m_nSaveToDlgX);
    app_key.QueryDword(L"SaveToDlgY", (DWORD&)m_nSaveToDlgY);

    app_key.QueryDword(L"FPSx100", (DWORD&)m_nFPSx100);
    app_key.QueryDword(L"DrawCursor", (DWORD&)m_bDrawCursor);
    app_key.QueryDword(L"NoSound", (DWORD&)m_bNoSound);
    app_key.QueryDword(L"MonitorID", (DWORD&)m_nMonitorID);
    app_key.QueryDword(L"CameraID", (DWORD&)m_nCameraID);

    app_key.QueryDword(L"Brightness", (DWORD&)m_nBrightness);
    app_key.QueryDword(L"Contrast", (DWORD&)m_nContrast);
    app_key.QueryDword(L"FOURCC", (DWORD&)m_dwFOURCC);

    WCHAR szText[MAX_PATH];

    if (ERROR_SUCCESS == app_key.QuerySz(L"Dir", szText, ARRAYSIZE(szText)))
    {
        m_strDir = szText;
    }
    if (ERROR_SUCCESS == app_key.QuerySz(L"MovieDir", szText, ARRAYSIZE(szText)))
    {
        m_strMovieDir = szText;
    }
    if (ERROR_SUCCESS == app_key.QuerySz(L"ImageFileName", szText, ARRAYSIZE(szText)))
    {
        m_strImageFileName = szText;
    }
    if (ERROR_SUCCESS == app_key.QuerySz(L"MovieFileName", szText, ARRAYSIZE(szText)))
    {
        m_strMovieFileName = szText;
    }
    if (ERROR_SUCCESS == app_key.QuerySz(L"MovieTempFileName", szText, ARRAYSIZE(szText)))
    {
        m_strMovieTempFileName = szText;
    }
    if (ERROR_SUCCESS == app_key.QuerySz(L"SoundFileName", szText, ARRAYSIZE(szText)))
    {
        m_strSoundFileName = szText;
    }

    if (type == PT_FINALIZING)
        type = PT_SCREENCAP;

    SetPictureType(hwnd, type);

    TCHAR szPath[MAX_PATH];
    for (INT i = m_nMovieID; i <= 999; ++i)
    {
        StringCbPrintf(szPath, sizeof(szPath), m_strMovieDir.c_str(), i);
        if (!PathIsDirectory(szPath))
        {
            m_nMovieID = i;
            break;
        }
    }

    return true;
}

bool Settings::save(HWND hwnd) const
{
    MRegKey author_key(HKEY_CURRENT_USER, L"Software\\Katayama Hirofumi MZ", TRUE);
    if (!author_key)
        return false;

    MRegKey app_key(author_key, L"YappyCam", TRUE);
    if (!app_key)
        return false;

    PictureType type = GetPictureType();
    app_key.SetDword(L"PicType", type);

    app_key.SetDword(L"Width", m_nWidth);
    app_key.SetDword(L"Height", m_nHeight);
    app_key.SetDword(L"MovieID", m_nMovieID);
    app_key.SetDword(L"SoundDevice", m_iSoundDev);
    app_key.SetDword(L"WaveFormat", m_iWaveFormat);

    app_key.SetDword(L"xCap", m_xCap);
    app_key.SetDword(L"yCap", m_yCap);
    app_key.SetDword(L"cxCap", m_cxCap);
    app_key.SetDword(L"cyCap", m_cyCap);

    app_key.SetDword(L"Window1X", m_nWindow1X);
    app_key.SetDword(L"Window1Y", m_nWindow1Y);
    app_key.SetDword(L"Window2X", m_nWindow2X);
    app_key.SetDword(L"Window2Y", m_nWindow2Y);
    app_key.SetDword(L"Window3X", m_nWindow3X);
    app_key.SetDword(L"Window3Y", m_nWindow3Y);

    app_key.SetDword(L"Window1CX", m_nWindow1CX);
    app_key.SetDword(L"Window1CY", m_nWindow1CY);
    app_key.SetDword(L"Window2CX", m_nWindow2CX);
    app_key.SetDword(L"Window2CY", m_nWindow2CY);
    app_key.SetDword(L"Window3CX", m_nWindow3CX);
    app_key.SetDword(L"Window3CY", m_nWindow3CY);

    app_key.SetDword(L"SoundDlgX", m_nSoundDlgX);
    app_key.SetDword(L"SoundDlgY", m_nSoundDlgY);
    app_key.SetDword(L"PicDlgX", m_nPicDlgX);
    app_key.SetDword(L"PicDlgY", m_nPicDlgY);
    app_key.SetDword(L"SaveToDlgX", m_nSaveToDlgX);
    app_key.SetDword(L"SaveToDlgY", m_nSaveToDlgY);

    app_key.SetDword(L"FPSx100", m_nFPSx100);
    app_key.SetDword(L"DrawCursor", m_bDrawCursor);
    app_key.SetDword(L"NoSound", m_bNoSound);
    app_key.SetDword(L"MonitorID", m_nMonitorID);
    app_key.SetDword(L"CameraID", m_nCameraID);

    app_key.SetDword(L"Brightness", m_nBrightness);
    app_key.SetDword(L"Contrast", m_nContrast);
    app_key.SetDword(L"FOURCC", m_dwFOURCC);

    app_key.SetSz(L"Dir", m_strDir.c_str());
    app_key.SetSz(L"MovieDir", m_strMovieDir.c_str());
    app_key.SetSz(L"ImageFileName", m_strImageFileName.c_str());
    app_key.SetSz(L"MovieFileName", m_strMovieFileName.c_str());
    app_key.SetSz(L"MovieTempFileName", m_strMovieTempFileName.c_str());
    app_key.SetSz(L"SoundFileName", m_strSoundFileName.c_str());

    return true;
}

void DoStartStopTimers(HWND hwnd, BOOL bStart)
{
    if (bStart)
    {
        SetTimer(hwnd, SOUND_TIMER_ID, 300, NULL);

        DWORD dwMSEC = DWORD(1000 * 100 / g_settings.m_nFPSx100);
        SetTimer(hwnd, CAP_TIMER_ID, dwMSEC, NULL);

        s_bWatching = TRUE;
    }
    else
    {
        s_bWatching = FALSE;
        KillTimer(hwnd, SOUND_TIMER_ID);
        KillTimer(hwnd, CAP_TIMER_ID);
    }
}

void Settings::update(HWND hwnd)
{
    update(hwnd, GetPictureType());
}

void Settings::update(HWND hwnd, PictureType type)
{
    BOOL bWatching = s_bWatching;

    if (bWatching)
        DoStartStopTimers(hwnd, FALSE);

    SetPictureType(hwnd, type);

    fix_size(hwnd);

    if (bWatching)
        DoStartStopTimers(hwnd, TRUE);
}

bool Settings::create_dirs() const
{
    CreateDirectory(m_strDir.c_str(), NULL);

    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, sizeof(szPath), m_strMovieDir.c_str(), m_nMovieID);
    CreateDirectory(szPath, NULL);

    return true;
}

void Settings::change_dirs(const WCHAR *dir)
{
    m_strDir = dir;

    TCHAR szPath[MAX_PATH];
    StringCbCopy(szPath, sizeof(szPath), dir);
    PathAppend(szPath, TEXT("movie-%03u"));
    m_strMovieDir = szPath;

    m_strImageFileName = TEXT("img-%04u.png");

    StringCbCopy(szPath, sizeof(szPath), dir);
    PathAppend(szPath, TEXT("movie-%03u.avi"));
    m_strMovieFileName = szPath;

    StringCbCopy(szPath, sizeof(szPath), dir);
    PathAppend(szPath, TEXT("movie-%03u-tmp.avi"));
    m_strMovieTempFileName = szPath;
}

INT GetHeightFromWidth(INT cx)
{
    if (g_settings.m_nWidth == 0)
        return 1;
    return cx * g_settings.m_nHeight / g_settings.m_nWidth;
}

INT GetWidthFromHeight(INT cy)
{
    if (g_settings.m_nHeight == 0)
        return 1;
    return cy * g_settings.m_nWidth / g_settings.m_nHeight;
}

static BOOL OnSizing(HWND hwnd, DWORD fwSide, LPRECT prc)
{
    RECT rc;
    DWORD style = GetWindowStyle(hwnd);
    DWORD exstyle = GetWindowExStyle(hwnd);
    BOOL bMenu = FALSE;

    SetRectEmpty(&rc);
    AdjustWindowRectEx(&rc, style, bMenu, exstyle);
    INT dx0 = rc.left, dy0 = rc.top;
    INT dx1 = rc.right, dy1 = rc.bottom;

    rc = *prc;
    rc.left -= dx0;
    rc.top -= dy0;
    rc.right -= dx1;
    rc.bottom -= dy1;

    INT padding_right = s_button_width + s_progress_width;
    rc.right -= padding_right;

    int x = rc.left;
    int y = rc.top;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    int cx2, cy2;
    bool f = float(cy) / cx > float(g_settings.m_nHeight) / g_settings.m_nWidth;

    switch (fwSide)
    {
    case WMSZ_TOP:
    case WMSZ_BOTTOM:
        cx2 = GetWidthFromHeight(cy);
        rc.left = x + cx / 2 - cx2 / 2;
        rc.right = x + cx / 2 + cx2 / 2;
        break;
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
        cy2 = GetHeightFromWidth(cx);
        rc.top = y + cy / 2 - cy2 / 2;
        rc.bottom = y + cy / 2 + cy2 / 2;
        break;
    case WMSZ_TOPLEFT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.left = x + cx - cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.top = y + cy - cy2;
        }
        break;
    case WMSZ_TOPRIGHT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.right = x + cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.top = y + cy - cy2;
        }
        break;
    case WMSZ_BOTTOMLEFT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.left = x + cx - cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.bottom = y + cy2;
        }
        break;
    case WMSZ_BOTTOMRIGHT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.right = x + cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.bottom = y + cy2;
        }
        break;
    }

    AdjustWindowRectEx(&rc, style, bMenu, exstyle);
    *prc = rc;
    prc->right += padding_right;

    InvalidateRect(hwnd, &rc, TRUE);

    return TRUE;
}

void Settings::fix_size0(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    OnSizing(hwnd, WMSZ_BOTTOMRIGHT, &rc);
    MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    SendMessage(hwnd, DM_REPOSITION, 0, 0);
}

void Settings::fix_size(HWND hwnd)
{
    INT x, y, cx, cy;
    switch (GetPictureType())
    {
    case PT_BLACK:
    case PT_WHITE:
        x = m_nWindow1X;
        y = m_nWindow1Y;
        cx = m_nWindow1CX;
        cy = m_nWindow1CY;
        break;
    case PT_SCREENCAP:
        x = m_nWindow2X;
        y = m_nWindow2Y;
        cx = m_nWindow2CX;
        cy = m_nWindow2CY;
        break;
    case PT_VIDEOCAP:
        x = m_nWindow3X;
        y = m_nWindow3Y;
        cx = m_nWindow3CX;
        cy = m_nWindow3CY;
        break;
    case PT_FINALIZING:
        return;
    }

    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }
    if (cx != CW_USEDEFAULT && cy != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, 0, 0, cx, cy,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }

    fix_size0(hwnd);
}

BOOL Settings::SetPictureType(HWND hwnd, PictureType type)
{
    assert(!s_bWatching);

    g_camera.release();
    s_frame.release();
    if (g_hbm)
    {
        DeleteObject(g_hbm);
        g_hbm = NULL;
    }

    switch (type)
    {
    case PT_BLACK:
    case PT_WHITE:
        SetDisplayMode(DM_BITMAP);
        m_nWidth = 320;
        m_nHeight = 240;
        break;
    case PT_SCREENCAP:
        SetDisplayMode(DM_BITMAP);
        m_nWidth = m_cxCap;
        m_nHeight = m_cyCap;
        break;
    case PT_VIDEOCAP:
        SetDisplayMode(DM_CAPFRAME);
        g_camera.open(m_nCameraID);
        if (!g_camera.isOpened())
        {
            DoStartStopTimers(hwnd, TRUE);
            return FALSE;
        }
        m_nWidth = (int)g_camera.get(cv::CAP_PROP_FRAME_WIDTH);
        m_nHeight = (int)g_camera.get(cv::CAP_PROP_FRAME_HEIGHT);
        break;
    case PT_FINALIZING:
        SetDisplayMode(DM_TEXT);
        break;
    }

    m_nPictureType = type;

    if (HDC hdc = CreateCompatibleDC(NULL))
    {
        LPVOID pvBits;
        HGDIOBJ hbmOld;
        RECT rc;

        g_bi.bmiHeader.biWidth = m_nWidth;
        g_bi.bmiHeader.biHeight = -m_nHeight;
        g_bi.bmiHeader.biBitCount = 24;
        g_bi.bmiHeader.biCompression = BI_RGB;
        g_hbm = CreateDIBSection(hdc, &g_bi, DIB_RGB_COLORS, &pvBits, NULL, 0);

        switch (m_nPictureType)
        {
        case PT_BLACK:
        case PT_SCREENCAP:
            hbmOld = SelectObject(hdc, g_hbm);
            PatBlt(hdc, 0, 0, m_nWidth, m_nHeight, BLACKNESS);
            SelectObject(hdc, hbmOld);
            break;
        case PT_WHITE:
            hbmOld = SelectObject(hdc, g_hbm);
            PatBlt(hdc, 0, 0, m_nWidth, m_nHeight, WHITENESS);
            SelectObject(hdc, hbmOld);
            break;
        case PT_VIDEOCAP:
            DeleteObject(g_hbm);
            g_hbm = NULL;
            break;
        case PT_FINALIZING:
            DeleteObject(g_hbm);
            g_hbm = NULL;
            break;
        }

        DeleteDC(hdc);
    }

    fix_size(hwnd);

    assert(!s_bWatching);

    return TRUE;
}

BOOL get_sound_devices(sound_devices_t& devices)
{
    devices.clear();

    // get an IMMDeviceEnumerator
    HRESULT hr;
    CComPtr<IMMDeviceEnumerator> pMMDeviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pMMDeviceEnumerator);
    if (FAILED(hr))
    {
        return FALSE;
    }

    // get an IMMDeviceCollection
    CComPtr<IMMDeviceCollection> pMMDeviceCollection;
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE,
                                                 &pMMDeviceCollection);
    if (FAILED(hr))
    {
        return FALSE;
    }

    // get the number of devices
    UINT nCount = 0;
    hr = pMMDeviceCollection->GetCount(&nCount);
    if (FAILED(hr) || nCount == 0)
    {
        return FALSE;
    }

    // enumerate devices
    for (UINT i = 0; i < nCount; ++i)
    {
        // get a device
        CComPtr<IMMDevice> pMMDevice;
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr))
        {
            return FALSE;
        }

        devices.push_back(pMMDevice);
    }

    return TRUE;
}

// special procedure to hide the focus
LRESULT CALLBACK
AlwaysHideFocusRectangleSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR idSubclass,
    DWORD_PTR dwRefData)
{
    switch (uMsg) {
    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd,
        AlwaysHideFocusRectangleSubclassProc, idSubclass);
        break;

    case WM_UPDATEUISTATE:
        wParam &= ~MAKELONG(0, UISF_HIDEFOCUS);
        break;
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

void HideButtonFocus(HWND hwnd, HWND hwndItem)
{
    SendMessage(hwndItem, WM_UPDATEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);
    SetWindowSubclass(hwndItem, AlwaysHideFocusRectangleSubclassProc, 0, 0);
}

HBITMAP DoLoadBitmap(HINSTANCE hInst, INT id)
{
    return (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0,
        LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    InitializeCriticalSection(&g_lock);

    // main window handle
    g_hMainWnd = hwnd;

    // set icon
    g_hMainIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(100));
    g_hMainIconSmall = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(100),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_hMainIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hMainIconSmall);

    // device context of the display
    g_hdcScreen = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

    // get sound devices and wave formats
    get_sound_devices(m_sound_devices);
    get_wave_formats(m_wave_formats);

    // load bitmaps
    HINSTANCE hInst = GetModuleHandle(NULL);
    s_hbmRec = DoLoadBitmap(hInst, IDB_REC);
    s_hbmPause = DoLoadBitmap(hInst, IDB_PAUSE);
    s_hbmStop = DoLoadBitmap(hInst, IDB_STOP);
    s_hbmDots = DoLoadBitmap(hInst, IDB_DOTS);

    // set the bitmap to each button
    SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
    SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmPause);
    SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmStop);
    SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);

    // hide button focus
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh1));
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh2));
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh3));
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh4));

    // no frames
    s_nFrames = 0;

    // s_button_width for layout purpose
    RECT rc;
    GetWindowRect(GetDlgItem(hwnd, psh1), &rc);
    s_button_width = rc.right - rc.left;

    // s_progress_width for layout purpose
    GetWindowRect(GetDlgItem(hwnd, scr1), &rc);
    s_progress_width = rc.right - rc.left;

    // load settings
    g_settings.load(hwnd);

    // setup sound device
    auto& format = m_wave_formats[g_settings.m_iWaveFormat];
    m_sound.SetInfo(format.channels, format.samples, format.bits);
    m_sound.SetDevice(m_sound_devices[g_settings.m_iSoundDev]);

    // uncheck some buttons
    Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);

    // fix window size
    g_settings.fix_size(hwnd);

    // ?
    PostMessage(hwnd, WM_SIZE, 0, 0);

    // restart hearing and watching
    g_settings.update(hwnd);
    m_sound.StartHearing();
    DoStartStopTimers(hwnd, TRUE);

    return TRUE;
}

static void OnConfig(HWND hwnd)
{
    HWND hButton = GetDlgItem(hwnd, psh4);

    // manage the pressed status of psh4
    if (GetAsyncKeyState(VK_MENU) < 0 &&
        GetAsyncKeyState(L'C') < 0)
    {
        // Alt+C
        Button_SetCheck(hButton, BST_CHECKED);
    }
    else
    {
        if (Button_GetCheck(hButton) & BST_CHECKED)
        {
            Button_SetCheck(hButton, BST_CHECKED);
        }
        else
        {
            Button_SetCheck(hButton, BST_UNCHECKED);
            return;
        }
    }

    // get the pos and size of the button, in screen coordinates
    RECT rc;
    GetWindowRect(hButton, &rc);

    // get the cursor pos in screen coordinates
    POINT pt;
    GetCursorPos(&pt);

    if (!PtInRect(&rc, pt))
    {
        // the center point
        pt.x = (rc.left + rc.right) / 2;
        pt.y = (rc.top + rc.bottom) / 2;
    }

    // load the menu from resource
    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
    if (!hMenu)
    {
        Button_SetCheck(hButton, BST_UNCHECKED);
        return;
    }
    HMENU hSubMenu = GetSubMenu(hMenu, 0);

    // setup the parameters to show a popup menu
    TPMPARAMS params;
    ZeroMemory(&params, sizeof(params));
    params.cbSize = sizeof(params);
    params.rcExclude = rc;

    // track the popup menu
    SetForegroundWindow(hwnd);
    UINT uFlags = TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_VERTICAL | TPM_RETURNCMD;
    UINT nCmd = TrackPopupMenuEx(hSubMenu, uFlags, rc.left, pt.y, hwnd, &params);

    // destroy the menu
    DestroyMenu(hMenu);

    // fix messaging (workaround)
    PostMessage(hwnd, WM_NULL, 0, 0);

    // invoke command
    if (nCmd != 0)
    {
        PostMessage(hwnd, WM_COMMAND, nCmd, 0);
    }

    // unpressed if necessary
    GetCursorPos(&pt);
    if (!PtInRect(&rc, pt) || GetAsyncKeyState(VK_LBUTTON) >= 0)
    {
        Button_SetCheck(hButton, BST_UNCHECKED);
    }
}

////////////////////////////////////////////////////////////////////////////
// finalizing

static INT s_nGotMovieID = 0;
static INT s_nFramesToWrite = 0;
static HANDLE s_hFinalizingThread = NULL;
BOOL s_bFinalizeCancelled = FALSE;
static PictureType s_nOldPictureType = PT_SCREENCAP;

BOOL DoSaveAviFile(HWND hwnd, LPCTSTR pszFileName, PAVISTREAM paviVideo,
                   PAVISTREAM paviAudio)
{
    INT i;
    PAVISTREAM pavis[2];
    AVICOMPRESSOPTIONS options[2];
    LPAVICOMPRESSOPTIONS lpOptions[2];
    INT nCount = 2;

    
    for (i = 0; i < nCount; i++)
    {
        ZeroMemory(&options[i], sizeof(AVICOMPRESSOPTIONS));
        lpOptions[i] = &options[i];
    }

    pavis[0] = paviVideo;
    pavis[1] = paviAudio;
    //AVISaveOptions(NULL, 0, nCount, pavis, lpOptions);

    INT nAVIERR;
    nAVIERR = AVISaveV(pszFileName, NULL, NULL, nCount, pavis, lpOptions);
    if (nAVIERR != AVIERR_OK)
    {
        assert(0);
        AVISaveOptionsFree(nCount, lpOptions);
        return FALSE;
    }

    //AVISaveOptionsFree(nCount, lpOptions);
    return TRUE;
}

BOOL DoUniteAviAndWav(HWND hwnd, const WCHAR *new_avi,
                      const WCHAR *old_avi, const WCHAR *wav_file)
{
    AVIFileInit();

    INT nAVIERR;

    PAVISTREAM paviVideo;
    nAVIERR = AVIStreamOpenFromFile(&paviVideo, old_avi, streamtypeVIDEO, 0,
                                    OF_READ | OF_SHARE_DENY_NONE, NULL);
    if (nAVIERR)
    {
        assert(0);
        AVIFileExit();
        return FALSE;
    }

    BOOL ret;
    PAVISTREAM paviAudio = NULL;
    if (wav_file)
    {
        // there is sound data
        nAVIERR = AVIStreamOpenFromFile(&paviAudio, wav_file, streamtypeAUDIO, 0,
                                        OF_READ | OF_SHARE_DENY_NONE, NULL);
        if (nAVIERR)
        {
            assert(0);
            AVIStreamRelease(paviVideo);
            AVIFileExit();
            return FALSE;
        }
        ret = DoSaveAviFile(hwnd, new_avi, paviVideo, paviAudio);
    }
    else
    {
        // there is no sound data
        ret = CopyFile(old_avi, new_avi, FALSE);
    }

    if (paviAudio)
    {
        AVIStreamRelease(paviAudio);
    }
    AVIStreamRelease(paviVideo);
    AVIFileExit();

    return ret;
}

void DoDeleteTempFiles(HWND hwnd)
{
    // delete the file of m_strMovieTempFileName
    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieTempFileName.c_str(),
                   s_nGotMovieID);
    DeleteFile(szPath);

    // strMovieDir
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                   s_nGotMovieID);
    std::wstring strMovieDir = szPath;

    // image_name
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                   s_nGotMovieID);
    PathAppend(szPath, g_settings.m_strImageFileName.c_str());
    std::wstring image_name = szPath;

    // sound_name
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                   s_nGotMovieID);
    PathAppend(szPath, g_settings.m_strSoundFileName.c_str());
    std::wstring sound_name = szPath;

    // delete frame files
    for (INT i = 0; i < s_nFramesToWrite; ++i)
    {
        StringCbPrintf(szPath, sizeof(szPath), image_name.c_str(), i);
        DeleteFile(szPath);
    }

    // delete sound file
    DeleteFile(sound_name.c_str());

    // remove the movie directory
    RemoveDirectory(strMovieDir.c_str());
}

static DWORD WINAPI FinalizingThreadFunction(LPVOID pContext)
{
    m_sound.StopHearing();

    // init progress bar
    HWND hScr1 = GetDlgItem(g_hMainWnd, scr1);
    SendMessage(hScr1, PBM_SETRANGE, 0, MAKELPARAM(0, s_nFramesToWrite));
    SendMessage(hScr1, PBM_SETPOS, 0, 0);

    // strOldMovieName and output_name
    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieTempFileName.c_str(),
                   s_nGotMovieID);
    std::wstring strOldMovieName = szPath;
    std::string output_name = ansi_from_wide(szPath);

    // image_name
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                   s_nGotMovieID);
    PathAppend(szPath, g_settings.m_strImageFileName.c_str());
    std::string image_name = ansi_from_wide(szPath);

    // get first frame
    TCHAR szText[MAX_PATH];
    CHAR szImageName[MAX_PATH];
    cv::Mat frame;
    StringCbPrintfA(szImageName, sizeof(szImageName),
                    image_name.c_str(), 0);
    frame = cv::imread(szImageName);
    if (!frame.data)
    {
        assert(0);
        StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_FINALIZEFAIL));
        g_settings.m_strStatusText = szText;
        m_sound.StartHearing();
        PostMessage(g_hMainWnd, WM_COMMAND, ID_FINALIZEFAIL, 0);
        return FALSE;
    }

    // get width and height
    int width, height;
    width = frame.cols;
    height = frame.rows;

    double fps = g_settings.m_nFPSx100 / 100.0;
    int fourcc = int(g_settings.m_dwFOURCC);

    // video writer
    cv::VideoWriter writer(output_name.c_str(), fourcc, fps, cv::Size(width, height));
    if (!writer.isOpened())
    {
        assert(0);
        StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_FINALIZEFAIL));
        g_settings.m_strStatusText = szText;
        m_sound.StartHearing();
        PostMessage(g_hMainWnd, WM_COMMAND, ID_FINALIZEFAIL, 0);
        return FALSE;
    }

    // for each frames...
    for (INT i = 0; i < s_nFramesToWrite; ++i)
    {
        if (s_bFinalizeCancelled)
        {
            // cancelled
            assert(0);
            StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_FINALIZEFAIL));
            g_settings.m_strStatusText = szText;
            m_sound.StartHearing();
            PostMessage(g_hMainWnd, WM_COMMAND, ID_FINALIZECANCEL, 0);
            return FALSE;
        }

        // load frame image
        StringCbPrintfA(szImageName, sizeof(szImageName),
                        image_name.c_str(), i);
        frame = cv::imread(szImageName);
        if (!frame.data)
        {
            assert(0);
            StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_FINALIZEFAIL));
            g_settings.m_strStatusText = szText;
            m_sound.StartHearing();
            PostMessage(g_hMainWnd, WM_COMMAND, ID_FINALIZEFAIL, 0);
            return FALSE;
        }

        // update status text
        StringCbPrintf(szText, sizeof(szText),
                       LoadStringDx(IDS_FINALIZEPERCENTS),
                       i * 100 / s_nFramesToWrite);
        g_settings.m_strStatusText = szText;

        // update progress
        PostMessage(hScr1, PBM_SETPOS, i, 0);

        // write frame
        writer << frame;
    }
    writer.release();

    // set 99%
    StringCbPrintf(szText, sizeof(szText),
                   LoadStringDx(IDS_FINALIZEPERCENTS), 99);
    g_settings.m_strStatusText = szText;
    SendMessage(hScr1, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(hScr1, PBM_SETPOS, 99, 0);

    // strNewMovieName
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieFileName.c_str(),
                   s_nGotMovieID);
    std::wstring strNewMovieName = szPath;

    // strWavName
    StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                   s_nGotMovieID);
    PathAppend(szPath, g_settings.m_strSoundFileName.c_str());
    std::wstring strWavName = szPath;

    // unite the movie and the sound
    BOOL ret = DoUniteAviAndWav(g_hMainWnd,
                                strNewMovieName.c_str(),
                                strOldMovieName.c_str(),
        (g_settings.m_bNoSound ? NULL : strWavName.c_str()));
    if (ret)
    {
        // success
        StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_FINALIZED));
        g_settings.m_strStatusText = szText;
        m_sound.StartHearing();
        PostMessage(g_hMainWnd, WM_COMMAND, ID_FINALIZED, 0);
        return TRUE;
    }
    else
    {
        // failure
        StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_FINALIZEFAIL));
        g_settings.m_strStatusText = szText;
        m_sound.StartHearing();
        PostMessage(g_hMainWnd, WM_COMMAND, ID_FINALIZEFAIL, 0);
        return FALSE;
    }
}

BOOL DoCreateFinalizingThread(HWND hwnd)
{
    // close previous handle
    if (s_hFinalizingThread)
    {
        CloseHandle(s_hFinalizingThread);
        s_hFinalizingThread = NULL;
    }

    // start up a new thread for finalizing
    DWORD tid = 0;
    s_bFinalizeCancelled = FALSE;
    s_hFinalizingThread = ::CreateThread(NULL, 0, FinalizingThreadFunction, NULL, 0, &tid);
    return s_hFinalizingThread != NULL;
}

static void OnStop(HWND hwnd)
{
    // stop
    DoStartStopTimers(hwnd, FALSE);
    m_sound.StopHearing();
    g_writer.release();
    s_bWriting = FALSE;
    m_sound.SetRecording(FALSE);

    if (!s_nFrames)
    {
        // not recorded yet
        Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
        SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmPause);
        SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmStop);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);

        // restart hearing and watching
        m_sound.StartHearing();
        DoStartStopTimers(hwnd, TRUE);
        return;
    }

    // now, the movie has been recorded

    // wait for the thread
    if (s_hFinalizingThread)
    {
        s_bFinalizeCancelled = TRUE;
        WaitForSingleObject(s_hFinalizingThread, INFINITE);
        CloseHandle(s_hFinalizingThread);
        s_hFinalizingThread = NULL;
        g_settings.SetPictureType(hwnd, s_nOldPictureType);
    }

    // prepare for finalizing
    s_nFramesToWrite = s_nFrames;
    s_nFrames = 0;
    SendDlgItemMessage(hwnd, scr1, PBM_SETPOS, 0, 0);
    s_nOldPictureType = g_settings.GetPictureType();
    g_settings.SetPictureType(hwnd, PT_FINALIZING);
    g_settings.m_strStatusText = LoadStringDx(IDS_FINALIZING);
    InvalidateRect(g_hMainWnd, NULL, TRUE);

    // ask for finalizing
    INT nID = MessageBox(hwnd, LoadStringDx(IDS_FINALIZEQUE),
                         LoadStringDx(IDS_WANNAFINALIZE),
                         MB_ICONINFORMATION | MB_YESNOCANCEL);
    switch (nID)
    {
    case IDYES:
        // disable buttons
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        EnableWindow(GetDlgItem(hwnd, psh2), FALSE);
        EnableWindow(GetDlgItem(hwnd, psh3), FALSE);
        EnableWindow(GetDlgItem(hwnd, psh4), FALSE);

        // create a thread
        DoCreateFinalizingThread(hwnd);
        break;
    case IDNO:
        // cancel
        DoDeleteTempFiles(hwnd);
        PostMessage(hwnd, WM_COMMAND, ID_FINALIZECANCEL, 0);
        break;
    case IDCANCEL:
        // cancel
        PostMessage(hwnd, WM_COMMAND, ID_FINALIZECANCEL, 0);
        break;
    }
}

static void OnFinalized(HWND hwnd)
{
    // close the thread handle
    if (s_hFinalizingThread)
    {
        CloseHandle(s_hFinalizingThread);
        s_hFinalizingThread = NULL;
    }

    // uncheck buttons
    Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);

    // if any config window is shown, enable some buttons
    if (!IsWindow(g_hwndSoundInput) &&
        !IsWindow(g_hwndPictureInput) &&
        !IsWindow(g_hwndSaveTo))
    {
        //
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);
        EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
        EnableWindow(GetDlgItem(hwnd, psh4), TRUE);
    }

    // enable some buttons
    SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmPause);
    SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmStop);
    EnableWindow(GetDlgItem(hwnd, psh2), TRUE);
    EnableWindow(GetDlgItem(hwnd, psh3), TRUE);

    TCHAR szPath[MAX_PATH];
    TCHAR szText[256 + 128];

    // delete temporary movie file
    StringCbPrintf(szPath, sizeof(szPath),
                   g_settings.m_strMovieTempFileName.c_str(),
                   s_nGotMovieID);
    DeleteFile(szPath);

    // build movie file path
    StringCbPrintf(szPath, sizeof(szPath),
                   g_settings.m_strMovieFileName.c_str(),
                   s_nGotMovieID);

    // build the message text
    StringCbPrintf(szText, sizeof(szText),
                   LoadStringDx(IDS_FINALIZEDONE), szPath);

    // delete the temporary files
    DoDeleteTempFiles(hwnd);

    // query for browse
    INT nID = MessageBox(hwnd, szText, LoadStringDx(IDS_APPTITLE),
                         MB_ICONINFORMATION | MB_YESNO);
    if (nID == IDYES)
    {
        // open explorer and select the file icon
        StringCbPrintf(szText, sizeof(szText),
                       L"/e,/select,%s", szPath);
        ShellExecute(hwnd, NULL, L"explorer", szText, NULL, SW_SHOWNORMAL);
    }

    // restart hearing and watching
    g_settings.SetPictureType(hwnd, s_nOldPictureType);
    m_sound.StartHearing();
    DoStartStopTimers(hwnd, TRUE);
}

static void OnFinalizeFail(HWND hwnd)
{
    // close the thread handle
    if (s_hFinalizingThread)
    {
        CloseHandle(s_hFinalizingThread);
        s_hFinalizingThread = NULL;
    }

    // uncheck some buttons
    Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);

    // enable some buttons
    if (!IsWindow(g_hwndSoundInput) &&
        !IsWindow(g_hwndPictureInput) &&
        !IsWindow(g_hwndSaveTo))
    {
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);
        EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
        EnableWindow(GetDlgItem(hwnd, psh4), TRUE);
    }

    // enable some buttons
    SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmPause);
    SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmStop);
    EnableWindow(GetDlgItem(hwnd, psh2), TRUE);
    EnableWindow(GetDlgItem(hwnd, psh3), TRUE);

    // resume picture type
    g_settings.SetPictureType(hwnd, s_nOldPictureType);

    // show a message of failure
    MessageBox(hwnd, LoadStringDx(IDS_FINALIZEFAIL), NULL, MB_ICONERROR);

    // restart hearing and watching
    m_sound.StartHearing();
    DoStartStopTimers(hwnd, TRUE);
}

static void OnFinalizeCancel(HWND hwnd)
{
    // close the thread handle
    if (s_hFinalizingThread)
    {
        CloseHandle(s_hFinalizingThread);
        s_hFinalizingThread = NULL;
    }

    // uncheck some buttons
    Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);

    // enable some buttons
    if (!IsWindow(g_hwndSoundInput) &&
        !IsWindow(g_hwndPictureInput) &&
        !IsWindow(g_hwndSaveTo))
    {
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);
        EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
        EnableWindow(GetDlgItem(hwnd, psh4), TRUE);
    }

    // enable some buttons
    SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmPause);
    SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmStop);
    EnableWindow(GetDlgItem(hwnd, psh2), TRUE);
    EnableWindow(GetDlgItem(hwnd, psh3), TRUE);

    // resume the picture type
    g_settings.SetPictureType(hwnd, s_nOldPictureType);

    // show a message of cancellation
    MessageBox(hwnd, LoadStringDx(IDS_FINALIZECANCELLED), NULL, MB_ICONERROR);

    // restart hearing and watching
    m_sound.StartHearing();
    DoStartStopTimers(hwnd, TRUE);
}

////////////////////////////////////////////////////////////////////////////

static void OnRec(HWND hwnd)
{
    if (Button_GetCheck(GetDlgItem(hwnd, psh1)) & BST_CHECKED)
    {
        // build image file path
        TCHAR szPath[MAX_PATH];
        StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                       g_settings.m_nMovieID);
        PathAppend(szPath, g_settings.m_strImageFileName.c_str());
        LPCSTR image_name = ansi_from_wide(szPath);

        // open writer
        g_writer.release();
        g_writer.open(image_name, 0, 0,
                      cv::Size(g_settings.m_nWidth, g_settings.m_nHeight));
        if (!g_writer.isOpened())
        {
            ErrorBoxDx(hwnd, TEXT("Unable to open image writer"));
            Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
            return;
        }

        // close config windows
        if (IsWindow(g_hwndSoundInput))
            PostMessage(g_hwndSoundInput, WM_CLOSE, 0, 0);
        if (IsWindow(g_hwndPictureInput))
            PostMessage(g_hwndPictureInput, WM_CLOSE, 0, 0);
        if (IsWindow(g_hwndSaveTo))
            PostMessage(g_hwndSaveTo, WM_CLOSE, 0, 0);

        // update UI
        EnableWindow(GetDlgItem(hwnd, psh4), FALSE);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

        // set sound path
        StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                       g_settings.m_nMovieID);
        PathAppend(szPath, g_settings.m_strSoundFileName.c_str());
        m_sound.SetSoundFile(szPath);

        // create directories
        g_settings.create_dirs();

        // OK, start recording
        s_nFrames = 0;
        s_nGotMovieID = g_settings.m_nMovieID;
        s_nFramesToWrite = 0;
        ++g_settings.m_nMovieID;
        s_bWriting = TRUE;
        if (!g_settings.m_bNoSound)
        {
            m_sound.SetRecording(TRUE);
        }
    }
    else
    {
        OnStop(hwnd);
    }
}

static void OnPause(HWND hwnd)
{
    // is the "pause" button pressed?
    if (Button_GetCheck(GetDlgItem(hwnd, psh2)) & BST_CHECKED)
    {
        // disable recording
        s_bWriting = FALSE;
        if (!g_settings.m_bNoSound)
        {
            m_sound.SetRecording(FALSE);
        }
    }
    else
    {
        // enable recording
        s_bWriting = TRUE;
        if (!g_settings.m_bNoSound)
        {
            m_sound.SetRecording(TRUE);
        }
    }
}

static void OnAbout(HWND hwnd)
{
    MSGBOXPARAMS params = { sizeof(params) };
    params.hwndOwner = hwnd;
    params.hInstance = g_hInst;
    params.lpszText = MAKEINTRESOURCE(IDS_VERSIONTEXT);
    params.lpszCaption = MAKEINTRESOURCE(IDS_VERSIONCAPTION);
    params.dwStyle = MB_USERICON;
    params.lpszIcon = MAKEINTRESOURCE(100);
    MessageBoxIndirect(&params);
}

static void OnExit(HWND hwnd)
{
    EndDialog(hwnd, IDCANCEL);
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            OnRec(hwnd);
        }
        break;
    case psh2:
        if (codeNotify == BN_CLICKED)
        {
            OnPause(hwnd);
        }
        break;
    case psh3:
        if (codeNotify == BN_CLICKED)
        {
            OnStop(hwnd);
        }
        break;
    case psh4:
        if (codeNotify == BN_CLICKED)
        {
            OnConfig(hwnd);
        }
        break;
    case ID_ABOUT:
        OnAbout(hwnd);
        break;
    case ID_SOUNDINPUT:
        if (0)
        {
            SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
            EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        }
        DoSoundInputDialogBox(hwnd);
        break;
    case ID_PICTUREINPUT:
        if (0)
        {
            SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
            EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        }
        DoPictureInputDialogBox(hwnd);
        break;
    case ID_CONFIGCLOSED:
        if (0)
        {
            if (!IsWindow(g_hwndSoundInput) &&
                !IsWindow(g_hwndPictureInput) &&
                !IsWindow(g_hwndSaveTo))
            {
                SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
                EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
            }
        }
        break;
    case ID_FINALIZED:
        OnFinalized(hwnd);
        break;
    case ID_FINALIZEFAIL:
        OnFinalizeFail(hwnd);
        break;
    case ID_FINALIZECANCEL:
        OnFinalizeCancel(hwnd);
        break;
    case ID_SAVETO:
        if (0)
        {
            SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
            EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        }
        DoSaveToDialogBox(hwnd);
        break;
    case ID_EXIT:
        OnExit(hwnd);
        break;
    }
}

static void OnDraw(HWND hwnd, HDC hdc, INT cx, INT cy)
{
    // COLORONCOLOR is quick
    SetStretchBltMode(hdc, COLORONCOLOR);

    EnterCriticalSection(&g_lock);

    switch (g_settings.GetDisplayMode())
    {
    case DM_CAPFRAME:
        if (s_frame.data)   // if frame data exists
        {
            if (g_settings.m_nBrightness != 0 ||
                g_settings.m_nContrast != 100)
            {
                // modify the brightness and contrast values
                double alpha = g_settings.m_nContrast / 100.0;
                double beta = g_settings.m_nBrightness;

                cv::Mat image;
                s_frame.convertTo(image, -1, alpha, beta);
                StretchDIBits(hdc, 0, 0, cx, cy,
                              0, 0, g_settings.m_nWidth, g_settings.m_nHeight,
                              image.data, &g_bi, DIB_RGB_COLORS, SRCCOPY);
                if (s_bWriting && g_writer.isOpened())
                {
                    // write a frame
                    g_writer << image;
                    ++s_nFrames;
                }
            }
            else
            {
                // no modification
                StretchDIBits(hdc, 0, 0, cx, cy,
                              0, 0, g_settings.m_nWidth, g_settings.m_nHeight,
                              s_frame.data, &g_bi, DIB_RGB_COLORS, SRCCOPY);
                if (s_bWriting && g_writer.isOpened())
                {
                    // write a frame
                    g_writer << s_frame;
                    ++s_nFrames;
                }
            }
        }
        else
        {
            // black out if no image
            PatBlt(hdc, 0, 0, cx, cy, BLACKNESS);
        }
        break;
    case DM_BITMAP:
        if (HDC hdcMem = CreateCompatibleDC(NULL))
        {
            BITMAP bm;
            if (GetObject(g_hbm, sizeof(bm), &bm))
            {
                if (g_hbm)
                {
                    HGDIOBJ hbmOld = SelectObject(hdcMem, g_hbm);
                    StretchBlt(hdc, 0, 0, cx, cy,
                               hdcMem, 0, 0, bm.bmWidth, bm.bmHeight,
                               SRCCOPY);
                    SelectObject(hdcMem, hbmOld);
                }
                else
                {
                    // black out if no image
                    PatBlt(hdc, 0, 0, cx, cy, BLACKNESS);
                }

                if (g_hbm && s_bWriting && g_writer.isOpened())
                {
                    // write a frame
                    cv::Size size(bm.bmWidth, bm.bmHeight);
                    cv::Mat mat(size, CV_8UC3, bm.bmBits, bm.bmWidthBytes);
                    g_writer << mat;
                    ++s_nFrames;
                }
            }
            else
            {
                // black out if no image
                PatBlt(hdc, 0, 0, cx, cy, BLACKNESS);
            }

            DeleteDC(hdcMem);
        }
        break;
    case DM_TEXT:
        {
            // white background
            RECT rc;
            SetRect(&rc, 0, 0, cx, cy);
            FillRect(hdc, &rc, GetStockBrush(WHITE_BRUSH));

            // show image
            if (HFONT hFont = GetWindowFont(hwnd))
            {
                HGDIOBJ hFontOld = SelectObject(hdc, hFont);
                UINT uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
                DrawText(hdc,
                         g_settings.m_strStatusText.c_str(),
                         INT(g_settings.m_strStatusText.size()),
                         &rc, uFormat);
                SelectObject(hdc, hFontOld);
            }
        }
        break;
    }

    LeaveCriticalSection(&g_lock);
}

static void OnPaint(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    INT cx = rc.right - s_button_width - s_progress_width;
    INT cy = rc.bottom;

    PAINTSTRUCT ps;
    if (HDC hdc = BeginPaint(hwnd, &ps))
    {
        OnDraw(hwnd, hdc, cx, cy);

        EndPaint(hwnd, &ps);
    }
}

static BOOL OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    return TRUE;
}

static void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    if (g_settings.m_nWidth == 0 || g_settings.m_nHeight == 0)
        return;

    RECT rc;
    DWORD style = GetWindowStyle(hwnd);
    DWORD exstyle = GetWindowExStyle(hwnd);
    BOOL bMenu = FALSE;

    // restrict the maximum size of the main window
    SetRectEmpty(&rc);
    rc.right = 192;
    rc.bottom = rc.right * g_settings.m_nHeight / g_settings.m_nWidth;
    AdjustWindowRectEx(&rc, style, bMenu, exstyle);
    lpMinMaxInfo->ptMinTrackSize.x = rc.right - rc.left;
    lpMinMaxInfo->ptMinTrackSize.y = rc.bottom - rc.top;
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);

    // record the positions for usability
    switch (g_settings.GetPictureType())
    {
    case PT_BLACK:
    case PT_WHITE:
    case PT_FINALIZING:
        g_settings.m_nWindow1X = rc.left;
        g_settings.m_nWindow1Y = rc.top;
        break;
    case PT_SCREENCAP:
        g_settings.m_nWindow2X = rc.left;
        g_settings.m_nWindow2Y = rc.top;
        break;
    case PT_VIDEOCAP:
        g_settings.m_nWindow3X = rc.left;
        g_settings.m_nWindow3Y = rc.top;
        break;
    }
}

static void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (IsMinimized(hwnd))
        return;

    // get client area minus buttons and progress bar
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.right -= s_button_width + s_progress_width;
    cx = rc.right;
    cy = rc.bottom;

    // move the STATIC control
    MoveWindow(GetDlgItem(hwnd, stc1), 0, 0, cx, cy, TRUE);

    INT x = cx;
    INT y = 0, cy2;
    INT i = 0;

    // move psh1
    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh1), x, y, s_button_width, cy2, TRUE);
    ++i;

    // move psh2
    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh2), x, y, s_button_width, cy2, TRUE);
    ++i;

    // move psh3
    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh3), x, y, s_button_width, cy2, TRUE);
    ++i;

    // move psh4
    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh4), x, y, s_button_width, cy2, TRUE);
    ++i;

    // move the progress bar
    x = cx + s_button_width;
    y = 0;
    MoveWindow(GetDlgItem(hwnd, scr1), x, y, s_progress_width, cy, TRUE);

    if (!IsMaximized(hwnd))
    {
        // record the sizes for usability
        RECT rcWnd;
        GetWindowRect(hwnd, &rcWnd);
        INT cxWnd = rcWnd.right - rcWnd.left;
        INT cyWnd = rcWnd.bottom - rcWnd.top;
        switch (g_settings.GetPictureType())
        {
        case PT_BLACK:
        case PT_WHITE:
        case PT_FINALIZING:
            g_settings.m_nWindow1CX = cxWnd;
            g_settings.m_nWindow1CY = cyWnd;
            break;
        case PT_SCREENCAP:
            g_settings.m_nWindow2CX = cxWnd;
            g_settings.m_nWindow2CY = cyWnd;
            break;
        case PT_VIDEOCAP:
            g_settings.m_nWindow3CX = cxWnd;
            g_settings.m_nWindow3CY = cyWnd;
            break;
        }
    }

    // trigger to redraw
    InvalidateRect(hwnd, &rc, TRUE);
}

static void OnDestroy(HWND hwnd)
{
    // stop watching and hearing
    DoStartStopTimers(hwnd, FALSE);
    m_sound.StopHearing();

    // destroy icon
    DestroyIcon(g_hMainIcon);
    g_hMainIcon = NULL;
    DestroyIcon(g_hMainIconSmall);
    g_hMainIconSmall = NULL;

    // save application's settings
    g_settings.save(hwnd);

    DeleteCriticalSection(&g_lock);

    // delete bitmaps
    DeleteObject(s_hbmRec);
    DeleteObject(s_hbmPause);
    DeleteObject(s_hbmStop);
    DeleteObject(s_hbmDots);

    // delete screen DC
    DeleteDC(g_hdcScreen);
    g_hdcScreen = NULL;

    // no use of the main window any more
    g_hMainWnd = NULL;
}

void DoDrawCursor(HDC hDC, INT dx, INT dy)
{
    INT x, y;
    CURSORINFO ci;
    ICONINFO ii;

    ci.cbSize = sizeof(CURSORINFO);
    ::GetCursorInfo(&ci);
    if (!(ci.flags & CURSOR_SHOWING))
        return;

    ::GetIconInfo(ci.hCursor, &ii);

    x = ci.ptScreenPos.x - ii.xHotspot - dx;
    y = ci.ptScreenPos.y - ii.yHotspot - dy;
    ::DrawIcon(hDC, x, y, ci.hCursor);
}

void DoScreenCap(HWND hwnd, BOOL bCursor)
{
    // sanity check
    if (g_settings.m_nWidth != g_settings.m_cxCap ||
        g_settings.m_nHeight != g_settings.m_cyCap)
    {
        return;
    }

    if (HDC hdcMem = CreateCompatibleDC(g_hdcScreen))
    {
        EnterCriticalSection(&g_lock);

        // screen capture!
        HGDIOBJ hbmOld = SelectObject(hdcMem, g_hbm);
        BitBlt(hdcMem,
               0, 0,
               g_settings.m_cxCap, g_settings.m_cxCap,
               g_hdcScreen,
               g_settings.m_xCap, g_settings.m_yCap,
               SRCCOPY | CAPTUREBLT);
        if (bCursor)
        {
            DoDrawCursor(hdcMem, g_settings.m_xCap, g_settings.m_yCap);
        }
        SelectObject(hdcMem, hbmOld);

        LeaveCriticalSection(&g_lock);
        DeleteDC(hdcMem);
    }
}

static void OnTimer(HWND hwnd, UINT id)
{
    switch (id)
    {
    case CAP_TIMER_ID:
        switch (g_settings.GetPictureType())
        {
        case PT_BLACK:
        case PT_WHITE:
        case PT_FINALIZING:
            // no capture
            break;
        case PT_SCREENCAP:
            // take a screen capture
            DoScreenCap(hwnd, g_settings.m_bDrawCursor);
            break;
        case PT_VIDEOCAP:
            // take a camera capture
            EnterCriticalSection(&g_lock);
            g_camera >> s_frame;
            LeaveCriticalSection(&g_lock);
            break;
        }
        // trigger to redraw
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    case SOUND_TIMER_ID:
        if (!g_settings.m_bNoSound)
        {
            // show sound loudness
            LONG nValue = m_sound.m_nValue;
            LONG nMax = m_sound.m_nMax;
            SendDlgItemMessage(hwnd, scr1, PBM_SETRANGE32, 0, nMax);
            SendDlgItemMessage(hwnd, scr1, PBM_SETPOS, nValue, 0);
        }
        else
        {
            // no sound
            SendDlgItemMessage(hwnd, scr1, PBM_SETRANGE32, 0, 0);
            SendDlgItemMessage(hwnd, scr1, PBM_SETPOS, 0, 0);
        }
        break;
    }
}

// the dialog procedure of the main window
static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
        HANDLE_MSG(hwnd, WM_MOVE, OnMove);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
        case WM_SIZING:
        {
            if (OnSizing(hwnd, (DWORD)wParam, (LPRECT)lParam))
            {
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
                return TRUE;
            }
            break;
        }
        case WM_UPDATEUISTATE:
        {

        }
    }
    return 0;
}

int WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        int         nCmdShow)
{
    g_hInst = hInstance;

    // don't start two applications at once
    if (HWND hwnd = FindWindow(L"#32770", LoadStringDx(IDS_APPTITLE)))
    {
        ShowWindow(hwnd, SW_RESTORE);
        PostMessage(hwnd, DM_REPOSITION, 0, 0);
        SetForegroundWindow(hwnd);
        return EXIT_SUCCESS;
    }

    // initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        ErrorBoxDx(NULL, TEXT("CoInitializeEx"));
        return EXIT_FAILURE;
    }

    // initialize comctl32
    InitCommonControls();

    // show the main window
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);

    // uninitialize COM
    CoUninitialize();

    return EXIT_SUCCESS;
}
