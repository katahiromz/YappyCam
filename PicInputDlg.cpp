#include "YappyCam.hpp"
#include "SetDlgItemDouble/SetDlgItemDouble.h"

HWND g_hwndPictureInput = NULL;
static BOOL s_bInit = FALSE;
static HWND s_hPages[2] = { NULL };

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

static BOOL s_bPage1Init = FALSE;

static void Page1_SetData(HWND hwnd)
{
    SendDlgItemMessage(hwnd, scr1, UDM_SETPOS, 0, MAKELPARAM(g_settings.m_xCap, 0));
    SendDlgItemMessage(hwnd, scr2, UDM_SETPOS, 0, MAKELPARAM(g_settings.m_yCap, 0));
    SendDlgItemMessage(hwnd, scr3, UDM_SETPOS, 0, MAKELPARAM(g_settings.m_cxCap, 0));
    SendDlgItemMessage(hwnd, scr4, UDM_SETPOS, 0, MAKELPARAM(g_settings.m_cyCap, 0));

    if (g_settings.m_bDrawCursor)
    {
        CheckDlgButton(hwnd, chx1, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwnd, chx1, BST_UNCHECKED);
    }
}

static BOOL Page1_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    SendDlgItemMessage(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(3000, -3000));
    SendDlgItemMessage(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(3000, -3000));
    SendDlgItemMessage(hwnd, scr3, UDM_SETRANGE, 0, MAKELPARAM(3000, -3000));
    SendDlgItemMessage(hwnd, scr4, UDM_SETRANGE, 0, MAKELPARAM(3000, -3000));

    Page1_SetData(hwnd);

    s_bPage1Init = TRUE;
    return TRUE;
}

static void Page1_OnEdt(HWND hwnd)
{
    if (!s_bPage1Init)
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
        g_settings.m_cxCap = nValue;
    }

    bTranslated = FALSE;
    nValue = GetDlgItemInt(hwnd, edt4, &bTranslated, TRUE);
    if (bTranslated)
    {
        g_settings.m_cyCap = nValue;
    }
}

static void Page1_OnChx1(HWND hwnd)
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

static void Page1_OnPsh1(HWND hwnd)
{
    g_settings.m_xCap = 0;
    g_settings.m_yCap = 0;
    g_settings.m_cxCap = GetSystemMetrics(SM_CXSCREEN);
    g_settings.m_cyCap = GetSystemMetrics(SM_CYSCREEN);

    Page1_SetData(hwnd);
}

static void Page1_OnPsh2(HWND hwnd)
{
}

static void Page1_OnPsh3(HWND hwnd)
{
    if (HDC hdc = CreateDC(L"DISPLAY", NULL, NULL, NULL))
    {
        const INT PEN_WIDTH = 5;
        if (HPEN hPen = CreatePen(PS_SOLID, PEN_WIDTH, RGB(0, 255, 255)))
        {
            SetROP2(hdc, R2_XORPEN);
            SelectObject(hdc, hPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));

            for (INT i = 0; i < 4; ++i)
            {
                Rectangle(hdc,
                          g_settings.m_xCap,
                          g_settings.m_yCap,
                          g_settings.m_xCap + g_settings.m_cxCap,
                          g_settings.m_yCap + g_settings.m_cyCap);
                Sleep(200);
            }

            DeleteObject(hPen);
        }
        DeleteDC(hdc);
    }
}

static void Page1_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case edt1:
    case edt2:
    case edt3:
    case edt4:
        if (codeNotify == EN_CHANGE)
        {
            Page1_OnEdt(hwnd);
        }
        break;
    case chx1:
        if (codeNotify == BN_CLICKED)
        {
            Page1_OnChx1(hwnd);
        }
        break;
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
    case psh3:
        if (codeNotify == BN_CLICKED)
        {
            Page1_OnPsh3(hwnd);
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

static INT_PTR CALLBACK
Page2DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    }
    return 0;
}

static void DoCreatePages(HWND hwnd)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    s_hPages[0] = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SCREEN), hwnd, Page1DialogProc);
    s_hPages[1] = CreateDialog(hInst, MAKEINTRESOURCE(IDD_WEBCAMERA), hwnd, Page2DialogProc);
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

    switch (i)
    {
    case 0:
        g_settings.SetPictureType(g_hMainWnd, PT_BLACK);
        DoChoosePage(hwnd, -1);
        break;
    case 1:
        g_settings.SetPictureType(g_hMainWnd, PT_WHITE);
        DoChoosePage(hwnd, -1);
        break;
    case 2:
        g_settings.SetPictureType(g_hMainWnd, PT_SCREENCAP);
        DoChoosePage(hwnd, 0);
        break;
    case 3:
        g_settings.SetPictureType(g_hMainWnd, PT_VIDEOCAP);
        DoChoosePage(hwnd, 1);
        break;
    }

    Page1_SetData(s_hPages[0]);
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndPictureInput = hwnd;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_BLACK));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_WHITE));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_SCREEN));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_WEBCAMERA));

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
    default:
        break;
    }

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    ComboBox_AddString(hCmb2, L"0.5");
    ComboBox_AddString(hCmb2, L"1.0");
    ComboBox_AddString(hCmb2, L"2.0");
    ComboBox_AddString(hCmb2, L"3.0");
    ComboBox_AddString(hCmb2, L"4.0");
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
    DoChoosePage(hwnd, 0);

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
        if (codeNotify == CBN_SELCHANGE)
        {
            OnCmb1(hwnd);
        }
        break;
    case cmb2:
        if (codeNotify == CBN_SELCHANGE || codeNotify == CBN_EDITCHANGE)
        {
            OnCmb2(hwnd);
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

    ShowWindow(g_hwndPictureInput, SW_SHOWNORMAL);
    UpdateWindow(g_hwndPictureInput);

    return g_hwndPictureInput != NULL;
}
