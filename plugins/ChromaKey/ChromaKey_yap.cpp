// ChromaKey_yap.cpp --- PluginFramework Plugin #2
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "Plugin.h"
#include "mregkey.hpp"
#include <shlwapi.h>
#include "color_value.h"
#include "color_button.hpp"
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <cassert>
#include <strsafe.h>
#include "resource.h"

static HINSTANCE s_hinstDLL;
static PLUGIN *s_pi;
static COLORREF s_rgbColor1;
static COLORREF s_rgbColor2;
static std::wstring s_strImageFile;
static std::string s_strImageFileA;
static INT s_nWindowX;
static INT s_nWindowY;
static BOOL s_bDialogInit = FALSE;

static COLOR_BUTTON s_color_button_1;
static COLOR_BUTTON s_color_button_2;

static cv::VideoCapture s_image_cap;

static void DoSetColorText(HWND hwnd, INT nItemID, COLORREF value)
{
    char buf[64];
    value = color_value_fix(value);
    color_value_store(buf, 64, value);
    SetDlgItemTextA(hwnd, nItemID, buf);
}

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
    s_rgbColor1 = RGB(0, 0, 0x80);
    s_rgbColor2 = RGB(0x80, 0x80, 0xFF);
    s_strImageFile.clear();
    s_strImageFileA.clear();
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

    MRegKey hkeyApp(hkeyCompany, TEXT("ChromaKey_yap"), FALSE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.QueryDword(TEXT("WindowX"), (DWORD&)s_nWindowX);
    hkeyApp.QueryDword(TEXT("WindowY"), (DWORD&)s_nWindowY);
    hkeyApp.QueryDword(TEXT("Color1"), (DWORD&)s_rgbColor1);
    hkeyApp.QueryDword(TEXT("Color2"), (DWORD&)s_rgbColor2);

    WCHAR szPath[MAX_PATH];
    hkeyApp.QuerySz(TEXT("ImageFile"), szPath, ARRAYSIZE(szPath));
    s_strImageFile = szPath;
    s_strImageFileA = ansi_from_wide(szPath);

    return TRUE;
}

static LRESULT DoSaveSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    MRegKey hkeyCompany(HKEY_CURRENT_USER,
                        TEXT("Software\\Katayama Hirofumi MZ"),
                        TRUE);
    if (!hkeyCompany)
        return FALSE;

    MRegKey hkeyApp(hkeyCompany, TEXT("ChromaKey_yap"), TRUE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.SetDword(TEXT("WindowX"), s_nWindowX);
    hkeyApp.SetDword(TEXT("WindowY"), s_nWindowY);
    hkeyApp.SetDword(TEXT("Color1"), s_rgbColor1);
    hkeyApp.SetDword(TEXT("Color2"), s_rgbColor2);

    hkeyApp.SetSz(TEXT("ImageFile"), s_strImageFile.c_str());

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
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("ChromaKey.yap"));
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
    s_image_cap.release();
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

    cv::Mat image;
    if (!s_image_cap.read(image))
    {
        s_image_cap.release();
        s_image_cap.open(s_strImageFileA.c_str());

        if (!s_image_cap.read(image))
        {
            image = cv::imread(s_strImageFileA.c_str());
            if (!image.data)
            {
                image = cv::Mat::zeros(mat.size(), CV_8UC3);
            }
        }
    }
    cv::resize(image, image, mat.size());

    cv::Scalar s_min(GetBValue(s_rgbColor1), GetGValue(s_rgbColor1), GetRValue(s_rgbColor1));
    cv::Scalar s_max(GetBValue(s_rgbColor2), GetGValue(s_rgbColor2), GetRValue(s_rgbColor2));

    cv::Mat mask;
    cv::inRange(mat, s_min, s_max, mask);
    mask = ~mask;

    mat.copyTo(image, mask);
    mat = image;

    return TRUE;
}

static LRESULT Plugin_Enable(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case 0:
        pi->bEnabled = FALSE;
        if (!pi->bEnabled)
        {
            s_image_cap.release();
        }
        return TRUE;
    case 1:
        pi->bEnabled = TRUE;
        s_image_cap.release();
        s_image_cap.open(s_strImageFileA.c_str());
        return TRUE;
    case 2:
        pi->bEnabled = !pi->bEnabled;
        if (pi->bEnabled)
        {
            s_image_cap.release();
            s_image_cap.open(s_strImageFileA.c_str());
        }
        else
        {
            s_image_cap.release();
        }
        return TRUE;
    }

    return FALSE;
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    s_pi->plugin_window = hwnd;

    s_color_button_1.SetHWND(GetDlgItem(hwnd, psh1));
    s_color_button_2.SetHWND(GetDlgItem(hwnd, psh2));
    s_color_button_1.SetColor(s_rgbColor1);
    s_color_button_2.SetColor(s_rgbColor2);

    DoSetColorText(hwnd, edt1, s_color_button_1.GetColor());
    DoSetColorText(hwnd, edt2, s_color_button_2.GetColor());
    SetDlgItemText(hwnd, edt3, s_strImageFile.c_str());

    s_bDialogInit = TRUE;
    return TRUE;
}

static void OnPsh3(HWND hwnd)
{
    TCHAR szFile[MAX_PATH] = TEXT("");
    OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    ofn.lpstrTitle = LoadStringDx(IDS_CHOOSEIMAGE);
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING |
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn))
    {
        SetDlgItemText(hwnd, edt3, szFile);
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
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            if (s_color_button_1.DoChooseColor())
            {
                s_bDialogInit = FALSE;
                s_rgbColor1 = s_color_button_1.GetColor();
                DoSetColorText(hwnd, edt2, s_rgbColor1);
                s_bDialogInit = TRUE;
            }
        }
        break;
    case psh2:
        if (codeNotify == BN_CLICKED)
        {
            if (s_color_button_2.DoChooseColor())
            {
                s_bDialogInit = FALSE;
                s_rgbColor2 = s_color_button_2.GetColor();
                DoSetColorText(hwnd, edt2, s_rgbColor2);
                s_bDialogInit = TRUE;
            }
        }
        break;
    case psh3:
        if (codeNotify == BN_CLICKED)
        {
            OnPsh3(hwnd);
        }
        break;
    case psh4:
        if (codeNotify == BN_CLICKED)
        {
            s_rgbColor1 = RGB(0, 0x80, 0);
            s_rgbColor2 = RGB(0x80, 0xFF, 0x80);
            DoSetColorText(hwnd, edt1, s_rgbColor1);
            DoSetColorText(hwnd, edt2, s_rgbColor2);
        }
        break;
    case psh5:
        if (codeNotify == BN_CLICKED)
        {
            s_rgbColor1 = RGB(0, 0, 0x80);
            s_rgbColor2 = RGB(0x80, 0x80, 0xFF);
            DoSetColorText(hwnd, edt1, s_rgbColor1);
            DoSetColorText(hwnd, edt2, s_rgbColor2);
        }
        break;
    case psh6:
        if (codeNotify == BN_CLICKED)
        {
            s_rgbColor1 = RGB(0x80, 0, 0x80);
            s_rgbColor2 = RGB(0xFF, 0x80, 0xFF);
            DoSetColorText(hwnd, edt1, s_rgbColor1);
            DoSetColorText(hwnd, edt2, s_rgbColor2);
        }
        break;
    case edt1:
        if (codeNotify == EN_CHANGE || codeNotify == EN_KILLFOCUS)
        {
            char buf[64];
            GetDlgItemTextA(hwnd, edt1, buf, 64);
            uint32_t value = color_value_parse(buf);
            if (value != uint32_t(-1))
            {
                value = color_value_fix(value);
                s_color_button_1.SetColor(value);
                s_rgbColor1 = value;

                if (codeNotify == EN_KILLFOCUS)
                {
                    s_bDialogInit = FALSE;
                    DoSetColorText(hwnd, edt1, value);
                    s_bDialogInit = TRUE;
                }
            }
        }
        break;
    case edt2:
        if (codeNotify == EN_CHANGE || codeNotify == EN_KILLFOCUS)
        {
            char buf[64];
            GetDlgItemTextA(hwnd, edt2, buf, 64);
            uint32_t value = color_value_parse(buf);
            if (value != uint32_t(-1))
            {
                value = color_value_fix(value);
                s_color_button_2.SetColor(value);
                s_rgbColor2 = value;

                if (codeNotify == EN_KILLFOCUS)
                {
                    s_bDialogInit = FALSE;
                    DoSetColorText(hwnd, edt2, value);
                    s_bDialogInit = TRUE;
                }
            }
        }
        break;
    case edt3:
        if (codeNotify == EN_CHANGE)
        {
            TCHAR szFile[MAX_PATH];
            GetDlgItemText(hwnd, edt3, szFile, ARRAYSIZE(szFile));
            StrTrim(szFile, TEXT(" \t"));
            s_strImageFile = szFile;
            s_strImageFileA = ansi_from_wide(szFile);
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

static void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
    if (!s_color_button_1.OnParentDrawItem(hwnd, lpDrawItem))
    {
        s_color_button_2.OnParentDrawItem(hwnd, lpDrawItem);
    }
}

static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DRAWITEM, OnDrawItem);
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
