// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PSTRUTIL_H_
#define PSTRUTIL_H_

///
/// \file pstrutil.h
///

#include "pstring.h"
#include "ptypes.h"

#include <exception>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace plib
{
	template<class T>
	struct string_info
	{
		using mem_t = typename T::mem_t;
	};

	template<>
	struct string_info<std::string>
	{
		using mem_t = char;
	};

	template<typename T>
	pstring to_string(const T &v)
	{
		return pstring(std::to_string(v));
	}

	template<typename T>
	pwstring to_wstring(const T &v)
	{
		return pwstring(std::to_wstring(v));
	}

	template<typename T>
	typename T::size_type find_first_not_of(const T &str, const T &no)
	{
		typename T::size_type pos = 0;
		for (auto it = str.begin(); it != str.end(); ++it, ++pos)
		{
			bool f = true;
			for (typename T::value_type const jt : no)
			{
				if (*it == jt)
				{
					f = false;
					break;
				}
			}
			if (f)
				return pos;
		}
		return T::npos;
	}

	template<typename T>
	typename T::size_type find_last_not_of(const T &str, const T &no)
	{
		// FIXME: use reverse iterator
		typename T::size_type last_found = T::npos;
		typename T::size_type pos = 0;
		for (auto it = str.begin(); it != str.end(); ++it, ++pos)
		{
			bool f = true;
			for (typename T::value_type const jt : no)
			{
				if (*it == jt)
				{
					f = false;
					break;
				}
			}
			if (f)
				last_found = pos;
		}
		return last_found;
	}

	template<typename T>
	typename T::size_type find_last_of(const T &str, const T &no)
	{
		typename T::size_type last_found = T::npos;
		typename T::size_type pos = 0;
		for (auto it = str.begin(); it != str.end(); ++it, ++pos)
		{
			bool f = false;
			for (typename T::value_type const jt : no)
			{
				if (*it == jt)
				{
					f = true;
					break;
				}
			}
			if (f)
				last_found = pos;
		}
		return last_found;
	}

	template<typename T>
	T ltrim(const T &str, const T &ws = T(" \t\n\r"))
	{
		auto f = find_first_not_of(str, ws);
		return (f == T::npos) ? T() : str.substr(f);
	}

	template<typename T>
	T rtrim(const T &str, const T &ws = T(" \t\n\r"))
	{
		auto f = find_last_not_of(str, ws);
		return (f == T::npos) ? T() : str.substr(0, f + 1);
	}

	template<typename T>
	T trim(const T &str, const T &ws = T(" \t\n\r"))
	{
		return rtrim(ltrim(str, ws), ws);
	}

	template<typename T>
	T left(const T &str, typename T::size_type len)
	{
		return str.substr(0, len);
	}

	template<typename T>
	T right(const T &str, typename T::size_type nlen)
	{
		return nlen >= str.length() ? str : str.substr(str.length() - nlen, nlen);
	}

	template<typename T>
	bool startsWith(const T &str, const T &arg)
	{
		return (arg == left(str, arg.length()));
	}

	template<typename T>
	bool endsWith(const T &str, const T &arg)
	{
		return (right(str, arg.length()) == arg);
	}

	template<typename T, typename TA>
	bool startsWith(const T &str, const TA &arg)
	{
		return startsWith(str, static_cast<pstring>(arg));
	}

	template<typename T, typename TA>
	bool endsWith(const T &str, const TA &arg)
	{
		return endsWith(str, static_cast<pstring>(arg));
	}

	template<typename T>
	std::size_t strlen(const T *str)
	{
		const T *p = str;
		while (*p)
			p++;
		return static_cast<std::size_t>(p - str);
	}

	template<typename T>
	T ucase(const T &str)
	{
		T ret;
		for (const auto &c : str)
			if (c >= 'a' && c <= 'z')
				ret += (c - 'a' + 'A');
			else
				ret += c;
		return ret;
	}

	template<typename T>
	T rpad(const T &str, const T &ws, const typename T::size_type cnt)
	{
		// FIXME: pstringbuffer ret(*this);

		T ret(str);
		typename T::size_type wsl = ws.length();
		for (auto i = ret.length(); i < cnt; i+=wsl)
			ret += ws;
		return ret;
	}

	template<typename T>
	T replace_all(const T &str, const T &search, const T &replace)
	{
		T ret;
		const typename T::size_type slen = search.length();

		typename T::size_type last_s = 0;
		typename T::size_type s = str.find(search, last_s);
		while (s != T::npos)
		{
			ret += str.substr(last_s, s - last_s);
			ret += replace;
			last_s = s + slen;
			s = str.find(search, last_s);
		}
		ret += str.substr(last_s);
		return ret;
	}

	template<typename T, typename T1, typename T2>
	T replace_all(const T &str, const T1 &search, const T2 &replace)
	{
		return replace_all(str, static_cast<T>(search), static_cast<T>(replace));
	}

} // namespace plib

#endif // PSTRUTIL_H_
