#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

static BOOL s_bInit = FALSE;
HWND g_hwndPlugins = NULL;

// plugins
extern std::vector<PLUGIN> s_plugins;

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

    HWND hLst1 = GetDlgItem(hwnd, lst1);
    ListView_SetExtendedListViewStyle(hLst1, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

    LV_COLUMN column;
    TCHAR szText[64];

    ZeroMemory(&column, sizeof(column));
    column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    column.fmt = LVCFMT_LEFT;
    column.cx = 150;
    StringCbCopy(szText, sizeof(szText), L"Name");
    column.pszText = szText;
    ListView_InsertColumn(hLst1, 0, &column);
    column.iSubItem++;

    column.fmt = LVCFMT_LEFT;
    column.cx = 150;
    StringCbCopy(szText, sizeof(szText), L"Filename");
    column.pszText = szText;
    ListView_InsertColumn(hLst1, 1, &column);
    column.iSubItem++;

    LV_ITEM item = { LVIF_TEXT };
    for (auto& plugin : s_plugins)
    {
        item.iSubItem = 0;
        item.pszText = plugin.plugin_product_name;
        ListView_InsertItem(hLst1, &item);

        item.iSubItem = 1;
        item.pszText = plugin.plugin_filename;
        ListView_SetItem(hLst1, &item);

        ListView_SetCheckState(hLst1, item.iItem, plugin.bEnabled);

        ++item.iItem;
    }

    ListView_SetItemState(hLst1, 0, LVIS_SELECTED, LVIS_SELECTED);

    s_bInit = TRUE;

    return TRUE;
}

static void OnPsh3(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_SHOWDIALOG, (WPARAM)g_hMainWnd, TRUE);
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    case psh3:
        OnPsh3(hwnd);
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

static LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    switch (pnmhdr->code)
    {
    case LVN_ITEMCHANGED:
        {
            HWND hLst1 = GetDlgItem(hwnd, lst1);
            INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
            if (iItem < 0 || iItem >= INT(s_plugins.size()))
            {
                EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
                EnableWindow(GetDlgItem(hwnd, psh2), FALSE);
                EnableWindow(GetDlgItem(hwnd, psh3), FALSE);
            }
            else
            {
                if (iItem == 0)
                    EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
                else
                    EnableWindow(GetDlgItem(hwnd, psh1), TRUE);

                INT cItems = ListView_GetItemCount(hLst1);
                if (iItem < cItems - 1)
                    EnableWindow(GetDlgItem(hwnd, psh2), TRUE);
                else
                    EnableWindow(GetDlgItem(hwnd, psh2), FALSE);

                EnableWindow(GetDlgItem(hwnd, psh3), TRUE);
            }
        }
        break;
    case NM_DBLCLK:
        OnPsh3(hwnd);
        break;
    }
    return 0;
}

static INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_NOTIFY, OnNotify);
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
