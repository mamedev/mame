// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_util.h
 *
 */

#ifndef NL_UTIL_H_
#define NL_UTIL_H_

#include <cmath>
#include <cstring>
#include <cstdlib>

#include "plib/pstring.h"
#include "plib/plists.h"

class nl_util
{
// this is purely static
private:
	nl_util() {};

public:


	static const pstring environment(const pstring &var, const pstring &default_val = "")
	{
		if (getenv(var.cstr()) == NULL)
			return default_val;
		else
			return pstring(getenv(var.cstr()));
	}
};

class nl_math
{
// this is purely static
private:
	nl_math() {};

public:
	ATTR_HOT inline static float exp(const float x) { return std::exp(x); }
	ATTR_HOT inline static double abs(const double x) { return std::fabs(x); }
	ATTR_HOT inline static float abs(const float x) { return std::fabs(x); }
	ATTR_HOT inline static double log(const double x) { return std::log(x); }
	ATTR_HOT inline static float log(const float x) { return std::log(x); }
#if defined(_MSC_VER) && _MSC_VER < 1800
	ATTR_HOT inline static double e_log1p(const double x) { return nl_math::log(1.0 + x); }
	ATTR_HOT inline static float e_log1p(const float x) { return nl_math::log(1.0 + x); }
#else
	ATTR_HOT inline static double e_log1p(const double x) { return log1p(x); }
	ATTR_HOT inline static float e_log1p(const float x) { return log1pf(x); }
#endif
	ATTR_HOT inline static double sqrt(const double x) { return std::sqrt(x); }
	ATTR_HOT inline static float sqrt(const float x) { return std::sqrt(x); }

	// this one has an accuracy of better than 5%. That's enough for our purpose
	// add c3 and it'll be better than 1%

#if 0
	inline static double fastexp_h(const double x)
	{
		/* static */ const double ln2r = 1.442695040888963387;
		/* static */ const double ln2  = 0.693147180559945286;
		/* static */ const double c3   = 0.166666666666666667;
		/* static */ const double c4   = 1.0 / 24.0;
		/* static */ const double c5   = 1.0 / 120.0;

		const double y = x * ln2r;
		const UINT32 t = y;
		const double z = (x - ln2 * (double) t);
		const double e = (1.0 + z * (1.0 + z * (0.5 + z * (c3  + z * (c4 + c5*z)))));

		if (t < 63)
			//return (double)((UINT64) 1 <<  t)*(1.0 + z + 0.5 * zz + c3 * zzz+c4*zzzz+c5*zzzzz);
			return (double)((UINT64) 1 <<  t) * e;
		else
			return pow(2.0, t)*e;
	}

	ATTR_HOT inline static double exp(const double x)
	{
		if (x<0)
			return 1.0 / fastexp_h(-x);
		else
			return fastexp_h(x);
	}
#else
	ATTR_HOT inline static double exp(const double x)  { return std::exp(x); }
#endif

};

#endif /* NL_UTIL_H_ */
