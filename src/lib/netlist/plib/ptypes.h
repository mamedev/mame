// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * ptypes.h
 *
 */

#ifndef PTYPES_H_
#define PTYPES_H_

#include <limits>
#include <string>
#include <type_traits>

#include "pconfig.h"

#define COPYASSIGNMOVE(name, def)  \
		name(const name &) = def; \
		name(name &&) noexcept = def; \
		name &operator=(const name &) = def; \
		name &operator=(name &&) noexcept = def;

#define COPYASSIGN(name, def)  \
		name(const name &) = def; \
		name &operator=(const name &) = def; \

namespace plib
{
	template<typename T> struct is_integral : public std::is_integral<T> { };
	template<typename T> struct numeric_limits : public std::numeric_limits<T> { };

	/* 128 bit support at least on GCC is not fully supported */
#if PHAS_INT128
	template<> struct is_integral<UINT128> { static constexpr bool value = true; };
	template<> struct is_integral<INT128> { static constexpr bool value = true; };
	template<> struct numeric_limits<UINT128>
	{
		static constexpr UINT128 max()
		{
			return ~((UINT128)0);
		}
	};
	template<> struct numeric_limits<INT128>
	{
		static constexpr INT128 max()
		{
			return (~((UINT128)0)) >> 1;
		}
	};
#endif

	//============================================================
	// prevent implicit copying
	//============================================================

	struct nocopyassignmove
	{
		nocopyassignmove(const nocopyassignmove &) = delete;
		nocopyassignmove(nocopyassignmove &&) = delete;
		nocopyassignmove &operator=(const nocopyassignmove &) = delete;
		nocopyassignmove &operator=(nocopyassignmove &&) = delete;
	protected:
		nocopyassignmove() = default;
		~nocopyassignmove() = default;
	};

	struct nocopyassign
	{
		nocopyassign(const nocopyassign &) = delete;
		nocopyassign &operator=(const nocopyassign &) = delete;
	protected:
		nocopyassign() = default;
		~nocopyassign() = default;
		nocopyassign(nocopyassign &&) = default;
		nocopyassign &operator=(nocopyassign &&) = default;
	};

	//============================================================
	// Avoid unused variable warnings
	//============================================================
	template<typename... Ts>
	inline void unused_var(Ts&&...) {}



	//============================================================
	//  penum - strongly typed enumeration
	//============================================================

	struct penum_base
	{
	protected:
		static int from_string_int(const char *str, const char *x);
		static std::string nthstr(int n, const char *str);
	};

} // namespace plib

#define P_ENUM(ename, ...) \
	struct ename : public plib::penum_base { \
		enum E { __VA_ARGS__ }; \
		ename (E v) : m_v(v) { } \
		bool set_from_string (const std::string &s) { \
			static char const *const strings = # __VA_ARGS__; \
			int f = from_string_int(strings, s.c_str()); \
			if (f>=0) { m_v = static_cast<E>(f); return true; } else { return false; } \
		} \
		operator E() const {return m_v;} \
		bool operator==(const ename &rhs) const {return m_v == rhs.m_v;} \
		bool operator==(const E &rhs) const {return m_v == rhs;} \
		std::string name() const { \
			static char const *const strings = # __VA_ARGS__; \
			return nthstr(static_cast<int>(m_v), strings); \
		} \
		private: E m_v; };


#endif /* PTYPES_H_ */
