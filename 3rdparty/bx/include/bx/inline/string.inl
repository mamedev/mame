/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_STRING_H_HEADER_GUARD
#	error "Must be included from bx/string.h!"
#endif // BX_STRING_H_HEADER_GUARD

#if BX_CRT_MSVC && !defined(va_copy)
#	define va_copy(_a, _b) (_a) = (_b)
#endif // BX_CRT_MSVC && !defined(va_copy)

namespace bx
{
	template <typename Ty>
	inline void stringPrintfVargs(Ty& _out, const char* _format, va_list _argList)
	{
		char temp[2048];

		char* out = temp;
		int32_t len = bx::vsnprintf(out, sizeof(temp), _format, _argList);
		if ( (int32_t)sizeof(temp) < len)
		{
			out = (char*)alloca(len+1);
			len = bx::vsnprintf(out, len, _format, _argList);
		}
		out[len] = '\0';
		_out.append(out);
	}

	template <typename Ty>
	inline void stringPrintf(Ty& _out, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		stringPrintfVargs(_out, _format, argList);
		va_end(argList);
	}

	template <typename Ty>
	inline Ty replaceAll(const Ty& _str, const char* _from, const char* _to)
	{
		Ty str = _str;
		typename Ty::size_type startPos = 0;
		const typename Ty::size_type fromLen = strnlen(_from);
		const typename Ty::size_type toLen   = strnlen(_to);
		while ( (startPos = str.find(_from, startPos) ) != Ty::npos)
		{
			str.replace(startPos, fromLen, _to);
			startPos += toLen;
		}

		return str;
	}

	inline StringView::StringView()
	{
		clear();
	}

	inline StringView::StringView(const StringView& _rhs)
	{
		set(_rhs.m_ptr, _rhs.m_len);
	}

	inline StringView& StringView::operator=(const StringView& _rhs)
	{
		set(_rhs.m_ptr, _rhs.m_len);
		return *this;
	}

	inline StringView::StringView(const char* _ptr, int32_t _len)
	{
		set(_ptr, _len);
	}

	inline void StringView::set(const char* _ptr, int32_t _len)
	{
		clear();

		if (NULL != _ptr)
		{
			int32_t len = strnlen(_ptr, _len);
			if (0 != len)
			{
				m_len = len;
				m_ptr = _ptr;
			}
		}
	}

	inline void StringView::clear()
	{
		m_ptr = "";
		m_len = 0;
	}

	inline const char* StringView::getPtr() const
	{
		return m_ptr;
	}

	inline const char* StringView::getTerm() const
	{
		return m_ptr + m_len;
	}

	inline bool StringView::isEmpty() const
	{
		return 0 == m_len;
	}

	inline int32_t StringView::getLength() const
	{
		return m_len;
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT()
		: StringView()
	{
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT(const StringT<AllocatorT>& _rhs)
		: StringView()
	{
		set(_rhs.m_ptr, _rhs.m_len);
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>& StringT<AllocatorT>::operator=(const StringT<AllocatorT>& _rhs)
	{
		set(_rhs.m_ptr, _rhs.m_len);
		return *this;
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT(const char* _ptr, int32_t _len)
	{
		set(_ptr, _len);
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT(const StringView& _rhs)
	{
		set(_rhs.getPtr(), _rhs.getLength() );
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::~StringT()
	{
		clear();
	}

	template<bx::AllocatorI** AllocatorT>
	inline void StringT<AllocatorT>::set(const char* _ptr, int32_t _len)
	{
		clear();
		append(_ptr, _len);
	}

	template<bx::AllocatorI** AllocatorT>
	inline void StringT<AllocatorT>::append(const char* _ptr, int32_t _len)
	{
		if (0 != _len)
		{
			int32_t old = m_len;
			int32_t len = m_len + strnlen(_ptr, _len);
			char* ptr = (char*)BX_REALLOC(*AllocatorT, 0 != m_len ? const_cast<char*>(m_ptr) : NULL, len+1);
			m_len = len;
			strlncpy(ptr + old, len-old+1, _ptr, _len);

			*const_cast<char**>(&m_ptr) = ptr;
		}
	}

	template<bx::AllocatorI** AllocatorT>
	inline void StringT<AllocatorT>::clear()
	{
		if (0 != m_len)
		{
			BX_FREE(*AllocatorT, const_cast<char*>(m_ptr) );

			StringView::clear();
		}
	}

} // namespace bx
