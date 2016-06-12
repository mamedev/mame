// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nltime.h
 */

#ifndef NLTIME_H_
#define NLTIME_H_

#include "nl_config.h"
#include "plib/pstate.h"

//============================================================
//  MACROS
//============================================================

#define NLTIME_FROM_NS(t)  netlist_time::from_nsec(t)
#define NLTIME_FROM_US(t)  netlist_time::from_usec(t)
#define NLTIME_FROM_MS(t)  netlist_time::from_msec(t)
#define NLTIME_IMMEDIATE   netlist_time::from_nsec(1)

// ----------------------------------------------------------------------------------------
// netlist_time
// ----------------------------------------------------------------------------------------

namespace netlist
{
	struct netlist_time final
	{
	public:

#if (PHAS_INT128)
		using INTERNALTYPE = UINT128;
#else
		using INTERNALTYPE = UINT64;
#endif
		static constexpr INTERNALTYPE RESOLUTION = NETLIST_INTERNAL_RES;

		constexpr netlist_time() NOEXCEPT : m_time(0) {}
		constexpr netlist_time(const netlist_time &rhs) NOEXCEPT = default;
		constexpr netlist_time(netlist_time &&rhs) NOEXCEPT = default;

		constexpr explicit netlist_time(const double t)
		: m_time((INTERNALTYPE) ( t * (double) RESOLUTION)) { }
		constexpr explicit netlist_time(const INTERNALTYPE nom, const INTERNALTYPE den)
		: m_time(nom * (RESOLUTION / den)) { }

		netlist_time &operator=(const netlist_time rhs) { m_time = rhs.m_time; return *this; }

		netlist_time &operator+=(const netlist_time &rhs) { m_time += rhs.m_time; return *this; }
		netlist_time &operator-=(const netlist_time &rhs) { m_time -= rhs.m_time; return *this; }

		friend netlist_time operator-(netlist_time lhs, const netlist_time &rhs)
		{
			lhs -= rhs;
			return lhs;
		}

		friend netlist_time operator+(netlist_time lhs, const netlist_time &rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend netlist_time operator*(netlist_time lhs, const UINT64 factor)
		{
			lhs.m_time *= static_cast<INTERNALTYPE>(factor);
			return lhs;
		}

		friend UINT64 operator/(const netlist_time &lhs, const netlist_time &rhs)
		{
			return static_cast<UINT64>(lhs.m_time / rhs.m_time);
		}

		friend bool operator<(const netlist_time &lhs, const netlist_time &rhs)
		{
			return (lhs.m_time < rhs.m_time);
		}

		friend bool operator>(const netlist_time &lhs, const netlist_time &rhs)
		{
			return rhs < lhs;
		}

		friend bool operator<=(const netlist_time &lhs, const netlist_time &rhs)
		{
			return !(lhs > rhs);
		}

		friend bool operator>=(const netlist_time &lhs, const netlist_time &rhs)
		{
			return !(lhs < rhs);
		}

		friend bool operator==(const netlist_time &lhs, const netlist_time &rhs)
		{
			return lhs.m_time == rhs.m_time;
		}

		friend bool operator!=(const netlist_time &lhs, const netlist_time &rhs)
		{
			return lhs.m_time != rhs.m_time;
		}

		constexpr INTERNALTYPE as_raw() const { return m_time; }
		constexpr double as_double() const { return (double) m_time / (double) RESOLUTION; }

		// for save states ....
		INTERNALTYPE *get_internaltype_ptr() { return &m_time; }

		static inline constexpr netlist_time from_nsec(const INTERNALTYPE ns) { return netlist_time(ns, U64(1000000000)); }
		static inline constexpr netlist_time from_usec(const INTERNALTYPE us) { return netlist_time(us, U64(1000000)); }
		static inline constexpr netlist_time from_msec(const INTERNALTYPE ms) { return netlist_time(ms, U64(1000)); }
		static inline constexpr netlist_time from_hz(const INTERNALTYPE hz) { return netlist_time(1 , hz); }
		static inline constexpr netlist_time from_raw(const INTERNALTYPE raw) { return netlist_time(raw, RESOLUTION); }

		static inline constexpr netlist_time zero() { return netlist_time(0, RESOLUTION); }
		static inline constexpr netlist_time quantum() { return netlist_time(1, RESOLUTION); }
		static inline constexpr netlist_time never() { return netlist_time(std::numeric_limits<netlist_time::INTERNALTYPE>::max(), RESOLUTION); }
	private:
		INTERNALTYPE m_time;
	};

}

namespace plib {
template<> inline void state_manager_t::save_item(const void *owner, netlist::netlist_time &nlt, const pstring &stname)
{
	save_state_ptr(owner, stname, datatype_t(sizeof(netlist::netlist_time::INTERNALTYPE), false, true, false), 1, nlt.get_internaltype_ptr());
}
}

#endif /* NLTIME_H_ */
