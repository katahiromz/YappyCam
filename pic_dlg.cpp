#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"
#include "SetDlgItemDouble/SetDlgItemDouble.h"

////////////////////////////////////////////////////////////////////////////
// monitor info

static std::vector<MONITORINFO> s_monitors;
static MONITORINFO s_primary;

static BOOL DoGetMonitors(void)
{
    return DoGetMonitorsEx(s_monitors, s_primary);
}

////////////////////////////////////////////////////////////////////////////
// SetByDragDlgProc

static INT s_SBD_x = 0;
static INT s_SBD_y = 0;
static INT s_SBD_cx = 0;
static INT s_SBD_cy = 0;
static HBITMAP s_SBD_hbm = NULL;
static LPVOID s_SBD_pvBits = NULL;
static BOOL s_SBD_bDragging = FALSE;
static POINT s_SBD_pt[2];

static BOOL SBD_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    s_SBD_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    s_SBD_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    s_SBD_cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    s_SBD_cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    s_SBD_hbm = NULL;
    s_SBD_bDragging = FALSE;
    if (HDC hdc = CreateDC(L"DISPLAY", NULL, NULL, NULL))
    {
        if (HDC hdcMem = CreateCompatibleDC(hdc))
        {
            BITMAPINFO bi;
            ZeroMemory(&bi, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth = s_SBD_cx;
            bi.bmiHeader.biHeight = s_SBD_cy;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            s_SBD_hbm = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, &s_SBD_pvBits, NULL, 0);
            HGDIOBJ hbmOld = SelectObject(hdcMem, s_SBD_hbm);
            BitBlt(hdcMem, 0, 0, s_SBD_cx, s_SBD_cy,
                   hdc, s_SBD_x, s_SBD_y, SRCCOPY | CAPTUREBLT);
            SelectObject(hdcMem, hbmOld);

            LPDWORD pdwBits = (LPDWORD)s_SBD_pvBits;
            for (INT y = 0; y < s_SBD_cy; ++y)
            {
                BOOL b = (y & 1);
                for (INT x = 0; x < s_SBD_cx; ++x)
                {
                    if (b)
                        *pdwBits = 0;
                    b = !b;
                    ++pdwBits;
                }
            }

            LOGFONT lf;
            HFONT hFont = GetStockFont(DEFAULT_GUI_FONT);
            GetObject(hFont, sizeof(lf), &lf);
            lf.lfHeight = -24;
            lf.lfWeight = FW_BOLD;
            hFont = CreateFontIndirect(&lf);
            if (hFont)
            {
                HGDIOBJ hFontOld = SelectObject(hdcMem, hFont);
                const UINT uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
                hbmOld = SelectObject(hdcMem, s_SBD_hbm);
                for (auto& monitor : s_monitors)
                {
                    SetBkMode(hdcMem, OPAQUE);
                    SetTextColor(hdcMem, RGB(0, 0, 0));
                    SetBkColor(hdcMem, RGB(255, 192, 192));
                    DrawText(hdcMem, LoadStringDx(IDS_DRAGONME), -1,
                             &monitor.rcMonitor, uFormat);
                }
                SelectObject(hdcMem, hbmOld);
                SelectObject(hdcMem, hFontOld);
                DeleteObject(hFont);
            }

            DeleteDC(hdcMem);
        }
        DeleteDC(hdc);
    }
    MoveWindow(hwnd, s_SBD_x, s_SBD_y, s_SBD_cx, s_SBD_cy, TRUE);
    return TRUE;
}

static void SBD_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    }
}

static BOOL SBD_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    return TRUE;
}

static void SBD_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    if (HDC hdc = BeginPaint(hwnd, &ps))
    {
        if (HDC hdcMem = CreateCompatibleDC(hdc))
        {
            HGDIOBJ hbmOld = SelectObject(hdcMem, s_SBD_hbm);
            BitBlt(hdc, s_SBD_x, s_SBD_y, s_SBD_cx, s_SBD_cy, hdcMem, 0, 0, SRCCOPY);
            
            if (s_SBD_bDragging)
            {
#undef min
                INT x = std::min(s_SBD_pt[0].x, s_SBD_pt[1].x);
                INT y = std::min(s_SBD_pt[0].y, s_SBD_pt[1].y);
                INT cx = abs(s_SBD_pt[1].x - s_SBD_pt[0].x);
                INT cy = abs(s_SBD_pt[1].y - s_SBD_pt[0].y);
                if (HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0)))
                {
                    if (HBRUSH hbr = CreateSolidBrush(RGB(192, 0, 0)))
                    {
                        HGDIOBJ hPenOld = SelectObject(hdc, hPen);
                        HGDIOBJ hbrOld = SelectObject(hdc, hbr);
                        {
                            Rectangle(hdc, x, y, x + cx, y + cy);
                        }
                        SelectObject(hdc, hbrOld);
                        SelectObject(hdc, hPenOld);
                        DeleteObject(hbr);
                    }
                    DeleteObject(hPen);
                }
            }
            SelectObject(hdcMem, hbmOld);
            DeleteDC(hdcMem);
        }
        EndPaint(hwnd, &ps);
    }
}

static void SBD_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    GetCursorPos(&s_SBD_pt[0]);
    s_SBD_pt[1] = s_SBD_pt[0];
    SetCapture(hwnd);
    s_SBD_bDragging = TRUE;
}

static void SBD_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (s_SBD_bDragging)
    {
        GetCursorPos(&s_SBD_pt[1]);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

static void SBD_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (s_SBD_bDragging)
    {
        GetCursorPos(&s_SBD_pt[1]);
        InvalidateRect(hwnd, NULL, TRUE);
    }
    EndDialog(hwnd, IDOK);
}

static void SBD_OnDestroy(HWND hwnd)
{
    DeleteObject(s_SBD_hbm);
    s_SBD_hbm = NULL;
}

static void SBD_OnRButtonUp(HWND hwnd, int x, int y, UINT flags)
{
    EndDialog(hwnd, IDCANCEL);
}

static INT_PTR CALLBACK
SetByDragDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, SBD_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, SBD_OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, SBD_OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, SBD_OnPaint);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, SBD_OnLButtonDown);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE, SBD_OnMouseMove);
        HANDLE_MSG(hwnd, WM_LBUTTONUP, SBD_OnLButtonUp);
        HANDLE_MSG(hwnd, WM_RBUTTONUP, SBD_OnRButtonUp);
        case WM_CAPTURECHANGED:
        case WM_CANCELMODE:
            EndDialog(hwnd, IDCANCEL);
            break;
        HANDLE_MSG(hwnd, WM_DESTROY, SBD_OnDestroy);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Page0DialogProc

static BOOL s_bPage0Init = FALSE;

static void Page0_SetData(HWND hwnd)
{
    s_bPage0Init = FALSE;

    SetDlgItemInt(hwnd, edt1, g_settings.m_xCap, TRUE);
    SetDlgItemInt(hwnd, edt2, g_settings.m_yCap, TRUE);
    SetDlgItemInt(hwnd, edt3, g_settings.m_xCap + g_settings.m_cxCap, TRUE);
    SetDlgItemInt(hwnd, edt4, g_settings.m_yCap + g_settings.m_cyCap, TRUE);

    if (g_settings.m_bDrawCursor)
    {
        CheckDlgButton(hwnd, chx1, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwnd, chx1, BST_UNCHECKED);
    }

    if (g_settings.m_bFollowChange)
    {
        CheckDlgButton(hwnd, chx2, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwnd, chx2, BST_UNCHECKED);
    }

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_SetCurSel(hCmb1, g_settings.m_nMonitorID);

    s_bPage0Init = TRUE;
}

static BOOL Page0_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    DoGetMonitors();

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    // g_settings.m_nMonitorID == 0: primary monitor.
    // g_settings.m_nMonitorID == 1: virtual screen.
    // g_settings.m_nMonitorID == 2: Monitor #0.
    // g_settings.m_nMonitorID == 3: Monitor #1.
    // ...

    ComboBox_AddString(hCmb1, LoadStringDx(IDS_PRIMARYMONITOR));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_VIRTUALSCREEN));

    TCHAR szText[64];
    INT iMonitor = 0;
    for (auto& info : s_monitors)
    {
        StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_MONITOR), iMonitor);
        ComboBox_AddString(hCmb1, szText);
        ++iMonitor;
    }
    ComboBox_SetCurSel(hCmb1, g_settings.m_nMonitorID);

    SendDlgItemMessage(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(32767, -32768));
    SendDlgItemMessage(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(32767, -32768));
    SendDlgItemMessage(hwnd, scr3, UDM_SETRANGE, 0, MAKELPARAM(32767, -32768));
    SendDlgItemMessage(hwnd, scr4, UDM_SETRANGE, 0, MAKELPARAM(32767, -32768));

    Page0_SetData(hwnd);

    s_bPage0Init = TRUE;
    return TRUE;
}

static void Page0_SetMonitorID(HWND hwnd, INT i)
{
    if (i < 0 || i >= INT(s_monitors.size() + 2))
        return;

    switch (i)
    {
    case 0:
        {
            auto& rc = s_primary.rcMonitor;
            g_settings.m_xCap = rc.left;
            g_settings.m_yCap = rc.top;
            g_settings.m_cxCap = rc.right - rc.left;
            g_settings.m_cyCap = rc.bottom - rc.top;
        }
        break;
    case 1:
        g_settings.m_xCap = GetSystemMetrics(SM_XVIRTUALSCREEN);
        g_settings.m_yCap = GetSystemMetrics(SM_YVIRTUALSCREEN);
        g_settings.m_cxCap = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        g_settings.m_cyCap = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        break;
    default:
        {
            auto& rc = s_monitors[i - 2].rcMonitor;
            g_settings.m_xCap = rc.left;
            g_settings.m_yCap = rc.top;
            g_settings.m_cxCap = rc.right - rc.left;
            g_settings.m_cyCap = rc.bottom - rc.top;
        }
        break;
    }

    g_settings.m_nMonitorID = i;
    g_settings.m_nWidth = g_settings.m_cxCap;
    g_settings.m_nHeight = g_settings.m_cyCap;

    Page0_SetData(hwnd);
    g_settings.update(g_hMainWnd);
}

static void Page0_OnCmb1(HWND hwnd)
{
    if (!s_bPage0Init)
        return;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    INT i = ComboBox_GetCurSel(hCmb1);
    Page0_SetMonitorID(hwnd, i);
}

static void Page0_OnEdt(HWND hwnd)
{
    if (!s_bPage0Init)
        return;

    BOOL bTranslated;
    INT nValue;

    bTranslated = FALSE;
    nValue = GetDlgItemInt(hwnd, edt1, &bTranslated, TRUE);
    if (bTranslated)
    {
        g_settings.m_xCap = nValue;
    }

    bTranslated = FALSE;
    nValue = GetDlgItemInt(hwnd, edt2, &bTranslated, TRUE);
    if (bTranslated)
    {
        g_settings.m_yCap = nValue;
    }

    bTranslated = FALSE;
    nValue = GetDlgItemInt(hwnd, edt3, &bTranslated, TRUE);
    if (bTranslated)
    {
        g_settings.m_cxCap = nValue - g_settings.m_xCap;
    }

    bTranslated = FALSE;
    nValue = GetDlgItemInt(hwnd, edt4, &bTranslated, TRUE);
    if (bTranslated)
    {
        g_settings.m_cyCap = nValue - g_settings.m_yCap;
    }

    g_settings.m_nWidth = g_settings.m_cxCap;
    g_settings.m_nHeight = g_settings.m_cyCap;
    g_settings.update(g_hMainWnd);
}

static void Page0_OnChx1(HWND hwnd)
{
    if (IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED)
    {
        g_settings.m_bDrawCursor = TRUE;
    }
    else
    {
        g_settings.m_bDrawCursor = FALSE;
    }
}

static void Page0_OnChx2(HWND hwnd)
{
    if (IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED)
    {
        g_settings.m_bFollowChange = TRUE;
    }
    else
    {
        g_settings.m_bFollowChange = FALSE;
    }
}

static void Page0_OnPsh1(HWND hwnd)
{
    Page0_SetMonitorID(hwnd, g_settings.m_nMonitorID);
}

static void Page0_OnPsh2(HWND hwnd)
{
    INT_PTR nID = DialogBox(GetModuleHandle(NULL),
                            MAKEINTRESOURCE(IDD_SETBYDRAG),
                            NULL, SetByDragDlgProc);
    if (nID == IDOK)
    {
        INT x = std::min(s_SBD_pt[0].x, s_SBD_pt[1].x);
        INT y = std::min(s_SBD_pt[0].y, s_SBD_pt[1].y);
        INT cx = abs(s_SBD_pt[1].x - s_SBD_pt[0].x);
        INT cy = abs(s_SBD_pt[1].y - s_SBD_pt[0].y);
        g_settings.m_xCap = x;
        g_settings.m_yCap = y;
        g_settings.m_cxCap = cx;
        g_settings.m_cyCap = cy;
        Page0_SetData(hwnd);
        g_settings.update(g_hMainWnd);
    }
}

static void Page0_OnPsh3(HWND hwnd)
{
    if (HDC hdc = CreateDC(L"DISPLAY", NULL, NULL, NULL))
    {
        const INT PEN_WIDTH = 5;
        if (HPEN hPen = CreatePen(PS_SOLID, PEN_WIDTH, RGB(0, 255, 255)))
        {
            SetROP2(hdc, R2_XORPEN);
            SelectObject(hdc, hPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));

            timeEndPeriod(TIME_PERIOD);

            for (INT i = 0; i < 4; ++i)
            {
                Rectangle(hdc,
                          g_settings.m_xCap,
                          g_settings.m_yCap,
                          g_settings.m_xCap + g_settings.m_cxCap,
                          g_settings.m_yCap + g_settings.m_cyCap);
                Sleep(200);
            }

            timeBeginPeriod(TIME_PERIOD);

            DeleteObject(hPen);
        }
        DeleteDC(hdc);
    }
}

static void Page0_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case cmb1:
        Page0_OnCmb1(hwnd);
        break;
    case edt1:
    case edt2:
    case edt3:
    case edt4:
        if (codeNotify == EN_CHANGE)
        {
            Page0_OnEdt(hwnd);
        }
        break;
    case chx1:
        if (codeNotify == BN_CLICKED)
        {
            Page0_OnChx1(hwnd);
        }
        break;
    case chx2:
        if (codeNotify == BN_CLICKED)
        {
            Page0_OnChx2(hwnd);
        }
        break;
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            Page0_OnPsh1(hwnd);
        }
        break;
    case psh2:
        if (codeNotify == BN_CLICKED)
        {
            Page0_OnPsh2(hwnd);
        }
        break;
    case psh3:
        if (codeNotify == BN_CLICKED)
        {
            Page0_OnPsh3(hwnd);
        }
        break;
    }
}

static void Page0_OnDestroy(HWND hwnd)
{
    s_bPage0Init = FALSE;
}

static INT_PTR CALLBACK
Page0DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Page0_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Page0_OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, Page0_OnDestroy);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Page1DialogProc

static BOOL s_bPage1Init = FALSE;

static BOOL Page1_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    TCHAR szText[64];
    for (INT i = 0; i < 10; ++i)
    {
        StringCbPrintf(szText, sizeof(szText), LoadStringDx(IDS_CAMERA), i);
        ComboBox_AddString(hCmb1, szText);
    }
    ComboBox_SetCurSel(hCmb1, g_settings.m_nCameraID);

    SendDlgItemMessage(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(500, -500));
    SendDlgItemMessage(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(500, -500));

    SendDlgItemMessage(hwnd, scr1, UDM_SETPOS, 0, MAKELPARAM(g_settings.m_nBrightness, 0));
    SendDlgItemMessage(hwnd, scr2, UDM_SETPOS, 0, MAKELPARAM(g_settings.m_nContrast, 0));

    SetDlgItemInt(hwnd, edt3, g_settings.m_nWidth, TRUE);
    SetDlgItemInt(hwnd, edt4, g_settings.m_nHeight, TRUE);

    s_bPage1Init = TRUE;
    return TRUE;
}

static void Page1_OnCmb1(HWND hwnd)
{
    if (!s_bPage1Init)
        return;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    g_settings.m_nCameraID = ComboBox_GetCurSel(hCmb1);

    g_settings.update(g_hMainWnd);

    SetDlgItemInt(hwnd, edt3, g_settings.m_nWidth, TRUE);
    SetDlgItemInt(hwnd, edt4, g_settings.m_nHeight, TRUE);
}

static void Page1_OnEdt1(HWND hwnd)
{
    if (!s_bPage1Init)
        return;

    BOOL bTranslated = FALSE;
    INT nValue = GetDlgItemInt(hwnd, edt1, &bTranslated, TRUE);

    if (bTranslated)
    {
        g_settings.m_nBrightness = nValue;
    }
}

static void Page1_OnEdt2(HWND hwnd)
{
    if (!s_bPage1Init)
        return;

    BOOL bTranslated = FALSE;
    INT nValue = GetDlgItemInt(hwnd, edt2, &bTranslated, TRUE);

    if (bTranslated)
    {
        g_settings.m_nContrast = nValue;
    }
}

static void Page1_OnPsh1(HWND hwnd)
{
    SetDlgItemInt(hwnd, edt1, 0, TRUE);
    Page1_OnEdt1(hwnd);
}

static void Page1_OnPsh2(HWND hwnd)
{
    SetDlgItemInt(hwnd, edt2, 100, TRUE);
    Page1_OnEdt2(hwnd);
}

static void Page1_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            Page1_OnPsh1(hwnd);
        }
        break;
    case psh2:
        if (codeNotify == BN_CLICKED)
        {
            Page1_OnPsh2(hwnd);
        }
        break;
    case cmb1:
        switch (codeNotify)
        {
        case CBN_DROPDOWN:
        case CBN_CLOSEUP:
        case CBN_SETFOCUS:
        case CBN_EDITUPDATE:
        case CBN_SELENDCANCEL:
            break;
        default:
            Page1_OnCmb1(hwnd);
            break;
        }
        break;
    case edt1:
        if (codeNotify == EN_CHANGE)
        {
            Page1_OnEdt1(hwnd);
        }
        break;
    case edt2:
        if (codeNotify == EN_CHANGE)
        {
            Page1_OnEdt2(hwnd);
        }
        break;
    }
}

static void Page1_OnDestroy(HWND hwnd)
{
    s_bPage1Init = FALSE;
}

static INT_PTR CALLBACK
Page1DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Page1_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Page1_OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, Page1_OnDestroy);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Page2DialogProc

static BOOL s_bPage2Init = FALSE;

static void Page2_DoUpdateInput(HWND hwnd)
{
    g_settings.update(g_hMainWnd);
}

static BOOL Page2_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    DragAcceptFiles(hwnd, TRUE);
    SetDlgItemText(hwnd, cmb1, g_settings.m_strInputFileName.c_str());

    s_bPage2Init = TRUE;
    return TRUE;
}

static void Page2_OnCmb1(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    TCHAR szText[MAX_PATH];
    INT i = ComboBox_GetCurSel(hCmb1);
    if (i == CB_ERR)
    {
        ComboBox_GetText(hCmb1, szText, ARRAYSIZE(szText));
    }
    else
    {
        ComboBox_GetLBText(hCmb1, i, szText);
    }

    StrTrim(szText, TEXT(" \t"));

    DWORD attrs = GetFileAttributes(szText);
    if (attrs != 0xFFFFFFFF && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        g_settings.m_strInputFileName = szText;
        Page2_DoUpdateInput(hwnd);
    }
}

static void Page2_OnPsh1(HWND hwnd)
{
    TCHAR szFile[MAX_PATH];
    StringCbCopy(szFile, sizeof(szFile), g_settings.m_strInputFileName.c_str());

    OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    ofn.lpstrTitle = L"Input Image File";
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING |
                OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"jpg";
    if (GetOpenFileName(&ofn))
    {
        g_settings.m_strInputFileName = szFile;
        SetDlgItemText(hwnd, cmb1, szFile);
        Page2_DoUpdateInput(hwnd);
    }
}

static void Page2_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            Page2_OnPsh1(hwnd);
        }
        break;
    case cmb1:
        switch (codeNotify)
        {
        case CBN_DROPDOWN:
        case CBN_CLOSEUP:
        case CBN_SETFOCUS:
        case CBN_EDITUPDATE:
        case CBN_SELENDCANCEL:
            break;
        default:
            Page2_OnCmb1(hwnd);
            break;
        }
        break;
    }
}

static void Page2_OnDestroy(HWND hwnd)
{
    s_bPage2Init = FALSE;
}

static void Page2_OnDropFiles(HWND hwnd, HDROP hdrop)
{
    TCHAR szPath[MAX_PATH];
    DragQueryFile(hdrop, 0, szPath, ARRAYSIZE(szPath));
    DragFinish(hdrop);

    DWORD attrs = GetFileAttributes(szPath);
    if (attrs != 0xFFFFFFFF && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        g_settings.m_strInputFileName = szPath;
        SetDlgItemText(hwnd, cmb1, szPath);
        Page2_DoUpdateInput(hwnd);
    }
}

static INT_PTR CALLBACK
Page2DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Page2_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Page2_OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, Page2_OnDestroy);
        HANDLE_MSG(hwnd, WM_DROPFILES, Page2_OnDropFiles);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// DoPictureInputDialogBox

HWND g_hwndPictureInput = NULL;
static BOOL s_bInit = FALSE;
static HWND s_hPages[3] = { NULL };

static void DoChoosePage(HWND hwnd, INT iPage)
{
    for (INT i = 0; i < ARRAYSIZE(s_hPages); ++i)
    {
        if (i == iPage)
            ShowWindow(s_hPages[i], SW_SHOW);
        else
            ShowWindow(s_hPages[i], SW_HIDE);
    }
}

void DoAdjustPages(HWND hwnd)
{
    HWND hStc1 = GetDlgItem(hwnd, stc1);

    RECT rc;
    GetWindowRect(hStc1, &rc);
    MapWindowRect(NULL, hwnd, &rc);

    for (INT i = 0; i < ARRAYSIZE(s_hPages); ++i)
    {
        MoveWindow(s_hPages[i], rc.left, rc.top,
                   rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }
}

static void DoCreatePages(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    s_hPages[0] = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SCREEN), hwnd, Page0DialogProc);
    s_hPages[1] = CreateDialog(hInst, MAKEINTRESOURCE(IDD_WEBCAMERA), hwnd, Page1DialogProc);
    s_hPages[2] = CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMAGEFILE), hwnd, Page2DialogProc);
}

static void DoDestroyPages(HWND hwnd)
{
    for (INT i = 0; i < ARRAYSIZE(s_hPages); ++i)
    {
        DestroyWindow(s_hPages[i]);
        s_hPages[i]  = NULL;
    }
}

static void OnCmb1(HWND hwnd)
{
    if (!s_bInit)
        return;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    INT i = ComboBox_GetCurSel(hCmb1);
    if (i < 0)
        return;

    PictureType type;
    switch (i)
    {
    case 0:
        type = PT_BLACK;
        DoChoosePage(hwnd, -1);
        break;
    case 1:
        type = PT_WHITE;
        DoChoosePage(hwnd, -1);
        break;
    case 2:
        type = PT_SCREENCAP;
        DoChoosePage(hwnd, 0);
        break;
    case 3:
        type = PT_VIDEOCAP;
        DoChoosePage(hwnd, 1);
        break;
    case 4:
        type = PT_IMAGEFILE;
        DoChoosePage(hwnd, 2);
    }

    s_bInit = FALSE;
    Page0_SetData(s_hPages[0]);
    s_bInit = TRUE;

    g_settings.update(g_hMainWnd, type);
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndPictureInput = hwnd;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_BLACK));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_WHITE));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_SCREEN));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_WEBCAMERA));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_IMAGEFILE));

    switch (g_settings.GetPictureType())
    {
    case PT_BLACK:
        ComboBox_SetCurSel(hCmb1, 0);
        break;
    case PT_WHITE:
        ComboBox_SetCurSel(hCmb1, 1);
        break;
    case PT_SCREENCAP:
        ComboBox_SetCurSel(hCmb1, 2);
        break;
    case PT_VIDEOCAP:
        ComboBox_SetCurSel(hCmb1, 3);
        break;
    case PT_IMAGEFILE:
        ComboBox_SetCurSel(hCmb1, 4);
        break;
    default:
        EndDialog(hwnd, IDCLOSE);
        return FALSE;
    }

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    ComboBox_AddString(hCmb2, L"0.5");
    ComboBox_AddString(hCmb2, L"1.0");
    ComboBox_AddString(hCmb2, L"2.0");
    ComboBox_AddString(hCmb2, L"4.0");
    ComboBox_AddString(hCmb2, L"8.0");
    ComboBox_AddString(hCmb2, L"12.0");
    ComboBox_AddString(hCmb2, L"14.0");
    ComboBox_AddString(hCmb2, L"16.0");
    ComboBox_AddString(hCmb2, L"18.0");
    ComboBox_AddString(hCmb2, L"20.0");
    SetDlgItemDouble(hwnd, cmb2, (g_settings.m_nFPSx100 / 100.0), "%0.1f");

    INT x = g_settings.m_nPicDlgX;
    INT y = g_settings.m_nPicDlgY;
    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        SendMessage(hwnd, DM_REPOSITION, 0, 0);
    }

    DoCreatePages(hwnd);
    DoAdjustPages(hwnd);

    switch (g_settings.GetPictureType())
    {
    case PT_BLACK:
        DoChoosePage(hwnd, -1);
        break;
    case PT_WHITE:
        DoChoosePage(hwnd, -1);
        break;
    case PT_SCREENCAP:
        DoChoosePage(hwnd, 0);
        break;
    case PT_VIDEOCAP:
        DoChoosePage(hwnd, 1);
        break;
    case PT_IMAGEFILE:
        DoChoosePage(hwnd, 2);
        break;
    default:
        break;
    }

    s_bInit = TRUE;

    return TRUE;
}

static void OnCmb2(HWND hwnd)
{
    if (!s_bInit)
        return;

    s_bInit = FALSE;

    TCHAR szText[MAX_PATH], *endptr = NULL;

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    INT i = ComboBox_GetCurSel(hCmb2);
    if (i < 0)
    {
        ComboBox_GetText(hCmb2, szText, ARRAYSIZE(szText));
    }
    else
    {
        ComboBox_GetLBText(hCmb2, i, szText);
    }

    double e = std::wcstod(szText, &endptr);
    BOOL bTranslated = endptr - szText == wcslen(szText);

    if (bTranslated)
    {
        if (e < MIN_FPS)
        {
            e = MIN_FPS;
        }
        else if (e > MAX_FPS)
        {
            e = MAX_FPS;
        }

        g_settings.m_nFPSx100 = UINT(e * 100 + 0.005);

        DoStartStopTimers(g_hMainWnd, FALSE);
        DoStartStopTimers(g_hMainWnd, TRUE);
    }

    s_bInit = TRUE;
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    case cmb1:
        switch (codeNotify)
        {
        case CBN_DROPDOWN:
        case CBN_CLOSEUP:
        case CBN_SETFOCUS:
        case CBN_EDITUPDATE:
        case CBN_SELENDCANCEL:
            break;
        default:
            OnCmb1(hwnd);
            break;
        }
        break;
    case cmb2:
        switch (codeNotify)
        {
        case CBN_DROPDOWN:
        case CBN_CLOSEUP:
        case CBN_SETFOCUS:
        case CBN_EDITUPDATE:
        case CBN_SELENDCANCEL:
            break;
        default:
            OnCmb2(hwnd);
            break;
        }
        break;
    }
}

static void OnDestroy(HWND hwnd)
{
    s_bInit = FALSE;
    g_hwndPictureInput = NULL;

    DoDestroyPages(hwnd);

    PostMessage(g_hMainWnd, WM_COMMAND, ID_CONFIGCLOSED, 0);
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    g_settings.m_nPicDlgX = rc.left;
    g_settings.m_nPicDlgY = rc.top;
}

static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_MOVE, OnMove);
    }
    return 0;
}

BOOL DoPictureInputDialogBox(HWND hwndParent)
{
    if (g_hwndPictureInput)
    {
        SendMessage(g_hwndPictureInput, DM_REPOSITION, 0, 0);
        SetForegroundWindow(g_hwndPictureInput);
        return TRUE;
    }

    CreateDialog(GetModuleHandle(NULL),
                 MAKEINTRESOURCE(IDD_PICINPUT),
                 hwndParent,
                 DialogProc);
    assert(g_hwndPictureInput);
    if (g_hwndPictureInput)
    {
        ShowWindow(g_hwndPictureInput, SW_SHOWNORMAL);
        UpdateWindow(g_hwndPictureInput);

        return TRUE;
    }
    return FALSE;
}
