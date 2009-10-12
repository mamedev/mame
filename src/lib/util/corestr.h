/***************************************************************************

    corestr.h

    Core string functions used throughout MAME.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __CORESTR_H__
#define __CORESTR_H__

#include "osdcore.h"
#include <string.h>


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* since stricmp is not part of the standard, we use this instead */
int core_stricmp(const char *s1, const char *s2);

/* this macro prevents people from using stricmp directly */
#undef stricmp
#define stricmp !MUST_USE_CORE_STRICMP_INSTEAD!

/* this macro prevents people from using strcasecmp directly */
#undef strcasecmp
#define strcasecmp !MUST_USE_CORE_STRICMP_INSTEAD!


/* since strnicmp is not part of the standard, we use this instead */
int core_strnicmp(const char *s1, const char *s2, size_t n);

/* this macro prevents people from using strnicmp directly */
#undef strnicmp
#define strnicmp !MUST_USE_CORE_STRNICMP_INSTEAD!

/* this macro prevents people from using strncasecmp directly */
#undef strncasecmp
#define strncasecmp !MUST_USE_CORE_STRNICMP_INSTEAD!


/* since strdup is not part of the standard, we use this instead */
char *core_strdup(const char *str);

/* this macro prevents people from using strdup directly */
#undef strdup
#define strdup !MUST_USE_CORE_STRDUP_INSTEAD!


/* additional string compare helper */
int core_strwildcmp(const char *sp1, const char *sp2);


/* I64 printf helper */
char *core_i64_hex_format(UINT64 value, UINT8 mindigits);


#endif /* __CORESTR_H__ */
