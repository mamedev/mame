// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PENUM_H_
#define PENUM_H_

///
/// \file penum.h
///

#include "pstring.h"

namespace plib
{

	///
	/// \brief strongly typed enumeration
	///
	///
	struct penum_base
	{
	protected:
		// Implementation in putil.cpp.
		// Putting the code here leads to a performance decrease.
		static int from_string_int(const pstring &str, const pstring &x);
		static pstring nthstr(std::size_t n, const pstring &str);
	};

} // namespace plib

#define PENUM(ename, ...) \
	struct ename : public plib::penum_base { \
		enum E { __VA_ARGS__ }; \
		constexpr ename (const E &v) : m_v(v) { } \
		template <typename T> explicit constexpr ename(const T &val) : m_v(static_cast<E>(val)) { } \
		bool set_from_string (const pstring &s) { \
			int f = from_string_int(strings(), s); \
			if (f>=0) { m_v = static_cast<E>(f); return true; } \
			return false;\
		} \
		constexpr operator E() const noexcept {return m_v;} \
		constexpr bool operator==(const ename &rhs) const noexcept {return m_v == rhs.m_v;} \
		constexpr bool operator==(const E &rhs) const noexcept {return m_v == rhs;} \
		pstring name() const { \
			return nthstr(m_v, strings()); \
		} \
		template <typename S> void save_state(S &saver) { saver.save_item(m_v, "m_v"); } \
		private: E m_v; \
		static pstring strings() {\
			static const char * static_strings = # __VA_ARGS__; \
			return pstring(static_strings); \
		} \
	};

#endif // PENUM_H_
