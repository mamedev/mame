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
#include <Rocket/Core/WString.h>

namespace Rocket {
namespace Core {

// Constructs an empty string.
WString::WString()
{
}

// Constructs a string as a copy of another UCS-2 string.
WString::WString(const WStringBase& ucs2_string) : WStringBase(ucs2_string)
{
}

// Constructs a string from a sequence of UCS-2 encoded characters.
WString::WString(const word* ucs2_string_begin, const word* ucs2_string_end) : WStringBase(ucs2_string_begin, ucs2_string_end)
{
}

// Constructs a string from multiples of a UCS-2 encoded character.
WString::WString(WStringBase::size_type count, word ucs2_char) : WStringBase(count, ucs2_char)
{
}

// Constructs a string from a sequence of UTF-8 encoded characters.
WString::WString(const char* utf8_string)
{
	std::vector< word > ucs2_string;
	StringUtilities::UTF8toUCS2(utf8_string, ucs2_string);

	if (ucs2_string.size() > 1)
		Assign(&ucs2_string[0], &ucs2_string[ucs2_string.size() - 1]);
}

// Constructs a string from a sequence of UTF-8 encoded characters.
	WString::WString(const char* utf8_string_begin, const char* utf8_string_end)
{
	std::vector< word > ucs2_string;
	StringUtilities::UTF8toUCS2(String(utf8_string_begin, utf8_string_end), ucs2_string);

	if (ucs2_string.size() > 1)
		Assign(&ucs2_string[0], &ucs2_string[ucs2_string.size() - 1]);
}

// Constructs a string from a sequence of UTF-8 encoded characters.
	WString::WString(const String& utf8_string)
{
	std::vector< word > ucs2_string;
	StringUtilities::UTF8toUCS2(utf8_string, ucs2_string);

	if (ucs2_string.size() > 1)
		Assign(&ucs2_string[0], &ucs2_string[ucs2_string.size() - 1]);
}

// Destroys the string.
WString::~WString()
{
}

// Converts the string to UTF-8 encoding.
String& WString::ToUTF8(String& utf8_string, bool append) const
{
	if (!append)
	{
		utf8_string.Clear();
	}
	StringUtilities::UCS2toUTF8(CString(), Length(), utf8_string);
	return utf8_string;
}

/// Assigns a UCS-2 string to this string.
WString& WString::operator=(const WStringBase& string)
{
	WStringBase::operator=(string);
	return *this;
}

// Assigns a UCS-2 string to this string.
WString& WString::operator=(const WString& string)
{
	WStringBase::operator=(string);
	return *this;
}

// Assigns a null-terminated UCS-2 string to this string.
WString& WString::operator=(const word* string)
{
	WStringBase::operator=(string);
	return *this;
}

// Converts a UTF-8 encoded string into UCS-2 and assigns it to this string.
WString& WString::operator=(const char* string)
{
	std::vector< word > ucs2_string;
	StringUtilities::UTF8toUCS2(string, ucs2_string);

	if (ucs2_string.size() > 1)
		Assign(&ucs2_string[0], &ucs2_string[ucs2_string.size() - 1]);
	else
		Clear();

	return *this;
}

// Checks equality between two UCS-2 encoded strings.
bool WString::operator==(const WString& string) const
{
	return WStringBase::operator==(string);
}

// Checks equality between this string and another string in UTF-8 encoded.
bool WString::operator==(const char* _string) const
{
	return WStringBase::operator==(WString(_string));
}

WStringBase::size_type WString::Find(const WString& s, size_type pos) const
{
	return WStringBase::Find(s, pos);
}

WStringBase::size_type WString::Find(const word* s, size_type pos) const
{
	return WStringBase::Find(s, pos);
}

WStringBase::size_type WString::Find(const word& s, size_type pos) const
{
	word buffer[2] = { s, 0 };
	return WStringBase::Find(buffer, pos);
}

word& WString::operator[](size_type offset)
{
	return WStringBase::operator[](offset);
}

word WString::operator[](size_type offset) const
{
	return WStringBase::operator[](offset);
}

}
}
