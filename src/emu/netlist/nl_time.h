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

#define RESOLUTION NETLIST_INTERNAL_RES

namespace netlist
{
	struct netlist_time
	{
	public:

		typedef UINT64 INTERNALTYPE;

		ATTR_HOT /* inline */ netlist_time() : m_time(0) {}

		ATTR_HOT friend /* inline */ const netlist_time operator-(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ const netlist_time operator+(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ const netlist_time operator*(const netlist_time &left, const UINT32 factor);
		ATTR_HOT friend /* inline */ UINT32 operator/(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ bool operator>(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ bool operator<(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ bool operator>=(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ bool operator<=(const netlist_time &left, const netlist_time &right);
		ATTR_HOT friend /* inline */ bool operator!=(const netlist_time &left, const netlist_time &right);

		ATTR_HOT /* inline */ const netlist_time &operator=(const netlist_time &right) { m_time = right.m_time; return *this; }
		ATTR_HOT /* inline */ const netlist_time &operator=(const double &right) { m_time = (INTERNALTYPE) ( right * (double) RESOLUTION); return *this; }

		// issues with ISO C++ standard
		//ATTR_HOT /* inline */ operator double() const { return as_double(); }

		ATTR_HOT /* inline */ const netlist_time &operator+=(const netlist_time &right) { m_time += right.m_time; return *this; }

		ATTR_HOT /* inline */ INTERNALTYPE as_raw() const { return m_time; }
		ATTR_HOT /* inline */ double as_double() const { return (double) m_time / (double) RESOLUTION; }

		// for save states ....
		ATTR_HOT /* inline */ INTERNALTYPE *get_internaltype_ptr() { return &m_time; }

		ATTR_HOT static /* inline */ const netlist_time from_nsec(const int ns) { return netlist_time((UINT64) ns * (RESOLUTION / U64(1000000000))); }
		ATTR_HOT static /* inline */ const netlist_time from_usec(const int us) { return netlist_time((UINT64) us * (RESOLUTION / U64(1000000))); }
		ATTR_HOT static /* inline */ const netlist_time from_msec(const int ms) { return netlist_time((UINT64) ms * (RESOLUTION / U64(1000))); }
		ATTR_HOT static /* inline */ const netlist_time from_hz(const UINT64 hz) { return netlist_time(RESOLUTION / hz); }
		ATTR_HOT static /* inline */ const netlist_time from_double(const double t) { return netlist_time((INTERNALTYPE) ( t * (double) RESOLUTION)); }
		ATTR_HOT static /* inline */ const netlist_time from_raw(const INTERNALTYPE raw) { return netlist_time(raw); }

		static const netlist_time zero;

	protected:

		ATTR_HOT /* inline */ netlist_time(const INTERNALTYPE val) : m_time(val) {}

	private:
		INTERNALTYPE m_time;
	};

	#undef RESOLUTION

	ATTR_HOT inline const netlist_time operator-(const netlist_time &left, const netlist_time &right)
	{
		return netlist_time::from_raw(left.m_time - right.m_time);
	}

	ATTR_HOT inline const netlist_time operator*(const netlist_time &left, const UINT32 factor)
	{
		return netlist_time::from_raw(left.m_time * factor);
	}

	ATTR_HOT inline UINT32 operator/(const netlist_time &left, const netlist_time &right)
	{
		return left.m_time / right.m_time;
	}

	ATTR_HOT inline const netlist_time operator+(const netlist_time &left, const netlist_time &right)
	{
		return netlist_time::from_raw(left.m_time + right.m_time);
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
	save_state_ptr(stname, DT_INT64, owner, sizeof(netlist::netlist_time::INTERNALTYPE), 1, nlt.get_internaltype_ptr(), false);
}


#endif /* NLTIME_H_ */
