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

#ifndef ROCKETCOREWSTRING_H
#define ROCKETCOREWSTRING_H

#include <Rocket/Core/Types.h>
#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

typedef StringBase< word > WStringBase;

/**
	Stores a string of characters encoded in UCS-2 format (UTF-16 without support for supplementary characters).

	@author Peter Curry
 */

class ROCKETCORE_API WString : public WStringBase
{
public:
	/// Constructs an empty string.
	WString();

	/// Constructs a string as a copy of another UCS-2 string.
	WString(const WStringBase& ucs2_string);
	/// Constructs a string from a sequence of UCS-2 encoded characters.
	WString(const word* ucs2_string_begin, const word* ucs2_string_end);
	/// Constructs a string from multiples of a UCS-2 encoded character.
	WString(WStringBase::size_type count, word ucs2_char);

	/// Constructs a string from a null-terminated sequence of UTF-8 encoded characters.
	WString(const char* utf8_string);
	/// Constructs a string from a sequence of UTF-8 encoded characters.
	WString(const char* utf8_string_begin, const char* utf8_string_end);
	/// Constructs a string from a sequence of UTF-8 encoded characters.
	WString(const String& utf8_string);

	/// Destroys the string.
	~WString();

	/// Converts the string to UTF-8 encoding.
	String& ToUTF8(String& utf8_string, bool append = false) const;

	/// Assigns a UCS-2 string to this string.
	WString& operator=(const WStringBase& string);
	/// Assigns a UCS-2 string to this string.
	WString& operator=(const WString& string);
	/// Assigns a null-terminated UCS-2 string to this string.
	WString& operator=(const word* string);
	/// Converts a UTF-8 encoded string into UCS-2 and assigns it to this string.
	WString& operator=(const char* string);

	/// Checks equality between two UCS-2 encoded strings.
	bool operator==(const WString& string) const;
	/// Checks equality between this string and another string in UTF-8 encoding.
	bool operator==(const char* string) const;

	WStringBase::size_type Find(const WString& s, WStringBase::size_type pos = WStringBase::npos) const;
	WStringBase::size_type Find(const word* s, WStringBase::size_type pos = WStringBase::npos) const;
	WStringBase::size_type Find(const word& s, WStringBase::size_type pos = WStringBase::npos) const;

	word& operator[](WStringBase::size_type offset);
	word operator[](WStringBase::size_type offset) const;
};

}
}

#endif
