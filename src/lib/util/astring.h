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

#include <stdarg.h>
#include "osdcomm.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* base astring structure */
typedef struct _astring_base astring_base;
struct _astring_base
{
	char *			text;
	int				alloclen;
	char			smallbuf[64 - sizeof(int) - sizeof(char *)];
};


/* class for C++, direct map for C */
#ifdef __cplusplus
class astring;
#else
typedef astring_base astring;
#endif



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



/***************************************************************************
    C++ WRAPPERS
***************************************************************************/

#ifdef __cplusplus

/* derived class for C++ */
class astring : public astring_base
{
private:
	astring &init();

public:
	astring() { init(); }
	~astring();

	astring(const char *string) { init().cpy(string); }
	astring(const char *str1, const char *str2) { init().cpy(str1).cat(str2); }
	astring(const char *str1, const char *str2, const char *str3) { init().cpy(str1).cat(str2).cat(str3); }
	astring(const char *str1, const char *str2, const char *str3, const char *str4) { init().cpy(str1).cat(str2).cat(str3).cat(str4); }
	astring(const char *str1, const char *str2, const char *str3, const char *str4, const char *str5) { init().cpy(str1).cat(str2).cat(str3).cat(str4).cat(str5); }
	astring(const astring &string) { init().cpy(string); }

	astring &operator=(const char *string) { return cpy(string); }
	astring &operator=(const astring &string) { return cpy(string); }

	astring &reset() { return cpy(""); }

	operator const char *() const { return astring_c(this); }
	const char *cstr() const { return astring_c(this); }
	int len() const { return astring_len(this); }

	astring &cpy(const astring &src) { return *astring_cpy(this, &src); }
	astring &cpy(const char *src) { return *astring_cpyc(this, src); }
	astring &cpy(const char *src, int count) { return *astring_cpych(this, src, count); }
	astring &cpysubstr(const astring &src, int start, int count) { return *astring_cpysubstr(this, &src, start, count); }

	astring &cat(const astring &src) { return ins(-1, src); }
	astring &cat(const char *src) { return ins(-1, src); }
	astring &cat(const char *src, int count) { return ins(-1, src, count); }
	astring &catsubstr(const astring &src, int start, int count) { return inssubstr(-1, src, start, count); }

	astring &ins(int insbefore, const astring &src) { return *astring_ins(this, insbefore, &src); }
	astring &ins(int insbefore, const char *src) { return *astring_insc(this, insbefore, src); }
	astring &ins(int insbefore, const char *src, int count) { return *astring_insch(this, insbefore, src, count); }
	astring &inssubstr(int insbefore, const astring &src, int start, int count) { return *astring_inssubstr(this, insbefore, &src, start, count); }

	astring &substr(int start, int count) { return *astring_substr(this, start, count); }
	astring &del(int start, int count) { return *astring_del(this, start, count); }

	int printf(const char *format, ...) { va_list ap; va_start(ap, format); int result = astring_vprintf(this, format, ap); va_end(ap); return result; }
	int vprintf(const char *format, va_list args) { return astring_vprintf(this, format, args); }
	int catprintf(const char *format, ...) { va_list ap; va_start(ap, format); int result = astring_catvprintf(this, format, ap); va_end(ap); return result; }
	int catvprintf(const char *format, va_list args) { return astring_catvprintf(this, format, args); }

	int cmp(const astring &str2) const { return astring_cmp(this, &str2); }
	int cmp(const char *str2) const { return astring_cmpc(this, str2); }
	int cmp(const char *str2, int count) const { return astring_cmpch(this, str2, count); }
	int cmpsubstr(const astring &str2, int start, int count) const { return astring_cmpsubstr(this, &str2, start, count); }

	int icmp(const astring &str2) const { return astring_icmp(this, &str2); }
	int icmp(const char *str2) const { return astring_icmpc(this, str2); }
	int icmp(const char *str2, int count) const { return astring_icmpch(this, str2, count); }
	int icmpsubstr(const astring &str2, int start, int count) const { return astring_icmpsubstr(this, &str2, start, count); }

	int chr(int start, int ch) const { return astring_chr(this, start, ch); }
	int rchr(int start, int ch) const { return astring_rchr(this, start, ch); }

	int find(int start, const astring &search) const { return astring_find(this, start, &search); }
	int find(int start, const char *search) const { return astring_findc(this, start, search); }

	int replace(int start, const astring &search, const astring &replace) { return astring_replace(this, start, &search, &replace); }
	int replace(int start, const char *search, const char *replace) { return astring_replacec(this, start, search, replace); }

	astring &delchr(int ch) { return *astring_delchr(this, ch); }
	astring &replacechr(int ch, int newch) { return *astring_replacechr(this, ch, newch); }
	astring &toupper() { return *astring_toupper(this); }
	astring &tolower() { return *astring_tolower(this); }
	astring &trimspace() { return *astring_trimspace(this); }
};

#endif


#endif /* __ASTRING_H__ */
