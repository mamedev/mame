/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
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
		int32_t len = vsnprintf(out, sizeof(temp), _format, _argList);
		if (int32_t(sizeof(temp) ) < len)
		{
			out = (char*)alloca(len);
			len = vsnprintf(out, len, _format, _argList);
		}
		_out.append(out, out+len);
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
		const typename Ty::size_type fromLen = strLen(_from);
		const typename Ty::size_type toLen   = strLen(_to);
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

	inline StringView::StringView(const StringView& _rhs, int32_t _start, int32_t _len)
	{
		set(_rhs, _start, _len);
	}

	inline StringView& StringView::operator=(const char* _rhs)
	{
		set(_rhs);
		return *this;
	}

	inline StringView& StringView::operator=(const StringView& _rhs)
	{
		set(_rhs);
		return *this;
	}

	inline StringView::StringView(char* _ptr)
	{
		set(_ptr, INT32_MAX);
	}

	inline StringView::StringView(const char* _ptr)
	{
		set(_ptr, INT32_MAX);
	}

	inline StringView::StringView(char* _ptr, int32_t _len)
	{
		set(_ptr, _len);
	}

	inline StringView::StringView(const char* _ptr, int32_t _len)
	{
		set(_ptr, _len);
	}

	inline StringView::StringView(const char* _ptr, const char* _term)
	{
		set(_ptr, _term);
	}

	template<typename Ty>
	inline StringView::StringView(const Ty& _container)
	{
		set(_container);
	}

	inline void StringView::set(char* _ptr)
	{
		set(_ptr, INT32_MAX);
	}

	inline void StringView::set(const char* _ptr)
	{
		set(_ptr, INT32_MAX);
	}

	inline void StringView::set(const char* _ptr, int32_t _len)
	{
		clear();

		if (NULL != _ptr)
		{
			m_len = INT32_MAX == _len ? strLen(_ptr) : _len;
			m_ptr = _ptr;
		}
	}

	inline void StringView::set(const char* _ptr, const char* _term)
	{
		set(_ptr, int32_t(_term-_ptr) );
	}

	template<typename Ty>
	inline void StringView::set(const Ty& _container)
	{
		set(_container.data(), int32_t(_container.length() ) );
	}

	inline void StringView::set(const StringView& _str, int32_t _start, int32_t _len)
	{
		const int32_t start = min(_start, _str.m_len);
		const int32_t len   = clamp(_str.m_len - start, 0, min(_len, _str.m_len) );
		set(_str.m_ptr + start, len);
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
		set(_rhs);
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>& StringT<AllocatorT>::operator=(const StringT<AllocatorT>& _rhs)
	{
		set(_rhs);
		return *this;
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT(const StringView& _rhs)
	{
		set(_rhs);
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::~StringT()
	{
		clear();
	}

	template<bx::AllocatorI** AllocatorT>
	inline void StringT<AllocatorT>::set(const StringView& _str)
	{
		clear();
		append(_str);
	}

	template<bx::AllocatorI** AllocatorT>
	inline void StringT<AllocatorT>::append(const StringView& _str)
	{
		if (0 != _str.getLength() )
		{
			int32_t old = m_len;
			int32_t len = m_len + strLen(_str);
			char* ptr = (char*)BX_REALLOC(*AllocatorT, 0 != m_len ? const_cast<char*>(m_ptr) : NULL, len+1);
			m_len = len;
			strCopy(ptr + old, len-old+1, _str);

			*const_cast<char**>(&m_ptr) = ptr;
		}
	}

	template<bx::AllocatorI** AllocatorT>
	inline void StringT<AllocatorT>::append(const char* _ptr, const char* _term)
	{
		append(StringView(_ptr, _term) );
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

	template<bx::AllocatorI** AllocatorT>
	inline const char* StringT<AllocatorT>::getCPtr() const
	{
		return getPtr();
	}

	inline StringView strSubstr(const StringView& _str, int32_t _start, int32_t _len)
	{
		return StringView(_str, _start, _len);
	}

	inline LineReader::LineReader(const bx::StringView& _str)
		: m_str(_str)
	{
		reset();
	}

	inline void LineReader::reset()
	{
		m_curr = m_str;
		m_line = 0;
	}

	inline StringView LineReader::next()
	{
		if (m_curr.getPtr() != m_str.getTerm() )
		{
			++m_line;

			StringView curr(m_curr);
			m_curr = bx::strFindNl(m_curr);

			StringView line(curr.getPtr(), m_curr.getPtr() );

			return strRTrim(line, "\n\r");
		}

		return m_curr;
	}

	inline bool LineReader::isDone() const
	{
		return m_curr.getPtr() == m_str.getTerm();
	}

	inline uint32_t LineReader::getLine() const
	{
		return m_line;
	}

} // namespace bx
