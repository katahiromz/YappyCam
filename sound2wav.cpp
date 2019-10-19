#include "misc.hpp"
#include <cstdio>
#include <string>

int main(int argc, char **argv)
{
    if (argc >= 2 && lstrcmpiA(argv[1], "--version") == 0)
    {
        puts("sound2wav version 0.8 by katahiromz");
        return EXIT_SUCCESS;
    }
    if (argc <= 1)
    {
        puts("Usage: sound2wav input.sound [output.wav]");
        return EXIT_SUCCESS;
    }

    std::wstring strInput = wide_from_ansi(argv[1]);

    std::wstring strOutput;
    if (argc >= 3)
    {
        strOutput = wide_from_ansi(argv[2]);
    }
    else
    {
        strOutput = strInput;
        strOutput += L".wav";
    }

    if (DoConvertSoundToWav(NULL, strInput.c_str(), strOutput.c_str()))
    {
        puts("Converted.");
        return EXIT_SUCCESS;
    }

    puts("FAILED.");
    return EXIT_FAILURE;
}
