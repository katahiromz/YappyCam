#include "misc.hpp"
#include <cstdio>
#include <string>
#include <shlwapi.h>
#include <strsafe.h>
#include <opencv2/opencv.hpp>

BOOL Finalize(const char *dir, const char *avi_file)
{
    WCHAR szPath[MAX_PATH];

    // output_name
    std::string output_name = dir;
    output_name += "-tmp.avi";

    // strMovieDir
    std::wstring strMovieDir = wide_from_ansi(dir);

    // strOldMovieName
    std::wstring strOldMovieName = wide_from_ansi(dir);
    strOldMovieName += L"-tmp.avi";

    // strMovieInfoFile
    StringCbCopy(szPath, sizeof(szPath), strMovieDir.c_str());
    PathAppend(szPath, L"movie_info.ini");
    std::wstring strMovieInfoFile = szPath;

    INT nStartIndex = 0;
    INT nEndIndex = 999;
    INT nFPSx100 = 400;
    BOOL bNoSound = FALSE;
    if (PathFileExists(strMovieInfoFile.c_str()))
    {
        nStartIndex = GetPrivateProfileInt(L"Info", L"StartIndex", nStartIndex, strMovieInfoFile.c_str());
        nEndIndex = GetPrivateProfileInt(L"Info", L"EndIndex", nEndIndex, strMovieInfoFile.c_str());
        nFPSx100 = GetPrivateProfileInt(L"Info", L"FPSx100", nFPSx100, strMovieInfoFile.c_str());
        bNoSound = !!GetPrivateProfileInt(L"Info", L"NoSound", bNoSound, strMovieInfoFile.c_str());
    }

    // image_name
    std::string image_name;
    if (PathFileExists(strMovieInfoFile.c_str()))
    {
        TCHAR szText[128];
        GetPrivateProfileString(L"Info", L"ImageFile", L"", szText, ARRAYSIZE(szText), strMovieInfoFile.c_str());
        if (szText[0])
        {
            StringCbCopy(szPath, sizeof(szPath), strMovieDir.c_str());
            PathAppend(szPath, szText);
            image_name = ansi_from_wide(szPath);
        }
    }
    if (image_name.empty())
    {
        assert(0);
        return FALSE;
    }

    // strSoundTempFile
    std::wstring strSoundTempFile;
    if (!bNoSound && PathFileExists(strMovieInfoFile.c_str()))
    {
        TCHAR szText[128];
        GetPrivateProfileString(L"Info", L"SoundTempFile", L"", szText, ARRAYSIZE(szText), strMovieInfoFile.c_str());
        if (szText[0])
        {
            StringCbCopy(szPath, sizeof(szPath), strMovieDir.c_str());
            PathAppend(szPath, szText);
            if (PathFileExists(szPath))
            {
                strSoundTempFile = szPath;
            }
        }
    }
    if (!bNoSound && strSoundTempFile.empty())
    {
        assert(0);
        return FALSE;
    }

    // strWavName
    StringCbCopy(szPath, sizeof(szPath), strMovieDir.c_str());
    PathAppend(szPath, L"sound.wav");
    std::wstring strWavName = szPath;

    // get first frame
    CHAR szImageName[MAX_PATH];
    cv::Mat frame;
    StringCbPrintfA(szImageName, sizeof(szImageName), image_name.c_str(), nStartIndex);
    frame = cv::imread(szImageName);
    if (!frame.data)
    {
        assert(0);
        return FALSE;
    }

    // get width and height
    int width, height;
    width = frame.cols;
    height = frame.rows;

    double fps = nFPSx100 / 100.0;
    int fourcc = 0x7634706d; // mp4v

    // video writer
    cv::VideoWriter writer(output_name.c_str(), fourcc, fps, cv::Size(width, height));
    if (!writer.isOpened())
    {
        assert(0);
        return FALSE;
    }

    // for each frames...
    for (INT i = nStartIndex; i <= nEndIndex; ++i)
    {
        // load frame image
        StringCbPrintfA(szImageName, sizeof(szImageName), image_name.c_str(), i);
        frame = cv::imread(szImageName);
        if (!frame.data)
        {
            continue;
        }

        // write frame
        writer << frame;
    }
    writer.release();

    // strNewMovieName
    std::wstring strNewMovieName = wide_from_ansi(avi_file);

    BOOL ret;
    if (bNoSound)
    {
        ret = CopyFile(strOldMovieName.c_str(), strNewMovieName.c_str(), FALSE);
        assert(ret);
    }
    else
    {
        // convert sound to wav
        std::wstring strDotExt = PathFindExtension(strSoundTempFile.c_str());
        if (lstrcmpi(strDotExt.c_str(), L".wav") == 0)
        {
            CopyFile(strSoundTempFile.c_str(), strWavName.c_str(), FALSE);
        }
        else
        {
            DoConvertSoundToWav(NULL, strSoundTempFile.c_str(), strWavName.c_str());
        }

        // unite the movie and the sound
        ret = DoUniteAviAndWav(NULL,
                               strNewMovieName.c_str(),
                               strOldMovieName.c_str(),
                               strWavName.c_str());
        assert(ret);
    }

    return ret;
}

int main(int argc, char **argv)
{
    if (argc >= 2 && lstrcmpiA(argv[1], "--version") == 0)
    {
        puts("finalize version 0.8 by katahiromz");
        return EXIT_SUCCESS;
    }
    if (argc <= 1)
    {
        puts("Usage: finalize dir [output.avi]");
        return EXIT_SUCCESS;
    }

    std::string strOutput;
    if (argc >= 3)
    {
        strOutput = argv[2];
    }
    else
    {
        strOutput = argv[1];
        strOutput += ".avi";
    }

    if (Finalize(argv[1], strOutput.c_str()))
    {
        puts("Finalized.");
        return EXIT_SUCCESS;
    }

    puts("FAILED.");
    return EXIT_FAILURE;
}
