#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

static BOOL s_bInit = FALSE;
HWND g_hwndPlugins = NULL;

// plugins
extern std::vector<PLUGIN> s_plugins;

static void OnRefreshListView(HWND hwnd, INT iItem = -1)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);

    if (iItem == -1)
        iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);

    ListView_DeleteAllItems(hLst1);

    LV_ITEM item = { LVIF_TEXT };
    for (auto& plugin : s_plugins)
    {
        item.iSubItem = 0;
        item.pszText = plugin.plugin_product_name;
        ListView_InsertItem(hLst1, &item);

        item.iSubItem = 1;
        item.pszText = plugin.plugin_filename;
        ListView_SetItem(hLst1, &item);

        item.iSubItem = 2;
        switch (plugin.dwFlags & PLUGIN_FLAG_PASS1AND2)
        {
        case 0:
            break;
        case PLUGIN_FLAG_PASS1:
            item.pszText = LoadStringDx(IDS_PASS1);
            break;
        case PLUGIN_FLAG_PASS2:
            item.pszText = LoadStringDx(IDS_PASS2);
            break;
        case PLUGIN_FLAG_PASS1AND2:
            item.pszText = LoadStringDx(IDS_PASS1AND2);
            break;
        }
        ListView_SetItem(hLst1, &item);

        ListView_SetCheckState(hLst1, item.iItem, plugin.bEnabled);

        ++item.iItem;
    }

    ListView_SetItemState(hLst1, iItem, LVIS_SELECTED, LVIS_SELECTED);
}

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
    column.cx = 110;
    StringCbCopy(szText, sizeof(szText), LoadStringDx(IDS_NAME));
    column.pszText = szText;
    ListView_InsertColumn(hLst1, column.iSubItem, &column);
    column.iSubItem++;

    column.fmt = LVCFMT_LEFT;
    column.cx = 150;
    StringCbCopy(szText, sizeof(szText), LoadStringDx(IDS_FILENAME));
    column.pszText = szText;
    ListView_InsertColumn(hLst1, column.iSubItem, &column);
    column.iSubItem++;

    column.fmt = LVCFMT_LEFT;
    column.cx = 120;
    StringCbCopy(szText, sizeof(szText), LoadStringDx(IDS_PASS));
    column.pszText = szText;
    ListView_InsertColumn(hLst1, column.iSubItem, &column);
    column.iSubItem++;

    OnRefreshListView(hwnd, 0);

    s_bInit = TRUE;

    return TRUE;
}

static void OnPsh1(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem <= 0 || iItem >= INT(s_plugins.size()))
        return;

    std::swap(s_plugins[iItem - 1], s_plugins[iItem]);

    s_bInit = FALSE;
    OnRefreshListView(hwnd, iItem - 1);

    PF_ActOne(&s_plugins[iItem - 1], PLUGIN_ACTION_REFRESH, FALSE, 0);
    PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_REFRESH, FALSE, 0);

    s_bInit = TRUE;
}

static void OnPsh2(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem + 1 >= INT(s_plugins.size()))
        return;

    std::swap(s_plugins[iItem], s_plugins[iItem + 1]);

    s_bInit = FALSE;
    OnRefreshListView(hwnd, iItem + 1);

    PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_REFRESH, FALSE, 0);
    PF_ActOne(&s_plugins[iItem + 1], PLUGIN_ACTION_REFRESH, FALSE, 0);

    s_bInit = TRUE;
}

static void OnPsh3(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_SHOWDIALOG, (WPARAM)g_hMainWnd, TRUE);
}

static void OnPsh4(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    s_plugins[iItem].dwFlags &= ~PLUGIN_FLAG_PASS1AND2;
    s_plugins[iItem].dwFlags |= PLUGIN_FLAG_PASS1;

    s_bInit = FALSE;
    OnRefreshListView(hwnd, iItem);
    s_bInit = TRUE;
}

static void OnPsh5(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    s_plugins[iItem].dwFlags &= ~PLUGIN_FLAG_PASS1AND2;
    s_plugins[iItem].dwFlags |= PLUGIN_FLAG_PASS2;

    s_bInit = FALSE;
    OnRefreshListView(hwnd, iItem);
    s_bInit = TRUE;
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
        OnPsh1(hwnd);
        break;
    case psh2:
        OnPsh2(hwnd);
        break;
    case psh3:
        OnPsh3(hwnd);
        break;
    case psh4:
        OnPsh4(hwnd);
        break;
    case psh5:
        OnPsh5(hwnd);
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

static void OnListViewClick(HWND hwnd)
{
    if (!s_bInit)
        return;

    HWND hLst1 = GetDlgItem(hwnd, lst1);

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hLst1, &pt);

    LV_HITTESTINFO ht = { pt };
    ListView_HitTest(hLst1, &ht);

    INT iItem = ht.iItem;
    if (0 <= iItem && iItem < INT(s_plugins.size()))
    {
        s_bInit = FALSE;
        ListView_SetItemState(hLst1, iItem, LVIS_SELECTED, LVIS_SELECTED);
        s_bInit = TRUE;

        BOOL bEnabled = ListView_GetCheckState(hLst1, iItem);
        s_plugins[iItem].bEnabled = bEnabled;
    }
}

static LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    INT iItem;
    HWND hLst1 = GetDlgItem(hwnd, lst1);

    switch (pnmhdr->code)
    {
    case LVN_ITEMCHANGED:
        iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
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
        OnListViewClick(hwnd);
        break;
    case NM_CLICK:
        OnListViewClick(hwnd);
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
