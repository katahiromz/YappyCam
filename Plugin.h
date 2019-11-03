// Plugin.h --- PluginFramework Plugin interface
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#ifndef PLUGIN_H_
#define PLUGIN_H_
// TODO: Rename this file

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif
#include <opencv2/opencv.hpp>

// TODO: Change me!
#ifndef FRAMEWORK_NAME
    #define FRAMEWORK_NAME TEXT("YappyCamPlugin")
#endif

// TODO: Change me!
#ifndef FRAMEWORK_SPEC
    #define FRAMEWORK_SPEC TEXT("*.yap")
#endif

// TODO: Change me!
#ifndef FRAMEWORK_VERSION
    #define FRAMEWORK_VERSION 1
#endif

struct PLUGIN;
struct PLUGIN_FRAMEWORK_IMPL;

typedef BOOL (APIENTRY *PLUGIN_LOAD)(struct PLUGIN *pi, LPARAM lParam);
typedef BOOL (APIENTRY *PLUGIN_UNLOAD)(struct PLUGIN *pi, LPARAM lParam);
typedef LRESULT (APIENTRY *PLUGIN_ACT)(struct PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam);

typedef LRESULT (APIENTRY *PLUGIN_DRIVER)(struct PLUGIN *pi, UINT uFunc, WPARAM wParam, LPARAM lParam);

// NOTE: This structure must be a POD (Plain Old Data).
typedef struct PLUGIN
{
    // Don't change:
    DWORD framework_version;
    TCHAR framework_name[32];
    HINSTANCE framework_instance;
    HWND framework_window;
    TCHAR plugin_pathname[MAX_PATH];
    struct PLUGIN_FRAMEWORK_IMPL *framework_impl;
    PLUGIN_DRIVER driver;

    // Please fill them in Plugin_Load:
    DWORD plugin_version;
    TCHAR plugin_product_name[64];
    TCHAR plugin_filename[32];
    TCHAR plugin_company[64];
    TCHAR plugin_copyright[128];
    HINSTANCE plugin_instance;

    // Use freely:
    HWND plugin_window;
    void *p_user_data;
    LPARAM l_user_data;

    // TODO: Add more members and version up...
#define PLUGIN_INFO_PASS 0x1
#define PLUGIN_INFO_PICINPUT 0x2
#define PLUGIN_INFO_TRIGGERBOX 0x3
#define PLUGIN_INFO_SOUNDBOX 0x4
#define PLUGIN_INFO_BROADCASTER 0x5
#define PLUGIN_INFO_TYPEMASK 0xF
#define PLUGIN_INFO_NOCONFIG 0x10
    DWORD dwInfoFlags;
#define PLUGIN_STATE_PASS1 0x0
#define PLUGIN_STATE_PASS2 0x1
    DWORD dwStateFlags;
    BOOL bEnabled;
} PLUGIN;

#ifdef __cplusplus
extern "C" {
#endif

// API Name: Plugin_Load
// Purpose: The framework want to load the plugin component.
// TODO: Load the plugin component.
BOOL APIENTRY Plugin_Load(PLUGIN *pi, LPARAM lParam);

// API Name: Plugin_Unload
// Purpose: The framework want to unload the plugin component.
// TODO: Unload the plugin component.
BOOL APIENTRY Plugin_Unload(PLUGIN *pi, LPARAM lParam);

// API Name: Plugin_Act
// Purpose: Act something on the plugin.
// TODO: Act something on the plugin.
LRESULT APIENTRY Plugin_Act(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam);

// TODO: Add more APIs

//////////////////////////////////////////////////////////////////////////////
// Actions
//

// Action: PLUGIN_ACTION_STARTREC (1)
//      Meaning: Recording started.
//      Parameters: zero;
//      Return value: zero;
#define PLUGIN_ACTION_STARTREC 1

// Action: PLUGIN_ACTION_PAUSE (2)
//      Meaning: Paused or resumed.
//      Parameters:
//         wParam: TRUE for paused, FALSE for resumed;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_PAUSE 2

// Action: PLUGIN_ACTION_ENDREC (3)
//      Meaning: Recording ended.
//      Parameters: zero;
//      Return value: zero;
#define PLUGIN_ACTION_ENDREC 3

// Action: PLUGIN_ACTION_PASS1 (4)
//      Meaning: Do process Pass 1.
//      Parameters:
//         wParam: cv::Mat* pmat;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_PASS1 4

// Action: PLUGIN_ACTION_PASS2 (5)
//      Meaning: Do process Pass 2.
//      Parameters:
//         wParam: cv::Mat* pmat;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_PASS2 5

// Action: PLUGIN_ACTION_SHOWDIALOG (6)
//      Meaning: Show/Hide a modeless dialog for plugin settings
//      Parameters:
//         wParam: HWND hwndMainWnd; /* main window */
//         lParam: BOOL bShowOrHide; /* TRUE for show, FALSE for hide */
//      Return value: zero;
#define PLUGIN_ACTION_SHOWDIALOG 6

// Action: PLUGIN_ACTION_REFRESH (7)
//      Meaning: Refresh/Reset the plugin.
//      Parameters:
//         wParam: BOOL bResetSettings;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_REFRESH 7

// Action: PLUGIN_ACTION_SETSTATE (8)
//      Meaning: Set the state flags of the plugin.
//      Parameters:
//         wParam: DWORD dwState;
//         lParam: DWORD dwStateMask;
//      Return value:
//         dwStateFlags if successful and dwStateMask is zero;
//         TRUE if successful and dwStateMask is non-zero;
#define PLUGIN_ACTION_SETSTATE 8

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // ndef PLUGIN_H_
