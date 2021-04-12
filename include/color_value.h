/* color_value.h --- Color Value Parser */
/* Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com> */
/* This file is public domain software. */
#ifndef COLOR_VALUE_H_
#define COLOR_VALUE_H_  13   /* Version 13 */

#if defined(__cplusplus) && (__cplusplus >= 201103L)    /* C++11 */
    #include <cstdint>
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)    /* C99 */
    #include <stdint.h>
#else
    #ifdef __cplusplus
        #include <climits>
    #else
        #include <limits.h>
    #endif
    typedef unsigned char uint8_t;
    #if (UINT_MAX == 0xFFFFFFFF)
        typedef unsigned int uint32_t;
    #elif (ULONG_MAX == 0xFFFFFFFF)
        typedef unsigned long uint32_t;
    #else
        #error Your compiler is not support yet.
    #endif
    typedef char COLOR_VALUE_ASSERT_1[sizeof(uint8_t) == 1 ? 1 : -1];
    typedef char COLOR_VALUE_ASSERT_2[sizeof(uint32_t) == 4 ? 1 : -1];
#endif

#ifdef __cplusplus
    #include <cstdlib>
    #include <cstdio>
    #include <cstring>
    #include <cassert>
#else
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <assert.h>
#endif

#ifndef NO_STRSAFE
    #include <strsafe.h>
#endif

#ifdef __cplusplus
    using std::size_t;
#endif

#if defined(__cplusplus) && (__cplusplus >= 201103L)    /* C++11 */
    #include <unordered_map>
    #include <string>
#endif

typedef struct COLOR_VALUE_HEX
{
    const char *name;
    const char *hex;
} COLOR_VALUE_HEX;

static __inline const char *color_value_find(const char *name)
{
#ifdef __cplusplus
    using namespace std;
#endif
#if defined(__cplusplus) && (__cplusplus >= 201103L)    /* C++11 */
    static const std::unordered_map<std::string, std::string> s_map =
#else
    /* names are in lower case */
    static const COLOR_VALUE_HEX s_table[] =
#endif
    {
        { "black", "#000000" },
        { "white", "#ffffff" },
        { "red", "#ff0000" },
        { "yellow", "#ffff00" },
        { "blue", "#0000ff" },
        { "purple", "#800080" },
        { "silver", "#c0c0c0" },
        { "gray", "#808080" },
        { "green", "#008000" },
        { "maroon", "#800000" },
        { "fuchsia", "#ff00ff" },
        { "lime", "#00ff00" },
        { "olive", "#808000" },
        { "navy", "#000080" },
        { "orange", "#ffa500" },
        { "teal", "#008080" },
        { "aqua", "#00ffff" },
        { "aliceblue", "#f0f8ff" },
        { "antiquewhite", "#faebd7" },
        { "aquamarine", "#7fffd4" },
        { "azure", "#f0ffff" },
        { "beige", "#f5f5dc" },
        { "bisque", "#ffe4c4" },
        { "blanchedalmond", "#ffebcd" },
        { "blueviolet", "#8a2be2" },
        { "brown", "#a52a2a" },
        { "burlywood", "#deb887" },
        { "cadetblue", "#5f9ea0" },
        { "chartreuse", "#7fff00" },
        { "chocolate", "#d2691e" },
        { "coral", "#ff7f50" },
        { "cornflowerblue", "#6495ed" },
        { "cornsilk", "#fff8dc" },
        { "crimson", "#dc143c" },
        { "cyan", "#00ffff" },
        { "darkblue", "#00008b" },
        { "darkcyan", "#008b8b" },
        { "darkgoldenrod", "#b8860b" },
        { "darkgray", "#a9a9a9" },
        { "darkgreen", "#006400" },
        { "darkgrey", "#a9a9a9" },
        { "darkkhaki", "#bdb76b" },
        { "darkmagenta", "#8b008b" },
        { "darkolivegreen", "#556b2f" },
        { "darkorange", "#ff8c00" },
        { "darkorchid", "#9932cc" },
        { "darkred", "#8b0000" },
        { "darksalmon", "#e9967a" },
        { "darkseagreen", "#8fbc8f" },
        { "darkslateblue", "#483d8b" },
        { "darkslategray", "#2f4f4f" },
        { "darkslategrey", "#2f4f4f" },
        { "darkturquoise", "#00ced1" },
        { "darkviolet", "#9400d3" },
        { "deeppink", "#ff1493" },
        { "deepskyblue", "#00bfff" },
        { "dimgray", "#696969" },
        { "dimgrey", "#696969" },
        { "dodgerblue", "#1e90ff" },
        { "firebrick", "#b22222" },
        { "floralwhite", "#fffaf0" },
        { "forestgreen", "#228b22" },
        { "gainsboro", "#dcdcdc" },
        { "ghostwhite", "#f8f8ff" },
        { "gold", "#ffd700" },
        { "goldenrod", "#daa520" },
        { "greenyellow", "#adff2f" },
        { "grey", "#808080" },
        { "honeydew", "#f0fff0" },
        { "hotpink", "#ff69b4" },
        { "indianred", "#cd5c5c" },
        { "indigo", "#4b0082" },
        { "ivory", "#fffff0" },
        { "khaki", "#f0e68c" },
        { "lavender", "#e6e6fa" },
        { "lavenderblush", "#fff0f5" },
        { "lawngreen", "#7cfc00" },
        { "lemonchiffon", "#fffacd" },
        { "lightblue", "#add8e6" },
        { "lightcoral", "#f08080" },
        { "lightcyan", "#e0ffff" },
        { "lightgoldenrodyellow", "#fafad2" },
        { "lightgray", "#d3d3d3" },
        { "lightgreen", "#90ee90" },
        { "lightgrey", "#d3d3d3" },
        { "lightpink", "#ffb6c1" },
        { "lightsalmon", "#ffa07a" },
        { "lightseagreen", "#20b2aa" },
        { "lightskyblue", "#87cefa" },
        { "lightslategray", "#778899" },
        { "lightslategrey", "#778899" },
        { "lightsteelblue", "#b0c4de" },
        { "lightyellow", "#ffffe0" },
        { "limegreen", "#32cd32" },
        { "linen", "#faf0e6" },
        { "magenta", "#ff00ff" },
        { "mediumaquamarine", "#66cdaa" },
        { "mediumblue", "#0000cd" },
        { "mediumorchid", "#ba55d3" },
        { "mediumpurple", "#9370db" },
        { "mediumseagreen", "#3cb371" },
        { "mediumslateblue", "#7b68ee" },
        { "mediumspringgreen", "#00fa9a" },
        { "mediumturquoise", "#48d1cc" },
        { "mediumvioletred", "#c71585" },
        { "midnightblue", "#191970" },
        { "mintcream", "#f5fffa" },
        { "mistyrose", "#ffe4e1" },
        { "moccasin", "#ffe4b5" },
        { "navajowhite", "#ffdead" },
        { "oldlace", "#fdf5e6" },
        { "olivedrab", "#6b8e23" },
        { "orangered", "#ff4500" },
        { "orchid", "#da70d6" },
        { "palegoldenrod", "#eee8aa" },
        { "palegreen", "#98fb98" },
        { "paleturquoise", "#afeeee" },
        { "palevioletred", "#db7093" },
        { "papayawhip", "#ffefd5" },
        { "peachpuff", "#ffdab9" },
        { "peru", "#cd853f" },
        { "pink", "#ffc0cb" },
        { "plum", "#dda0dd" },
        { "powderblue", "#b0e0e6" },
        { "rosybrown", "#bc8f8f" },
        { "royalblue", "#4169e1" },
        { "saddlebrown", "#8b4513" },
        { "salmon", "#fa8072" },
        { "sandybrown", "#f4a460" },
        { "seagreen", "#2e8b57" },
        { "seashell", "#fff5ee" },
        { "sienna", "#a0522d" },
        { "skyblue", "#87ceeb" },
        { "slateblue", "#6a5acd" },
        { "slategray", "#708090" },
        { "slategrey", "#708090" },
        { "snow", "#fffafa" },
        { "springgreen", "#00ff7f" },
        { "steelblue", "#4682b4" },
        { "tan", "#d2b48c" },
        { "thistle", "#d8bfd8" },
        { "tomato", "#ff6347" },
        { "turquoise", "#40e0d0" },
        { "violet", "#ee82ee" },
        { "wheat", "#f5deb3" },
        { "whitesmoke", "#f5f5f5" },
        { "yellowgreen", "#9acd32" },
        { "rebeccapurple", "#663399" },
    };
#if defined(__cplusplus) && (__cplusplus >= 201103L)    /* C++11 */
    auto it = s_map.find(name);
    if (it != s_map.end())
        return it->second.c_str();
#else
    size_t i, count = sizeof(s_table) / sizeof(s_table[0]);
    for (i = 0; i < count; ++i)
    {
        if (strcmp(s_table[i].name, name) == 0)
            return s_table[i].hex;
    }
#endif
    return NULL;
}

static __inline int color_value_valid(uint32_t value)
{
    return (value != (uint32_t)-1) && (value <= 0xFFFFFF);
}

static __inline uint32_t color_value_fix(uint32_t value)
{
    uint32_t r = (uint8_t)(value >> 16);
    uint32_t g = (uint8_t)(value >> 8);
    uint32_t b = (uint8_t)value;
    return (b << 16) | (g << 8) | r;
}

static __inline uint32_t color_value_web_safe(uint32_t value)
{
    uint32_t r = (uint8_t)(value >> 16);
    uint32_t g = (uint8_t)(value >> 8);
    uint32_t b = (uint8_t)value;
    r = ((r + 0x33 / 2) / 0x33) * 0x33;
    g = ((g + 0x33 / 2) / 0x33) * 0x33;
    b = ((b + 0x33 / 2) / 0x33) * 0x33;
    assert(0 <= r && r <= 0xFF);
    assert(0 <= g && g <= 0xFF);
    assert(0 <= b && b <= 0xFF);
    return (r << 16) | (g << 8) | b;
}

static __inline void color_value_strlwr(char *dest, const char *src, size_t max_len)
{
#ifdef __cplusplus
    using namespace std;
#endif
    for (; *src && max_len > 1; ++src, ++dest, --max_len)
    {
        if ('A' <= *src && *src <= 'Z')
            *dest = *src + ('a' - 'A');
        else
            *dest = *src;
    }
    *dest = 0;
}

static __inline uint32_t color_value_parse(const char *color_name)
{
#ifdef __cplusplus
    using namespace std;
#endif
    assert(color_name != NULL);
    if (!color_name[0])
        return (uint32_t)(-1);

    if (color_name[0] == '#')
    {
        char *pch;
        size_t len = strlen(&color_name[1]);
        if (len == 3)
        {
            /* #rgb */
            unsigned long value = strtoul(&color_name[1], &pch, 16);
            if (*pch == 0 && value <= 0xFFF)
            {
                uint32_t r = ((value >> 8) & 0x0F) * 0xFF / 0xF;
                uint32_t g = ((value >> 4) & 0x0F) * 0xFF / 0xF;
                uint32_t b = ((value >> 0) & 0x0F) * 0xFF / 0xF;
                value = (r << 16) | (g << 8) | b;
                return (uint32_t)value;
            }
        }
        else if (len == 6)
        {
            /* #rrggbb */
            unsigned long value = strtoul(&color_name[1], &pch, 16);
            if (*pch == 0 && value <= 0xFFFFFF)
            {
                return (uint32_t)value;
            }
        }
    }
    else
    {
        char name[64];
        const char *hex;
        color_value_strlwr(name, color_name, 64);

        hex = color_value_find(name);
        if (hex)
        {
            return color_value_parse(hex);
        }
    }

    return (uint32_t)(-1);
}

static __inline void color_value_store(char *str, size_t max_len, uint32_t color_value)
{
#ifdef __cplusplus
    using namespace std;
#endif
    assert(max_len >= 8);
    color_value &= 0xFFFFFF;
#ifdef NO_STRSAFE
    sprintf(str, "#%06X", color_value);
#else
    StringCchPrintfA(str, max_len, "#%06X", color_value);
#endif
}

static __inline uint8_t color_value_max(uint8_t a, uint8_t b)
{
    return a <= b ? b : a;
}
static __inline uint8_t color_value_min(uint8_t a, uint8_t b)
{
    return a > b ? b : a;
}

static __inline float color_value_fabs(float f)
{
    return f >= 0 ? f : -f;
}

/* *h, *s, *v in [0, 1]. */
static __inline void
color_value_to_hsv(uint32_t rgb, float *h, float *s, float *v)
{
    uint8_t r = (uint8_t)(rgb >> 16);
    uint8_t g = (uint8_t)(rgb >> 8);
    uint8_t b = (uint8_t)rgb;
    uint8_t max_ = color_value_max(color_value_max(r, g), b);
    uint8_t min_ = color_value_min(color_value_min(r, g), b);
    float max_value = max_ / 255.0f, min_value = min_ / 255.0f;
    float hue = max_value - min_value, sat;
    if (hue > 0)
    {
        if (max_ == r)
        {
            hue = (g - b) / (hue * 255.0f);
            if (hue < 0)
                hue += 6.0f;
        }
        else if (max_ == g)
        {
            hue = 2.0f + (b - r) / (hue * 255.0f);
        }
        else
        {
            hue = 4.0f + (r - g) / (hue * 255.0f);
        }
    }
    *h = hue / 6.0f;
    sat = max_value - min_value;
    if (max_value > 0)
        sat /= max_value;
    *s = sat;
    *v = max_value;
    assert(0 <= *h && *h <= 1);
    assert(0 <= *s && *s <= 1);
    assert(0 <= *v && *v <= 1);
}

/* h, s, v in [0, 1]. */
static __inline void
color_value_from_hsv(uint32_t *rgb, float h, float s, float v)
{
    int i;
    float f, r, g, b;
    uint32_t r_value, g_value, b_value;
    assert(0 <= h && h <= 1);
    assert(0 <= s && s <= 1);
    assert(0 <= v && v <= 1);
    r = g = b = v;
    if (s > 0)
    {
        i = (int)(h * 6);
        f = h * 6 - i;
        switch (i)
        {
        default:
            assert(0);
            break;
        case 0: case 6:
            g *= 1 - s * (1 - f);
            b *= 1 - s;
            break;
        case 1:
            r *= 1 - s * f;
            b *= 1 - s;
            break;
        case 2:
            r *= 1 - s;
            b *= 1 - s * (1 - f);
            break;
        case 3:
            r *= 1 - s;
            g *= 1 - s * f;
            break;
        case 4:
            r *= 1 - s * (1 - f);
            g *= 1 - s;
            break;
        case 5:
            g *= 1 - s;
            b *= 1 - s * f;
            break;
        }
    }
    r_value = (uint8_t)(r * 255 + 0.5f);
    g_value = (uint8_t)(g * 255 + 0.5f);
    b_value = (uint8_t)(b * 255 + 0.5f);
    *rgb = (r_value << 16) | (g_value << 8) | b_value;
    assert(color_value_valid(*rgb));
}

static __inline void color_value_unittest(void)
{
#ifdef __cplusplus
    using namespace std;
#endif
    char buf[8];
    float h, s, v;
    uint32_t rgb;

    assert(color_value_parse("black") == 0x000000);
    assert(color_value_parse("white") == 0xFFFFFF);
    assert(color_value_parse("red") == 0xFF0000);
    assert(color_value_parse("lime") == 0x00FF00);
    assert(color_value_parse("blue") == 0x0000FF);
    assert(color_value_parse("Black") == 0x000000);
    assert(color_value_parse("White") == 0xFFFFFF);
    assert(color_value_parse("Red") == 0xFF0000);
    assert(color_value_parse("Lime") == 0x00FF00);
    assert(color_value_parse("Blue") == 0x0000FF);
    assert(color_value_parse("BLACK") == 0x000000);
    assert(color_value_parse("WHITE") == 0xFFFFFF);
    assert(color_value_parse("RED") == 0xFF0000);
    assert(color_value_parse("LIME") == 0x00FF00);
    assert(color_value_parse("BLUE") == 0x0000FF);

    assert(color_value_parse("#ffffff") == 0xFFFFFF);
    assert(color_value_parse("#FFFFFF") == 0xFFFFFF);
    assert(color_value_parse("#ff0000") == 0xFF0000);
    assert(color_value_parse("#FF0000") == 0xFF0000);
    assert(color_value_parse("#fff") == 0xFFFFFF);
    assert(color_value_parse("#FFF") == 0xFFFFFF);
    assert(color_value_parse("#f00") == 0xFF0000);
    assert(color_value_parse("#F00") == 0xFF0000);

    color_value_store(buf, 8, 0xFF0000);
    assert(strcmp(buf, "#FF0000") == 0);
    color_value_store(buf, 8, 0xFF00FF);
    assert(strcmp(buf, "#FF00FF") == 0);
    color_value_store(buf, 8, 0x00FFFF);
    assert(strcmp(buf, "#00FFFF") == 0);

    assert(color_value_fix(0xFF0000) == 0x0000FF);
    assert(color_value_fix(0xFFFF00) == 0x00FFFF);
    assert(color_value_fix(0x0000FF) == 0xFF0000);

    assert(color_value_web_safe(0xFFFFFF) == 0xFFFFFF);
    assert(color_value_web_safe(0xEEFFFF) == 0xFFFFFF);
    assert(color_value_web_safe(0xEEEEFF) == 0xFFFFFF);
    assert(color_value_web_safe(0x2222FF) == 0x3333FF);
    assert(color_value_web_safe(0x1122FF) == 0x0033FF);
    assert(color_value_web_safe(0x0022FF) == 0x0033FF);
    assert(color_value_web_safe(0xFF0088) == 0xFF0099);
    assert(color_value_web_safe(0xFFEE00) == 0xFFFF00);

    color_value_to_hsv(0x000000, &h, &s, &v);
    assert(h == 0.0f && s == 0.0f && v == 0.0f);
    color_value_to_hsv(0xFFFFFF, &h, &s, &v);
    assert(h == 0.0f && s == 0.0f && v == 1.0f);
    color_value_to_hsv(0xFF0000, &h, &s, &v);
    assert(h == 0.0f && s == 1.0f && v == 1.0f);
    color_value_to_hsv(0x00FF00, &h, &s, &v);
    assert(color_value_fabs(h - 0.3333f) < 0.001f && s == 1.0f && v == 1.0f);
    color_value_to_hsv(0x0000FF, &h, &s, &v);
    assert(color_value_fabs(h - 0.6667f) < 0.001f && s == 1.0f && v == 1.0f);
    color_value_to_hsv(0xFF00FF, &h, &s, &v);
    assert(color_value_fabs(h - 0.8333f) < 0.001f && s == 1.0f && v == 1.0f);
    color_value_to_hsv(0xFFFF00, &h, &s, &v);
    assert(color_value_fabs(h - 0.1667f) < 0.001f && s == 1.0f && v == 1.0f);

    color_value_from_hsv(&rgb, 0, 0, 0);
    assert(rgb == 0x000000);
    color_value_from_hsv(&rgb, 0, 0, 1);
    assert(rgb == 0xFFFFFF);
    color_value_from_hsv(&rgb, 0, 1, 1);
    assert(rgb == 0xFF0000);
    color_value_from_hsv(&rgb, 0.3333f, 1, 1);
    assert(rgb == 0x00FF00);
    color_value_from_hsv(&rgb, 0.6667f, 1, 1);
    assert(rgb == 0x0000FF);
    color_value_from_hsv(&rgb, 0.8333f, 1, 1);
    assert(rgb == 0xFF00FF);
    color_value_from_hsv(&rgb, 0.1667f, 1, 1);
    assert(rgb == 0xFFFF00);

    puts("OK");
}

#endif  /* ndef COLOR_VALUE_H_ */
