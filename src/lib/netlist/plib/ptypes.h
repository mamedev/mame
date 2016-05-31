// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * ptypes.h
 *
 */

#ifndef PTYPES_H_
#define PTYPES_H_

#include "pconfig.h"
#include "pstring.h"

namespace plib {

	//============================================================
	//  penum - strongly typed enumeration
	//============================================================

	struct enum_base
	{
	protected:
		static int from_string_int(const char *str, const char *x);
		static pstring nthstr(int n, const char *str);
	};

}

#define P_ENUM(ename, ...) \
	struct ename : public plib::enum_base { \
		enum e { __VA_ARGS__ }; \
		ename (e v) : m_v(v) { } \
		bool set_from_string (const pstring &s) { \
			static const char *strings = # __VA_ARGS__; \
			int f = from_string_int(strings, s.cstr()); \
			if (f>=0) { m_v = (e) f; return true; } else { return false; } \
		} \
		operator e() const {return m_v;} \
		int as_int() const {return (int) m_v;} \
		bool operator==(const ename &rhs) const {return m_v == rhs.m_v;} \
		bool operator==(const e &rhs) const {return m_v == (int) rhs;} \
		const pstring name() const { \
			static const char *strings = # __VA_ARGS__; \
			return nthstr((int) m_v, strings); \
		} \
		private: e m_v; };


#endif /* PTYPES_H_ */
