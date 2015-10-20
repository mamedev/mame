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

#define NLTIME_FROM_NS(_t)  netlist_time::from_nsec(_t)
#define NLTIME_FROM_US(_t)  netlist_time::from_usec(_t)
#define NLTIME_FROM_MS(_t)  netlist_time::from_msec(_t)
#define NLTIME_IMMEDIATE    netlist_time::from_nsec(1)

// ----------------------------------------------------------------------------------------
// net_list_time
// ----------------------------------------------------------------------------------------

#undef ATTR_HOT
#define ATTR_HOT

namespace netlist
{
	struct netlist_time
	{
	public:

#if (PHAS_INT128)
		typedef UINT128 INTERNALTYPE;
		static const pstate_data_type_e STATETYPE = DT_INT128;
#else
		typedef UINT64 INTERNALTYPE;
		static const pstate_data_type_e STATETYPE = DT_INT64;
#endif
		static const INTERNALTYPE RESOLUTION = NETLIST_INTERNAL_RES;

		ATTR_HOT netlist_time() : m_time(0) {}
		ATTR_HOT netlist_time(const netlist_time &rhs) : m_time(rhs.m_time) {}

		ATTR_HOT friend const netlist_time operator-(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend const netlist_time operator+(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend const netlist_time operator*(const netlist_time &left, const UINT64 factor);
		ATTR_HOT friend UINT64 operator/(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend bool operator>(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend bool operator<(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend bool operator>=(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend bool operator<=(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend bool operator!=(const netlist_time &left, const netlist_time &right);

		ATTR_HOT const netlist_time &operator=(const netlist_time &right) { m_time = right.m_time; return *this; }

		ATTR_HOT const netlist_time &operator+=(const netlist_time &right) { m_time += right.m_time; return *this; }

		ATTR_HOT INTERNALTYPE as_raw() const { return m_time; }
		ATTR_HOT double as_double() const { return (double) m_time / (double) RESOLUTION; }

		// for save states ....
		ATTR_HOT INTERNALTYPE *get_internaltype_ptr() { return &m_time; }

		ATTR_HOT static const netlist_time from_nsec(const INTERNALTYPE ns) { return netlist_time(ns * (RESOLUTION / U64(1000000000))); }
		ATTR_HOT static const netlist_time from_usec(const INTERNALTYPE us) { return netlist_time(us * (RESOLUTION / U64(1000000))); }
		ATTR_HOT static const netlist_time from_msec(const INTERNALTYPE ms) { return netlist_time(ms * (RESOLUTION / U64(1000))); }
		ATTR_HOT static const netlist_time from_hz(const INTERNALTYPE hz) { return netlist_time(RESOLUTION / hz); }
		ATTR_HOT static const netlist_time from_double(const double t) { return netlist_time((INTERNALTYPE) ( t * (double) RESOLUTION)); }
		ATTR_HOT static const netlist_time from_raw(const INTERNALTYPE raw) { return netlist_time(raw); }

		static const netlist_time zero;

	protected:

		ATTR_HOT netlist_time(const INTERNALTYPE val) : m_time(val) {}

	private:
		INTERNALTYPE m_time;
	};

	ATTR_HOT inline const netlist_time operator-(const netlist_time &left, const netlist_time &right)
	{
		return netlist_time(left.m_time - right.m_time);
	}

	ATTR_HOT inline const netlist_time operator*(const netlist_time &left, const UINT64 factor)
	{
		return netlist_time(left.m_time * factor);
	}

	ATTR_HOT inline UINT64 operator/(const netlist_time &left, const netlist_time &right)
	{
		return left.m_time / right.m_time;
	}

	ATTR_HOT inline const netlist_time operator+(const netlist_time &left, const netlist_time &right)
	{
		return netlist_time(left.m_time + right.m_time);
	}

	ATTR_HOT inline bool operator<(const netlist_time &left, const netlist_time &right)
	{
		return (left.m_time < right.m_time);
	}

	ATTR_HOT inline bool operator>(const netlist_time &left, const netlist_time &right)
	{
		return (left.m_time > right.m_time);
	}

	ATTR_HOT inline bool operator<=(const netlist_time &left, const netlist_time &right)
	{
		return (left.m_time <= right.m_time);
	}

	ATTR_HOT inline bool operator>=(const netlist_time &left, const netlist_time &right)
	{
		return (left.m_time >= right.m_time);
	}

	ATTR_HOT inline bool operator!=(const netlist_time &left, const netlist_time &right)
	{
		return (left.m_time != right.m_time);
	}

}

template<> ATTR_COLD inline void pstate_manager_t::save_item(netlist::netlist_time &nlt, const void *owner, const pstring &stname)
{
	save_state_ptr(stname, netlist::netlist_time::STATETYPE, owner, sizeof(netlist::netlist_time::INTERNALTYPE), 1, nlt.get_internaltype_ptr(), false);
}


#endif /* NLTIME_H_ */
