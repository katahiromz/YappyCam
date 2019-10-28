// Plugin1.c --- PluginFramework Plugin #1
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "Plugin.h"
#include <cassert>
#include <cstdio>
#include <strsafe.h>

static HINSTANCE s_hinstDLL;

extern "C" {

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

    pi->plugin_version = 1;
    StringCbCopy(pi->plugin_product_name, sizeof(pi->plugin_product_name), TEXT("Clock"));
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("Clock.yap"));
    StringCbCopy(pi->plugin_company, sizeof(pi->plugin_company), TEXT("Katayama Hirofumi MZ"));
    StringCbCopy(pi->plugin_copyright, sizeof(pi->plugin_copyright), TEXT("Copyright (C) 2019 Katayama Hirofumi MZ"));
    pi->plugin_instance = s_hinstDLL;
    pi->plugin_window = NULL;
    pi->dwFlags = PLUGIN_FLAG_PICREADER | PLUGIN_FLAG_PICWRITER;
    return TRUE;
}

// API Name: Plugin_Unload
// Purpose: The framework want to unload the plugin component.
// TODO: Unload the plugin component.
BOOL APIENTRY
Plugin_Unload(PLUGIN *pi, LPARAM lParam)
{
    return TRUE;
}

void DoDrawText(cv::Mat& mat, int align, int valign,
                const char *text, double scale, int thickness,
                cv::Scalar& color)
{
    int font = cv::FONT_HERSHEY_SIMPLEX;
    cv::Size screen_size(mat.cols, mat.rows);

    int baseline;
    cv::Size text_size = cv::getTextSize(text, font, scale, thickness, &baseline);

    cv::Point pt;

    switch (align)
    {
    case -1:
        pt.x = 0;
        break;
    case 0:
        pt.x = (screen_size.width - text_size.width) / 2;
        break;
    case 1:
        pt.x = screen_size.width - text_size.width;
        break;
    default:
        assert(0);
    }

    switch (valign)
    {
    case -1:
        pt.y = 0;
        break;
    case 0:
        pt.y = (screen_size.height - text_size.height) / 2;
        break;
    case 1:
        pt.y = screen_size.height - text_size.height;
        break;
    default:
        assert(0);
    }
    pt.y += text_size.height - 1;

    cv::putText(mat, text, pt, font, scale,
                color, thickness, cv::LINE_AA, false);
}

static LRESULT Plugin_PicRead(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    const cv::Mat *pmat = (const cv::Mat *)wParam;
    return 0;
}

static LRESULT Plugin_PicWrite(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    cv::Mat& mat = *(cv::Mat *)wParam;

    SYSTEMTIME st;
    GetLocalTime(&st);

    char szText[MAX_PATH];
    StringCbPrintfA(szText, sizeof(szText),
        "%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);

    cv::Scalar black(0, 0, 0);
    cv::Scalar white(255, 255, 255);

    DoDrawText(mat, 0, 0, szText, 3, 10, black);
    DoDrawText(mat, 0, 0, szText, 3, 5, white);
    return 0;
}

// API Name: Plugin_Act
// Purpose: Act something on the plugin.
// TODO: Act something on the plugin.
LRESULT APIENTRY
Plugin_Act(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam)
{
    switch (uAction)
    {
    case PLUGIN_ACTION_INIT:
    case PLUGIN_ACTION_UNINIT:
    case PLUGIN_ACTION_STARTREC:
    case PLUGIN_ACTION_PAUSE:
    case PLUGIN_ACTION_ENDREC:
        break;
    case PLUGIN_ACTION_PICREAD:
        return Plugin_PicRead(pi, wParam, lParam);
    case PLUGIN_ACTION_PICWRITE:
        return Plugin_PicWrite(pi, wParam, lParam);
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
