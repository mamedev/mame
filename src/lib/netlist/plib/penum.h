// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PENUM_H_
#define PENUM_H_

///
/// \file penum.h
///

#include "pstring.h"
#include "pstrutil.h"
#include "putil.h"

namespace plib
{

	//============================================================
	//  penum - strongly typed enumeration
	//============================================================
	struct penum_base
	{
	protected:
		// Implementation in putil.cpp.
		// Putting the code here leads to a performance decrease.
		static int from_string_int(const pstring &str, const pstring &x);
		static pstring nthstr(int n, const pstring &str);
	};

} // namespace plib

#define PENUM(ename, ...) \
	struct ename : public plib::penum_base { \
		enum E { __VA_ARGS__ }; \
		constexpr ename (const E &v) : m_v(v) { } \
		template <typename T> explicit constexpr ename(const T &val) { m_v = static_cast<E>(val); } \
		template <typename T> explicit constexpr ename(T && val) { m_v = static_cast<E>(val); } \
		bool set_from_string (const pstring &s) { \
			int f = from_string_int(strings(), s); \
			if (f>=0) { m_v = static_cast<E>(f); return true; } \
			return false;\
		} \
		constexpr operator E() const noexcept {return m_v;} \
		constexpr bool operator==(const ename &rhs) const noexcept {return m_v == rhs.m_v;} \
		constexpr bool operator==(const E &rhs) const noexcept {return m_v == rhs;} \
		pstring name() const { \
			return nthstr(static_cast<int>(m_v), strings()); \
		} \
		private: E m_v; \
		static pstring strings() {\
			static const char * lstrings = # __VA_ARGS__; \
			return pstring(lstrings); \
		} \
	};

#endif // PENUM_H_
