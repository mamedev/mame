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

template< typename T >
StringBase< T >::StringBase() : value((T*)local_buffer), buffer_size(LOCAL_BUFFER_SIZE), length(0), hash(0)
{
	value[0] = 0;
}

template< typename T >
StringBase< T >::StringBase(const StringBase< T >& copy) : value((T*)local_buffer), buffer_size(LOCAL_BUFFER_SIZE), length(0), hash(0)
{
	value[0] = 0;
	*this = copy;
}

template< typename T >
StringBase< T >::StringBase(const T* string) : value((T*)local_buffer), buffer_size(LOCAL_BUFFER_SIZE), length(0), hash(0)
{
	value[0] = 0;
	*this = string;
}

template< typename T >
StringBase< T >::StringBase(const T* string_start, const T* string_end) : value((T*)local_buffer), buffer_size(LOCAL_BUFFER_SIZE), length(0), hash(0)
{
	value[0] = 0;
	length = (string_end - string_start);

	if (length > 0)
	{
		Reserve(length);
		Copy(value, string_start, length, true);		
	}
}

template< typename T >
StringBase< T >::StringBase(size_type count, const T character) : value((T*)local_buffer), buffer_size(LOCAL_BUFFER_SIZE), length(0), hash(0)
{
	value[0] = 0;
	length = count;

	if (length > 0)
	{
		Reserve(length);
		for (size_type i = 0; i < length; i++)
			value[i] = character;
		value[length] = '\0';
	}
}

template< typename T >
StringBase< T >::StringBase(size_type ROCKET_UNUSED(max_length), const T* ROCKET_UNUSED(fmt), ...) : value((T*)local_buffer), buffer_size(LOCAL_BUFFER_SIZE), length(0), hash(0)
{
	value[0] = 0;
	// Can't implement this at the base level, requires template specialisation
	ROCKET_ERRORMSG("Not implemented.");
}

template< typename T >
StringBase< T >::~StringBase()
{
	if (value != (T*)local_buffer)
		free(value);
}

template< typename T >
bool StringBase< T >::Empty() const
{
	return length == 0;
}

template< typename T >
void StringBase< T >::Clear()
{
	if (value != (T*)local_buffer)
		free(value);

	length = 0;
	hash = 0;
	value = (T*)local_buffer;
	buffer_size = LOCAL_BUFFER_SIZE;
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::Length() const
{
	return length;
}

template< typename T >
unsigned int StringBase< T >::Hash() const
{
	if (hash == 0 && length > 0)
	{
		// FNV-1 hash algorithm
		unsigned char* bp = (unsigned char *)value;	// start of buffer
		unsigned char* be = (unsigned char *)value + (length * sizeof(T));
		
		// FNV-1a hash each octet in the buffer
		while (bp < be) 
		{
			// xor the bottom with the current octet
			hash ^= *bp++;
			
			/* multiply by the 32 bit FNV magic prime mod 2^32 */
			#if !defined(__GNUC__)
				const unsigned int FNV_32_PRIME = ((unsigned int)16777619);
				hash *= FNV_32_PRIME;
			#else
				hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
			#endif
		}
	}
	return hash;
}

template< typename T >
const T* StringBase< T >::CString() const
{
	return value;
}

template< typename T >
void StringBase< T >::Reserve(size_type size)
{
	size_type new_size = (size + 1) * sizeof(T);
	
	if (buffer_size >= new_size)
		return;
	
	// Pad out to a block of 16 bytes
	const int BLOCK_SIZE = 16;
	new_size = (new_size+BLOCK_SIZE-1)&(~(BLOCK_SIZE-1));
	
	buffer_size = new_size;
	
	if (value == (T*)local_buffer)
	{
		T* new_value = (T*)realloc(NULL, buffer_size);
		Copy(new_value, (T*)local_buffer, LOCAL_BUFFER_SIZE / sizeof(T));
		value = new_value;
	}
	else
	{
		value = (T*)realloc(value, buffer_size);
	}
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::Find(const T* find, size_type offset) const
{
	return _Find(find, GetLength(find), offset);	
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::Find(const StringBase< T >& find, size_type offset) const
{
	return _Find(find.CString(), find.Length(), offset);
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::RFind(const T* find, size_type offset) const
{
	return _RFind(find, GetLength(find), offset);	
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::RFind(const StringBase< T >& find, size_type offset) const
{
	return _RFind(find.CString(), find.Length(), offset);
}

template< typename T >
StringBase< T > StringBase< T >::Replace(const T* find, const T* replace) const
{
	return _Replace(find, GetLength(find), replace, GetLength(replace));
}

template< typename T >
StringBase< T > StringBase< T >::Replace(const StringBase< T >& find, const StringBase< T >& replace) const
{
	return _Replace(find.CString(), find.Length(), replace.CString(), replace.Length());
}

template< typename T >
StringBase< T > StringBase< T >::Substring(size_type start, size_type count) const
{	
	// Ensure we're not going of bounds
	if (count > length - start)
		count = length - start;

	if (start > length)
		count = 0;

	return StringBase< T >(&value[start], &value[start + count]);
}

template< typename T >
StringBase< T >& StringBase< T >::Append(const T* append, size_type count)
{
	return _Append(append, GetLength(append), count);
}

template< typename T >
StringBase< T >& StringBase< T >::Append(const StringBase< T >& append, size_type count)
{
	return _Append(append.CString(), append.Length(), count);
}

template< typename T >
StringBase< T >& StringBase< T >::Append(const T& append)
{
	T buffer[2] = { append, 0 };
	return (*this += buffer);
}

template< typename T >
StringBase< T >& StringBase< T >::Assign(const T* assign, size_type count)
{
	size_type assign_length = GetLength(assign);
	return _Assign(assign, count > assign_length ? assign_length : count);
}

template< typename T >
StringBase< T >& StringBase< T >::Assign(const T* assign, const T* end)
{	
	return _Assign(assign, end - assign);
}

template< typename T >
StringBase< T >& StringBase< T >::Assign(const StringBase< T >& assign, size_type count)
{
	return _Assign(assign.CString(), assign.length, count);
}

// Insert a string into this string
template< typename T >
void StringBase< T >::Insert(size_type index, const T* insert, size_type count)
{
	return _Insert(index, insert, GetLength(insert), count);
}

// Insert a string into this string
template< typename T >
void StringBase< T >::Insert(size_type index, const StringBase< T >& insert, size_type count)
{
	return _Insert(index, insert.value, insert.length, count);
}

// Insert a character into this string
template< typename T >
void StringBase< T >::Insert(size_type index, const T& insert)
{
	return _Insert(index, &insert, 1, 1);
}

/// Erase characters from this string
template< typename T >
void StringBase< T >::Erase(size_type index, size_type count)
{
	if (index >= length)
		return;

	if (count == npos)
	{
		Resize(index);
	}
	else
	{
		size_type erase_amount = count < length - index ? count : length - index;
			
		Copy(&value[index], &value[index + erase_amount], length - index - erase_amount, true);		

		length -= erase_amount;

		if (length == 0)
			Clear();
	}
}

template< typename T >
int StringBase< T >::FormatString(size_type ROCKET_UNUSED(max_length), const T* ROCKET_UNUSED(fmt), ...)
{
	ROCKET_ERRORMSG("Not implemented.");
	return -1;
}

template< typename T >
void StringBase< T >::Resize(size_type new_length)
{
	Reserve(new_length);
	length = new_length;
	value[length] = '\0';

	if (length == 0)
		Clear();
}

// Create a lowercase version of the string
template< typename T >
StringBase< T > StringBase< T >::ToLower() const
{
	// Loop through the string, looking for an uppercase character
	size_t copy_index = npos;
	for (size_t i = 0; i < length; i++)
	{
		if (value[i] >= 'A' && value[i] <= 'Z')
		{
			copy_index = i;
			break;
		}
	}

	// If theres no lowercase letters, simply copy us direct
	if (copy_index == npos)
		return StringBase< T >(*this);

	StringBase< T > lowercase(CString(), CString() + copy_index);
	// Otherwise trawl through the rest of the letters
	for (size_t i = copy_index; i < length; i++)
	{
		if (value[i] >= 'A' && value[i] <= 'Z')
			lowercase.Append((T)(value[i] + ('a' - 'A')));
		else
			lowercase.Append(value[i]);
	}

	return lowercase;
}

// Create a lowercase version of the string
template< typename T >
StringBase< T > StringBase< T >::ToUpper() const
{
	// Loop through the string, looking for an uppercase character
	size_t copy_index = npos;
	for (size_t i = 0; i < length; i++)
	{
		if (value[i] >= 'a' && value[i] <= 'z')
		{
			copy_index = i;
			break;
		}
	}

	// If theres no lowercase letters, simply copy us direct
	if (copy_index == npos)
		return StringBase< T >(*this);

	StringBase< T > uppercase(CString(), CString() + copy_index);
	// Otherwise trawl through the rest of the letters
	for (size_t i = copy_index; i < length; i++)
	{
		if (value[i] >= 'a' && value[i] <= 'z')
			uppercase.Append((T)(value[i] - ('a' - 'A')));
		else
			uppercase.Append(value[i]);
	}

	return uppercase;
}

template< typename T >
bool StringBase< T >::operator==(const T* compare) const
{
	size_type index = 0;
	
	while (index < length && compare[index] == value[index])
		index++;

	return index == length && compare[index] == '\0';	
}

template< typename T >
bool StringBase< T >::operator==(const StringBase< T >& compare) const
{
	if (length != compare.length)
		return false;
	
	if (Hash() != compare.Hash())
		return false;
		
	return (*this) == compare.value;
}

template< typename T >
bool StringBase< T >::operator!=(const T* compare) const
{
	return !(*this == compare);
}

template< typename T >
bool StringBase< T >::operator!=(const StringBase< T >& compare) const
{
	return !(*this == compare);
}

template< typename T >
bool StringBase< T >::operator<(const T* compare) const
{
	size_type index = 0;
	while (index < length && compare[index] == value[index])
		index++;

	// Check if we reached the end of the string
	if (index < length)
	{
		// If we didn't check if we reached the end of
		// the string we're comparing against, if so
		// then we're not less than
		if (compare[index] == 0)
			return false;

		// Check the character at index
		return value[index] < compare[index];
	}
	else
	{
		// We reached the end of our string,
		// if the string we're comparing with still
		// has data, then we're smaller
		if (compare[index] != 0)
			return true;		
	}

	return false;
}

template< typename T >
bool StringBase< T >::operator<(const StringBase< T >& compare) const
{
	return *this < compare.CString();
}

template< typename T >
StringBase< T >& StringBase< T >::operator=(const T* assign)
{
	return Assign(assign);
}

template< typename T >
StringBase< T >& StringBase< T >::operator=(const StringBase< T >& assign)
{	
	return Assign(assign);
}

template< typename T >
StringBase< T > StringBase< T >::operator+(const T* add) const
{	
	StringBase< T > combined(*this);
	combined.Append(add);	
	
	return combined;
}

template< typename T >
StringBase< T > StringBase< T >::operator+(const StringBase< T >& add) const
{	
	StringBase< T > combined(*this);
	combined.Append(add);	
	
	return combined;
}

template< typename T >
StringBase< T >& StringBase< T >::operator+=(const T* add)
{	
	return Append(add);
}

template< typename T >
StringBase< T >& StringBase< T >::operator+=(const StringBase< T >& add)
{	
	return _Append(add.CString(), add.length);
}

template< typename T >
StringBase< T >& StringBase< T >::operator+=(const T& add)
{	
	return Append(add);
}

template< typename T >
const T& StringBase< T >::operator[](size_type index) const
{
	ROCKET_ASSERT(index < length);
	return value[index];
}

template< typename T >
T& StringBase< T >::operator[](size_type index)
{
	ROCKET_ASSERT(index < length);
	return value[index];
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::GetLength(const T* string) const
{	
	const T* ptr = string;

	while (*ptr)
	{
		ptr++;		
	}

	return ptr - string;
}

template< typename T >
void StringBase< T >::Copy(T* target, const T* src, size_type length, bool terminate)
{
	// Copy values
	for (size_type i = 0; i < length; i++)
	{
		*target++ = *src++;
	}
	
	if (terminate)
	{		
		*target++ = 0;		
	}
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::_Find(const T* find, size_type find_length, size_type offset) const
{
	size_type needle_index = 0;	
	size_type haystack_index = offset;

	// If the find length is greater than the string we have, it can't be here
	if (find_length > length)
		return npos;

	// While there's still data in the haystack loop
	while (value[haystack_index])
	{
		// If the current haystack posize_typeer plus needle offset matches,
		// advance the needle index
		if (value[haystack_index + needle_index] == find[needle_index])
		{
			needle_index++;

			// If we reach the end of the search term, return the current haystack index
			if (needle_index == find_length)
				return haystack_index;
		}
		else		
		{
			// Advance haystack index by one and reset needle index.
			haystack_index++;
			needle_index = 0;
		}
	}

	return npos;
}

template< typename T >
typename StringBase< T >::size_type StringBase< T >::_RFind(const T* find, size_type find_length, size_type offset) const
{
	ROCKET_ASSERT(find_length > 0);

	size_type needle_index = 0;	
	size_type haystack_index = (offset < length ? offset : length) - find_length;

	// If the find length is greater than the string we have, it can't be here
	if (find_length > length)
		return npos;

	// While theres still data in the haystack loop
	for (;;)
	{
		// If the current haystack index plus needle offset matches,
		// advance the needle index
		if (value[haystack_index + needle_index] == find[needle_index])
		{
			needle_index++;

			// If we reach the end of the search term, return the current haystack index
			if (find[needle_index] == 0)
				return haystack_index;
		}
		else		
		{
			if (haystack_index == 0)
				return npos;

			// Advance haystack index backwards
			haystack_index--;
			needle_index = 0;
		}
	}
}

template< typename T >
StringBase< T > StringBase< T >::_Replace(const T* find, size_type find_length, const T* replace, size_type replace_length) const
{
	StringBase< T > result;

	size_type offset = 0;	
	// Loop until we reach the end of the string
	while (offset < Length())
	{
		// Look for the next search term
		size_type pos = _Find(find, find_length, offset);

		// Term not found, add remainder and return
		if (pos == npos)
			return result + (Substring(offset).CString());

		// Add the unchanged text and replacement after it
		result += Substring(offset, pos - offset);
		result._Append(replace, replace_length);

		// Advance the find position
		offset = pos + find_length;
	}
	
	hash = 0;

	return result;
}

template< typename T >
StringBase< T >& StringBase< T >::_Append(const T* append, size_type append_length, size_type count)
{	
	size_type add_length = count < append_length ? count : append_length;

	if (add_length == 0)
		return *this;

	Reserve(length + add_length);
	Copy(&value[length], append, add_length, true);
	length += add_length;
	
	hash = 0;
	
	return *this;
}

template< typename T >
StringBase< T >& StringBase< T >::_Assign(const T* assign, size_type assign_length, size_type count)
{		
	size_type new_length = count < assign_length ? count : assign_length;

	if (new_length == 0)
	{
		Clear();
	}
	else
	{
		Reserve(new_length);
		Copy(value, assign, new_length, true);
	}

	length = new_length;
	
	hash = 0;
	
	return *this;
}

template< typename T >
void StringBase< T >::_Insert(size_type index, const T* insert, size_type insert_length, size_type count)
{
	if (index >= length)
	{
		Append(insert, count);
		return;
	}
	
	size_type add_length = count < insert_length ? count : insert_length;

	Reserve(length + add_length);
	
	for (size_type i = length + 1; i > index; i--)
		value[i + add_length - 1] = value[i - 1];

	Copy(&value[index], insert, add_length);
	length += add_length;
	
	hash = 0;
}
