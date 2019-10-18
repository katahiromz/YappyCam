#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

HWND g_hwndHotKeys = NULL;
static BOOL s_bInit = FALSE;

static void DoSetHotKey(HWND hwnd, INT i)
{
    HWND hwndHotKey = GetDlgItem(hwnd, edt1 + i);

    WORD nHotKey = (WORD)g_settings.m_nHotKey[i];

    SendMessage(hwndHotKey, HKM_SETRULES, HKCOMB_NONE,
                MAKELPARAM(HOTKEYF_ALT, 0));
    SendMessage(hwndHotKey, HKM_SETHOTKEY, nHotKey, 0);
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndHotKeys = hwnd;

    INT x = g_settings.m_nHotKeysDlgX;
    INT y = g_settings.m_nHotKeysDlgY;
    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        SendMessage(hwnd, DM_REPOSITION, 0, 0);
    }

    DoSetHotKey(hwnd, 0);
    DoSetHotKey(hwnd, 1);
    DoSetHotKey(hwnd, 2);
    DoSetHotKey(hwnd, 3);
    DoSetHotKey(hwnd, 4);

    s_bInit = TRUE;

    return TRUE;
}

static void OnEdt(HWND hwnd, INT i)
{
    if (!s_bInit)
        return;

    WORD w = (WORD)SendDlgItemMessage(hwnd, edt1 + i, HKM_GETHOTKEY, 0, 0);
    g_settings.m_nHotKey[i] = w;
    DoSetupHotkeys(g_hMainWnd, TRUE);
}

static void OnEdt1(HWND hwnd)
{
    OnEdt(hwnd, 0);
}

static void OnEdt2(HWND hwnd)
{
    OnEdt(hwnd, 1);
}

static void OnEdt3(HWND hwnd)
{
    OnEdt(hwnd, 2);
}

static void OnEdt4(HWND hwnd)
{
    OnEdt(hwnd, 3);
}

static void OnEdt5(HWND hwnd)
{
    OnEdt(hwnd, 4);
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
        OnEdt1(hwnd);
        break;
    case edt2:
        OnEdt2(hwnd);
        break;
    case edt3:
        OnEdt3(hwnd);
        break;
    case edt4:
        OnEdt4(hwnd);
        break;
    case edt5:
        OnEdt5(hwnd);
        break;
    }
}

static void OnDestroy(HWND hwnd)
{
    s_bInit = FALSE;
    g_hwndHotKeys = NULL;

    PostMessage(g_hMainWnd, WM_COMMAND, ID_CONFIGCLOSED, 0);
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    g_settings.m_nHotKeysDlgX = rc.left;
    g_settings.m_nHotKeysDlgY = rc.top;
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

BOOL DoHotKeysDialogBox(HWND hwndParent)
{
    if (g_hwndHotKeys)
    {
        SendMessage(g_hwndHotKeys, DM_REPOSITION, 0, 0);
        SetForegroundWindow(g_hwndHotKeys);
        return TRUE;
    }

    CreateDialog(GetModuleHandle(NULL),
                 MAKEINTRESOURCE(IDD_HOTKEYS),
                 hwndParent,
                 DialogProc);

    ShowWindow(g_hwndHotKeys, SW_SHOWNORMAL);
    UpdateWindow(g_hwndHotKeys);

    return g_hwndHotKeys != NULL;
}
