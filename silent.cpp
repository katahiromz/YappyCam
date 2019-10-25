#include <windows.h>
#include <stdio.h>
#include "resource.h"

int main(void)
{
    ::PlaySound(MAKEINTRESOURCE(IDR_SILENT_WAV), GetModuleHandle(NULL),
                SND_ASYNC | SND_LOOP | SND_NODEFAULT |
                SND_RESOURCE);

    while (HWND hwnd = FindWindow(L"#32770", L"YappyCam"))
    {
        Sleep(500);
    }

    return 0;
}
