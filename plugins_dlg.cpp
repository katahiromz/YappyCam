#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

static BOOL s_bInit = FALSE;
HWND g_hwndPlugins = NULL;

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndPlugins = hwnd;

    INT x = g_settings.m_nPluginsDlgX;
    INT y = g_settings.m_nPluginsDlgY;
    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        SendMessage(hwnd, DM_REPOSITION, 0, 0);
    }

    s_bInit = TRUE;

    return TRUE;
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    }
}

static void OnDestroy(HWND hwnd)
{
    s_bInit = FALSE;
    g_hwndPlugins = NULL;

    PostMessage(g_hMainWnd, WM_COMMAND, ID_CONFIGCLOSED, 0);
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    g_settings.m_nPluginsDlgX = rc.left;
    g_settings.m_nPluginsDlgY = rc.top;
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

BOOL DoPluginsDialogBox(HWND hwndParent)
{
    if (g_hwndPlugins)
    {
        SendMessage(g_hwndPlugins, DM_REPOSITION, 0, 0);
        SetForegroundWindow(g_hwndPlugins);
        return TRUE;
    }

    CreateDialog(GetModuleHandle(NULL),
                 MAKEINTRESOURCE(IDD_PLUGINS),
                 hwndParent,
                 DialogProc);

    if (g_hwndPlugins)
    {
        ShowWindow(g_hwndPlugins, SW_SHOWNORMAL);
        UpdateWindow(g_hwndPlugins);

        return TRUE;
    }
    return FALSE;
}
