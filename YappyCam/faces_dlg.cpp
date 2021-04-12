#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

HWND g_hwndFaces = NULL;
static BOOL s_bInit = FALSE;

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndFaces = hwnd;

    INT x = g_settings.m_nFacesDlgX;
    INT y = g_settings.m_nFacesDlgY;
    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        SendMessage(hwnd, DM_REPOSITION, 0, 0);
    }

    //if (g_settings.m_bUseFaces)
    //    CheckDlgButton(hwnd, chx1, BST_CHECKED);
    //else
    //    CheckDlgButton(hwnd, chx1, BST_UNCHECKED);

    WCHAR szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpecW(szPath);
    PathAppendW(szPath, L"*.xml");

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    WIN32_FIND_DATAW find;
    HANDLE hFind = FindFirstFileW(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            ComboBox_AddString(hCmb1, find.cFileName);
        } while (FindNextFile(hFind, &find));

        FindClose(hFind);
    }

    INT i = ComboBox_FindStringExact(hCmb1, -1, g_settings.m_strCascadeClassifierW.c_str());
    if (i != CB_ERR)
        ComboBox_SetCurSel(hCmb1, i);

    s_bInit = TRUE;

    return TRUE;
}

static void OnChx1(HWND hwnd)
{
    if (IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED)
    {
        g_settings.m_bUseFaces = TRUE;
    }
    else
    {
        g_settings.m_bUseFaces = FALSE;
    }
}

static void OnCmb1(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    WCHAR szFileName[MAX_PATH];
    ComboBox_GetText(hCmb1, szFileName, ARRAYSIZE(szFileName));
    g_settings.m_strCascadeClassifierA = ansi_from_wide(szFileName);
    g_settings.m_strCascadeClassifierW = szFileName;

    WCHAR szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpecW(szPath);

    PathAppendW(szPath, szFileName);

    cv::CascadeClassifier cc(ansi_from_wide(szPath));

    BOOL bEnableFaces = g_settings.m_bUseFaces;
    g_settings.m_bUseFaces = FALSE;
    g_face_lock.lock(__LINE__);
    g_cascade = std::move(cc);
    g_face_lock.unlock(__LINE__);
    g_settings.m_bUseFaces = bEnableFaces;
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (!s_bInit)
        return;

    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    //case chx1:
    //    OnChx1(hwnd);
    //    break;
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
    }
}

static void OnDestroy(HWND hwnd)
{
    s_bInit = FALSE;
    g_hwndFaces = NULL;

    PostMessage(g_hMainWnd, WM_COMMAND, ID_CONFIGCLOSED, 0);
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    g_settings.m_nFacesDlgX = rc.left;
    g_settings.m_nFacesDlgY = rc.top;
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

BOOL DoFacesDialogBox(HWND hwndParent)
{
    if (g_hwndFaces)
    {
        SendMessage(g_hwndFaces, DM_REPOSITION, 0, 0);
        SetForegroundWindow(g_hwndFaces);
        return TRUE;
    }

    CreateDialog(GetModuleHandle(NULL),
                 MAKEINTRESOURCE(IDD_FACES),
                 hwndParent,
                 DialogProc);

    if (g_hwndFaces)
    {
        ShowWindow(g_hwndFaces, SW_SHOWNORMAL);
        UpdateWindow(g_hwndFaces);

        return TRUE;
    }
    return FALSE;
}
