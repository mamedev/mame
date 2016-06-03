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
	struct netlist_time
	{
	public:

#if (PHAS_INT128)
		using INTERNALTYPE = UINT128;
		static const pstate_data_type_e STATETYPE = DT_INT128;
#else
		using INTERNALTYPE = UINT64;
		static const pstate_data_type_e STATETYPE = pstate_data_type_e::DT_INT64;
#endif
		static const INTERNALTYPE RESOLUTION = NETLIST_INTERNAL_RES;

		netlist_time() : m_time(0) {}
		explicit netlist_time(const double t)
		: m_time((INTERNALTYPE) ( t * (double) RESOLUTION)) { }
		explicit netlist_time(const INTERNALTYPE nom, const INTERNALTYPE den)
		: m_time(nom * (RESOLUTION / den)) { }

		netlist_time(const netlist_time &rhs) NOEXCEPT = default;
		netlist_time(netlist_time &&rhs) NOEXCEPT = default;

		netlist_time &operator+=(const netlist_time &right) { m_time += right.m_time; return *this; }
		netlist_time &operator-=(const netlist_time &right) { m_time -= right.m_time; return *this; }

		friend netlist_time operator-(netlist_time left, const netlist_time &right)
		{
			left -= right;
			return left;
		}

		friend netlist_time operator+(netlist_time left, const netlist_time &right)
		{
			left += right;
			return left;
		}

		friend netlist_time operator*(netlist_time left, const UINT64 factor)
		{
			left.m_time *= factor;
			return left;
		}

		friend UINT64 operator/(const netlist_time &left, const netlist_time &right)
		{
			return left.m_time / right.m_time;
		}

		friend bool operator<(const netlist_time &left, const netlist_time &right)
		{
			return (left.m_time < right.m_time);
		}

		friend bool operator>(const netlist_time &left, const netlist_time &right)
		{
			return right < left;
		}

		friend bool operator<=(const netlist_time &left, const netlist_time &right)
		{
			return !(left > right);
		}

		friend bool operator>=(const netlist_time &left, const netlist_time &right)
		{
			return !(left < right);
		}

		friend bool operator==(const netlist_time &left, const netlist_time &right)
		{
			return (left.m_time == right.m_time);
		}

		friend bool operator!=(const netlist_time &left, const netlist_time &right)
		{
			return !(left == right);
		}

		netlist_time &operator=(const netlist_time &right) { m_time = right.m_time; return *this; }

		INTERNALTYPE as_raw() const { return m_time; }
		double as_double() const { return (double) m_time / (double) RESOLUTION; }

		// for save states ....
		INTERNALTYPE *get_internaltype_ptr() { return &m_time; }

		static netlist_time from_nsec(const INTERNALTYPE ns) { return netlist_time(ns, U64(1000000000)); }
		static netlist_time from_usec(const INTERNALTYPE us) { return netlist_time(us, U64(1000000)); }
		static netlist_time from_msec(const INTERNALTYPE ms) { return netlist_time(ms, U64(1000)); }
		static netlist_time from_hz(const INTERNALTYPE hz) { return netlist_time(1 , hz); }
		static netlist_time from_raw(const INTERNALTYPE raw) { return netlist_time(raw, RESOLUTION); }

		static const netlist_time zero;

	protected:

	private:
		INTERNALTYPE m_time;
	};



}

namespace plib {
template<> inline void pstate_manager_t::save_item(const void *owner, netlist::netlist_time &nlt, const pstring &stname)
{
	save_state_ptr(owner, stname, netlist::netlist_time::STATETYPE, sizeof(netlist::netlist_time::INTERNALTYPE), 1, nlt.get_internaltype_ptr(), false);
}
}

#endif /* NLTIME_H_ */
