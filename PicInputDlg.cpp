#include "YappyCam.hpp"
#include "SetDlgItemDouble/SetDlgItemDouble.h"

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

    SetDlgItemDouble(hwnd, edt1, (g_settings.m_nFPSx100 / 100.0), "%0.2f");

    s_bInit = TRUE;

    return TRUE;
}

static void OnEdt1(HWND hwnd)
{
    if (!s_bInit)
        return;

    s_bInit = FALSE;
    BOOL bTranslated = FALSE;
    double e = GetDlgItemDouble(hwnd, edt1, &bTranslated);
    if (bTranslated)
    {
        if (e < 0.01)
        {
            e = 0.01;
        }
        else if (e > 8.00)
        {
            e = 8.00;
        }

        g_settings.m_nFPSx100 = UINT(e * 100 + 0.005);
        s_bInit = TRUE;
        DoStartStopTimers(g_hMainWnd, FALSE);
        DoStartStopTimers(g_hMainWnd, TRUE);
    }
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
    case edt1:
        if (codeNotify == EN_CHANGE)
        {
            OnEdt1(hwnd);
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
