#include "YappyCam.hpp"
#include "SetDlgItemDouble/SetDlgItemDouble.h"

#define MIN_FPS 0.1
#define MAX_FPS 7.0

HWND g_hwndPictureInput = NULL;
static BOOL s_bInit = FALSE;

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
        break;
    case 1:
        g_settings.SetPictureType(g_hMainWnd, PT_WHITE);
        break;
    case 2:
        g_settings.SetPictureType(g_hMainWnd, PT_SCREENCAP);
        break;
    case 3:
        g_settings.SetPictureType(g_hMainWnd, PT_VIDEOCAP);
        break;
    }
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndPictureInput = hwnd;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, L"Black");
    ComboBox_AddString(hCmb1, L"White");
    ComboBox_AddString(hCmb1, L"Screen");
    ComboBox_AddString(hCmb1, L"Web Camera");

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

    PostMessage(g_hMainWnd, WM_COMMAND, ID_CONFIGCLOSED, 0);
}

static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
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
