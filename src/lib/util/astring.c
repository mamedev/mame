/***************************************************************************

    astring.c

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

****************************************************************************/

#include "astring.h"
#include "osdcore.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <new>


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const astring dummy_astring;



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  ensure_room - ensure we have room for a
//  given string, or else reallocate our buffer
//-------------------------------------------------

bool astring::ensure_room(int length)
{
	// always fail to expand the dummy
	if (this == &dummy_astring)
		return false;

	// if we have the room, do nothing
	if (m_alloclen >= length + 1)
		return true;

	// allocate a new buffer with some slop
	int alloclen = length + 256;
	char *newbuf = new char[alloclen];

	// swap in the new buffer and free the old one
	char *oldbuf = (m_text == m_smallbuf) ? NULL : m_text;
	m_text = strcpy(newbuf, m_text);
	m_len = strlen(m_text);
	m_alloclen = alloclen;
	delete[] oldbuf;

	return true;
}


//-------------------------------------------------
//  safe_string_base - return a "safe" string
//  base for a given start index
//-------------------------------------------------

inline char *astring::safe_string_base(int start) const
{
	int max = len();
	return (start >= 0 && start < max) ? m_text + start : m_text + max;
}


//-------------------------------------------------
//  normalize_substr - normalize substr parameters
//-------------------------------------------------

inline void astring::normalize_substr(int &start, int &count, int length) const
{
	// limit start
	if (start < 0)
		start = 0;
	else if (start > length)
		start = length;

	// update count
	if (count == -1 || start + count > length)
		count = length - start;
}



//**************************************************************************
//  ASTRING ALLOCATION
//**************************************************************************

//-------------------------------------------------
//  init - constructor helper
//-------------------------------------------------

astring &astring::init()
{
	// initialize ourselves to point to the internal buffer
	m_text = m_smallbuf;
	m_alloclen = ARRAY_LENGTH(m_smallbuf);
	m_smallbuf[0] = 0;
	m_len = 0;
	return *this;
}


//-------------------------------------------------
//  ~astring - destructor
//-------------------------------------------------

astring::~astring()
{
	if (m_text != m_smallbuf)
		delete[] m_text;
}



/***************************************************************************
    INLINE ASTRING CHANGES
***************************************************************************/

//-------------------------------------------------
//  cpy - copy a character array into an astring
//-------------------------------------------------

astring &astring::cpy(const char *src, int count)
{
	// make room; if we fail or if we are the dummy, do nothing
	if (!ensure_room(count))
		return *this;

	// copy the raw data and NULL-terminate
	if (count > 0 && m_text != src)
		memcpy(m_text, src, count);
	m_text[count] = 0;
	m_len = count;
	return *this;
}


//-------------------------------------------------
//  cpysubstr - copy a substring of one string to
//  another
//-------------------------------------------------

astring &astring::cpysubstr(const astring &src, int start, int count)
{
	normalize_substr(start, count, src.len());
	return cpy(src.m_text + start, count);
}


//-------------------------------------------------
//  ins - insert a character array into an astring
//-------------------------------------------------

astring &astring::ins(int insbefore, const char *src, int count)
{
	// make room; if we fail or if we are the dummy, do nothing
	int dstlength = len();
	if (!ensure_room(dstlength + count))
		return *this;

	// adjust insbefore to be logical
	if (insbefore < 0 || insbefore > dstlength)
		insbefore = dstlength;

	// copy the data an NULL-terminate
	if (insbefore < dstlength)
		memmove(m_text + insbefore + count, m_text + insbefore, dstlength - insbefore);
	memcpy(m_text + insbefore, src, count);
	m_text[dstlength + count] = 0;
	m_len = dstlength + count;
	return *this;
}


//-------------------------------------------------
//  inssubstr - insert a substring of one string
//  into another
//-------------------------------------------------

astring &astring::inssubstr(int insbefore, const astring &src, int start, int count)
{
	normalize_substr(start, count, src.len());
	return ins(insbefore, src.m_text + start, count);
}


//-------------------------------------------------
//  substr - extract a substring of ourself,
//  removing everything else
//-------------------------------------------------

astring &astring::substr(int start, int count)
{
	// ignore attempts to do this on the dummy
	if (this == &dummy_astring)
		return *this;

	// normalize parameters
	normalize_substr(start, count, len());

	// move the data and NULL-terminate
	if (count > 0 && start > 0)
		memmove(m_text, m_text + start, count);
	m_text[count] = 0;
	m_len = count;
	return *this;
}


//-------------------------------------------------
//  del - delete a substring of ourself, keeping
//  everything else
//-------------------------------------------------

astring &astring::del(int start, int count)
{
	// ignore attempts to do this on the dummy
	if (this == &dummy_astring)
		return *this;

	// normalize parameters
	int strlength = len();
	normalize_substr(start, count, strlength);

	// move the data and NULL-terminate
	if (count > 0)
		memmove(m_text + start, m_text + start + count, strlength - (start + count));
	m_text[strlength - count] = 0;
	m_len = strlength - count;
	return *this;
}


//-------------------------------------------------
//  vprintf - vprintf text into an astring
//-------------------------------------------------*/

int astring::vprintf(const char *format, va_list args)
{
	// sprintf into the temporary buffer
	char tempbuf[4096];
	int result = vsprintf(tempbuf, format, args);

	// set the result
	cpy(tempbuf);
	return result;
}


//-------------------------------------------------
//  catprintf - formatted vprintf to the end of
//  an astring
//-------------------------------------------------

int astring::catvprintf(const char *format, va_list args)
{
	// sprintf into the temporary buffer
	char tempbuf[4096];
	int result = vsprintf(tempbuf, format, args);

	// append the result
	cat(tempbuf);
	return result;
}



/***************************************************************************
    ASTRING QUERIES
***************************************************************************/

//-------------------------------------------------
//  cmp - compare a character array to an astring
//-------------------------------------------------

int astring::cmp(const char *str2, int count) const
{
	// loop while equal until we hit the end of strings
	int index;
	for (index = 0; index < count; index++)
		if (m_text[index] == 0 || m_text[index] != str2[index])
			break;

	// determine the final result
	if (index < count)
		return m_text[index] - str2[index];
	if (m_text[index] == 0)
		return 0;
	return 1;
}


//-------------------------------------------------
//  cmpsubstr - compare a substring to an astring
//-------------------------------------------------

int astring::cmpsubstr(const astring &str2, int start, int count) const
{
	normalize_substr(start, count, str2.len());
	return cmp(str2.m_text + start, count);
}


//-------------------------------------------------
//  icmp - compare a character array to an astring
//-------------------------------------------------

int astring::icmp(const char *str2, int count) const
{
	// loop while equal until we hit the end of strings
	int index;
	for (index = 0; index < count; index++)
		if (m_text[index] == 0 || tolower(m_text[index]) != tolower(str2[index]))
			break;

	// determine the final result
	if (index < count)
		return tolower(m_text[index]) - tolower(str2[index]);
	if (m_text[index] == 0)
		return 0;
	return 1;
}


//-------------------------------------------------
//  icmpsubstr - compare a substring to an astring
//-------------------------------------------------

int astring::icmpsubstr(const astring &str2, int start, int count) const
{
	normalize_substr(start, count, str2.len());
	return icmp(str2.m_text + start, count);
}


//-------------------------------------------------
//  chr - return the index of a character in an
//  astring
//-------------------------------------------------

int astring::chr(int start, int ch) const
{
	char *result = strchr(safe_string_base(start), ch);
	return (result != NULL) ? (result - m_text) : -1;
}


//-------------------------------------------------
//  rchr - return the index of a character in an
//  astring, searching from the end
//-------------------------------------------------

int astring::rchr(int start, int ch) const
{
	char *result = strrchr(safe_string_base(start), ch);
	return (result != NULL) ? (result - m_text) : -1;
}


//-------------------------------------------------
//  find - find a C string in an astring
//-------------------------------------------------*/

int astring::find(int start, const char *search) const
{
	char *result = strstr(safe_string_base(start), search);
	return (result != NULL) ? (result - m_text) : -1;
}


//-------------------------------------------------
//  replacec - search in an astring for a C string,
//  replacing all instances with another C string
//  and returning the number of matches
//-------------------------------------------------

int astring::replace(int start, const char *search, const char *replace)
{
	int searchlen = strlen(search);
	int replacelen = strlen(replace);
	int matches = 0;

	for (int curindex = find(start, search); curindex != -1; curindex = find(curindex + replacelen, search))
	{
		matches++;
		del(curindex, searchlen).ins(curindex, replace);
	}
	return matches;
}



//**************************************************************************
//  ASTRING UTILITIES
//**************************************************************************

//-------------------------------------------------
//  delchr - delete all instances of 'ch'
//-------------------------------------------------

astring &astring::delchr(int ch)
{
	// simple deletion
	char *dst = m_text;
	for (char *src = m_text; *src != 0; src++)
		if (*src != ch)
			*dst++ = *src;
	*dst = 0;
	m_len = strlen(m_text);
	return *this;
}


//-------------------------------------------------
//  replacechr - replace all instances of 'ch'
//  with 'newch'
//-------------------------------------------------

astring &astring::replacechr(int ch, int newch)
{
	// simple replacement
	for (char *text = m_text; *text != 0; text++)
		if (*text == ch)
			*text = newch;
	return *this;
}


//-------------------------------------------------
//  makeupper - convert string to all upper-case
//-------------------------------------------------

astring &astring::makeupper()
{
	// just makeupper() on all characters
	for (char *text = m_text; *text != 0; text++)
		*text = toupper((UINT8)*text);
	return *this;
}


//-------------------------------------------------
//  makelower - convert string to all lower-case
//-------------------------------------------------

astring &astring::makelower()
{
	// just tolower() on all characters
	for (char *text = m_text; *text != 0; text++)
		*text = tolower((UINT8)*text);
	return *this;
}


//-------------------------------------------------
//  trimspace - remove all space characters from
//  beginning/end
//-------------------------------------------------

astring &astring::trimspace()
{
	// first remove stuff from the end
	for (char *ptr = m_text + len() - 1; ptr >= m_text && (!(*ptr & 0x80) && isspace(UINT8(*ptr))); ptr--)
		*ptr = 0;

	// then count how much to remove from the beginning
	char *ptr;
	for (ptr = m_text; *ptr != 0 && (!(*ptr & 0x80) && isspace(UINT8(*ptr))); ptr++) ;
	if (ptr > m_text)
		substr(ptr - m_text);
	m_len = strlen(m_text);
	return *this;
}
