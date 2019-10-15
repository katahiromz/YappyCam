#ifndef SOUND_HPP_
#define SOUND_HPP_

#include <windows.h>
#include <mmsystem.h>
#ifdef DEFINE_GUIDS
#include <initguid.h>
#endif
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include "CComPtr.hpp"
#include <vector>
#include <cstdio>

struct WAVE_FORMAT_INFO
{
    DWORD flags;
    DWORD samples;
    WORD bits;
    WORD channels;
};

bool get_wave_formats(std::vector<WAVE_FORMAT_INFO>& formats);

bool save_pcm_wave_file(LPTSTR lpszFileName, LPWAVEFORMATEX lpwf,
                        LPCVOID lpWaveData, DWORD dwDataSize);

class Sound
{
public:
    WAVEFORMATEX m_wfx;
    LONG m_nValue;
    LONG m_nMax;
    BOOL m_bRecording;
    void SetInfo(WORD nChannels, DWORD nSamplesPerSec, WORD wBitsPerSample);

    Sound();
    ~Sound();

    void SetDevice(CComPtr<IMMDevice> pDevice);

    BOOL StartHearing();
    BOOL StopHearing();

    BOOL SetRecording(BOOL bRecording);

    void SaveToFile();

    DWORD ThreadProc();

    void SetSoundFile(const TCHAR *filename);

protected:
    HANDLE m_hShutdownEvent;
    HANDLE m_hWakeUp;
    HANDLE m_hThread;
    CComPtr<IMMDevice> m_pDevice;
    CComPtr<IAudioClient> m_pAudioClient;
    CComPtr<IAudioCaptureClient> m_pCaptureClient;
    MMCKINFO m_ckRIFF;
    MMCKINFO m_ckData;
    CRITICAL_SECTION m_lock;
    UINT32 m_nFrames;
    TCHAR m_szSoundFile[MAX_PATH];
    std::vector<BYTE> m_wave_data;

    static DWORD WINAPI ThreadFunction(LPVOID pContext);
    void ScanBuffer(const BYTE *pb, DWORD cb, DWORD dwFlags);
};

#endif  // ndef SOUND_HPP_
