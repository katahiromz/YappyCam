#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>

void usage(void)
{
    std::cout <<
        "diskrate 1.0 by katahiromz\n" << 
        "Usage: diskrate X N\n" <<
        "Writes random data of X bytes N times to a temporary file." <<
        std::endl;
}

int main(int argc, char **argv)
{
    const char *fname = "delete-me.dat";

    if (argc < 3 ||
        strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "--version") == 0)
    {
        usage();
        return EXIT_SUCCESS;
    }

    int X = atoi(argv[1]);
    int N = atoi(argv[2]);
    if (X <= 0 || N <= 0)
    {
        std::cout << "ERROR: invalid argument" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "X: " << X << std::endl;
    std::cout << "N: " << N << std::endl;
    std::cout << "N * X: " << (N * X) << std::endl;

    std::vector<char> data;
    data.resize(X);

    for (int i = 0; i < X; ++i)
    {
        data[i] = char(rand() & 0xFF);
    }

    using clock = std::chrono::high_resolution_clock;
    clock::time_point start, end;

    start = clock::now();
    if (FILE *fp = fopen(fname, "wb"))
    {
        for (int n = 0; n < N; ++n)
        {
            fwrite(&data[0], X, 1, fp);
        }
        fclose(fp);
    }
    end = clock::now();

    auto total = (long long)(N) * X;
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << msec << " [msec]" << std::endl;
    std::cout << (long long)(total / (msec / 1000.0f)) << " [bytes/sec]" << std::endl;

    unlink(fname);

    return 0;
}
