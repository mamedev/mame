/*-
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TINYSTL_STRING_H
#define TINYSTL_STRING_H

#include <string.h> // strlen
#include "stddef.h"
#include "hash.h"

namespace tinystl {

	template<typename Alloc>
	class stringT {
	public:
		stringT();
		stringT(const stringT<Alloc>& other);
		stringT(const char* sz);
		stringT(const char* sz, size_t len);
		~stringT();

		stringT<Alloc>& operator=(const stringT<Alloc>& other);

		const char* c_str() const;
		size_t size() const;
		bool empty() const;

		void reserve(size_t _size);
		void resize(size_t _size);

		void append(const char* first, const char* last);
		void append(const char* str);

		void swap(stringT<Alloc>& other);

	private:
		typedef char* pointer;
		pointer m_first;
		pointer m_last;
		pointer m_capacity;

		static const size_t c_nbuffer = 12;
		char m_buffer[12];
	};

	typedef stringT<TINYSTL_ALLOCATOR> string;

	template<typename Alloc>
	inline stringT<Alloc>::stringT()
		: m_first(m_buffer)
		, m_last(m_buffer)
		, m_capacity(m_buffer + c_nbuffer)
	{
		resize(0);
	}

	template<typename Alloc>
	inline stringT<Alloc>::stringT(const stringT<Alloc>& other)
		: m_first(m_buffer)
		, m_last(m_buffer)
		, m_capacity(m_buffer + c_nbuffer)
	{
		reserve(other.size());
		append(other.m_first, other.m_last);
	}

	template<typename Alloc>
	inline stringT<Alloc>::stringT(const char* sz)
		: m_first(m_buffer)
		, m_last(m_buffer)
		, m_capacity(m_buffer + c_nbuffer)
	{
		size_t len = 0;
		for (const char* it = sz; *it; ++it)
			++len;

		reserve(len);
		append(sz, sz + len);
	}

	template<typename Alloc>
	inline stringT<Alloc>::stringT(const char* sz, size_t len)
		: m_first(m_buffer)
		, m_last(m_buffer)
		, m_capacity(m_buffer + c_nbuffer)
	{
		reserve(len);
		append(sz, sz + len);
	}

	template<typename Alloc>
	inline stringT<Alloc>::~stringT() {
		if (m_first != m_buffer)
			Alloc::static_deallocate(m_first, m_capacity - m_first);
	}

	template<typename Alloc>
	inline stringT<Alloc>& stringT<Alloc>::operator=(const stringT<Alloc>& other) {
		stringT<Alloc>(other).swap(*this);
		return *this;
	}

	template<typename Alloc>
	inline const char* stringT<Alloc>::c_str() const {
		return m_first;
	}

	template<typename Alloc>
	inline size_t stringT<Alloc>::size() const
	{
		return (size_t)(m_last - m_first);
	}

	template<typename Alloc>
	inline bool stringT<Alloc>::empty() const
	{
		return 0 == size();
	}

	template<typename Alloc>
	inline void stringT<Alloc>::reserve(size_t capacity) {
		if (m_first + capacity + 1 <= m_capacity) {
			return;
		}

		const size_t _size = (size_t)(m_last - m_first);

		pointer newfirst = (pointer)Alloc::static_allocate(capacity + 1);
		for (pointer it = m_first, newit = newfirst, end = m_last; it != end; ++it, ++newit) {
			*newit = *it;
		}

		if (m_first != m_buffer) {
			Alloc::static_deallocate(m_first, m_capacity - m_first);
		}

		m_first = newfirst;
		m_last = newfirst + _size;
		m_capacity = m_first + capacity;
	}

	template<typename Alloc>
	inline void stringT<Alloc>::resize(size_t _size) {
		reserve(_size);
		for (pointer it = m_last, end = m_first + _size + 1; it < end; ++it)
			*it = 0;

		m_last += _size;
	}

	template<typename Alloc>
	inline void stringT<Alloc>::append(const char* first, const char* last) {
		const size_t newsize = (size_t)((m_last - m_first) + (last - first) + 1);
		if (m_first + newsize > m_capacity)
			reserve((newsize * 3) / 2);

		for (; first != last; ++m_last, ++first)
			*m_last = *first;
		*m_last = 0;
	}

	template<typename Alloc>
	inline void stringT<Alloc>::append(const char* str) {
		append(str, str + strlen(str) );
	}

	template<typename Alloc>
	inline void stringT<Alloc>::swap(stringT<Alloc>& other) {
		const pointer tfirst = m_first, tlast = m_last, tcapacity = m_capacity;
		m_first = other.m_first, m_last = other.m_last, m_capacity = other.m_capacity;
		other.m_first = tfirst, other.m_last = tlast, other.m_capacity = tcapacity;

		char tbuffer[c_nbuffer];

		if (m_first == other.m_buffer)
			for  (pointer it = other.m_buffer, end = m_last, out = tbuffer; it != end; ++it, ++out)
				*out = *it;

		if (other.m_first == m_buffer) {
			other.m_last = other.m_last - other.m_first + other.m_buffer;
			other.m_first = other.m_buffer;
			other.m_capacity = other.m_buffer + c_nbuffer;

			for (pointer it = other.m_first, end = other.m_last, in = m_buffer; it != end; ++it, ++in)
				*it = *in;
			*other.m_last = 0;
		}

		if (m_first == other.m_buffer) {
			m_last = m_last - m_first + m_buffer;
			m_first = m_buffer;
			m_capacity = m_buffer + c_nbuffer;

			for (pointer it = m_first, end = m_last, in = tbuffer; it != end; ++it, ++in)
				*it = *in;
			*m_last = 0;
		}
	}

	template<typename Alloc>
	inline bool operator==(const stringT<Alloc>& lhs, const stringT<Alloc>& rhs) {
		typedef const char* pointer;

		const size_t lsize = lhs.size(), rsize = rhs.size();
		if (lsize != rsize)
			return false;

		pointer lit = lhs.c_str(), rit = rhs.c_str();
		pointer lend = lit + lsize;
		while (lit != lend)
			if (*lit++ != *rit++)
				return false;

		return true;
	}

	template<typename Alloc>
	static inline size_t hash(const stringT<Alloc>& value) {
		return hash_string(value.c_str(), value.size());
	}
}

#endif
