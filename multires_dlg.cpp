#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

static RESO_MAP s_map;
static ASPECT_MODE s_mode;

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    s_mode = g_settings.m_nAspectMode;
    SendDlgItemMessage(hwnd, lst1, LB_RESETCONTENT, 0, 0);

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_ASPECT_IGNORE));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_ASPECT_CUT));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_ASPECT_BLACK));
    ComboBox_AddString(hCmb1, LoadStringDx(IDS_ASPECT_WHITE));
    ComboBox_SetCurSel(hCmb1, s_mode);

    for (auto& pair : s_map)
    {
        DWORD dw = pair.first;
        WORD cx = LOWORD(dw);
        WORD cy = HIWORD(dw);

        TCHAR szText[128];
        StringCbPrintf(szText, sizeof(szText), TEXT("%u x %u"), cx, cy);
        SendDlgItemMessage(hwnd, lst1, LB_ADDSTRING, 0, (LPARAM)szText);
    }

    DWORD i = 0, dwMax = 0, iMax = 0;
    for (auto& pair : s_map)
    {
        DWORD dwCount = pair.second;

        if (dwMax < dwCount)
        {
            iMax = i;
            dwMax = dwCount;
        }

        ++i;
    }

    i = 0;
    for (auto& pair : s_map)
    {
        if (i == iMax)
        {
            DWORD dw = pair.first;
            WORD cx = LOWORD(dw);
            WORD cy = HIWORD(dw);

            TCHAR szText[128];
            StringCbPrintf(szText, sizeof(szText), TEXT("%u x %u"), cx, cy);
            i = (INT)SendDlgItemMessage(hwnd, lst1, LB_FINDSTRINGEXACT, -1, (LPARAM)szText);
            SendDlgItemMessage(hwnd, lst1, LB_SETCURSEL, i, 0);
            break;
        }
        ++i;
    }

    return TRUE;
}

static void OnOK(HWND hwnd)
{
    INT i = (INT)SendDlgItemMessage(hwnd, lst1, LB_GETCURSEL, 0, 0);
    if (i < 0)
    {
        ErrorBoxDx(hwnd, L"Not selected yet");
        return;
    }

    TCHAR szText[128];
    SendDlgItemMessage(hwnd, lst1, LB_GETTEXT, i, (LPARAM)szText);

    std::wstring str = szText;
    size_t k = str.find(L" x ");
    if (k == std::wstring::npos)
    {
        ErrorBoxDx(hwnd, L"Not selected yet");
        return;
    }

    std::wstring str1 = str.substr(0, k);
    std::wstring str2 = str.substr(k + 3);

    INT cx = _wtoi(str1.c_str());
    INT cy = _wtoi(str2.c_str());

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    g_settings.m_nAspectMode = static_cast<ASPECT_MODE>(ComboBox_GetCurSel(hCmb1));

    EndDialog(hwnd, MAKELONG(cx, cy));
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        OnOK(hwnd);
        break;
    case IDCANCEL:
        EndDialog(hwnd, 0);
        break;
    }
}

static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
    }
    return 0;
}

DWORD DoMultiResoDialogBox(HWND hwndParent, const RESO_MAP& map)
{
    s_map = map;

    DWORD dwReso;
    dwReso = (DWORD)DialogBox(GetModuleHandle(NULL),
                              MAKEINTRESOURCE(IDD_MULTIRESO),
                              hwndParent,
                              DialogProc);
    return dwReso;
}
