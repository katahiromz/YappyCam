// Noize_yap.cpp --- PluginFramework Plugin #2
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "../../Plugin.h"
#include "../mregkey.hpp"
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <cassert>
#include <strsafe.h>
#include "resource.h"

static HINSTANCE s_hinstDLL;
static PLUGIN *s_pi;
static INT s_nWindowX;
static INT s_nWindowY;
static INT s_nNoizeFactor;
static BOOL s_bDialogInit = FALSE;

LPTSTR LoadStringDx(INT nID)
{
    static UINT s_index = 0;
    const UINT cchBuffMax = 1024;
    static TCHAR s_sz[2][cchBuffMax];

    TCHAR *pszBuff = s_sz[s_index];
    s_index = (s_index + 1) % ARRAYSIZE(s_sz);
    pszBuff[0] = 0;
    if (!::LoadString(s_hinstDLL, nID, pszBuff, cchBuffMax))
        assert(0);
    return pszBuff;
}

LPSTR ansi_from_wide(LPCWSTR pszWide)
{
    static char s_buf[256];
    WideCharToMultiByte(CP_ACP, 0, pszWide, -1, s_buf, ARRAYSIZE(s_buf), NULL, NULL);
    return s_buf;
}

LPWSTR wide_from_ansi(LPCSTR pszAnsi)
{
    static WCHAR s_buf[256];
    MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, s_buf, ARRAYSIZE(s_buf));
    return s_buf;
}

extern "C" {

static LRESULT DoResetSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    s_nWindowX = CW_USEDEFAULT;
    s_nWindowY = CW_USEDEFAULT;
    s_nNoizeFactor = 256;
    pi->bEnabled = FALSE;
    pi->dwInfo = PLUGIN_INFO_PASS;
    pi->dwState = PLUGIN_STATE_PASS1;
    pi->p_user_data = NULL;
    pi->l_user_data = 0;
    return 0;
}

static LRESULT DoLoadSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    DoResetSettings(pi, wParam, lParam);

    MRegKey hkeyCompany(HKEY_CURRENT_USER,
                        TEXT("Software\\Katayama Hirofumi MZ"),
                        FALSE);
    if (!hkeyCompany)
        return FALSE;

    MRegKey hkeyApp(hkeyCompany, TEXT("Noize_yap"), FALSE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.QueryDword(TEXT("WindowX"), (DWORD&)s_nWindowX);
    hkeyApp.QueryDword(TEXT("WindowY"), (DWORD&)s_nWindowY);
    hkeyApp.QueryDword(TEXT("Factor"), (DWORD&)s_nNoizeFactor);

    return TRUE;
}

static LRESULT DoSaveSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    MRegKey hkeyCompany(HKEY_CURRENT_USER,
                        TEXT("Software\\Katayama Hirofumi MZ"),
                        TRUE);
    if (!hkeyCompany)
        return FALSE;

    MRegKey hkeyApp(hkeyCompany, TEXT("Noize_yap"), TRUE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.SetDword(TEXT("WindowX"), s_nWindowX);
    hkeyApp.SetDword(TEXT("WindowY"), s_nWindowY);
    hkeyApp.SetDword(TEXT("Factor"), s_nNoizeFactor);

    return TRUE;
}

// API Name: Plugin_Load
// Purpose: The framework want to load the plugin component.
// TODO: Load the plugin component.
BOOL APIENTRY
Plugin_Load(PLUGIN *pi, LPARAM lParam)
{
    if (!pi)
    {
        assert(0);
        return FALSE;
    }
    if (pi->framework_version < FRAMEWORK_VERSION)
    {
        assert(0);
        return FALSE;
    }
    if (lstrcmpi(pi->framework_name, FRAMEWORK_NAME) != 0)
    {
        assert(0);
        return FALSE;
    }
    if (pi->framework_instance == NULL)
    {
        assert(0);
        return FALSE;
    }

    pi->plugin_version = 2;
    StringCbCopy(pi->plugin_product_name, sizeof(pi->plugin_product_name), LoadStringDx(IDS_TITLE));
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("Noize.yap"));
    StringCbCopy(pi->plugin_company, sizeof(pi->plugin_company), TEXT("Katayama Hirofumi MZ"));
    StringCbCopy(pi->plugin_copyright, sizeof(pi->plugin_copyright), TEXT("Copyright (C) 2019 Katayama Hirofumi MZ"));
    pi->plugin_instance = s_hinstDLL;
    pi->plugin_window = NULL;
    DoLoadSettings(pi, 0, 0);

    s_pi = pi;

    return TRUE;
}

// API Name: Plugin_Unload
// Purpose: The framework want to unload the plugin component.
// TODO: Unload the plugin component.
BOOL APIENTRY
Plugin_Unload(PLUGIN *pi, LPARAM lParam)
{
    DoSaveSettings(pi, 0, 0);
    s_pi = NULL;
    return TRUE;
}

static LRESULT Plugin_DoPass(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    cv::Mat *pmat = (cv::Mat *)wParam;
    if (!pmat || !pmat->data)
        return TRUE;

    cv::Mat& mat = *pmat;
    int cols = mat.cols;
    int rows = mat.rows;

    int offset = 0;
    for (int y = 0; y < rows; ++y)
    {
        for (int i = 0; i < s_nNoizeFactor; ++i)
        {
            int k = rand() % (mat.elemSize() * cols);
            mat.data[offset + k] = (BYTE)rand();
        }
        offset += mat.step;
    }

    return TRUE;
}

#define MIN_VALUE 1
#define MAX_VALUE 512

static LRESULT Plugin_Enable(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case 0:
        pi->bEnabled = FALSE;
        return TRUE;
    case 1:
        pi->bEnabled = TRUE;
        return TRUE;
    case 2:
        pi->bEnabled = !pi->bEnabled;
        return TRUE;
    }
    return FALSE;
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    s_pi->plugin_window = hwnd;

    SendDlgItemMessage(hwnd, scr1, UDM_SETRANGE, 0, MAKELONG(MAX_VALUE, MIN_VALUE));
    SendDlgItemMessage(hwnd, scr1, UDM_SETPOS, 0, s_nNoizeFactor);

    s_bDialogInit = TRUE;
    return TRUE;
}

static void OnEdt1(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    BOOL bTranslated = FALSE;
    INT nValue = GetDlgItemInt(hwnd, edt1, &bTranslated, TRUE);

    if (bTranslated && MIN_VALUE <= nValue && nValue <= MAX_VALUE)
    {
        s_nNoizeFactor = nValue;
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
    s_pi->plugin_window = NULL;
    s_bDialogInit = FALSE;
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMinimized(hwnd) || IsMaximized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    s_nWindowX = rc.left;
    s_nWindowY = rc.top;
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

static LRESULT Plugin_ShowDialog(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    HWND hMainWnd = (HWND)wParam;
    BOOL bShowOrHide = (BOOL)lParam;

    s_pi = pi;

    if (bShowOrHide)
    {
        if (IsWindow(pi->plugin_window))
        {
            HWND hPlugin = pi->plugin_window;
            ShowWindow(hPlugin, SW_RESTORE);
            PostMessage(hPlugin, DM_REPOSITION, 0, 0);
            SetForegroundWindow(hPlugin);
            return TRUE;
        }
        else
        {
            CreateDialog(s_hinstDLL, MAKEINTRESOURCE(IDD_CONFIG), hMainWnd, DialogProc);
            if (pi->plugin_window)
            {
                ShowWindow(pi->plugin_window, SW_SHOWNORMAL);
                UpdateWindow(pi->plugin_window);
                return TRUE;
            }
        }
    }
    else
    {
        PostMessage(pi->plugin_window, WM_CLOSE, 0, 0);
        return TRUE;
    }

    return FALSE;
}

static LRESULT Plugin_Refresh(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    s_pi = pi;

    if (BOOL bResetSettings = (BOOL)wParam)
    {
        DoResetSettings(pi, 0, 0);
    }
    return TRUE;
}

LRESULT Plugin_SetState(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    DWORD dwState = (DWORD)wParam;
    DWORD dwStateMask = (DWORD)lParam;

    if (dwStateMask == 0)
        return pi->dwState;

    if (dwStateMask & ~(PLUGIN_STATE_PASS1 | PLUGIN_STATE_PASS2))
        return FALSE;

    pi->dwState &= ~dwStateMask;
    pi->dwState |= (dwState & dwStateMask);

    return TRUE;
}

LRESULT Plugin_DoBang(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    INT nBangID = (INT)wParam;
    switch (nBangID)
    {
    case BANGID_ENABLE:
        pi->bEnabled = TRUE;
        return TRUE;
    case BANGID_DISABLE:
        pi->bEnabled = FALSE;
        return TRUE;
    case BANGID_TOGGLE:
        pi->bEnabled = !pi->bEnabled;
        return TRUE;
    default:
        return FALSE;
    }
}

LRESULT Plugin_GetBangList(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    static const BANG_INFO s_list[] =
    {
        { BANGID_ENABLE, L"ON" },
        { BANGID_DISABLE, L"OFF" },
        { BANGID_TOGGLE, L"ON/OFF" },
    };

    if (LPINT pnBangCount = (LPINT)wParam)
        *pnBangCount = ARRAYSIZE(s_list);

    return (LRESULT)s_list;
}

// API Name: Plugin_Act
// Purpose: Act something on the plugin.
// TODO: Act something on the plugin.
LRESULT APIENTRY
Plugin_Act(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam)
{
    switch (uAction)
    {
    case PLUGIN_ACTION_STARTREC:
    case PLUGIN_ACTION_PAUSE:
    case PLUGIN_ACTION_ENDREC:
        return TRUE;
    case PLUGIN_ACTION_PASS:
        return Plugin_DoPass(pi, wParam, lParam);
    case PLUGIN_ACTION_ENABLE:
        return Plugin_Enable(pi, wParam, lParam);
    case PLUGIN_ACTION_SHOWDIALOG:
        return Plugin_ShowDialog(pi, wParam, lParam);
    case PLUGIN_ACTION_REFRESH:
        return Plugin_Refresh(pi, wParam, lParam);
    case PLUGIN_ACTION_SETSTATE:
        return Plugin_SetState(pi, wParam, lParam);
    case PLUGIN_ACTION_DOBANG:
        return Plugin_DoBang(pi, wParam, lParam);
    case PLUGIN_ACTION_GETBANGLIST:
        return Plugin_GetBangList(pi, wParam, lParam);
    }
    return 0;
}

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        s_hinstDLL = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

} // extern "C"
