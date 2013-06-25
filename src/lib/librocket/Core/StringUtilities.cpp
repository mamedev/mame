/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include <Rocket/Core/StringUtilities.h>
#include <ctype.h>
#include <stdio.h>

namespace Rocket {
namespace Core {

// Expands character-delimited list of values in a single string to a whitespace-trimmed list of values.
void StringUtilities::ExpandString(StringList& string_list, const String& string, const char delimiter)
{	
	char quote = 0;
	bool last_char_delimiter = true;
	const char* ptr = string.CString();
	const char* start_ptr = NULL;
	const char* end_ptr = ptr;

	while (*ptr)
	{
		// Switch into quote mode if the last char was a delimeter ( excluding whitespace )
		// and we're not already in quote mode
		if (last_char_delimiter && !quote && (*ptr == '"' || *ptr == '\''))
		{			
			quote = *ptr;
		}
		// Switch out of quote mode if we encounter a quote that hasn't been escaped
		else if (*ptr == quote && *(ptr-1) != '\\')
		{
			quote = 0;
		}
		// If we encouter a delimiter while not in quote mode, add the item to the list
		else if (*ptr == delimiter && !quote)
		{
			if (start_ptr)
				string_list.push_back(String(start_ptr, end_ptr + 1));
			else
				string_list.push_back("");
			last_char_delimiter = true;
			start_ptr = NULL;
		}
		// Otherwise if its not white space or we're in quote mode, advance the pointers
		else if (!isspace(*ptr) || quote)
		{
			if (!start_ptr)
				start_ptr = ptr;
			end_ptr = ptr;
			last_char_delimiter = false;
		}

		ptr++;
	}

	// If there's data pending, add it.
	if (start_ptr)
		string_list.push_back(String(start_ptr, end_ptr + 1));
}

// Joins a list of string values into a single string separated by a character delimiter.
void StringUtilities::JoinString(String& string, const StringList& string_list, const char delimiter)
{
	for (size_t i = 0; i < string_list.size(); i++)
	{
		string += string_list[i];
		if (delimiter != '\0' && i < string_list.size() - 1)
			string.Append(delimiter);
	}
}
	
// Hashes a string of data to an integer value using the FNV algorithm.
Hash StringUtilities::FNVHash(const char *string, int length)
{
	// FNV-1 hash algorithm
	Hash hval = 0;
	unsigned char* bp = (unsigned char *)string;	// start of buffer
	unsigned char* be = (unsigned char *)string + length;
	
	// FNV-1 hash each octet in the buffer
	while (*bp || (length >= 0 && bp < be)) 
	{
		// xor the bottom with the current octet
		hval ^= *bp++;
		
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if !defined(__GNUC__)		
		const unsigned int FNV_32_PRIME = ((unsigned int)16777619);
		hval *= FNV_32_PRIME;
#else
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
	}
			 
	return hval;
}
	
	// Defines, helper functions for the UTF8 / UCS2 conversion functions.
#define _NXT	0x80
#define _SEQ2	0xc0
#define _SEQ3	0xe0
#define _SEQ4	0xf0
#define _SEQ5	0xf8
#define _SEQ6	0xfc
	
#define _BOM	0xfeff
	
static int __wchar_forbidden(unsigned int sym)
{
	// Surrogate pairs
	if (sym >= 0xd800 && sym <= 0xdfff)
		return -1;
	
	return 0;
}

static int __utf8_forbidden(unsigned char octet)
{
	switch (octet)
	{
		case 0xc0:
		case 0xc1:
		case 0xf5:
		case 0xff:
			return -1;
			
		default:
			return 0;
	}
}



// Converts a character array in UTF-8 encoding to a vector of words.
bool StringUtilities::UTF8toUCS2(const String& input, std::vector< word >& output)
{
	if (input.Empty())
		return true;
	
	unsigned char* p = (unsigned char*) input.CString();
	unsigned char* lim = p + input.Length();
	
	// Skip the UTF-8 byte order marker if it exists.
	if (input.Substring(0, 3) == "\xEF\xBB\xBF")
		p += 3;
	
	int num_bytes;
	for (; p < lim; p += num_bytes)
	{
		if (__utf8_forbidden(*p) != 0)
			return false;
		
		// Get number of bytes for one wide character.
		word high;
		num_bytes = 1;
		
		if ((*p & 0x80) == 0)
		{
			high = (wchar_t)*p;
		}
		else if ((*p & 0xe0) == _SEQ2)
		{
			num_bytes = 2;
			high = (wchar_t)(*p & 0x1f);
		}
		else if ((*p & 0xf0) == _SEQ3)
		{
			num_bytes = 3;
			high = (wchar_t)(*p & 0x0f);
		}
		else if ((*p & 0xf8) == _SEQ4)
		{
			num_bytes = 4;
			high = (wchar_t)(*p & 0x07);
		}
		else if ((*p & 0xfc) == _SEQ5)
		{
			num_bytes = 5;
			high = (wchar_t)(*p & 0x03);
		}
		else if ((*p & 0xfe) == _SEQ6)
		{
			num_bytes = 6;
			high = (wchar_t)(*p & 0x01);
		}
		else
		{
			return false;
		}
		
		// Does the sequence header tell us the truth about length?
		if (lim - p <= num_bytes - 1)
		{
			return false;
		}
		
		// Validate the sequence. All symbols must have higher bits set to 10xxxxxx.
		if (num_bytes > 1)
		{
			int i;
			for (i = 1; i < num_bytes; i++)
			{
				if ((p[i] & 0xc0) != _NXT)
					break;
			}
			
			if (i != num_bytes)
			{
				return false;
			}
		}
		
		// Make up a single UCS-4 (32-bit) character from the required number of UTF-8 tokens. The first byte has
		// been determined earlier, the second and subsequent bytes contribute the first six of their bits into the
		// final character code.
		unsigned int ucs4_char = 0;
		int num_bits = 0;
		for (int i = 1; i < num_bytes; i++)
		{
			ucs4_char |= (word)(p[num_bytes - i] & 0x3f) << num_bits;
			num_bits += 6;
		}
		ucs4_char |= high << num_bits;
		
		// Check for surrogate pairs.
		if (__wchar_forbidden(ucs4_char) != 0)
		{
			return false;
		}
		
		// Only add the character to the output if it exists in the Basic Multilingual Plane (ie, fits in a single
		// word).
		if (ucs4_char <= 0xffff)
			output.push_back((word) ucs4_char);
	}
	
	output.push_back(0);
	return true;
}

// Converts a vector of words in UCS-2 encoding a character array in UTF-8 encoding.
bool StringUtilities::UCS2toUTF8(const std::vector< word >& input, String& output)
{
	return UCS2toUTF8(&input[0], input.size(), output);
}

// Converts an array of words in UCS-2 encoding into a character array in UTF-8 encoding.
bool StringUtilities::UCS2toUTF8(const word* input, size_t input_size, String& output)
{
	unsigned char *oc;
	size_t n;
	
	word* w = (word*) input;
	word* wlim = w + input_size;
	
	//Log::Message(LC_CORE, Log::LT_ALWAYS, "UCS2TOUTF8 size: %d", input_size);
	for (; w < wlim; w++)
	{
		if (__wchar_forbidden(*w) != 0)
			return false;
		
		if (*w == _BOM)
			continue;
		
		//if (*w < 0)
		//	return false;
		if (*w <= 0x007f)
			n = 1;
		else if (*w <= 0x07ff)
			n = 2;
		else //if (*w <= 0x0000ffff)
			n = 3;
		/*else if (*w <= 0x001fffff)
		 n = 4;
		 else if (*w <= 0x03ffffff)
		 n = 5;
		 else // if (*w <= 0x7fffffff)
		 n = 6;*/
		
		// Convert to little endian.
		word ch = (*w >> 8) & 0x00FF;
		ch |= (*w << 8) & 0xFF00;
		//		word ch = EMPConvertEndian(*w, ROCKET_ENDIAN_BIG);
		
		oc = (unsigned char *)&ch;
		switch (n)
		{
			case 1:
				output += oc[1];
				break;
				
			case 2:
				output += (_SEQ2 | (oc[1] >> 6) | ((oc[0] & 0x07) << 2));
				output += (_NXT | (oc[1] & 0x3f));
				break;
				
			case 3:
				output += (_SEQ3 | ((oc[0] & 0xf0) >> 4));
				output += (_NXT | (oc[1] >> 6) | ((oc[0] & 0x0f) << 2));
				output += (_NXT | (oc[1] & 0x3f));
				break;
				
			case 4:
				break;
				
			case 5:
				break;
				
			case 6:
				break;
		}
		
		//Log::Message(LC_CORE, Log::LT_ALWAYS, "Converting...%c(%d) %d -> %d", *w, *w, w - input, output.Length());
	}
	
	return true;
}

// Strip whitespace characters from the beginning and end of a string.
String StringUtilities::StripWhitespace(const String& string)
{
	const char* start = string.CString();
	const char* end = start + string.Length();
	
	while (start < end && IsWhitespace(*start))
		start++;
	
	while (end > start && IsWhitespace(*(end - 1)))
		end--;
	
	if (start < end)
		return String(start, end);
	
	return String();
}

// Operators for STL containers using strings.
bool StringUtilities::StringComparei::operator()(const String& lhs, const String& rhs) const
{
	return strcasecmp(lhs.CString(), rhs.CString()) < 0;
}

}
}
