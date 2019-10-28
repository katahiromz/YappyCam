// PluginFramework.h --- PluginFramework
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#ifndef PLUGIN_FRAMEWORK_H_
#define PLUGIN_FRAMEWORK_H_

#include "Plugin.h"
#include <vector>

BOOL PF_LoadOne(PLUGIN *pi, const TCHAR *pathname);
INT PF_LoadAll(std::vector<PLUGIN>& pis, const TCHAR *dir);
BOOL PF_IsLoaded(const PLUGIN *pi);
LRESULT PF_ActOne(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam);
LRESULT PF_ActAll(std::vector<PLUGIN>& pis, UINT uAction, WPARAM wParam, LPARAM lParam);
BOOL PF_UnloadOne(PLUGIN *pi);
BOOL PF_UnloadAll(std::vector<PLUGIN>& pis);

#endif  // ndef PLUGIN_FRAMEWORK_H_
