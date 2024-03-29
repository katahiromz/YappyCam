#define _CRT_SECURE_NO_WARNINGS
#include "YappyCam.hpp"

HWND g_hwndSoundInput = NULL;
static BOOL s_bInit = FALSE;

BOOL get_device_name(CComPtr<IMMDevice> pMMDevice, std::wstring& name)
{
    name.clear();

    CComPtr<IPropertyStore> pPropertyStore;
    HRESULT hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
    if (FAILED(hr))
        return FALSE;

    PROPVARIANT pv;
    PropVariantInit(&pv);
    hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
    if (FAILED(hr))
        return FALSE;

    if (VT_LPWSTR != pv.vt)
    {
        name = L"(unknown)";
        PropVariantClear(&pv);
        return FALSE;
    }

    name = pv.pwszVal;
    PropVariantClear(&pv);
    return TRUE;
}

void DoUpdateDeviceEx(HWND hwnd, INT iDev, INT iFormat)
{
    if (iDev < 0 || size_t(iDev) >= g_sound_devices.size())
        return;

    if (iFormat < 0 || size_t(iFormat) >= m_wave_formats.size())
        return;

    g_settings.m_iSoundDev = iDev;
    g_settings.m_iWaveFormat = iFormat;

    const auto& format = m_wave_formats[iFormat];
    g_sound.SetInfo(format.channels, format.samples, format.bits);
    g_sound.SetDevice(g_sound_devices[iDev]);
}

void DoUpdateDevice(HWND hwnd)
{
    if (!s_bInit)
        return;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    INT iDev = ComboBox_GetCurSel(hCmb1);

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    INT iFormat = ComboBox_GetCurSel(hCmb2);

    DoUpdateDeviceEx(hwnd, iDev, iFormat);
}

static void OnCmb1(HWND hwnd)
{
    g_sound.StopHearing();

    DoUpdateDevice(hwnd);

    g_sound.StartHearing();
}

static void OnCmb2(HWND hwnd)
{
    g_sound.StopHearing();

    DoUpdateDevice(hwnd);

    g_sound.StartHearing();
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hwndSoundInput = hwnd;

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    std::wstring name;
    for (auto& dev : g_sound_devices)
    {
        get_device_name(dev, name);
        ComboBox_AddString(hCmb1, name.c_str());
    }
    ComboBox_SetCurSel(hCmb1, g_settings.m_iSoundDev);

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    for (auto& format : m_wave_formats)
    {
        TCHAR szText[64];
        StringCbPrintf(szText, sizeof(szText),
            LoadStringDx(IDS_FORMAT),
            format.samples, format.bits, format.channels);
        ComboBox_AddString(hCmb2, szText);
    }
    ComboBox_SetCurSel(hCmb2, g_settings.m_iWaveFormat);

    INT x = g_settings.m_nSoundDlgX;
    INT y = g_settings.m_nSoundDlgY;
    if (x != CW_USEDEFAULT && y != CW_USEDEFAULT)
    {
        SetWindowPos(hwnd, NULL, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        SendMessage(hwnd, DM_REPOSITION, 0, 0);
    }

    if (g_settings.m_bNoSound)
        CheckDlgButton(hwnd, chx1, BST_CHECKED);
    else
        CheckDlgButton(hwnd, chx1, BST_UNCHECKED);

    s_bInit = TRUE;

    return TRUE;
}

static void OnChx1(HWND hwnd)
{
    g_settings.m_bNoSound = (IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED);
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        DestroyWindow(hwnd);
        break;
    case cmb1:
        OnCmb1(hwnd);
        break;
    case cmb2:
        OnCmb2(hwnd);
        break;
    case chx1:
        if (codeNotify == BN_CLICKED)
        {
            OnChx1(hwnd);
        }
        break;
    }
}

static void OnDestroy(HWND hwnd)
{
    s_bInit = FALSE;
    g_hwndSoundInput = NULL;

    PostMessage(g_hMainWnd, WM_COMMAND, ID_CONFIGCLOSED, 0);
}

static void OnMove(HWND hwnd, int x, int y)
{
    if (IsMaximized(hwnd) || IsMinimized(hwnd))
        return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    g_settings.m_nSoundDlgX = rc.left;
    g_settings.m_nSoundDlgY = rc.top;
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

BOOL DoSoundInputDialogBox(HWND hwndParent)
{
    if (g_hwndSoundInput)
    {
        SendMessage(g_hwndSoundInput, DM_REPOSITION, 0, 0);
        SetForegroundWindow(g_hwndSoundInput);
        return TRUE;
    }

    CreateDialog(GetModuleHandle(NULL),
                 MAKEINTRESOURCE(IDD_SOUNDINPUT),
                 hwndParent,
                 DialogProc);
    assert(g_hwndSoundInput);
    if (g_hwndSoundInput)
    {
        ShowWindow(g_hwndSoundInput, SW_SHOWNORMAL);
        UpdateWindow(g_hwndSoundInput);
        return TRUE;
    }
    return FALSE;
}
