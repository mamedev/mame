/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
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

	inline constexpr StringLiteral::StringLiteral()
		: m_ptr("")
		, m_len(0)
	{
	}

	template<int32_t SizeT>
	inline constexpr StringLiteral::StringLiteral(const char (&str)[SizeT])
		: m_ptr(str)
		, m_len(SizeT - 1)
	{
		BX_ASSERT('\0' == m_ptr[SizeT - 1], "Must be 0 terminated.");
	}

	inline constexpr int32_t StringLiteral::getLength() const
	{
		return m_len;
	}

	inline constexpr const char* StringLiteral::getCPtr() const
	{
		return m_ptr;
	}

	inline void StringLiteral::clear()
	{
		m_ptr = "";
		m_len = 0;
	}

	inline bool StringLiteral::isEmpty() const
	{
		return 0 == m_len;
	}

	inline StringView::StringView()
	{
		clear();
	}

	inline constexpr StringView::StringView(const StringLiteral& _str)
		: m_ptr(_str.getCPtr() )
		, m_len(_str.getLength() )
		, m_0terminated(true)
	{
	}

	inline StringView::StringView(const StringView& _rhs)
	{
		set(_rhs);
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

	inline StringView::StringView(const char* _ptr)
	{
		set(_ptr, INT32_MAX);
	}

	inline StringView::StringView(const char* _ptr, int32_t _len)
	{
		set(_ptr, _len);
	}

	inline StringView::StringView(const char* _ptr, const char* _term)
	{
		set(_ptr, _term);
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
			m_0terminated = INT32_MAX == _len;
		}
	}

	inline void StringView::set(const char* _ptr, const char* _term)
	{
		set(_ptr, int32_t(_term-_ptr) );
	}

	inline void StringView::set(const StringView& _str)
	{
		set(_str, 0, INT32_MAX);
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
		m_0terminated = true;
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

	inline bool StringView::is0Terminated() const
	{
		return m_0terminated;
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT()
		: StringView()
		, m_capacity(0)
	{
		clear();
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT(const StringT<AllocatorT>& _rhs)
		: StringView()
		, m_capacity(0)
	{
		set(_rhs);
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::StringT(const StringView& _rhs)
		: StringView()
		, m_capacity(0)
	{
		set(_rhs);
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>::~StringT()
	{
		clear();
	}

	template<bx::AllocatorI** AllocatorT>
	inline StringT<AllocatorT>& StringT<AllocatorT>::operator=(const StringT<AllocatorT>& _rhs)
	{
		set(_rhs);
		return *this;
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
			const int32_t old = m_len;
			const int32_t len = m_len + _str.getLength();

			char* ptr = const_cast<char*>(m_ptr);

			if (len+1 > m_capacity)
			{
				const int32_t capacity = alignUp(len+1, 256);
				ptr = (char*)bx::realloc(*AllocatorT, 0 != m_capacity ? ptr : NULL, capacity);

				*const_cast<char**>(&m_ptr) = ptr;
				m_capacity = capacity;
			}

			m_len = len;
			strCopy(ptr + old, len-old+1, _str);
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
		m_0terminated = true;

		if (0 != m_capacity)
		{
			bx::free(*AllocatorT, const_cast<char*>(m_ptr) );

			StringView::clear();
			m_capacity = 0;
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

			return strRTrim(strRTrim(line, "\n"), "\r");
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

	inline int32_t strLen(const StringView& _str, int32_t _max)
	{
		return min(_str.getLength(), _max);
	}

	inline bool hasPrefix(const StringView& _str, const StringView& _prefix)
	{
		const int32_t len = _prefix.getLength();
		return _str.getLength() >= len
			&& 0 == strCmp(_str, _prefix, len)
			;
	}

	inline bool hasSuffix(const StringView& _str, const StringView& _suffix)
	{
		const int32_t len = _suffix.getLength();
		return _str.getLength() >= len
			&& 0 == strCmp(StringView(_str.getTerm() - len, _str.getTerm() ), _suffix, len)
			;
	}

	inline StringView strTrimPrefix(const StringView& _str, const StringView& _prefix)
	{
		if (hasPrefix(_str, _prefix) )
		{
			return StringView(_str.getPtr() + _prefix.getLength(), _str.getTerm() );
		}

		return _str;
	}

	inline StringView strTrimSuffix(const StringView& _str, const StringView& _suffix)
	{
		if (hasSuffix(_str, _suffix) )
		{
			return StringView(_str.getPtr(), _str.getTerm() - _suffix.getLength() );
		}

		return _str;
	}

} // namespace bx
