#include "YappyCam.hpp"

HWND g_hwndSaveTo = NULL;
static BOOL s_bInit = FALSE;

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndSaveTo = hwnd;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    HWND hCmb3 = GetDlgItem(hwnd, cmb3);

    ComboBox_AddString(hCmb2, TEXT("img-%04u.png"));
    ComboBox_AddString(hCmb2, TEXT("img-%04u.jpg"));
    ComboBox_AddString(hCmb2, TEXT("img-%04u.bmp"));

    ComboBox_AddString(hCmb3, TEXT("0x7634706d"));
    ComboBox_AddString(hCmb3, TEXT("0x3447504d"));
    ComboBox_AddString(hCmb3, TEXT("0x34363248"));

    WCHAR szValue[64];
    StringCbPrintf(szValue, sizeof(szValue), L"0x%08lX", g_settings.m_dwFOURCC);
    ComboBox_SetText(hCmb3, szValue);

    SetDlgItemText(hwnd, cmb1, g_settings.m_strDir.c_str());
    SetDlgItemText(hwnd, cmb2, g_settings.m_strImageFileName.c_str());

    INT x = g_settings.m_nSaveToDlgX;
    INT y = g_settings.m_nSaveToDlgY;
    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        SendMessage(hwnd, DM_REPOSITION, 0, 0);
    }

    s_bInit = TRUE;

    return TRUE;
}

static void OnPsh1(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    TCHAR szPath[MAX_PATH];
    ComboBox_GetText(hCmb1, szPath, ARRAYSIZE(szPath));
    StrTrim(szPath, L" \t");

    TCHAR szText[MAX_PATH + 64];
    StringCbPrintf(szText, sizeof(szText),
                   L"/e,/select,%s", szPath);
    ShellExecute(hwnd, NULL, L"explorer", szText, NULL, SW_SHOWNORMAL);
}

static INT CALLBACK
BrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
    }
    return 0;
}

static void OnPsh2(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    TCHAR szPath[MAX_PATH];
    ComboBox_GetText(hCmb1, szPath, ARRAYSIZE(szPath));
    StrTrim(szPath, L" \t");

    BROWSEINFO info = { hwnd };
    TCHAR szDisplayName[MAX_PATH] = { 0 };
    info.pidlRoot = NULL;
    info.pszDisplayName = szDisplayName;
    info.lpszTitle = LoadStringDx(IDS_SETLOCATION);
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_VALIDATE;
    info.lpfn = BrowseCallbackProc;
    info.lParam = (LPARAM)szPath;
    info.iImage = 0;

    if (LPITEMIDLIST pidl = SHBrowseForFolder(&info))
    {
        SHGetPathFromIDList(pidl, szPath);
        ComboBox_SetText(hCmb1, szPath);

        CoTaskMemFree(pidl);
    }
}

static void OnCmb1(HWND hwnd)
{
    if (!s_bInit)
        return;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    TCHAR szPath[MAX_PATH];
    ComboBox_GetText(hCmb1, szPath, ARRAYSIZE(szPath));
    StrTrim(szPath, L" \t");

    g_settings.m_strDir = szPath;
    g_settings.change_dirs(szPath);
}

static void OnCmb2(HWND hwnd)
{
    if (!s_bInit)
        return;

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    TCHAR szFilename[MAX_PATH];
    ComboBox_GetText(hCmb2, szFilename, ARRAYSIZE(szFilename));
    StrTrim(szFilename, L" \t");

    g_settings.m_strImageFileName = szFilename;
}

static void OnCmb3(HWND hwnd)
{
    if (!s_bInit)
        return;

    HWND hCmb3 = GetDlgItem(hwnd, cmb3);
    TCHAR szValue[MAX_PATH];
    ComboBox_GetText(hCmb3, szValue, ARRAYSIZE(szValue));
    StrTrim(szValue, L" \t");

    DWORD dwFOURCC = wcstoul(szValue, NULL, 0);
    g_settings.m_dwFOURCC = dwFOURCC;
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            OnPsh1(hwnd);
        }
        break;
    case psh2:
        if (codeNotify == BN_CLICKED)
        {
            OnPsh2(hwnd);
        }
        break;
    case cmb1:
        if (codeNotify == CBN_SELCHANGE || codeNotify == CBN_EDITCHANGE)
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
    case cmb3:
        if (codeNotify == CBN_SELCHANGE || codeNotify == CBN_EDITCHANGE)
        {
            OnCmb3(hwnd);
        }
        break;
    }
}

static void OnDestroy(HWND hwnd)
{
    s_bInit = FALSE;
    g_hwndSaveTo = NULL;
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    g_settings.m_nSaveToDlgX = rc.left;
    g_settings.m_nSaveToDlgY = rc.top;
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

BOOL DoSaveToDialogBox(HWND hwndParent)
{
    if (g_hwndSaveTo)
    {
        SendMessage(g_hwndSaveTo, DM_REPOSITION, 0, 0);
        SetForegroundWindow(g_hwndSaveTo);
        return TRUE;
    }

    CreateDialog(GetModuleHandle(NULL),
                 MAKEINTRESOURCE(IDD_SAVETO),
                 hwndParent,
                 DialogProc);

    ShowWindow(g_hwndSaveTo, SW_SHOWNORMAL);
    UpdateWindow(g_hwndSaveTo);

    return g_hwndSaveTo != NULL;
}
