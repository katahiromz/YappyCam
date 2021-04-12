// Clock_yap.cpp --- PluginFramework Plugin #1
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "../../Plugin.h"
#include "../../mregkey.hpp"
#include "../color_value.h"
#include "../color_button.hpp"
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <cassert>
#include <strsafe.h>
#include "resource.h"

enum ALIGN
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};
enum VALIGN
{
    VALIGN_TOP,
    VALIGN_MIDDLE,
    VALIGN_BOTTOM
};

static HINSTANCE s_hinstDLL;
static PLUGIN *s_pi;
static std::string s_strCaption;
static double s_eScale;
static INT s_nAlign;
static INT s_nVAlign;
static INT s_nMargin;
static INT s_nThickness;
static INT s_nWindowX;
static INT s_nWindowY;
static BOOL s_bDialogInit = FALSE;
static COLOR_BUTTON s_color_button_1;
static COLOR_BUTTON s_color_button_2;

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

std::string DoGetCaption(const char *fmt, const SYSTEMTIME& st)
{
    std::string ret;

    char buf[32];
    for (const char *pch = fmt; *pch; ++pch)
    {
        if (*pch != '&')
        {
            ret += *pch;
            continue;
        }

        ++pch;
        switch (*pch)
        {
        case '&':
            ret += '&';
            break;
        case 'y':
            StringCbPrintfA(buf, sizeof(buf), "%04u", st.wYear);
            ret += buf;
            break;
        case 'M':
            StringCbPrintfA(buf, sizeof(buf), "%02u", st.wMonth);
            ret += buf;
            break;
        case 'd':
            StringCbPrintfA(buf, sizeof(buf), "%02u", st.wDay);
            ret += buf;
            break;
        case 'h':
            StringCbPrintfA(buf, sizeof(buf), "%02u", st.wHour);
            ret += buf;
            break;
        case 'm':
            StringCbPrintfA(buf, sizeof(buf), "%02u", st.wMinute);
            ret += buf;
            break;
        case 's':
            StringCbPrintfA(buf, sizeof(buf), "%02u", st.wSecond);
            ret += buf;
            break;
        case 'f':
            StringCbPrintfA(buf, sizeof(buf), "%03u", st.wMilliseconds);
            ret += buf;
            break;
        default:
            ret += '&';
            ret += *pch;
            break;
        }
    }

    return ret;
}

extern "C" {

static LRESULT DoResetSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    s_nMargin = 3;
    s_nAlign = ALIGN_LEFT;
    s_nVAlign = VALIGN_TOP;
    s_eScale = 0.2;
    s_strCaption = "&h:&m:&s.&f";
    s_nWindowX = CW_USEDEFAULT;
    s_nWindowY = CW_USEDEFAULT;
    s_nThickness = 2;

    s_color_button_1.SetColor(RGB(0, 0, 0));
    s_color_button_2.SetColor(RGB(255, 255, 255));

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

    MRegKey hkeyApp(hkeyCompany, TEXT("Clock_yap"), FALSE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.QueryDword(TEXT("Margin"), (DWORD&)s_nMargin);
    hkeyApp.QueryDword(TEXT("Align"), (DWORD&)s_nAlign);
    hkeyApp.QueryDword(TEXT("VAlign"), (DWORD&)s_nVAlign);
    hkeyApp.QueryDword(TEXT("WindowX"), (DWORD&)s_nWindowX);
    hkeyApp.QueryDword(TEXT("WindowY"), (DWORD&)s_nWindowY);
    hkeyApp.QueryDword(TEXT("Thickness"), (DWORD&)s_nThickness);

    hkeyApp.QueryDword(TEXT("Color1"), (DWORD&)s_color_button_1.m_rgbColor);
    hkeyApp.QueryDword(TEXT("Color2"), (DWORD&)s_color_button_2.m_rgbColor);

    DWORD dwValue;
    TCHAR szText[64];

    if (!hkeyApp.QueryDword(TEXT("Scale"), (DWORD&)dwValue))
    {
        s_eScale = dwValue / 100.0;
    }

    if (!hkeyApp.QuerySz(TEXT("Caption"), szText, ARRAYSIZE(szText)))
    {
        s_strCaption = ansi_from_wide(szText);
    }

    return TRUE;
}

static LRESULT DoSaveSettings(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    MRegKey hkeyCompany(HKEY_CURRENT_USER,
                        TEXT("Software\\Katayama Hirofumi MZ"),
                        TRUE);
    if (!hkeyCompany)
        return FALSE;

    MRegKey hkeyApp(hkeyCompany, TEXT("Clock_yap"), TRUE);
    if (!hkeyApp)
        return FALSE;

    hkeyApp.SetDword(TEXT("Margin"), s_nMargin);
    hkeyApp.SetDword(TEXT("Align"), s_nAlign);
    hkeyApp.SetDword(TEXT("VAlign"), s_nVAlign);
    hkeyApp.SetDword(TEXT("WindowX"), s_nWindowX);
    hkeyApp.SetDword(TEXT("WindowY"), s_nWindowY);
    hkeyApp.SetDword(TEXT("Thickness"), s_nThickness);

    hkeyApp.SetDword(TEXT("Color1"), s_color_button_1.m_rgbColor);
    hkeyApp.SetDword(TEXT("Color2"), s_color_button_2.m_rgbColor);

    DWORD dwValue = DWORD(s_eScale * 100);
    hkeyApp.SetDword(TEXT("Scale"), (DWORD&)dwValue);

    TCHAR szText[64];
    hkeyApp.SetSz(TEXT("Caption"), wide_from_ansi(s_strCaption.c_str()));

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
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("Clock.yap"));
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

void DoDrawText(cv::Mat& mat, const char *text, double scale, int thickness,
                cv::Scalar& color)
{
    int font = cv::FONT_HERSHEY_SIMPLEX;
    cv::Size screen_size(mat.cols, mat.rows);

    scale *= screen_size.height * 0.01;

    int baseline;
    cv::Size text_size = cv::getTextSize(text, font, scale, thickness, &baseline);

    cv::Point pt;

    switch (s_nAlign)
    {
    case ALIGN_LEFT:
        pt.x = 0;
        pt.x += s_nMargin * screen_size.height / 100;
        break;
    case ALIGN_CENTER:
        pt.x = (screen_size.width - text_size.width + thickness) / 2;
        break;
    case ALIGN_RIGHT:
        pt.x = screen_size.width - text_size.width;
        pt.x -= s_nMargin * screen_size.height / 100;
        pt.x += thickness;
        break;
    default:
        assert(0);
    }

    switch (s_nVAlign)
    {
    case VALIGN_TOP:
        pt.y = 0;
        pt.y += s_nMargin * screen_size.height / 100 - thickness / 2;
        break;
    case VALIGN_MIDDLE:
        pt.y = (screen_size.height - text_size.height) / 2;
        break;
    case VALIGN_BOTTOM:
        pt.y = screen_size.height - text_size.height + thickness / 2;
        pt.y -= s_nMargin * screen_size.height / 100;
        pt.y -= baseline;
        break;
    default:
        assert(0);
    }
    pt.y += text_size.height - 1;

    cv::putText(mat, text, pt, font, scale,
                color, thickness, cv::LINE_AA, false);
}

static LRESULT Plugin_DoPass(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    cv::Mat *pmat = (cv::Mat *)wParam;
    if (!pmat || !pmat->data)
        return TRUE;

    cv::Mat& mat = *pmat;

    SYSTEMTIME st;
    GetLocalTime(&st);

    std::string strText = DoGetCaption(s_strCaption.c_str(), st);

    COLORREF rgb1 = s_color_button_1.GetColor();
    COLORREF rgb2 = s_color_button_2.GetColor();

    cv::Scalar back(GetBValue(rgb1), GetGValue(rgb1), GetRValue(rgb1));
    cv::Scalar fore(GetBValue(rgb2), GetGValue(rgb2), GetRValue(rgb2));

    DoDrawText(mat, strText.c_str(), s_eScale, s_nThickness * 3, back);
    DoDrawText(mat, strText.c_str(), s_eScale, s_nThickness, fore);
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

static void DoSetColorText(HWND hwnd, INT nItemID, COLORREF value)
{
    char buf[64];
    value = color_value_fix(value);
    color_value_store(buf, 64, value);
    SetDlgItemTextA(hwnd, nItemID, buf);
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    s_pi->plugin_window = hwnd;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, TEXT("&h:&m"));
    ComboBox_AddString(hCmb1, TEXT("&h:&m:&s"));
    ComboBox_AddString(hCmb1, TEXT("&h:&m:&s.&f"));
    ComboBox_AddString(hCmb1, TEXT("&y.&M.&d &h:&m:&s"));
    ComboBox_AddString(hCmb1, TEXT("&y.&M.&d &h:&m:&s.&f"));
    ComboBox_AddString(hCmb1, TEXT("&y.&M.&d"));
    ComboBox_AddString(hCmb1, TEXT("Sample Text"));
    SetDlgItemTextA(hwnd, cmb1, s_strCaption.c_str());

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    ComboBox_AddString(hCmb2, LoadStringDx(IDS_LEFT));
    ComboBox_AddString(hCmb2, LoadStringDx(IDS_CENTER));
    ComboBox_AddString(hCmb2, LoadStringDx(IDS_RIGHT));
    switch (s_nAlign)
    {
    case ALIGN_LEFT:
        ComboBox_SetCurSel(hCmb2, 0);
        break;
    case ALIGN_CENTER:
        ComboBox_SetCurSel(hCmb2, 1);
        break;
    case ALIGN_RIGHT:
        ComboBox_SetCurSel(hCmb2, 2);
        break;
    }

    HWND hCmb3 = GetDlgItem(hwnd, cmb3);
    ComboBox_AddString(hCmb3, LoadStringDx(IDS_TOP));
    ComboBox_AddString(hCmb3, LoadStringDx(IDS_MIDDLE));
    ComboBox_AddString(hCmb3, LoadStringDx(IDS_BOTTOM));
    switch (s_nVAlign)
    {
    case VALIGN_TOP:
        ComboBox_SetCurSel(hCmb3, 0);
        break;
    case VALIGN_MIDDLE:
        ComboBox_SetCurSel(hCmb3, 1);
        break;
    case VALIGN_BOTTOM:
        ComboBox_SetCurSel(hCmb3, 2);
        break;
    }

    DWORD dwValue = DWORD(s_eScale * 100);
    SendDlgItemMessage(hwnd, scr1, UDM_SETRANGE, 0, MAKELONG(100, 0));
    SendDlgItemMessage(hwnd, scr1, UDM_SETPOS, 0, MAKELONG(dwValue, 0));

    SendDlgItemMessage(hwnd, scr2, UDM_SETRANGE, 0, MAKELONG(300, 0));
    SendDlgItemMessage(hwnd, scr2, UDM_SETPOS, 0, MAKELONG(s_nMargin, 0));

    SendDlgItemMessage(hwnd, scr3, UDM_SETRANGE, 0, MAKELONG(30, 0));
    SendDlgItemMessage(hwnd, scr3, UDM_SETPOS, 0, MAKELONG(s_nThickness, 0));

    s_color_button_1.SetHWND(GetDlgItem(hwnd, psh1));
    s_color_button_2.SetHWND(GetDlgItem(hwnd, psh2));

    DoSetColorText(hwnd, edt4, s_color_button_1.GetColor());
    DoSetColorText(hwnd, edt5, s_color_button_2.GetColor());

    s_bDialogInit = TRUE;
    return TRUE;
}

static void OnEdt1(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    BOOL bTranslated = FALSE;
    INT nValue = GetDlgItemInt(hwnd, edt1, &bTranslated, TRUE);
    if (bTranslated)
    {
        s_eScale = nValue / 100.0;
    }
}

static void OnEdt2(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    BOOL bTranslated = FALSE;
    INT nValue = GetDlgItemInt(hwnd, edt2, &bTranslated, TRUE);
    if (bTranslated)
    {
        s_nMargin = nValue;
    }
}

static void OnEdt3(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    BOOL bTranslated = FALSE;
    INT nValue = GetDlgItemInt(hwnd, edt3, &bTranslated, TRUE);
    if (bTranslated)
    {
        s_nThickness = nValue;
    }
}

static void OnCmb1(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);

    TCHAR szText[MAX_PATH];
    INT iItem = ComboBox_GetCurSel(hCmb1);
    if (iItem == CB_ERR)
    {
        ComboBox_GetText(hCmb1, szText, ARRAYSIZE(szText));
    }
    else
    {
        ComboBox_GetLBText(hCmb1, iItem, szText);
    }

    s_strCaption = ansi_from_wide(szText);
}

static void OnCmb2(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    switch (ComboBox_GetCurSel(hCmb2))
    {
    case 0:
        s_nAlign = ALIGN_LEFT;
        break;
    case 1:
        s_nAlign = ALIGN_CENTER;
        break;
    case 2:
        s_nAlign = ALIGN_RIGHT;
        break;
    case CB_ERR:
    default:
        assert(0);
        break;
    }
}

static void OnCmb3(HWND hwnd)
{
    if (!s_bDialogInit)
        return;

    HWND hCmb3 = GetDlgItem(hwnd, cmb3);
    switch (ComboBox_GetCurSel(hCmb3))
    {
    case 0:
        s_nVAlign = VALIGN_TOP;
        break;
    case 1:
        s_nVAlign = VALIGN_MIDDLE;
        break;
    case 2:
        s_nVAlign = VALIGN_BOTTOM;
        break;
    case CB_ERR:
    default:
        assert(0);
        break;
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
    case edt2:
        if (codeNotify == EN_CHANGE)
        {
            OnEdt2(hwnd);
        }
        break;
    case edt3:
        if (codeNotify == EN_CHANGE)
        {
            OnEdt3(hwnd);
        }
        break;
    case edt4:
        if (codeNotify == EN_CHANGE || codeNotify == EN_KILLFOCUS)
        {
            char buf[64];
            GetDlgItemTextA(hwnd, edt4, buf, 64);
            uint32_t value = color_value_parse(buf);
            if (value != uint32_t(-1))
            {
                value = color_value_fix(value);
                s_color_button_1.SetColor(value);

                if (codeNotify == EN_KILLFOCUS)
                {
                    s_bDialogInit = FALSE;
                    DoSetColorText(hwnd, edt4, value);
                    s_bDialogInit = TRUE;
                }
            }
        }
        break;
    case edt5:
        if (codeNotify == EN_CHANGE || codeNotify == EN_KILLFOCUS)
        {
            char buf[64];
            GetDlgItemTextA(hwnd, edt5, buf, 64);
            uint32_t value = color_value_parse(buf);
            if (value != uint32_t(-1))
            {
                value = color_value_fix(value);
                s_color_button_2.SetColor(value);

                if (codeNotify == EN_KILLFOCUS)
                {
                    s_bDialogInit = FALSE;
                    DoSetColorText(hwnd, edt5, value);
                    s_bDialogInit = TRUE;
                }
            }
        }
        break;
    case psh1:
        if (codeNotify == BN_CLICKED)
        {
            if (s_color_button_1.DoChooseColor())
            {
                s_bDialogInit = FALSE;
                DoSetColorText(hwnd, edt4, s_color_button_1.GetColor());
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
                DoSetColorText(hwnd, edt5, s_color_button_2.GetColor());
                s_bDialogInit = TRUE;
            }
        }
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
    case cmb2:
        if (codeNotify == CBN_SELCHANGE)
        {
            OnCmb2(hwnd);
        }
        break;
    case cmb3:
        if (codeNotify == CBN_SELCHANGE)
        {
            OnCmb3(hwnd);
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
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_MOVE, OnMove);
        HANDLE_MSG(hwnd, WM_DRAWITEM, OnDrawItem);
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
