#define DEFINE_GUIDS
#include "sound.hpp"
#include "resource.h"
#include <cmath>
#include <strsafe.h>

static const WAVE_FORMAT_INFO s_wave_formats[] =
{
    { WAVE_FORMAT_1M08, 11025, 8, 1 },
    { WAVE_FORMAT_1S08, 11025, 8, 2 },
    { WAVE_FORMAT_1M16, 11025, 16, 1 },
    { WAVE_FORMAT_1S16, 11025, 16, 2 },
    { WAVE_FORMAT_2M08, 22050, 8, 1 },
    { WAVE_FORMAT_2S08, 22050, 8, 2 },
    { WAVE_FORMAT_2M16, 22050, 16, 1 },
    { WAVE_FORMAT_2S16, 22050, 16, 2 },
    { WAVE_FORMAT_4M08, 44100, 8, 1 },
    { WAVE_FORMAT_4S08, 44100, 8, 2 },
    { WAVE_FORMAT_4M16, 44100, 16, 1 },
    { WAVE_FORMAT_4S16, 44100, 16, 2 },
    { WAVE_FORMAT_44M08, 44100, 8, 1 },
    { WAVE_FORMAT_44S08, 44100, 8, 2 },
    { WAVE_FORMAT_44M16, 44100, 16, 1 },
    { WAVE_FORMAT_44S16, 44100, 16, 2 },
    { WAVE_FORMAT_48M08, 48000, 8, 1 },
    { WAVE_FORMAT_48S08, 48000, 8, 2 },
    { WAVE_FORMAT_48M16, 48000, 16, 1 },
    { WAVE_FORMAT_48S16, 48000, 16, 2 },
    { WAVE_FORMAT_96M08, 96000, 8, 1 },
    { WAVE_FORMAT_96S08, 96000, 8, 2 },
    { WAVE_FORMAT_96M16, 96000, 16, 1 },
    { WAVE_FORMAT_96S16, 96000, 16, 2 },
};

bool get_wave_formats(std::vector<WAVE_FORMAT_INFO>& formats)
{
    formats.assign(std::begin(s_wave_formats), std::end(s_wave_formats));
    return true;
}

Sound::Sound()
    : m_nValue(0)
    , m_nMax(0)
    , m_bRecording(FALSE)
    , m_hShutdownEvent(NULL)
    , m_hWakeUp(NULL)
    , m_hThread(NULL)
{
    m_hShutdownEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hWakeUp = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_nFrames = 0;
    ::InitializeCriticalSection(&m_lock);

    ZeroMemory(&m_wfx, sizeof(m_wfx));
    m_wfx.wFormatTag = WAVE_FORMAT_PCM;
    m_wfx.cbSize = 0;
    SetInfo(1, 22050, 8);

    m_szSoundFile[0] = 0;
}

void Sound::SetInfo(WORD nChannels, DWORD nSamplesPerSec, WORD wBitsPerSample)
{
    m_wfx.nChannels = nChannels;
    m_wfx.nSamplesPerSec = nSamplesPerSec;
    m_wfx.wBitsPerSample = wBitsPerSample;
    m_wfx.nBlockAlign = m_wfx.wBitsPerSample * m_wfx.nChannels / 8;
    m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;
}

Sound::~Sound()
{
    if (m_hShutdownEvent)
    {
        ::CloseHandle(m_hShutdownEvent);
        m_hShutdownEvent = NULL;
    }
    if (m_hWakeUp)
    {
        ::CloseHandle(m_hWakeUp);
        m_hWakeUp = NULL;
    }
    if (m_hThread)
    {
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    ::DeleteCriticalSection(&m_lock);
}

DWORD WINAPI Sound::ThreadFunction(LPVOID pContext)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return FALSE;

    Sound *pRecording = reinterpret_cast<Sound *>(pContext);
    DWORD ret = pRecording->ThreadProc();
    CoUninitialize();
    return ret;
}

void Sound::ScanBuffer(const BYTE *pb, DWORD cb, DWORD dwFlags)
{
    m_nMax = 0;
    m_nValue = 0;
    if (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT)
        return;

    float sum = 0;
    double x;
    switch (m_wfx.wBitsPerSample)
    {
    case 8:
        // A PCM WAVE 8-bit sample is unsigned 0-to-255 value.
        for (DWORD i = 0; i < cb; ++i)
        {
            INT s = INT(pb[i]) - 0xFF / 2;
            float e = float(s) / (0xFF / 2);
            sum += e * e;
        }
        x = std::sqrt(sum / cb);
        x = 20 * std::log10(x);
        // Now, x is decibel.
        m_nValue = 35 + INT(x);
        m_nMax = 40;
        break;
    case 16:
        // A PCM WAVE 16-bit sample is signed 16-bit value.
        {
            DWORD cw = cb / 2;
            const WORD *pw = reinterpret_cast<const WORD *>(pb);
            for (DWORD i = 0; i < cw; ++i)
            {
                INT s = SHORT(pw[i]);
                float e = float(s) / (0xFFFF / 2);
                sum += e * e;
            }
            x = std::sqrt(sum / cw);
            x = 20 * std::log10(x);
            // Now, x is decibel.
            m_nValue = 35 + INT(x);
            m_nMax = 40;
        }
        break;
    default:
        assert(0);
        break;
    }
}

DWORD Sound::ThreadProc()
{
    HRESULT hr;

    if (m_pAudioClient)
        m_pAudioClient.Detach();

    hr = m_pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_pAudioClient);
    assert(SUCCEEDED(hr));

    REFERENCE_TIME DevicePeriod;
    hr = m_pAudioClient->GetDevicePeriod(&DevicePeriod, NULL);
    assert(SUCCEEDED(hr));

    m_wave_data.clear();

    UINT32 nBlockAlign = m_wfx.nBlockAlign;
    WORD wBitsPerSample = m_wfx.wBitsPerSample;
    m_nFrames = 0;

#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
    #define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif
#ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
    #define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#endif
    DWORD StreamFlags =
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
        AUDCLNT_STREAMFLAGS_NOPERSIST |
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
        AUDCLNT_STREAMFLAGS_LOOPBACK;

    hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                    StreamFlags,
                                    0, 0, &m_wfx, 0);
    if (SUCCEEDED(hr))
    {
        ::PlaySound(MAKEINTRESOURCE(IDR_SILENT_WAV), GetModuleHandle(NULL),
                    SND_ASYNC | SND_LOOP | SND_NODEFAULT |
                    SND_RESOURCE);
    }
    else if (hr == AUDCLNT_E_WRONG_ENDPOINT_TYPE)
    {
        StreamFlags &= ~AUDCLNT_STREAMFLAGS_LOOPBACK;
        hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                        StreamFlags,
                                        0, 0, &m_wfx, 0);
    }
    assert(SUCCEEDED(hr));

    hr = m_pAudioClient->SetEventHandle(m_hWakeUp);
    assert(SUCCEEDED(hr));

    if (m_pCaptureClient)
        m_pCaptureClient.Detach();

    hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_pCaptureClient);
    assert(SUCCEEDED(hr));

    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(L"Audio", &nTaskIndex);
    assert(hTask);

    hr = m_pAudioClient->Start();
    assert(SUCCEEDED(hr));

    HANDLE waitArray[2] = { m_hShutdownEvent, m_hWakeUp };

    bool bKeepRecording = true;
    bool bFirstPacket = true;
    BYTE *pbData;
    UINT32 uNumFrames;
    DWORD dwFlags;
    BOOL bRecorded = FALSE;

    for (UINT32 nPasses = 0; bKeepRecording; nPasses++)
    {
        UINT32 nNextPacketSize;
        for (hr = m_pCaptureClient->GetNextPacketSize(&nNextPacketSize);
             SUCCEEDED(hr) && nNextPacketSize > 0;
             hr = m_pCaptureClient->GetNextPacketSize(&nNextPacketSize))
        {
            hr = m_pCaptureClient->GetBuffer(&pbData, &uNumFrames, &dwFlags, NULL, NULL);
            assert(SUCCEEDED(hr));

            LONG cbToWrite = uNumFrames * nBlockAlign;

            if (m_bRecording)
            {
                bRecorded = TRUE;
                ::EnterCriticalSection(&m_lock);
                const size_t max_size = 1 * 1024 * 1024; // 1 MB
                if (m_wave_data.size() >= max_size)
                {
                    FlushData(FALSE);
                }
                else
                {
                    m_wave_data.insert(m_wave_data.end(), pbData, pbData + cbToWrite);
                }
                ::LeaveCriticalSection(&m_lock);
            }

            ScanBuffer(pbData, cbToWrite, dwFlags);

            m_nFrames += uNumFrames;
            hr = m_pCaptureClient->ReleaseBuffer(uNumFrames);
            assert(SUCCEEDED(hr));

            bFirstPacket = false;
        }

        DWORD waitResult = ::WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0:
            bKeepRecording = false;
            break;
        case WAIT_OBJECT_0 + 1:
            break;
        default:
            bKeepRecording = false;
            break;
        }
    }

    if (bRecorded)
    {
        FlushData(TRUE);
    }

    return 0;
}
