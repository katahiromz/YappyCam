/* SetDlgItemDouble.h */
/* Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>. */
/* This file is public domain software. */
#ifndef SETDLGITEMDOUBLE_H_
#define SETDLGITEMDOUBLE_H_     4   /* Version 4 */

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif

#ifdef __cplusplus
    #include <cstdlib>
    #include <cmath>
    #include <cctype>
#else
    #include <stdlib.h>
    #include <math.h>
    #include <ctype.h>
#endif

#ifndef NO_STRSAFE
    #include <strsafe.h>
#else
    #include <shlwapi.h>
#endif

#ifndef M_OPTIONAL
    #ifdef __cplusplus
        #define M_OPTIONAL = 0
        #define M_OPTIONAL_(arg) = arg
    #else
        #define M_OPTIONAL
        #define M_OPTIONAL_(arg)
    #endif
#endif

#ifndef SETDLGITEMDOUBLE_DEF_FMT
    #define SETDLGITEMDOUBLE_DEF_FMT "%.2f"
#endif

#ifndef ARRAYSIZE
    #define ARRAYSIZE(array)    (sizeof(array) / sizeof((array)[0]))
#endif

#ifndef HAVE_STRTOF
    #if defined(__cplusplus) && __cplusplus >= 201103L              /* C++11 */
        #define HAVE_STRTOF
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L  /* C99 */
        #define HAVE_STRTOF
    #endif
#endif

#ifndef HAVE_ISINF
    #if defined(__cplusplus) && __cplusplus >= 201103L              /* C++11 */
        #define HAVE_ISINF
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L  /* C99 */
        #define HAVE_ISINF
    #endif
#endif

__inline double WINAPI
GetDlgItemDouble(HWND hDlg, int nItemID, BOOL *pbTranslated M_OPTIONAL_(NULL))
{
#ifdef __cplusplus
    using namespace std;
#endif
    char text[64], *pch;
    double eValue;

    if (pbTranslated)
        *pbTranslated = FALSE;

    if (!GetDlgItemTextA(hDlg, nItemID, text, ARRAYSIZE(text)))
        return 0;

    /* return zero if space only or empty */
    for (pch = text; isspace(*pch); ++pch)
        ;

    if (!*pch)
        return 0;

    eValue = strtod(text, &pch);

    /* ignore trailing space */
    while (isspace(*pch))
        ++pch;

    if (pbTranslated)
    {
        *pbTranslated = (*pch == 0) &&
#ifdef HAVE_ISINF
                        !isinf(eValue) &&
#else
                        (-HUGE_VAL < eValue && eValue < HUGE_VAL) &&
#endif
                        (eValue == eValue);
    }

    return eValue;
}

__inline BOOL WINAPI
SetDlgItemDouble(HWND hDlg, int nItemID, double eValue,
                 const char *fmt M_OPTIONAL_(NULL))
{
#ifdef __cplusplus
    using namespace std;
#endif
    char text[64];

    if (fmt == NULL)
        fmt = SETDLGITEMDOUBLE_DEF_FMT;

#ifdef NO_STRSAFE
    wnsprintfA(text, ARRAYSIZE(text), fmt, eValue);
#else
    StringCbPrintfA(text, sizeof(text), fmt, eValue);
#endif

    return SetDlgItemTextA(hDlg, nItemID, text);
}

__inline float WINAPI
GetDlgItemFloat(HWND hDlg, int nItemID, BOOL *pbTranslated M_OPTIONAL_(NULL))
{
#ifdef __cplusplus
    using namespace std;
#endif
    char text[64], *pch;
    float eValue;

    if (pbTranslated)
        *pbTranslated = FALSE;

    if (!GetDlgItemTextA(hDlg, nItemID, text, ARRAYSIZE(text)))
        return 0;

    /* return zero if space only or empty */
    for (pch = text; isspace(*pch); ++pch)
        ;

    if (!*pch)
        return 0;

#ifdef HAVE_STRTOF
    eValue = strtof(text, &pch);
#else
    eValue = (float)strtod(text, &pch);
#endif

    /* ignore trailing space */
    while (isspace(*pch))
        ++pch;

    if (pbTranslated)
    {
        *pbTranslated = (*pch == 0) &&
#ifdef HAVE_ISINF
                        !isinf(eValue) &&
#else
                        (-HUGE_VAL < eValue && eValue < HUGE_VAL) &&
#endif
                        (eValue == eValue);
    }

    return eValue;
}

#define SetDlgItemFloat SetDlgItemDouble

#endif  /* ndef SETDLGITEMDOUBLE_H_ */
