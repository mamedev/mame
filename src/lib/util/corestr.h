/***************************************************************************

    corestr.h

    Core string functions used throughout MAME.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CORESTR_H__
#define __CORESTR_H__

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


#endif /* __CORESTR_H__ */
