#define _CRT_SECURE_NO_WARNINGS
#include "misc.hpp"
#include <vector>
#include <cassert>

void ErrorBoxDx(HWND hwnd, LPCTSTR pszText)
{
    MessageBox(hwnd, pszText, NULL, MB_ICONERROR);
}

LPTSTR LoadStringDx(INT nID)
{
    static UINT s_index = 0;
    const UINT cchBuffMax = 1024;
    static TCHAR s_sz[2][cchBuffMax];

    TCHAR *pszBuff = s_sz[s_index];
    s_index = (s_index + 1) % ARRAYSIZE(s_sz);
    pszBuff[0] = 0;
    if (!::LoadString(NULL, nID, pszBuff, cchBuffMax))
        assert(0);
    return pszBuff;
}

LPSTR ansi_from_wide(LPCWSTR pszWide)
{
    static char s_buf[256];
    WideCharToMultiByte(CP_ACP, 0, pszWide, -1, s_buf, ARRAYSIZE(s_buf), NULL, NULL);
    return s_buf;
}

LPWSTR wide_from_ansi(LPCSTR pszAnsi)
{
    static WCHAR s_buf[256];
    MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, s_buf, ARRAYSIZE(s_buf));
    return s_buf;
}

bool save_pcm_wave_file(LPCTSTR lpszFileName, LPWAVEFORMATEX lpwf,
                        LPCVOID lpWaveData, DWORD dwDataSize)
{
    HMMIO    hmmio;
    MMCKINFO mmckRiff;
    MMCKINFO mmckFmt;
    MMCKINFO mmckData;
    
    hmmio = mmioOpen(const_cast<LPTSTR>(lpszFileName), NULL, MMIO_CREATE | MMIO_WRITE);
    if (hmmio == NULL)
        return false;

    mmckRiff.fccType = mmioStringToFOURCC(TEXT("WAVE"), 0);
    mmioCreateChunk(hmmio, &mmckRiff, MMIO_CREATERIFF);

    mmckFmt.ckid = mmioStringToFOURCC(TEXT("fmt "), 0);
    mmioCreateChunk(hmmio, &mmckFmt, 0);
    mmioWrite(hmmio, (const char *)lpwf, sizeof(PCMWAVEFORMAT));
    mmioAscend(hmmio, &mmckFmt, 0);

    mmckData.ckid = mmioStringToFOURCC(TEXT("data"), 0);
    mmioCreateChunk(hmmio, &mmckData, 0);
    mmioWrite(hmmio, (const char *)lpWaveData, dwDataSize);
    mmioAscend(hmmio, &mmckData, 0);

    mmioAscend(hmmio, &mmckRiff, 0);
    mmioClose(hmmio, 0);

    return true;
}

BOOL DoConvertSoundToWav(HWND hwnd, const TCHAR *temp_file, const TCHAR *wav_file)
{
    HANDLE hTempFile;
    hTempFile = CreateFile(temp_file, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, 0, NULL);
    if (hTempFile == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL bOK = FALSE;
    WAVEFORMATEX wfx;
    DWORD cbRead;
    DWORD cbFile = GetFileSize(hTempFile, NULL);
    if (cbFile == 0xFFFFFFFF || cbFile <= sizeof(wfx))
    {
        CloseHandle(hTempFile);
        return FALSE;
    }

    if (ReadFile(hTempFile, &wfx, sizeof(wfx), &cbRead, NULL) &&
        cbRead == sizeof(wfx))
    {
        DWORD cbData = cbFile - cbRead;
        std::vector<BYTE> data;
        data.resize(cbData);

        if (ReadFile(hTempFile, &data[0], cbData, &cbRead, NULL) &&
            cbRead == data.size())
        {
            if (save_pcm_wave_file(wav_file, &wfx, &data[0], cbData))
            {
                bOK  = TRUE;
            }
        }
    }

    CloseHandle(hTempFile);
    return bOK;
}

BOOL DoSaveAviFile(HWND hwnd, LPCTSTR pszFileName, PAVISTREAM paviVideo,
                   PAVISTREAM paviAudio)
{
    INT i;
    PAVISTREAM pavis[2];
    AVICOMPRESSOPTIONS options[2];
    LPAVICOMPRESSOPTIONS lpOptions[2];
    INT nCount = 2;

    
    for (i = 0; i < nCount; i++)
    {
        ZeroMemory(&options[i], sizeof(AVICOMPRESSOPTIONS));
        lpOptions[i] = &options[i];
    }

    pavis[0] = paviVideo;
    pavis[1] = paviAudio;
    //AVISaveOptions(NULL, 0, nCount, pavis, lpOptions);

    INT nAVIERR;
    nAVIERR = AVISaveV(pszFileName, NULL, NULL, nCount, pavis, lpOptions);
    if (nAVIERR != AVIERR_OK)
    {
        assert(0);
        //AVISaveOptionsFree(nCount, lpOptions);
        return FALSE;
    }

    //AVISaveOptionsFree(nCount, lpOptions);
    return TRUE;
}

BOOL DoUniteAviAndWav(HWND hwnd, const WCHAR *new_avi,
                      const WCHAR *old_avi, const WCHAR *wav_file)
{
    AVIFileInit();

    INT nAVIERR;

    PAVISTREAM paviVideo;
    nAVIERR = AVIStreamOpenFromFile(&paviVideo, old_avi, streamtypeVIDEO, 0,
                                    OF_READ | OF_SHARE_DENY_NONE, NULL);
    if (nAVIERR)
    {
        assert(0);
        AVIFileExit();
        return FALSE;
    }

    BOOL ret;
    PAVISTREAM paviAudio = NULL;
    if (wav_file)
    {
        // there is sound data
        nAVIERR = AVIStreamOpenFromFile(&paviAudio, wav_file, streamtypeAUDIO, 0,
                                        OF_READ | OF_SHARE_DENY_NONE, NULL);
        if (nAVIERR)
        {
            assert(0);
            AVIStreamRelease(paviVideo);
            AVIFileExit();
            return FALSE;
        }
        ret = DoSaveAviFile(hwnd, new_avi, paviVideo, paviAudio);
    }
    else
    {
        // there is no sound data
        ret = CopyFile(old_avi, new_avi, FALSE);
    }

    if (paviAudio)
    {
        AVIStreamRelease(paviAudio);
    }
    AVIStreamRelease(paviVideo);
    AVIFileExit();

    return ret;
}
