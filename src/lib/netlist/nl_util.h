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
#include <algorithm>
#include <initializer_list>

#include "plib/pstring.h"
#include "plib/plists.h"

class nl_util
{
// this is purely static
private:
	nl_util() {};

public:

	static const pstring buildpath(std::initializer_list<pstring> list )
	{
		pstring ret = "";
		for( auto elem : list )
		{
			if (ret == "")
				ret = elem;
			else
#ifdef _WIN32
				ret = ret + '\\' + elem;
#else
				ret = ret + '/' + elem;
#endif
		}
		return ret;
	}

	static const pstring environment(const pstring &var, const pstring &default_val = "")
	{
		if (getenv(var.cstr()) == nullptr)
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
	template <typename T>
	static T abs(const T &x) { return std::abs(x); }

	template <typename T>
	static T max(const T &x, const T &y) { return std::max(x, y); }

	template <typename T>
	static T min(const T &x, const T &y) { return std::min(x, y); }

	template <typename T>
	static T log(const T &x) { return std::log(x); }

	#if defined(_MSC_VER) && _MSC_VER < 1800
	ATTR_HOT inline static double e_log1p(const double &x) { return nl_math::log(1.0 + x); }
	ATTR_HOT inline static float e_log1p(const float &x) { return nl_math::log(1.0 + x); }
#else
	template <typename T>
	static T e_log1p(const T &x) { return log1p(x); }
#endif

	template <typename T>
	static T sqrt(const T &x) { return std::sqrt(x); }

	template <typename T>
	static T hypot(const T &x, const T &y) { return std::hypot(x, y); }

	// this one has an accuracy of better than 5%. That's enough for our purpose
	// add c3 and it'll be better than 1%

#if 0
	ATTR_HOT inline static float exp(const float &x) { return std::exp(x); }
	inline static double fastexp_h(const double &x)
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

	ATTR_HOT inline static double exp(const double &x)
	{
		if (x<0)
			return 1.0 / fastexp_h(-x);
		else
			return fastexp_h(x);
	}
#else
	template <typename T>
	static double exp(const T &x)  { return std::exp(x); }
#endif

};

template <typename _SC>
class powned_ptr
{
private:
	powned_ptr()
	: m_ptr(nullptr), m_is_owned(true) { }
public:
	powned_ptr(_SC *p, bool owned)
	: m_ptr(p), m_is_owned(owned)
	{ }
	powned_ptr(const powned_ptr &r) = delete;
	powned_ptr & operator =(const powned_ptr &r) = delete;

	powned_ptr(powned_ptr &&r)
	{
		m_is_owned = r.m_is_owned;
		m_ptr = r.m_ptr;
		r.m_is_owned = false;
		r.m_ptr = nullptr;
	}

	template<typename _DC>
	powned_ptr(powned_ptr<_DC> &&r)
	{
		_SC *dest_ptr = &dynamic_cast<_SC &>(*r.get());
		bool o = r.is_owned();
		r.release();
		m_is_owned = o;
		m_ptr = dest_ptr;
	}

	~powned_ptr()
	{
		if (m_is_owned)
			delete m_ptr;
	}
	template<typename _DC, typename... _Args>
	static powned_ptr Create(_Args&&... __args)
	{
		powned_ptr a;
		_DC *x = new _DC(std::forward<_Args>(__args)...);
		a.m_ptr = static_cast<_SC *>(x);
		return a;
	}

	template<typename... _Args>
	static powned_ptr Create(_Args&&... __args)
	{
		powned_ptr a;
		a.m_ptr = new _SC(std::forward<_Args>(__args)...);
		return a;
	}
	void release()
	{
		m_is_owned = false;
		m_ptr = nullptr;
	}

	bool is_owned() const { return m_is_owned; }

	powned_ptr & operator =(powned_ptr &r)
	{
		m_is_owned = r.m_is_owned;
		m_ptr = r.m_ptr;
		r.m_is_owned = false;
		r.m_ptr = nullptr;
		return *this;
	}
	_SC * operator ->() { return m_ptr; }
	_SC & operator *() { return *m_ptr; }
	_SC * get() const { return m_ptr; }
private:
	_SC *m_ptr;
	bool m_is_owned;
};


#endif /* NL_UTIL_H_ */
