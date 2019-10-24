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
#include "ccomptr.hpp"
#include <vector>
#include <cstdio>
#include <strsafe.h>

/////////////////////////////////////////////////////////////////////////////

struct WAVE_FORMAT_INFO
{
    DWORD flags;
    DWORD samples;
    WORD bits;
    WORD channels;
};

bool get_wave_formats(std::vector<WAVE_FORMAT_INFO>& formats);

/////////////////////////////////////////////////////////////////////////////
// Sound class

class Sound
{
public:
    WAVEFORMATEX m_wfx;
    LONG m_nValue;
    LONG m_nMax;
    BOOL m_bRecording;
    FILE *m_fp;
    CRITICAL_SECTION m_lock;

    Sound();
    ~Sound();

    void SetDevice(CComPtr<IMMDevice> pDevice);
    void SetInfo(WORD nChannels, DWORD nSamplesPerSec, WORD wBitsPerSample);

    BOOL StartHearing();
    BOOL StopHearing();

    BOOL SetRecording(BOOL bRecording);

    void FlushData(BOOL bLock);

    DWORD ThreadProc();

    void SetSoundFile(const TCHAR *filename);
    BOOL OpenSoundFile();

protected:
    HANDLE m_hShutdownEvent;
    HANDLE m_hWakeUp;
    HANDLE m_hThread;
    CComPtr<IMMDevice> m_pDevice;
    CComPtr<IAudioClient> m_pAudioClient;
    CComPtr<IAudioCaptureClient> m_pCaptureClient;
    MMCKINFO m_ckRIFF;
    MMCKINFO m_ckData;
    UINT32 m_nFrames;
    TCHAR m_szSoundFile[MAX_PATH];

    static DWORD WINAPI ThreadFunction(LPVOID pContext);
    void ScanBuffer(const BYTE *pb, DWORD cb, DWORD dwFlags);
};

/////////////////////////////////////////////////////////////////////////////
// inlining

inline BOOL Sound::SetRecording(BOOL bRecording)
{
    m_bRecording = bRecording;
    return TRUE;
}

inline void Sound::SetDevice(CComPtr<IMMDevice> pDevice)
{
    m_pDevice = pDevice;
}

inline void Sound::SetSoundFile(const TCHAR *filename)
{
    StringCbCopy(m_szSoundFile, sizeof(m_szSoundFile), filename);
}

inline BOOL Sound::StartHearing()
{
    DWORD tid = 0;
    m_hThread = ::CreateThread(NULL, 0, Sound::ThreadFunction, this, 0, &tid);
    return m_hThread != NULL;
}

inline BOOL Sound::StopHearing()
{
    m_bRecording = FALSE;

    SetEvent(m_hShutdownEvent);

    if (m_pAudioClient)
    {
        m_pAudioClient->Stop();
    }

    if (m_hThread)
    {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    ::PlaySound(NULL, NULL, 0);

    return TRUE;
}

inline void Sound::FlushData(BOOL bLock)
{
    if (bLock)
    {
        EnterCriticalSection(&m_lock);
    }

    fclose(m_fp);
    m_fp = NULL;

    if (bLock)
    {
        LeaveCriticalSection(&m_lock);
    }
}

static char s_sound_buf[40 * 1024];

inline BOOL Sound::OpenSoundFile()
{
    assert(!m_fp);
    m_fp = _wfopen(m_szSoundFile, L"wb");
    if (m_fp)
    {
        setvbuf(m_fp, s_sound_buf, _IOFBF, sizeof(s_sound_buf));
        if (fwrite(&m_wfx, sizeof(m_wfx), 1, m_fp))
        {
            return TRUE;
        }
        fclose(m_fp);
        m_fp = NULL;
        DeleteFile(m_szSoundFile);
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

#endif  // ndef SOUND_HPP_
