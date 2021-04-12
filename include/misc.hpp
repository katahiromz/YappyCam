#ifndef MISC_HPP_
#define MISC_HPP_

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif
#include <mmsystem.h>
#include <vfw.h>
#include <vector>

void ErrorBoxDx(HWND hwnd, LPCTSTR pszText);
LPTSTR LoadStringDx(INT nID);
LPSTR ansi_from_wide(LPCWSTR pszWide);
LPWSTR wide_from_ansi(LPCSTR pszAnsi);
BOOL DoConvertSoundToWav(HWND hwnd, const TCHAR *temp_file, const TCHAR *wav_file);
bool save_pcm_wave_file(LPCTSTR lpszFileName, LPWAVEFORMATEX lpwf,
                        LPCVOID lpWaveData, DWORD dwDataSize);

BOOL DoSaveAviFile(HWND hwnd, LPCTSTR pszFileName, PAVISTREAM paviVideo,
                   PAVISTREAM paviAudio);
BOOL DoUniteAviAndWav(HWND hwnd, const WCHAR *new_avi,
                      const WCHAR *old_avi, const WCHAR *wav_file);

BOOL DoGetMonitorsEx(std::vector<MONITORINFO>& monitors, MONITORINFO& primary);

#endif  // ndef MISC_HPP_
