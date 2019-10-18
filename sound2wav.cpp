#include "Misc.hpp"
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc >= 2 && lstrcmpiA(argv[1], "--version") == 0)
    {
        puts("sound2wav version 0.0 by katahiromz");
        return EXIT_SUCCESS;
    }
    if (argc <= 2)
    {
        puts("Usage: sound2wav input.sound output.wav");
        return EXIT_SUCCESS;
    }

    WCHAR szInput[MAX_PATH];
    WCHAR szOutput[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, szInput, ARRAYSIZE(szInput));
    MultiByteToWideChar(CP_ACP, 0, argv[2], -1, szOutput, ARRAYSIZE(szOutput));

    if (DoConvertSoundToWav(NULL, szInput, szOutput))
    {
        puts("Converted.");
        return EXIT_SUCCESS;
    }

    puts("FAILED.");
    return EXIT_FAILURE;
}
