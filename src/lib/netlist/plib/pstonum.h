// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PSTONUM_H_
#define PSTONUM_H_

///
/// \file pstonum.h
///

#include "pconfig.h"
#include "pexception.h"
#include "pmath.h" // for pstonum
#include "pstring.h"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <locale>
#include <sstream>

namespace plib
{
	// ----------------------------------------------------------------------------------------
	// number conversions
	// ----------------------------------------------------------------------------------------

	template <typename T, typename S>
	T pstonum_locale(const std::locale &loc, const S &arg, std::size_t *idx)
	{
		std::stringstream ss;
		ss.imbue(loc);
		ss << arg;
		auto len(ss.tellp());
		T x(constants<T>::zero());
		if (ss >> x)
		{
			auto pos(ss.tellg());
			if (pos == static_cast<decltype(pos)>(-1))
				pos = len;
			*idx = static_cast<std::size_t>(pos);
		}
		else
			*idx = constants<std::size_t>::zero();
		//printf("%s, %f, %lu %ld\n", arg, (double)x, *idx, (long int) ss.tellg());
		return x;
	}

	template <typename T, typename E = void>
	struct pstonum_helper;

	template<typename T>
	struct pstonum_helper<T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type>
	{
		template <typename S>
		long long operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			//return std::stoll(arg, idx);
			return pstonum_locale<long long>(loc, arg, idx);
		}
	};

	template<typename T>
	struct pstonum_helper<T, typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value>::type>
	{
		template <typename S>
		unsigned long long operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			//return std::stoll(arg, idx);
			return pstonum_locale<unsigned long long>(loc, arg, idx);
		}
	};

	template<typename T>
	struct pstonum_helper<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
	{
		template <typename S>
		long double operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			return pstonum_locale<long double>(loc, arg, idx);
		}
	};

#if PUSE_FLOAT128
	template<>
	struct pstonum_helper<__float128>
	{
		// FIXME: use strtoflt128 from quadmath.h
		template <typename S>
		__float128 operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			return static_cast<__float128>(pstonum_locale<long double>(loc, arg, idx));
		}
	};
#endif

	template<typename T, typename S>
	T pstonum(const S &arg, const std::locale &loc = std::locale::classic())
	{
		decltype(arg.c_str()) cstr = arg.c_str();
		std::size_t idx(0);
		auto ret = pstonum_helper<T>()(loc, cstr, &idx);
		using ret_type = decltype(ret);
		if (ret >= static_cast<ret_type>(std::numeric_limits<T>::lowest())
			&& ret <= static_cast<ret_type>(std::numeric_limits<T>::max()))
			//&& (ret == T(0) || plib::abs(ret) >= std::numeric_limits<T>::min() ))
		{
			if (cstr[idx] != 0)
				pthrow<pexception>(pstring("Continuation after numeric value ends: ") + pstring(cstr));
		}
		else
		{
			pthrow<pexception>(pstring("Out of range: ") + pstring(cstr));
		}
		return static_cast<T>(ret);
	}

	template<typename R, typename T>
	R pstonum_ne(const T &str, bool &err, std::locale loc = std::locale::classic()) noexcept
	{
		try
		{
			err = false;
			return pstonum<R>(str, loc);
		}
		catch (...)
		{
			err = true;
			return R(0);
		}
	}

	template<typename R, typename T>
	R pstonum_ne_def(const T &str, R def, std::locale loc = std::locale::classic()) noexcept
	{
		try
		{
			return pstonum<R>(str, loc);
		}
		catch (...)
		{
			return def;
		}
	}

} // namespace plib

#endif // PUTIL_H_
