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
#define PLUGIN_FLAG_PICREADER 0x00000001
#define PLUGIN_FLAG_PICWRITER 0x00000002
    DWORD dwFlags;
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

// Action: PLUGIN_ACTION_INIT (1)
//      Meaning: Initialize.
//      Parameters: zero;
//      Return value: zero;
#define PLUGIN_ACTION_INIT 1

// Action: PLUGIN_ACTION_UNINIT (2)
//      Meaning: Uninitialize.
//      Parameters: zero;
//      Return value: zero;
#define PLUGIN_ACTION_UNINIT 2

// Action: PLUGIN_ACTION_STARTREC (3)
//      Meaning: Recording started.
//      Parameters: zero;
//      Return value: zero;
#define PLUGIN_ACTION_STARTREC 3

// Action: PLUGIN_ACTION_PAUSE (4)
//      Meaning: Paused or resumed.
//      Parameters:
//         wParam: TRUE for paused, FALSE for resumed;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_PAUSE 4

// Action: PLUGIN_ACTION_ENDREC (5)
//      Meaning: Recording ended.
//      Parameters: zero;
//      Return value: zero;
#define PLUGIN_ACTION_ENDREC 5

// Action: PLUGIN_ACTION_PICREAD (6)
//      Meaning: Read from a picture.
//      Parameters:
//         wParam: const cv::Mat* pmat;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_PICREAD 6

// Action: PLUGIN_ACTION_PICWRITE (7)
//      Meaning: Write on a picture.
//      Parameters:
//         wParam: cv::Mat* pmat;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_PICWRITE 7

// Action: PLUGIN_ACTION_SETTINGS (8)
//      Meaning: Show/Hide a modeless dialog for plugin settings
//      Parameters:
//         wParam: HWND hwndMainWnd; /* main window */
//         lParam: BOOL bShowOrHide; /* TRUE for show, FALSE for hide */
//      Return value: zero;
#define PLUGIN_ACTION_SETTINGS 8

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // ndef PLUGIN_H_
