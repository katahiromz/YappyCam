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

// The bang IDs.
typedef enum BANGID
{
    BANGID_NONE = 0,
    BANGID_ENABLE,
    BANGID_DISABLE,
    BANGID_TOGGLE,
    BANGID_SOUND_0 = 100,
    BANGID_TRIGGER_0 = 200
} BANGID;

typedef struct BANG_INFO
{
    INT nBangID;
    LPCWSTR name;
} BANG_INFO;

// Driver: DRIVERFUNC_LISTPLUGINS (1)
//      Meaning: Get the plugin from filename.
//      Parameters:
//         wParam: zero;
//         lParam: LPINT pnPluginCount;
//      Return value:
//         The pointer to the PLUGIN structures, or NULL.
#define DRIVERFUNC_LISTPLUGINS 1

// Driver: DRIVERFUNC_GETPLUGIN (2)
//      Meaning: Get the plugin from filename.
//      Parameters:
//         wParam: LPCWSTR filename;
//         lParam: zero;
//      Return value:
//         The pointer to The PLUGIN structure, or NULL.
#define DRIVERFUNC_GETPLUGIN 2

// Driver: DRIVERFUNC_GETBANGLIST (3)
//      Meaning: Get the list of the bangs;
//      Parameters:
//         wParam: LPINT pnBangCount;
//         lParam: LPCWSTR filename;
//      Return value:
//         const BANG_INFO *pList;
#define DRIVERFUNC_GETBANGLIST 3

// Driver: DRIVERFUNC_DOBANG (4)
//      Meaning: Do something.
//      Parameters:
//         wParam: INT nBangID;
//         lParam: LPCWSTR filename;
//      Return value:
//         TRUE or FALSE;
#define DRIVERFUNC_DOBANG 4

// Driver: DRIVERFUNC_GETFACES (5)
//      Meaning: Get faces.
//      Parameters:
//         wParam: SIZE_T nNumFaces;
//         lParam: cv::Rect *pFaces;
//      Return value:
//         The number of faces.
#define DRIVERFUNC_GETFACES 5

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

    // TODO: Add more members and version up...
#define PLUGIN_INFO_PASS 0x1
#define PLUGIN_INFO_PICINPUT 0x2
#define PLUGIN_INFO_SWITCHBOARD 0x3
#define PLUGIN_INFO_BROADCASTER 0x5
#define PLUGIN_INFO_TYPEMASK 0xF
#define PLUGIN_INFO_NOCONFIG 0x10
#define PLUGIN_INFO_USEFACES 0x20
    DWORD dwInfo;
#define PLUGIN_STATE_PASS1 0x0
#define PLUGIN_STATE_PASS2 0x1
    DWORD dwState;
    BOOL bEnabled;

    // Use freely:
    HWND plugin_window;
    void *p_user_data;
    LPARAM l_user_data;
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
//      Return value: TRUE to continue;
#define PLUGIN_ACTION_STARTREC 1

// Action: PLUGIN_ACTION_PAUSE (2)
//      Meaning: Paused or resumed.
//      Parameters:
//         wParam: TRUE for paused, FALSE for resumed;
//         lParam: zero;
//      Return value: TRUE to continue;
#define PLUGIN_ACTION_PAUSE 2

// Action: PLUGIN_ACTION_ENDREC (3)
//      Meaning: Recording ended.
//      Parameters: zero;
//      Return value: TRUE to continue;
#define PLUGIN_ACTION_ENDREC 3

// Action: PLUGIN_ACTION_PASS (4)
//      Meaning: Do process Pass.
//      Parameters:
//         wParam: cv::Mat* pmat;
//         lParam: zero;
//      Return value: TRUE to continue;
#define PLUGIN_ACTION_PASS 4

// Action: PLUGIN_ACTION_ENABLE (5)
//      Meaning: Enable/Disable the plugin.
//      Parameters:
//         wParam: INT nSwitch;
//            0: Disable, 1: Enable, 2: Toggle;
//         lParam: zero;
//      Return value: zero;
#define PLUGIN_ACTION_ENABLE 5

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
//      Return value: TRUE to continue;
#define PLUGIN_ACTION_REFRESH 7

// Action: PLUGIN_ACTION_SETSTATE (8)
//      Meaning: Set the state flags of the plugin.
//      Parameters:
//         wParam: DWORD dwState;
//         lParam: DWORD dwStateMask;
//      Return value:
//         dwState if successful and dwStateMask is zero;
//         TRUE if successful and dwStateMask is non-zero;
#define PLUGIN_ACTION_SETSTATE 8

// Action: PLUGIN_ACTION_DOBANG (9)
//      Meaning: Do something.
//      Parameters:
//         wParam: INT nBangID;
//         lParam: zero;
//      Return value:
//         TRUE to continue;
#define PLUGIN_ACTION_DOBANG 9

// Action: PLUGIN_ACTION_GETBANGLIST (10)
//      Meaning: Get the name of the bang.
//      Parameters:
//         wParam: LPINT pnBangCount;
//         lParam: zero;
//      Return value:
//         const BANG_INFO *pList or NULL;
#define PLUGIN_ACTION_GETBANGLIST 10

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // ndef PLUGIN_H_
