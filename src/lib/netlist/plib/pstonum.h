// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PSTONUM_H_
#define PSTONUM_H_

///
/// \file pstonum.h
///

#include "pconfig.h"
#include "pexception.h"
#include "pgsl.h"
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
	T pstonum_locale(const std::locale &loc, const S &arg, bool *err)
	{
		std::stringstream ss;
		ss.imbue(loc);
		ss << putf8string(arg);
		auto len(ss.tellp());
		T x(constants<T>::zero());
		if (ss >> x)
		{
			auto pos(ss.tellg());
			if (pos == decltype(pos)(-1))
				pos = len;
			*err = (pos != len);
		}
		else
			*err = true;
		return x;
	}

	template <typename T, typename E = void>
	struct pstonum_helper;

	template<typename T>
	struct pstonum_helper<T, std::enable_if_t<plib::is_integral<T>::value && plib::is_signed<T>::value>>
	{
		template <typename S>
		long long operator()(std::locale loc, const S &arg, bool *err)
		{
			//return std::stoll(arg, idx);
			return pstonum_locale<long long>(loc, arg, err);
		}
	};

	template<typename T>
	struct pstonum_helper<T, std::enable_if_t<plib::is_integral<T>::value && !plib::is_signed<T>::value>>
	{
		template <typename S>
		unsigned long long operator()(std::locale loc, const S &arg, bool *err)
		{
			//return std::stoll(arg, idx);
			return pstonum_locale<unsigned long long>(loc, arg, err);
		}
	};

	template<typename T>
	struct pstonum_helper<T, std::enable_if_t<std::is_floating_point<T>::value>>
	{
		template <typename S>
		long double operator()(std::locale loc, const S &arg, bool *err)
		{
			return pstonum_locale<long double>(loc, arg, err);
		}
	};

#if PUSE_FLOAT128
	template<>
	struct pstonum_helper<FLOAT128>
	{
		// FIXME: use `strtoflt128` from `quadmath.h`
		template <typename S>
		FLOAT128 operator()(std::locale loc, const S &arg, bool *err)
		{
			return narrow_cast<FLOAT128>(pstonum_locale<long double>(loc, arg, err));
		}
	};
#endif

	template<typename T, typename S>
	T pstonum(const S &arg, const std::locale &loc = std::locale::classic()) noexcept(false)
	{
		bool err(false);
		auto ret = pstonum_helper<T>()(loc, arg, &err);
		if (err)
			throw pexception(pstring("Error converting string to number: ") + pstring(arg));

		using ret_type = decltype(ret);
		if (ret >= narrow_cast<ret_type>(plib::numeric_limits<T>::lowest())
			&& ret <= narrow_cast<ret_type>(plib::numeric_limits<T>::max()))
		{
			return narrow_cast<T>(ret);
		}

		throw pexception(pstring("Out of range: ") + pstring(arg));
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
