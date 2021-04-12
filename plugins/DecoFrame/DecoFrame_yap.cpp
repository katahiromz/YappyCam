// DecoFrame_yap.cpp --- PluginFramework Plugin #2
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "../../Plugin.h"
#include "../mregkey.hpp"
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <cassert>
#include <strsafe.h>
#include "resource.h"

static HINSTANCE s_hinstDLL;
static PLUGIN *s_pi;
static INT s_nWindowX;
static INT s_nWindowY;
static BOOL s_bDialogInit = FALSE;
static cv::Mat s_deco;
static cv::Mat s_mask;
static WCHAR s_szFileName[MAX_PATH];

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
    pi->bEnabled = FALSE;
    pi->dwInfo = PLUGIN_INFO_PASS;
    pi->dwState = PLUGIN_STATE_PASS1;
    pi->p_user_data = NULL;
    pi->l_user_data = 0;
    StringCbCopyW(s_szFileName, sizeof(s_szFileName), L"GoldFrame.png");
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

    MRegKey hkeyApp(hkeyCompany, TEXT("DecoFrame_yap"), FALSE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.QueryDword(TEXT("WindowX"), (DWORD&)s_nWindowX);
    hkeyApp.QueryDword(TEXT("WindowY"), (DWORD&)s_nWindowY);

    WCHAR szText[MAX_PATH];
    hkeyApp.QuerySz(TEXT("ImageFile"), szText, ARRAYSIZE(szText));
    StringCbCopyW(s_szFileName, sizeof(s_szFileName), szText);

    return TRUE;
}

static LRESULT DoSaveSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    MRegKey hkeyCompany(HKEY_CURRENT_USER,
                        TEXT("Software\\Katayama Hirofumi MZ"),
                        TRUE);
    if (!hkeyCompany)
        return FALSE;

    MRegKey hkeyApp(hkeyCompany, TEXT("DecoFrame_yap"), TRUE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.SetDword(TEXT("WindowX"), s_nWindowX);
    hkeyApp.SetDword(TEXT("WindowY"), s_nWindowY);
    hkeyApp.SetSz(TEXT("ImageFile"), s_szFileName);

    return TRUE;
}

BOOL DoLoadFile(LPCWSTR pszFileName)
{
    std::string path = ansi_from_wide(pszFileName);
    s_deco = cv::imread(path.c_str(), cv::IMREAD_UNCHANGED);
    if (s_deco.channels() == 4)
    {
        std::vector<cv::Mat> rgba;
        cv::split(s_deco, rgba);

        std::vector<cv::Mat> rgb;
        rgb.push_back(rgba[0]);
        rgb.push_back(rgba[1]);
        rgb.push_back(rgba[2]);
        cv::merge(rgb, s_deco);

        std::vector<cv::Mat> aaa;
        aaa.push_back(rgba[3]);
        aaa.push_back(rgba[3]);
        aaa.push_back(rgba[3]);
        cv::merge(aaa, s_mask);
    }
    return !s_deco.empty() && !s_mask.empty();
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
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("DecoFrame.yap"));
    StringCbCopy(pi->plugin_company, sizeof(pi->plugin_company), TEXT("Katayama Hirofumi MZ"));
    StringCbCopy(pi->plugin_copyright, sizeof(pi->plugin_copyright), TEXT("Copyright (C) 2019 Katayama Hirofumi MZ"));
    pi->plugin_instance = s_hinstDLL;
    pi->plugin_window = NULL;
    DoLoadSettings(pi, 0, 0);

    WCHAR szDir[MAX_PATH], szPath[MAX_PATH];
    GetModuleFileName(NULL, szDir, ARRAYSIZE(szDir));
    PathRemoveFileSpecW(szDir);
    PathAppendW(szDir, L"DecoFrame");

    HANDLE hFind;
    WIN32_FIND_DATAW find;
    std::vector<std::wstring> paths;

    StringCbCopyW(szPath, sizeof(szPath), szDir);
    PathAppendW(szPath, L"*.png");
    hFind = FindFirstFileW(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            StringCbCopyW(szPath, sizeof(szPath), szDir);
            PathAppendW(szPath, find.cFileName);
            paths.push_back(szPath);
        } while (FindNextFile(hFind, &find));
    }

    StringCbCopyW(szPath, sizeof(szPath), szDir);
    PathAppendW(szPath, s_szFileName);

    if (!PathFileExistsW(szPath) || !DoLoadFile(szPath))
    {
        StringCbCopyW(szPath, sizeof(szPath), szDir);
        PathAppendW(szPath, L"CatEar.png");
        if (DoLoadFile(szPath))
        {
            StringCbCopyW(s_szFileName, sizeof(s_szFileName), L"Frame.png");
        }
        else
        {
            s_szFileName[0] = 0;
        }
    }

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

cv::Mat
DoPasteFrame(const cv::Mat& back, const cv::Mat& target,
             const cv::Point2f& p0, const cv::Point2f& p1)
{
    cv::Mat ret;
    back.copyTo(ret);

    std::vector<cv::Point2f> from, to;
    from.push_back(cv::Point2f(0, 0));
    from.push_back(cv::Point2f(target.cols, 0));
    from.push_back(cv::Point2f(target.cols, target.rows));

    to.push_back(p0);
    to.push_back(cv::Point2f(p1.x, p0.y));
    to.push_back(p1);

    cv::Mat mat = cv::getAffineTransform(from, to);

    cv::warpAffine(target, ret, mat, ret.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
    return ret;
}

static LRESULT Plugin_DoPass(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    cv::Mat *pmat = (cv::Mat *)wParam;
    if (!pmat || !pmat->data)
        return TRUE;

    if (s_deco.empty())
        return TRUE;

    cv::Mat& mat = *pmat;

    if (!s_deco.empty() && !s_mask.empty())
    {
        INT cx = mat.cols;
        INT cy = mat.rows;
        cv::Point p0(0, 0);
        cv::Point p1(cx, cy);

        cv::Mat img1 = DoPasteFrame(mat, s_deco, p0, p1);

        cv::Mat img2(mat.rows, mat.cols, s_mask.type());
        img2 = cv::Scalar::all(0);
        img2 = DoPasteFrame(img2, s_mask, p0, p1);

        std::vector<cv::Mat> aaa;
        cv::split(img2, aaa);

        cv::Mat img3 = mat;

        std::vector<cv::Mat> nega;
        nega.push_back(255 - aaa[0]);
        nega.push_back(255 - aaa[1]);
        nega.push_back(255 - aaa[2]);

        cv::Mat img4;
        cv::merge(nega, img4);

        mat = img1.mul(img2) / 255 + img3.mul(img4) / 255;
    }

    return TRUE;
}

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

    HANDLE hFind;
    WIN32_FIND_DATAW find;
    std::vector<std::wstring> files;

    WCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpecW(szPath);
    PathAppendW(szPath, L"DecoFrame");
    PathAppendW(szPath, L"*.png");
    hFind = FindFirstFileW(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            files.push_back(find.cFileName);
        } while (FindNextFile(hFind, &find));
    }

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    for (auto& file : files)
    {
        ComboBox_AddString(hCmb1, file.c_str());
    }

    INT i = ComboBox_FindStringExact(hCmb1, -1, s_szFileName);
    if (i != CB_ERR)
    {
        ComboBox_SetCurSel(hCmb1, i);
    }

    s_bDialogInit = TRUE;
    return TRUE;
}

static void OnCmb1(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    WCHAR szText[MAX_PATH];
    ComboBox_GetText(hCmb1, szText, ARRAYSIZE(szText));

    WCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpecW(szPath);
    PathAppendW(szPath, L"DecoFrame");
    PathAppendW(szPath, szText);

    if (PathFileExistsW(szPath) && DoLoadFile(szPath))
    {
        StringCbCopyW(s_szFileName, ARRAYSIZE(s_szFileName), szText);
    }
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (!s_bDialogInit)
        return;

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
