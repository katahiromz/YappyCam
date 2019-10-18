#ifndef MISC_HPP_
#define MISC_HPP_

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif
#include <mmsystem.h>

void ErrorBoxDx(HWND hwnd, LPCTSTR pszText);
LPTSTR LoadStringDx(INT nID);
LPSTR ansi_from_wide(LPCWSTR pszWide);
BOOL DoConvertSoundToWav(HWND hwnd, const TCHAR *temp_file, const TCHAR *wav_file);
bool save_pcm_wave_file(LPCTSTR lpszFileName, LPWAVEFORMATEX lpwf,
                        LPCVOID lpWaveData, DWORD dwDataSize);

#endif  // ndef MISC_HPP_
