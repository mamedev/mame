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

#ifndef ROCKETCORESTRINGBASE_H
#define ROCKETCORESTRINGBASE_H

#include <Rocket/Core/Debug.h>
#include <stdlib.h>

namespace Rocket {
namespace Core {

/**
	Base String Implementation
	@author Lloyd Weehuizen
 */
template< typename T >
class StringBase
{
public:
	typedef unsigned int size_type;
	static const size_type npos = (size_type)-1;

	StringBase();
	StringBase(const StringBase& copy);
	StringBase(const T* string);
	StringBase(const T* string_start, const T* string_end);
	StringBase(size_type length, const T character);
	StringBase(size_type max_length, const T* fmt, ...);

	~StringBase();

	/// Is the string currently empty
	inline bool Empty() const;
	/// Clear the string to empty
	void Clear();

	/// The length of the string
	inline size_type Length() const;
	/// Get the hash value of this string
	inline unsigned int Hash() const;
	/// Access the string as a standard C string
	inline const T* CString() const;
	
	/// Reserve space for at least this much data
	inline void Reserve(size_type size);

	/// Find the given string within this string
	/// @param find The string to search for
	/// @param offset Starting location of the search
	size_type Find(const T* find, size_type offset = 0) const;
	/// Find the given string within this string
	/// @param find The string to search for
	/// @param offset Starting location of the search
	size_type Find(const StringBase<T>& find, size_type offset = 0) const;
	/// Reverse find the given string within this string
	/// @param find The string to search for
	/// @param offset Starting location of the search
	size_type RFind(const T* find, size_type offset = npos) const;
	/// Reverse find the given string within this string
	/// @param find The string to search for
	/// @param offset Starting location of the search
	size_type RFind(const StringBase<T>& find, size_type offset = npos) const;

	/// Replace all occurances of the given string with another
	/// @param find The string to search for
	/// @param replace The string to replace it with
	StringBase<T> Replace(const T* find, const T* replace) const;
	/// Replace all occurances of the given string with another
	/// @param find The string to search for
	/// @param replace The string to replace it with
	StringBase<T> Replace(const StringBase<T>& find, const StringBase<T>& replace) const;

	/// Return a substring of this string
	/// @param start The starting position
	/// @param length The number of characters to copy
	inline StringBase<T> Substring(size_type start, size_type length = StringBase<T>::npos) const;

	/// Append the given string to this string
	/// @param append The string to appen
	/// @param count The number of characters to append
	inline StringBase<T>& Append(const T* append, size_type count = StringBase<T>::npos);
	/// Assign the given string to this string
	/// @param assign The string to assign
	/// @param count The number of characters to assign
	inline StringBase<T>& Append(const StringBase<T>& append, size_type count = StringBase<T>::npos);
	/// Append a single character
	/// @param append The character to append
	inline StringBase<T>& Append(const T& append);

	/// Assign the given string to this string
	/// @param assign The string to assign
	/// @param length The number of characters to assign
	inline StringBase<T>& Assign(const T* assign, size_type count = StringBase<T>::npos);
	/// Assign the given string to this string
	/// @param assign The string to assign
	/// @param count The number of characters to assign
	inline StringBase<T>& Assign(const T* assign, const T* end);
	/// Assign the given string to this string
	/// @param assign The string to assign
	/// @param count The number of characters to assign
	inline StringBase<T>& Assign(const StringBase<T>& assign, size_type count = StringBase<T>::npos);

	/// Insert a string into this string
	/// @param index Index to insert the characters
	/// @param insert String to insert
	/// @param count Number of characters to insert
	inline void Insert(size_type index, const T* insert, size_type count = StringBase<T>::npos);
	/// Insert a string into this string
	/// @param index Index to insert the characters
	/// @param insert String to insert
	/// @param count Number of characters to insert
	inline void Insert(size_type index, const StringBase<T>& insert, size_type count = StringBase<T>::npos);
	/// Insert a character into this string
	/// @param index Index to insert the characters
	/// @param insert Character to insert	
	inline void Insert(size_type index, const T& insert);

	/// Erase characters from this string
	/// @param index Index to erase the characters
	/// @param length Number of characters to erase
	inline void Erase(size_type index, size_type length = StringBase<T>::npos);

	/// sprsize_typef style string formatting.
	/// NOTE: This is not implemented in the base layer and requires template
	/// specialisation of the specific string type
	/// @param max_length Maximum length of the result
	/// @param format The sprsize_typef style formatting
	int FormatString(size_type max_length, const T* format, ...);

	/// Resize the string to the given size, inserts space if the string is getting bigger
	/// @param size New size
	void Resize(size_type size);

	/// Create a lowercase version of the string
	/// @returns The lower case representation of the string
	StringBase<T> ToLower() const;
	/// Create a lowercase version of the string
	/// @returns The lower case representation of the string
	StringBase<T> ToUpper() const;

	inline bool operator==(const T* compare) const;
	inline bool operator==(const StringBase<T>& compare) const;

	inline bool operator!=(const T* compare) const;
	inline bool operator!=(const StringBase<T>& compare) const;

	inline bool operator<(const T* compare) const;
	inline bool operator<(const StringBase<T>& compare) const;

	inline StringBase<T>& operator=(const T* assign);
	inline StringBase<T>& operator=(const StringBase<T>& assign);

	inline StringBase<T> operator+(const T* append) const;
	inline StringBase<T> operator+(const StringBase<T>& append) const;

	inline StringBase<T>& operator+=(const T* append);
	inline StringBase<T>& operator+=(const StringBase<T>& append);
	inline StringBase<T>& operator+=(const T& append);

	inline const T& operator[](size_type index) const;
	inline T& operator[](size_type index);

protected:	

	T* value;
	size_type buffer_size;
	size_type length;
	mutable unsigned int hash;
	static const size_type LOCAL_BUFFER_SIZE = 8;
	char local_buffer[LOCAL_BUFFER_SIZE];

	size_type GetLength(const T* string) const;

	// Copies the source string to target string
	inline void Copy(T* target, const T* src, size_type length, bool terminate = false);

	// Internal implementations of the public interfaces,
	// all these functions take the length of the const T*'s they're
	// dealing with which *MUST* be accurate.
	// Its up to the external interfaces to provide valid values for these functions
	inline size_type _Find(const T* find, size_type find_length, size_type offset = 0) const;
	inline size_type _RFind(const T* find, size_type find_length, size_type offset = 0) const;
	inline StringBase<T> _Replace(const T* find, size_type find_length, const T* replace, size_type replace_length) const;
	inline StringBase<T>& _Append(const T* append, size_type append_length, size_type count = StringBase<T>::npos);
	inline StringBase<T>& _Assign(const T* assign, size_type assign_length, size_type count = StringBase<T>::npos);
	inline void _Insert(size_type index, const T* insert, size_type insert_length, size_type count = StringBase<T>::npos);
};
	
#include <Rocket/Core/StringBase.inl>

}
}

#endif
