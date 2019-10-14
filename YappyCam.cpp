// KeepAspect.cpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#include "YappyCam.hpp"

#define SOUND_TIMER_ID  999
#define CAP_TIMER_ID    888

static cv::Mat s_frame;
static INT s_button_width;
static INT s_progress_width;
static HBITMAP s_hbmRec;
static HBITMAP s_hbmPause;
static HBITMAP s_hbmStop;
static HBITMAP s_hbmDots;

typedef std::vector<CComPtr<IMMDevice> > sound_devices_t;

Settings g_settings;

HWND g_hMainWnd = NULL;
Sound m_sound;
sound_devices_t m_sound_devices;
std::vector<WAVE_FORMAT_INFO> m_wave_formats;
cv::VideoCapture g_cap;
cv::VideoWriter g_writer;
BOOL g_bWriting = FALSE;
CRITICAL_SECTION g_lock;
HDC g_hdcScreen = NULL;
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
    m_nPictureType = PT_VIDEOCAP;
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

    m_nWindow1CX = m_nWindow1CY = CW_USEDEFAULT;
    m_nWindow2CX = m_nWindow2CY = CW_USEDEFAULT;
    m_nWindow3CX = m_nWindow3CY = CW_USEDEFAULT;

    m_nSoundDlgX = m_nSoundDlgY = CW_USEDEFAULT;
    m_nPicDlgX = m_nPicDlgY = CW_USEDEFAULT;

    m_nFPSx100 = UINT(DEFAULT_FPS * 100);
    m_bDrawCursor = TRUE;
    m_bNoSound = FALSE;

    TCHAR szPath[MAX_PATH];

    SHGetSpecialFolderPath(NULL, szPath, CSIDL_MYVIDEO, TRUE);
    PathAppend(szPath, TEXT("YappyCam"));
    m_strDir = szPath;

    PathAppend(szPath, TEXT("movie-%03u"));
    m_strMovieDir = szPath;

    m_strImageFileName = TEXT("img-%04u.png");

    SHGetSpecialFolderPath(NULL, szPath, CSIDL_MYVIDEO, FALSE);
    PathAppend(szPath, TEXT("YappyCam"));
    PathAppend(szPath, TEXT("movie-%3u.avi"));
    m_strMovieFileName = szPath;

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

    app_key.QueryDword(L"FPSx100", (DWORD&)m_nFPSx100);
    app_key.QueryDword(L"DrawCursor", (DWORD&)m_bDrawCursor);
    app_key.QueryDword(L"NoSound", (DWORD&)m_bNoSound);

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

    app_key.SetDword(L"FPSx100", m_nFPSx100);
    app_key.SetDword(L"DrawCursor", m_bDrawCursor);
    app_key.SetDword(L"NoSound", m_bNoSound);

    app_key.SetSz(L"Dir", m_strDir.c_str());
    app_key.SetSz(L"MovieDir", m_strMovieDir.c_str());
    app_key.SetSz(L"ImageFileName", m_strImageFileName.c_str());
    app_key.SetSz(L"MovieFileName", m_strMovieFileName.c_str());

    return true;
}

void Settings::update(HWND hwnd)
{
    PictureType type = GetPictureType();
    SetPictureType(hwnd, type);
}

bool Settings::create_dirs() const
{
    CreateDirectory(m_strDir.c_str(), NULL);

    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, sizeof(szPath), m_strMovieDir.c_str(), m_nMovieID);
    CreateDirectory(szPath, NULL);

    return true;
}

INT GetHeightFromWidth(INT cx)
{
    return cx * g_settings.m_nHeight / g_settings.m_nWidth;
}

INT GetWidthFromHeight(INT cy)
{
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

void DoFixWindowSize(HWND hwnd)
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
    case PT_STATUSTEXT:
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

    DoFixWindowSize(hwnd);
}

void DoStartStopTimers(HWND hwnd, BOOL bStart)
{
    if (bStart)
    {
        SetTimer(hwnd, SOUND_TIMER_ID, 300, NULL);

        DWORD dwMSEC = DWORD(1000 * 100 / g_settings.m_nFPSx100);
        SetTimer(hwnd, CAP_TIMER_ID, dwMSEC, NULL);
    }
    else
    {
        KillTimer(hwnd, SOUND_TIMER_ID);
        KillTimer(hwnd, CAP_TIMER_ID);
    }
}

BOOL Settings::SetPictureType(HWND hwnd, PictureType type)
{
    DoStartStopTimers(hwnd, FALSE);

    if (g_hbm)
    {
        DeleteObject(g_hbm);
        g_hbm = NULL;
    }

    g_cap.release();

    INT x, y, cx, cy;
    switch (type)
    {
    case PT_BLACK:
    case PT_WHITE:
    case PT_STATUSTEXT:
        x = g_settings.m_nWindow1X;
        y = g_settings.m_nWindow1Y;
        cx = g_settings.m_nWindow1CX;
        cy = g_settings.m_nWindow1CX;
        break;
    case PT_SCREENCAP:
        x = g_settings.m_nWindow2X;
        y = g_settings.m_nWindow2Y;
        cx = g_settings.m_nWindow2CX;
        cy = g_settings.m_nWindow2CX;
        break;
    case PT_VIDEOCAP:
        x = g_settings.m_nWindow3X;
        y = g_settings.m_nWindow3Y;
        cx = g_settings.m_nWindow3CX;
        cy = g_settings.m_nWindow3CX;
        break;
    }

    switch (type)
    {
    case PT_BLACK:
        SetDisplayMode(DM_BITMAP);
        m_cxCap = m_nWidth = 320;
        m_cyCap = m_nHeight = 240;
        break;
    case PT_WHITE:
        SetDisplayMode(DM_BITMAP);
        m_cxCap = m_nWidth = 320;
        m_cyCap = m_nHeight = 240;
        break;
    case PT_SCREENCAP:
        SetDisplayMode(DM_BITMAP);
        m_cxCap = m_nWidth = GetSystemMetrics(SM_CXSCREEN);
        m_cyCap = m_nHeight = GetSystemMetrics(SM_CYSCREEN);
        break;
    case PT_VIDEOCAP:
        SetDisplayMode(DM_CAPFRAME);
        g_cap.open(0);
        if (!g_cap.isOpened())
        {
            DoStartStopTimers(hwnd, TRUE);
            return FALSE;
        }
        m_nWidth = (int)g_cap.get(cv::CAP_PROP_FRAME_WIDTH);
        m_nHeight = (int)g_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        break;
    case PT_STATUSTEXT:
        SetDisplayMode(DM_BITMAP);
        m_nWidth = 320;
        m_nHeight = 240;
        break;
    }

    g_bi.bmiHeader.biWidth = m_nWidth;
    g_bi.bmiHeader.biHeight = -m_nHeight;
    g_bi.bmiHeader.biBitCount = 24;
    g_bi.bmiHeader.biCompression = BI_RGB;
    if (GetDisplayMode() == DM_BITMAP)
    {
        if (HDC hdc = CreateCompatibleDC(NULL))
        {
            LPVOID pvBits;
            HGDIOBJ hbmOld;
            RECT rc;

            g_hbm = CreateDIBSection(hdc, &g_bi, DIB_RGB_COLORS, &pvBits, NULL, 0);

            switch (type)
            {
            case PT_BLACK:
                hbmOld = SelectObject(hdc, g_hbm);
                PatBlt(hdc, 0, 0, m_nWidth, m_nHeight, BLACKNESS);
                SelectObject(hdc, hbmOld);
                break;
            case PT_WHITE:
            case PT_SCREENCAP:
                hbmOld = SelectObject(hdc, g_hbm);
                PatBlt(hdc, 0, 0, m_nWidth, m_nHeight, WHITENESS);
                SelectObject(hdc, hbmOld);
                break;
            case PT_VIDEOCAP:
                DeleteObject(g_hbm);
                g_hbm = NULL;
                break;
            case PT_STATUSTEXT:
                SetRect(&rc, 0, 0, m_nWidth, m_nHeight);
                hbmOld = SelectObject(hdc, g_hbm);
                DrawText(hdc, TEXT("No Image"), -1, &rc,
                         DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
                SelectObject(hdc, hbmOld);
                break;
            }

            DeleteDC(hdc);
        }
    }

    m_nPictureType = type;
    fix_size(hwnd);

    DoStartStopTimers(hwnd, TRUE);
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

    g_hMainWnd = hwnd;

    g_hdcScreen = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

    get_sound_devices(m_sound_devices);
    get_wave_formats(m_wave_formats);

    HINSTANCE hInst = GetModuleHandle(NULL);
    s_hbmRec = DoLoadBitmap(hInst, IDB_REC);
    s_hbmPause = DoLoadBitmap(hInst, IDB_PAUSE);
    s_hbmStop = DoLoadBitmap(hInst, IDB_STOP);
    s_hbmDots = DoLoadBitmap(hInst, IDB_DOTS);

    SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
    SendDlgItemMessage(hwnd, psh2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmPause);
    SendDlgItemMessage(hwnd, psh3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmStop);
    SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);

    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh1));
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh2));
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh3));
    HideButtonFocus(hwnd, GetDlgItem(hwnd, psh4));

    RECT rc;
    GetWindowRect(GetDlgItem(hwnd, psh1), &rc);
    s_button_width = rc.right - rc.left;

    GetWindowRect(GetDlgItem(hwnd, scr1), &rc);
    s_progress_width = rc.right - rc.left;

    g_settings.load(hwnd);

    auto& format = m_wave_formats[g_settings.m_iWaveFormat];
    m_sound.SetInfo(format.channels, format.samples, format.bits);
    m_sound.SetDevice(m_sound_devices[g_settings.m_iSoundDev]);

    Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);

    g_settings.fix_size(hwnd);
    PostMessage(hwnd, WM_SIZE, 0, 0);

    m_sound.StartHearing();
    DoStartStopTimers(hwnd, TRUE);

    return TRUE;
}

static void OnConfig(HWND hwnd)
{
    HWND hButton = GetDlgItem(hwnd, psh4);

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

    RECT rc;
    GetWindowRect(hButton, &rc);

    POINT pt;
    GetCursorPos(&pt);

    if (!PtInRect(&rc, pt))
    {
        pt.x = (rc.left + rc.right) / 2;
        pt.y = (rc.top + rc.bottom) / 2;
    }

    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
    if (!hMenu)
    {
        Button_SetCheck(hButton, BST_UNCHECKED);
        return;
    }

    HMENU hSubMenu = GetSubMenu(hMenu, 0);

    TPMPARAMS params;
    ZeroMemory(&params, sizeof(params));
    params.cbSize = sizeof(params);
    params.rcExclude = rc;

    SetForegroundWindow(hwnd);
    UINT uFlags = TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_VERTICAL | TPM_RETURNCMD;
    UINT nCmd = TrackPopupMenuEx(hSubMenu, uFlags, rc.left, pt.y, hwnd, &params);
    DestroyMenu(hMenu);

    PostMessage(hwnd, WM_NULL, 0, 0);

    if (nCmd != 0)
    {
        PostMessage(hwnd, WM_COMMAND, nCmd, 0);
    }

    GetCursorPos(&pt);
    if (!PtInRect(&rc, pt) || GetAsyncKeyState(VK_LBUTTON) >= 0)
    {
        Button_SetCheck(hButton, BST_UNCHECKED);
    }
}

static void OnStop(HWND hwnd)
{
    DoStartStopTimers(hwnd, FALSE);
    m_sound.StopHearing();
    g_writer.release();
    g_bWriting = FALSE;

    Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, psh2), BST_UNCHECKED);

    if (!IsWindow(g_hwndSoundInput) &&
        !IsWindow(g_hwndPictureInput))
    {
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmDots);
        EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
        EnableWindow(GetDlgItem(hwnd, psh4), TRUE);
    }

    SendDlgItemMessage(hwnd, scr1, PBM_SETPOS, 0, 0);

    m_sound.StartHearing();
    DoStartStopTimers(hwnd, TRUE);
}

static void OnRec(HWND hwnd)
{
    if (Button_GetCheck(GetDlgItem(hwnd, psh1)) & BST_CHECKED)
    {
        TCHAR szPath[MAX_PATH];
        StringCbPrintf(szPath, sizeof(szPath), g_settings.m_strMovieDir.c_str(),
                       g_settings.m_nMovieID);
        PathAppend(szPath, g_settings.m_strImageFileName.c_str());

        LPCSTR image_name = ansi_from_wide(szPath);

        g_writer.open(image_name, 0, 0,
                      cv::Size(g_settings.m_nWidth, g_settings.m_nHeight));
        if (!g_writer.isOpened())
        {
            ErrorBoxDx(hwnd, TEXT("Unable to open image writer"));
            Button_SetCheck(GetDlgItem(hwnd, psh1), BST_UNCHECKED);
            return;
        }
        g_bWriting = TRUE;

        EnableWindow(GetDlgItem(hwnd, psh4), FALSE);
        SendDlgItemMessage(hwnd, psh4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

        g_settings.create_dirs();
        ++g_settings.m_nMovieID;

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
    if (Button_GetCheck(GetDlgItem(hwnd, psh2)) & BST_CHECKED)
    {
        g_bWriting = FALSE;
        if (!g_settings.m_bNoSound)
        {
            m_sound.SetRecording(FALSE);
        }
    }
    else
    {
        g_bWriting = TRUE;
        if (!g_settings.m_bNoSound)
        {
            m_sound.SetRecording(TRUE);
        }
    }
}

static void OnAbout(HWND hwnd)
{
    MessageBox(hwnd,
               TEXT("YappyCam 0.0 by katahiromz"),
               TEXT("About YappyCam"),
               MB_ICONINFORMATION);
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
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        DoSoundInputDialogBox(hwnd);
        break;
    case ID_PICTUREINPUT:
        SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        DoPictureInputDialogBox(hwnd);
        break;
    case ID_CONFIGCLOSED:
        if (!IsWindow(g_hwndSoundInput) &&
            !IsWindow(g_hwndPictureInput))
        {
            SendDlgItemMessage(hwnd, psh1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)s_hbmRec);
            EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
        }
        break;
    }
}

static void OnDraw(HWND hwnd, HDC hdc, INT cx, INT cy)
{
    SetStretchBltMode(hdc, COLORONCOLOR);
    EnterCriticalSection(&g_lock);

    switch (g_settings.GetDisplayMode())
    {
    case DM_CAPFRAME:
        if (s_frame.data)
        {
            StretchDIBits(hdc, 0, 0, cx, cy,
                          0, 0, g_settings.m_nWidth, g_settings.m_nHeight,
                          s_frame.data, &g_bi, DIB_RGB_COLORS, SRCCOPY);
            if (g_bWriting && g_writer.isOpened())
            {
                g_writer << s_frame;
            }
        }
        break;
    case DM_BITMAP:
        if (HDC hdcMem = CreateCompatibleDC(NULL))
        {
            SetStretchBltMode(hdcMem, COLORONCOLOR);
            BITMAP bm;
            GetObject(g_hbm, sizeof(bm), &bm);

            HGDIOBJ hbmOld = SelectObject(hdcMem, g_hbm);
            StretchBlt(hdc, 0, 0, cx, cy,
                       hdcMem, 0, 0, bm.bmWidth, bm.bmHeight,
                       SRCCOPY);
            SelectObject(hdcMem, hbmOld);
            DeleteDC(hdcMem);
            if (g_bWriting && g_writer.isOpened())
            {
                cv::Size size(bm.bmWidth, bm.bmHeight);
                cv::Mat mat(size, CV_8UC3, bm.bmBits, bm.bmWidthBytes);
                g_writer << mat;
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
    RECT rc;
    DWORD style = GetWindowStyle(hwnd);
    DWORD exstyle = GetWindowExStyle(hwnd);
    BOOL bMenu = FALSE;

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

    switch (g_settings.GetPictureType())
    {
    case PT_BLACK:
    case PT_WHITE:
    case PT_STATUSTEXT:
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
    if (IsMaximized(hwnd))
    {
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    if (IsMinimized(hwnd))
        return;

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.right -= s_button_width + s_progress_width;
    cx = rc.right;
    cy = rc.bottom;

    MoveWindow(GetDlgItem(hwnd, stc1), 0, 0, cx, cy, TRUE);

    INT x = cx;
    INT y = 0, cy2;
    INT i = 0;

    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh1), x, y, s_button_width, cy2, TRUE);
    ++i;

    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh2), x, y, s_button_width, cy2, TRUE);
    ++i;

    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh3), x, y, s_button_width, cy2, TRUE);
    ++i;

    y = i * cy / 4;
    cy2 = (i + 1) * cy / 4 - y;
    MoveWindow(GetDlgItem(hwnd, psh4), x, y, s_button_width, cy2, TRUE);
    ++i;

    x = cx + s_button_width;
    y = 0;
    MoveWindow(GetDlgItem(hwnd, scr1), x, y, s_progress_width, cy, TRUE);

    {
        RECT rcWnd;
        GetWindowRect(hwnd, &rcWnd);
        INT cxWnd = rcWnd.right - rcWnd.left;
        INT cyWnd = rcWnd.bottom - rcWnd.top;
        switch (g_settings.GetPictureType())
        {
        case PT_BLACK:
        case PT_WHITE:
        case PT_STATUSTEXT:
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

    InvalidateRect(hwnd, &rc, TRUE);
}

static void OnDestroy(HWND hwnd)
{
    DoStartStopTimers(hwnd, FALSE);
    m_sound.StopHearing();

    g_settings.save(hwnd);

    DeleteCriticalSection(&g_lock);

    DeleteObject(s_hbmRec);
    DeleteObject(s_hbmPause);
    DeleteObject(s_hbmStop);
    DeleteObject(s_hbmDots);

    DeleteDC(g_hdcScreen);
    g_hdcScreen = NULL;

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
    if (HDC hdcMem = CreateCompatibleDC(g_hdcScreen))
    {
        SetStretchBltMode(hdcMem, COLORONCOLOR);
        EnterCriticalSection(&g_lock);
        HGDIOBJ hbmOld = SelectObject(hdcMem, g_hbm);
        StretchBlt(hdcMem, 0, 0, g_settings.m_nWidth, g_settings.m_nHeight,
                   g_hdcScreen,
                   g_settings.m_xCap, g_settings.m_yCap,
                   g_settings.m_cxCap, g_settings.m_cyCap,
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
        case PT_STATUSTEXT:
            break;
        case PT_SCREENCAP:
            DoScreenCap(hwnd, g_settings.m_bDrawCursor);
            break;
        case PT_VIDEOCAP:
            EnterCriticalSection(&g_lock);
            g_cap >> s_frame;
            LeaveCriticalSection(&g_lock);
            break;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    case SOUND_TIMER_ID:
        if (!g_settings.m_bNoSound)
        {
            LONG nValue = m_sound.m_nValue;
            LONG nMax = m_sound.m_nMax;
            SendDlgItemMessage(hwnd, scr1, PBM_SETRANGE32, 0, nMax);
            SendDlgItemMessage(hwnd, scr1, PBM_SETPOS, nValue, 0);
        }
        else
        {
            SendDlgItemMessage(hwnd, scr1, PBM_SETRANGE32, 0, 0);
            SendDlgItemMessage(hwnd, scr1, PBM_SETPOS, 0, 0);
        }
        break;
    }
}

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
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        ErrorBoxDx(NULL, TEXT("CoInitializeEx"));
        return EXIT_FAILURE;
    }

    InitCommonControls();
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);
    CoUninitialize();
    return EXIT_SUCCESS;
}
