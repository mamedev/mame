/**
 * UTF-8 utility functions
 *
 * (c) 2010-2019 Steve Bennett <steveb@workware.net.au>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef UTF8_UTIL_H
#include "utf8.h"
#endif

#ifdef USE_UTF8
int utf8_fromunicode(char *p, unsigned uc)
{
    if (uc <= 0x7f) {
        *p = uc;
        return 1;
    }
    else if (uc <= 0x7ff) {
        *p++ = 0xc0 | ((uc & 0x7c0) >> 6);
        *p = 0x80 | (uc & 0x3f);
        return 2;
    }
    else if (uc <= 0xffff) {
        *p++ = 0xe0 | ((uc & 0xf000) >> 12);
        *p++ = 0x80 | ((uc & 0xfc0) >> 6);
        *p = 0x80 | (uc & 0x3f);
        return 3;
    }
    /* Note: We silently truncate to 21 bits here: 0x1fffff */
    else {
        *p++ = 0xf0 | ((uc & 0x1c0000) >> 18);
        *p++ = 0x80 | ((uc & 0x3f000) >> 12);
        *p++ = 0x80 | ((uc & 0xfc0) >> 6);
        *p = 0x80 | (uc & 0x3f);
        return 4;
    }
}

int utf8_charlen(int c)
{
    if ((c & 0x80) == 0) {
        return 1;
    }
    if ((c & 0xe0) == 0xc0) {
        return 2;
    }
    if ((c & 0xf0) == 0xe0) {
        return 3;
    }
    if ((c & 0xf8) == 0xf0) {
        return 4;
    }
    /* Invalid sequence */
    return -1;
}

int utf8_strlen(const char *str, int bytelen)
{
    int charlen = 0;
    if (bytelen < 0) {
        bytelen = strlen(str);
    }
    while (bytelen > 0) {
        int c;
        int l = utf8_tounicode(str, &c);
        charlen++;
        str += l;
        bytelen -= l;
    }
    return charlen;
}

int utf8_strwidth(const char *str, int charlen)
{
    int width = 0;
    while (charlen) {
        int c;
        int l = utf8_tounicode(str, &c);
        width += utf8_width(c);
        str += l;
        charlen--;
    }
    return width;
}

int utf8_index(const char *str, int index)
{
    const char *s = str;
    while (index--) {
        int c;
        s += utf8_tounicode(s, &c);
    }
    return s - str;
}

int utf8_tounicode(const char *str, int *uc)
{
    unsigned const char *s = (unsigned const char *)str;

    if (s[0] < 0xc0) {
        *uc = s[0];
        return 1;
    }
    if (s[0] < 0xe0) {
        if ((s[1] & 0xc0) == 0x80) {
            *uc = ((s[0] & ~0xc0) << 6) | (s[1] & ~0x80);
            if (*uc >= 0x80) {
                return 2;
            }
            /* Otherwise this is an invalid sequence */
        }
    }
    else if (s[0] < 0xf0) {
        if (((str[1] & 0xc0) == 0x80) && ((str[2] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xe0) << 12) | ((s[1] & ~0x80) << 6) | (s[2] & ~0x80);
            if (*uc >= 0x800) {
                return 3;
            }
            /* Otherwise this is an invalid sequence */
        }
    }
    else if (s[0] < 0xf8) {
        if (((str[1] & 0xc0) == 0x80) && ((str[2] & 0xc0) == 0x80) && ((str[3] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xf0) << 18) | ((s[1] & ~0x80) << 12) | ((s[2] & ~0x80) << 6) | (s[3] & ~0x80);
            if (*uc >= 0x10000) {
                return 4;
            }
            /* Otherwise this is an invalid sequence */
        }
    }

    /* Invalid sequence, so just return the byte */
    *uc = *s;
    return 1;
}

struct utf8range {
    int lower;     /* lower inclusive */
    int upper;     /* upper exclusive */
};

/* From http://unicode.org/Public/UNIDATA/UnicodeData.txt */
static const struct utf8range unicode_range_combining[] = {
        { 0x0300, 0x0370 },     { 0x0483, 0x048a },     { 0x0591, 0x05d0 },     { 0x0610, 0x061b },
        { 0x064b, 0x0660 },     { 0x0670, 0x0671 },     { 0x06d6, 0x06dd },     { 0x06df, 0x06e5 },
        { 0x06e7, 0x06ee },     { 0x0711, 0x0712 },     { 0x0730, 0x074d },     { 0x07a6, 0x07b1 },
        { 0x07eb, 0x07f4 },     { 0x0816, 0x0830 },     { 0x0859, 0x085e },     { 0x08d4, 0x0904 },
        { 0x093a, 0x0958 },     { 0x0962, 0x0964 },     { 0x0981, 0x0985 },     { 0x09bc, 0x09ce },
        { 0x09d7, 0x09dc },     { 0x09e2, 0x09e6 },     { 0x0a01, 0x0a05 },     { 0x0a3c, 0x0a59 },
        { 0x0a70, 0x0a72 },     { 0x0a75, 0x0a85 },     { 0x0abc, 0x0ad0 },     { 0x0ae2, 0x0ae6 },
        { 0x0afa, 0x0b05 },     { 0x0b3c, 0x0b5c },     { 0x0b62, 0x0b66 },     { 0x0b82, 0x0b83 },
        { 0x0bbe, 0x0bd0 },     { 0x0bd7, 0x0be6 },     { 0x0c00, 0x0c05 },     { 0x0c3e, 0x0c58 },
        { 0x0c62, 0x0c66 },     { 0x0c81, 0x0c85 },     { 0x0cbc, 0x0cde },     { 0x0ce2, 0x0ce6 },
        { 0x0d00, 0x0d05 },     { 0x0d3b, 0x0d4e },     { 0x0d57, 0x0d58 },     { 0x0d62, 0x0d66 },
        { 0x0d82, 0x0d85 },     { 0x0dca, 0x0de6 },     { 0x0df2, 0x0df4 },     { 0x0e31, 0x0e32 },
        { 0x0e34, 0x0e3f },     { 0x0e47, 0x0e4f },     { 0x0eb1, 0x0eb2 },     { 0x0eb4, 0x0ebd },
        { 0x0ec8, 0x0ed0 },     { 0x0f18, 0x0f1a },     { 0x0f35, 0x0f3a },     { 0x0f3e, 0x0f40 },
        { 0x0f71, 0x0f88 },     { 0x0f8d, 0x0fbe },     { 0x0fc6, 0x0fc7 },     { 0x102b, 0x103f },
        { 0x1056, 0x105a },     { 0x105e, 0x1065 },     { 0x1067, 0x106e },     { 0x1071, 0x1075 },
        { 0x1082, 0x1090 },     { 0x109a, 0x109e },     { 0x135d, 0x1360 },     { 0x1712, 0x1720 },
        { 0x1732, 0x1735 },     { 0x1752, 0x1760 },     { 0x1772, 0x1780 },     { 0x17b4, 0x17d4 },
        { 0x17dd, 0x17e0 },     { 0x180b, 0x180e },     { 0x1885, 0x1887 },     { 0x18a9, 0x18aa },
        { 0x1920, 0x1940 },     { 0x1a17, 0x1a1e },     { 0x1a55, 0x1a80 },     { 0x1ab0, 0x1b05 },
        { 0x1b34, 0x1b45 },     { 0x1b6b, 0x1b74 },     { 0x1b80, 0x1b83 },     { 0x1ba1, 0x1bae },
        { 0x1be6, 0x1bfc },     { 0x1c24, 0x1c3b },     { 0x1cd0, 0x1ce9 },     { 0x1ced, 0x1cee },
        { 0x1cf2, 0x1cf5 },     { 0x1cf7, 0x1d00 },     { 0x1dc0, 0x1e00 },     { 0x20d0, 0x2100 },
        { 0x2cef, 0x2cf2 },     { 0x2d7f, 0x2d80 },     { 0x2de0, 0x2e00 },     { 0x302a, 0x3030 },
        { 0x3099, 0x309b },     { 0xa66f, 0xa67e },     { 0xa69e, 0xa6a0 },     { 0xa6f0, 0xa6f2 },
        { 0xa802, 0xa803 },     { 0xa806, 0xa807 },     { 0xa80b, 0xa80c },     { 0xa823, 0xa828 },
        { 0xa880, 0xa882 },     { 0xa8b4, 0xa8ce },     { 0xa8e0, 0xa8f2 },     { 0xa926, 0xa92e },
        { 0xa947, 0xa95f },     { 0xa980, 0xa984 },     { 0xa9b3, 0xa9c1 },     { 0xa9e5, 0xa9e6 },
        { 0xaa29, 0xaa40 },     { 0xaa43, 0xaa44 },     { 0xaa4c, 0xaa50 },     { 0xaa7b, 0xaa7e },
        { 0xaab0, 0xaab5 },     { 0xaab7, 0xaab9 },     { 0xaabe, 0xaac2 },     { 0xaaeb, 0xaaf0 },
        { 0xaaf5, 0xab01 },     { 0xabe3, 0xabf0 },     { 0xfb1e, 0xfb1f },     { 0xfe00, 0xfe10 },
        { 0xfe20, 0xfe30 },
};

/* From http://unicode.org/Public/UNIDATA/EastAsianWidth.txt */
static const struct utf8range unicode_range_wide[] = {
        { 0x1100, 0x115f },     { 0x231a, 0x231b },     { 0x2329, 0x232a },     { 0x23e9, 0x23ec },
        { 0x23f0, 0x23f0 },     { 0x23f3, 0x23f3 },     { 0x25fd, 0x25fe },     { 0x2614, 0x2615 },
        { 0x2648, 0x2653 },     { 0x267f, 0x267f },     { 0x2693, 0x2693 },     { 0x26a1, 0x26a1 },
        { 0x26aa, 0x26ab },     { 0x26bd, 0x26be },     { 0x26c4, 0x26c5 },     { 0x26ce, 0x26ce },
        { 0x26d4, 0x26d4 },     { 0x26ea, 0x26ea },     { 0x26f2, 0x26f3 },     { 0x26f5, 0x26f5 },
        { 0x26fa, 0x26fa },     { 0x26fd, 0x26fd },     { 0x2705, 0x2705 },     { 0x270a, 0x270b },
        { 0x2728, 0x2728 },     { 0x274c, 0x274c },     { 0x274e, 0x274e },     { 0x2753, 0x2755 },
        { 0x2757, 0x2757 },     { 0x2795, 0x2797 },     { 0x27b0, 0x27b0 },     { 0x27bf, 0x27bf },
        { 0x2b1b, 0x2b1c },     { 0x2b50, 0x2b50 },     { 0x2b55, 0x2b55 },     { 0x2e80, 0x2e99 },
        { 0x2e9b, 0x2ef3 },     { 0x2f00, 0x2fd5 },     { 0x2ff0, 0x2ffb },     { 0x3001, 0x303e },
        { 0x3041, 0x3096 },     { 0x3099, 0x30ff },     { 0x3105, 0x312e },     { 0x3131, 0x318e },
        { 0x3190, 0x31ba },     { 0x31c0, 0x31e3 },     { 0x31f0, 0x321e },     { 0x3220, 0x3247 },
        { 0x3250, 0x32fe },     { 0x3300, 0x4dbf },     { 0x4e00, 0xa48c },     { 0xa490, 0xa4c6 },
        { 0xa960, 0xa97c },     { 0xac00, 0xd7a3 },     { 0xf900, 0xfaff },     { 0xfe10, 0xfe19 },
        { 0xfe30, 0xfe52 },     { 0xfe54, 0xfe66 },     { 0xfe68, 0xfe6b },     { 0x16fe0, 0x16fe1 },
        { 0x17000, 0x187ec },   { 0x18800, 0x18af2 },   { 0x1b000, 0x1b11e },   { 0x1b170, 0x1b2fb },
        { 0x1f004, 0x1f004 },   { 0x1f0cf, 0x1f0cf },   { 0x1f18e, 0x1f18e },   { 0x1f191, 0x1f19a },
        { 0x1f200, 0x1f202 },   { 0x1f210, 0x1f23b },   { 0x1f240, 0x1f248 },   { 0x1f250, 0x1f251 },
        { 0x1f260, 0x1f265 },   { 0x1f300, 0x1f320 },   { 0x1f32d, 0x1f335 },   { 0x1f337, 0x1f37c },
        { 0x1f37e, 0x1f393 },   { 0x1f3a0, 0x1f3ca },   { 0x1f3cf, 0x1f3d3 },   { 0x1f3e0, 0x1f3f0 },
        { 0x1f3f4, 0x1f3f4 },   { 0x1f3f8, 0x1f43e },   { 0x1f440, 0x1f440 },   { 0x1f442, 0x1f4fc },
        { 0x1f4ff, 0x1f53d },   { 0x1f54b, 0x1f54e },   { 0x1f550, 0x1f567 },   { 0x1f57a, 0x1f57a },
        { 0x1f595, 0x1f596 },   { 0x1f5a4, 0x1f5a4 },   { 0x1f5fb, 0x1f64f },   { 0x1f680, 0x1f6c5 },
        { 0x1f6cc, 0x1f6cc },   { 0x1f6d0, 0x1f6d2 },   { 0x1f6eb, 0x1f6ec },   { 0x1f6f4, 0x1f6f8 },
        { 0x1f910, 0x1f93e },   { 0x1f940, 0x1f94c },   { 0x1f950, 0x1f96b },   { 0x1f980, 0x1f997 },
        { 0x1f9c0, 0x1f9c0 },   { 0x1f9d0, 0x1f9e6 },   { 0x20000, 0x2fffd },   { 0x30000, 0x3fffd },
};

#define ARRAYSIZE(A) sizeof(A) / sizeof(*(A))

static int cmp_range(const void *key, const void *cm)
{
    const struct utf8range *range = (const struct utf8range *)cm;
    int ch = *(int *)key;
    if (ch < range->lower) {
        return -1;
    }
    if (ch >= range->upper) {
        return 1;
    }
    return 0;
}

static int utf8_in_range(const struct utf8range *range, int num, int ch)
{
    const struct utf8range *r =
        bsearch(&ch, range, num, sizeof(*range), cmp_range);

    if (r) {
        return 1;
    }
    return 0;
}

int utf8_width(int ch)
{
    /* short circuit for common case */
    if (isascii(ch)) {
        return 1;
    }
    if (utf8_in_range(unicode_range_combining, ARRAYSIZE(unicode_range_combining), ch)) {
        return 0;
    }
    if (utf8_in_range(unicode_range_wide, ARRAYSIZE(unicode_range_wide), ch)) {
        return 2;
    }
    return 1;
}
#endif
