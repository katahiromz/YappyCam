#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

static BOOL s_bInit = FALSE;
HWND g_hwndPlugins = NULL;

void Lst1_SetSelection(HWND hLst1, INT iItem, BOOL bSelected = TRUE)
{
    ListView_SetItemState(hLst1, iItem,
        (bSelected ? LVIS_SELECTED | LVIS_FOCUSED : 0),
        LVIS_SELECTED | LVIS_FOCUSED);
}

void Lst1_SetItem(HWND hLst1, PLUGIN& plugin, INT iItem = -1)
{
    LV_ITEM item = { LVIF_TEXT };

    if (iItem == -1)
    {
        item.iItem = ListView_GetItemCount(hLst1);
        item.iSubItem = 0;
        item.pszText = plugin.plugin_product_name;
        iItem = ListView_InsertItem(hLst1, &item);
    }
    else
    {
        item.iItem = iItem;
        item.iSubItem = 0;
        item.pszText = plugin.plugin_product_name;
        ListView_SetItem(hLst1, &item);
    }

    item.iSubItem = 1;
    item.pszText = plugin.plugin_filename;
    ListView_SetItem(hLst1, &item);

    item.iSubItem = 2;
    switch (plugin.dwInfo & PLUGIN_INFO_TYPEMASK)
    {
    case PLUGIN_INFO_PASS:
        if (plugin.dwState & PLUGIN_STATE_PASS2)
            item.pszText = LoadStringDx(IDS_PASS2);
        else
            item.pszText = LoadStringDx(IDS_PASS1);
        break;
    case PLUGIN_INFO_PICINPUT:
        item.pszText = LoadStringDx(IDS_PICINPUT);
        break;
    case PLUGIN_INFO_SWITCHBOARD:
        item.pszText = LoadStringDx(IDS_SWITCHBOARD);
        break;
    case PLUGIN_INFO_BROADCASTER:
        item.pszText = LoadStringDx(IDS_BROADCASTER);
        break;
    default:
        item.pszText = LoadStringDx(IDS_UNKNOWN);
        break;
    }
    ListView_SetItem(hLst1, &item);

    ListView_SetCheckState(hLst1, iItem, plugin.bEnabled);
}

static void OnRefreshListView(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);

    ListView_DeleteAllItems(hLst1);

    for (auto& plugin : s_plugins)
    {
        Lst1_SetItem(hLst1, plugin);
    }
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
    ListView_SetExtendedListViewStyle(hLst1,
        LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT |
        LVS_EX_INFOTIP | LVS_EX_LABELTIP);

    LV_COLUMN column;
    TCHAR szText[64];

    ZeroMemory(&column, sizeof(column));
    column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    column.fmt = LVCFMT_LEFT;
    column.cx = 130;
    StringCbCopy(szText, sizeof(szText), LoadStringDx(IDS_NAME));
    column.pszText = szText;
    ListView_InsertColumn(hLst1, column.iSubItem, &column);
    column.iSubItem++;

    column.fmt = LVCFMT_LEFT;
    column.cx = 130;
    StringCbCopy(szText, sizeof(szText), LoadStringDx(IDS_FILENAME));
    column.pszText = szText;
    ListView_InsertColumn(hLst1, column.iSubItem, &column);
    column.iSubItem++;

    column.fmt = LVCFMT_LEFT;
    column.cx = 120;
    StringCbCopy(szText, sizeof(szText), LoadStringDx(IDS_TYPE));
    column.pszText = szText;
    ListView_InsertColumn(hLst1, column.iSubItem, &column);
    column.iSubItem++;

    OnRefreshListView(hwnd);

    s_bInit = TRUE;

    Lst1_SetSelection(hLst1, 0);

    return TRUE;
}

static void OnPsh1(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem <= 0 || iItem >= INT(s_plugins.size()))
        return;

    std::swap(s_plugins[iItem - 1], s_plugins[iItem]);
    PF_ActOne(&s_plugins[iItem - 1], PLUGIN_ACTION_REFRESH, FALSE, 0);
    PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_REFRESH, FALSE, 0);

    s_bInit = FALSE;
    Lst1_SetItem(hLst1, s_plugins[iItem - 1], iItem - 1);
    Lst1_SetItem(hLst1, s_plugins[iItem], iItem);
    Lst1_SetSelection(hLst1, iItem - 1, TRUE);
    Lst1_SetSelection(hLst1, iItem, FALSE);
    s_bInit = TRUE;

    ListView_EnsureVisible(hLst1, iItem - 1, FALSE);

    EnableWindow(GetDlgItem(hwnd, psh1), iItem - 1 > 0);
    EnableWindow(GetDlgItem(hwnd, psh2), TRUE);
    PF_RefreshAll(s_plugins);
}

static void OnPsh2(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem + 1 >= INT(s_plugins.size()))
        return;

    std::swap(s_plugins[iItem], s_plugins[iItem + 1]);
    PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_REFRESH, FALSE, 0);
    PF_ActOne(&s_plugins[iItem + 1], PLUGIN_ACTION_REFRESH, FALSE, 0);

    s_bInit = FALSE;
    Lst1_SetItem(hLst1, s_plugins[iItem], iItem);
    Lst1_SetItem(hLst1, s_plugins[iItem + 1], iItem + 1);
    Lst1_SetSelection(hLst1, iItem, FALSE);
    Lst1_SetSelection(hLst1, iItem + 1, TRUE);
    s_bInit = TRUE;

    ListView_EnsureVisible(hLst1, iItem + 1, FALSE);

    EnableWindow(GetDlgItem(hwnd, psh1), TRUE);
    EnableWindow(GetDlgItem(hwnd, psh2), iItem + 1 < INT(s_plugins.size() - 1));
    PF_RefreshAll(s_plugins);
}

static void OnPsh3(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    auto& plugin = s_plugins[iItem];
    if (plugin.dwInfo & PLUGIN_INFO_NOCONFIG)
        return;

    PF_ActOne(&plugin, PLUGIN_ACTION_SHOWDIALOG, (WPARAM)g_hMainWnd, TRUE);
    PF_RefreshAll(s_plugins);
}

static void OnPsh4(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    auto& plugin = s_plugins[iItem];
    if ((plugin.dwInfo & PLUGIN_INFO_TYPEMASK) != PLUGIN_INFO_PASS)
        return;

    PF_ActOne(&plugin, PLUGIN_ACTION_SETSTATE,
              PLUGIN_STATE_PASS1, PLUGIN_STATE_PASS1 | PLUGIN_STATE_PASS2);

    Lst1_SetItem(hLst1, plugin, iItem);
    PF_RefreshAll(s_plugins);
}

static void OnPsh5(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0 || iItem >= INT(s_plugins.size()))
        return;

    auto& plugin = s_plugins[iItem];
    if ((plugin.dwInfo & PLUGIN_INFO_TYPEMASK) != PLUGIN_INFO_PASS)
        return;

    PF_ActOne(&plugin, PLUGIN_ACTION_SETSTATE,
              PLUGIN_STATE_PASS2, PLUGIN_STATE_PASS1 | PLUGIN_STATE_PASS2);

    Lst1_SetItem(hLst1, plugin, iItem);
    PF_RefreshAll(s_plugins);
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

static void OnListViewItemChanges(HWND hwnd, BOOL bState, INT iItem = -1)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);

    if (iItem == -1)
    {
        iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    }

    if (0 <= iItem && iItem < INT(s_plugins.size()))
    {
        BOOL bEnabled = ListView_GetCheckState(hLst1, iItem);
        PF_ActOne(&s_plugins[iItem], PLUGIN_ACTION_ENABLE, bEnabled, 0);

        INT iSelected = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
        if (bState && iItem != iSelected)
        {
            s_bInit = FALSE;
            Lst1_SetSelection(hLst1, iSelected, FALSE);
            Lst1_SetSelection(hLst1, iItem, TRUE);
            s_bInit = TRUE;
        }
    }

    PF_RefreshAll(s_plugins);
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
    OnListViewItemChanges(hwnd, FALSE, iItem);
}

static LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    if (!s_bInit)
        return 0;

    INT iItem, cItems;
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    NM_LISTVIEW *pListView;

    switch (pnmhdr->code)
    {
    case LVN_ITEMCHANGED:
        pListView = (NM_LISTVIEW *)pnmhdr;
        iItem = pListView->iItem;

        if (iItem < 0 || iItem >= INT(s_plugins.size()))
        {
            EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
            EnableWindow(GetDlgItem(hwnd, psh2), FALSE);
            EnableWindow(GetDlgItem(hwnd, psh3), FALSE);
            EnableWindow(GetDlgItem(hwnd, psh4), FALSE);
            EnableWindow(GetDlgItem(hwnd, psh5), FALSE);
            return 0;
        }

        if (iItem == 0)
            EnableWindow(GetDlgItem(hwnd, psh1), FALSE);
        else
            EnableWindow(GetDlgItem(hwnd, psh1), TRUE);

        cItems = ListView_GetItemCount(hLst1);
        if (iItem < cItems - 1)
            EnableWindow(GetDlgItem(hwnd, psh2), TRUE);
        else
            EnableWindow(GetDlgItem(hwnd, psh2), FALSE);

        if (s_plugins[iItem].dwInfo & PLUGIN_INFO_NOCONFIG)
        {
            EnableWindow(GetDlgItem(hwnd, psh3), FALSE);
        }
        else
        {
            EnableWindow(GetDlgItem(hwnd, psh3), TRUE);
        }

        if ((s_plugins[iItem].dwInfo & PLUGIN_INFO_TYPEMASK) == PLUGIN_INFO_PASS)
        {
            if (s_plugins[iItem].dwState & PLUGIN_STATE_PASS2)
            {
                EnableWindow(GetDlgItem(hwnd, psh4), TRUE);
                EnableWindow(GetDlgItem(hwnd, psh5), FALSE);
            }
            else
            {
                EnableWindow(GetDlgItem(hwnd, psh4), FALSE);
                EnableWindow(GetDlgItem(hwnd, psh5), TRUE);
            }
        }
        else
        {
            EnableWindow(GetDlgItem(hwnd, psh4), FALSE);
            EnableWindow(GetDlgItem(hwnd, psh5), FALSE);
        }

        OnListViewItemChanges(hwnd, TRUE, iItem);
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
