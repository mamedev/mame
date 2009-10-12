/***************************************************************************

    astring.h

    Allocated string manipulation functions.

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

#ifndef __ASTRING_H__
#define __ASTRING_H__

#include "pool.h"
#include <stdarg.h>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _astring astring;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- astring allocation ----- */

/* allocate a new astring */
astring *astring_alloc(void);

/* free an astring */
void astring_free(astring *str);



/* ----- inline astring changes ----- */

/* copy one astring into another */
astring *astring_cpy(astring *dst, const astring *src);

/* copy a C string into an astring */
astring *astring_cpyc(astring *dst, const char *src);

/* copy a character array into an astring */
astring *astring_cpych(astring *dst, const char *src, int count);

/* copy a substring of one string into another */
astring *astring_cpysubstr(astring *dst, const astring *src, int start, int count);

/* insert one astring into another */
astring *astring_ins(astring *dst, int insbefore, const astring *src);

/* insert a C string into an astring */
astring *astring_insc(astring *dst, int insbefore, const char *src);

/* insert a character array into an astring */
astring *astring_insch(astring *dst, int insbefore, const char *src, int count);

/* insert a substring of one string into another */
astring *astring_inssubstr(astring *dst, int insbefore, const astring *src, int start, int count);

/* extract a substring of ourself, removing everything else */
astring *astring_substr(astring *str, int start, int count);

/* delete a substring from ourself, keeping everything else */
astring *astring_del(astring *str, int start, int count);

/* formatted printf to an astring */
int astring_printf(astring *dst, const char *format, ...) ATTR_PRINTF(2,3);

/* formatted vprintf to an astring */
int astring_vprintf(astring *dst, const char *format, va_list args);

/* formatted printf to the end of an astring */
int astring_catprintf(astring *dst, const char *format, ...) ATTR_PRINTF(2,3);

/* formatted vprintf to the end of an astring */
int astring_catvprintf(astring *dst, const char *format, va_list args);



/* ----- astring queries ----- */

/* return a pointer to a C string from an astring */
const char *astring_c(const astring *str);

/* return the length of an astring */
int astring_len(const astring *str);

/* compare two astrings */
int astring_cmp(const astring *str1, const astring *str2);

/* compare an astring to a C string */
int astring_cmpc(const astring *str1, const char *str2);

/* compare an astring to a character buffer */
int astring_cmpch(const astring *str1, const char *str2, int count);

/* compare an astring to a character buffer */
int astring_cmpsubstr(const astring *str1, const astring *str2, int start, int count);

/* case-insenstive compare two astrings */
int astring_icmp(const astring *str1, const astring *str2);

/* case-insenstive compare an astring to a C string */
int astring_icmpc(const astring *str1, const char *str2);

/* case-insenstive compare an astring to a character buffer */
int astring_icmpch(const astring *str1, const char *str2, int count);

/* case-insenstive compare an astring to a character buffer */
int astring_icmpsubstr(const astring *str1, const astring *str2, int start, int count);

/* search an astring for a character, returning offset or -1 if not found */
int astring_chr(const astring *str, int start, int ch);

/* reverse search an astring for a character, returning offset or -1 if not found */
int astring_rchr(const astring *str, int start, int ch);

/* search in an astring for another astring, returning offset or -1 if not found */
int astring_find(const astring *str, int start, const astring *search);

/* search in an astring for a C string, returning offset or -1 if not found */
int astring_findc(const astring *str, int start, const char *search);

/* search in an astring for another astring, replacing all instances with a third and returning the number of matches */
int astring_replace(astring *str, int start, const astring *search, const astring *replace);

/* search in an astring for a C string, replacing all instances with another C string and returning the number of matches */
int astring_replacec(astring *str, int start, const char *search, const char *replace);



/* ----- astring utilties ----- */

/* delete all instances of 'ch' */
astring *astring_delchr(astring *str, int ch);

/* replace all instances of 'ch' with 'newch' */
astring *astring_replacechr(astring *str, int ch, int newch);

/* convert string to all upper-case */
astring *astring_toupper(astring *str);

/* convert string to all lower-case */
astring *astring_tolower(astring *str);

/* remove all space characters from beginning/end */
astring *astring_trimspace(astring *str);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/* allocate a new duplicate of an astring */
INLINE astring *astring_dup(const astring *str)
{
	return astring_cpy(astring_alloc(), str);
}

/* allocate a new duplicate of an astring */
INLINE astring *astring_dupc(const char *str)
{
	return astring_cpyc(astring_alloc(), str);
}

/* allocate a new duplicate of an astring */
INLINE astring *astring_dupch(const char *str, int count)
{
	return astring_cpych(astring_alloc(), str, count);
}

/* allocate a duplicate of a substring */
INLINE astring *astring_dupsubstr(const astring *str, int start, int count)
{
	return astring_cpysubstr(astring_alloc(), str, start, count);
}


/* reset an astring to an empty string */
INLINE astring *astring_reset(astring *dst)
{
	return astring_cpyc(dst, "");
}


/* concatenate one astring to the end of another */
INLINE astring *astring_cat(astring *dst, const astring *src)
{
	return astring_ins(dst, -1, src);
}

/* concatenate a C string to the end of an astring */
INLINE astring *astring_catc(astring *dst, const char *src)
{
	return astring_insc(dst, -1, src);
}

/* concatenate a character array to the end of an astring */
INLINE astring *astring_catch(astring *dst, const char *src, int count)
{
	return astring_insch(dst, -1, src, count);
}

/* concatenate a substring of one string into another */
INLINE astring *astring_catsubstr(astring *dst, const astring *src, int start, int count)
{
	return astring_inssubstr(dst, -1, src, start, count);
}


/* assemble an astring from 2 C strings */
INLINE astring *astring_assemble_2(astring *dst, const char *src1, const char *src2)
{
	return astring_catc(astring_cpyc(dst, src1), src2);
}

/* assemble an astring from 3 C strings */
INLINE astring *astring_assemble_3(astring *dst, const char *src1, const char *src2, const char *src3)
{
	return astring_catc(astring_assemble_2(dst, src1, src2), src3);
}

/* assemble an astring from 4 C strings */
INLINE astring *astring_assemble_4(astring *dst, const char *src1, const char *src2, const char *src3, const char *src4)
{
	return astring_catc(astring_assemble_3(dst, src1, src2, src3), src4);
}

/* assemble an astring from 5 C strings */
INLINE astring *astring_assemble_5(astring *dst, const char *src1, const char *src2, const char *src3, const char *src4, const char *src5)
{
	return astring_catc(astring_assemble_4(dst, src1, src2, src3, src4), src5);
}


#endif /* __ASTRING_H__ */
