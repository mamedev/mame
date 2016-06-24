// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_cmos.h
 *
 */

#ifndef NLID_CMOS_H_
#define NLID_CMOS_H_

#include "nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(vdd_vss)
	{
		NETLIB_CONSTRUCTOR(vdd_vss)
		, m_vdd(*this, "VDD")
		, m_vss(*this, "VSS")
		{
		}

		NETLIB_UPDATEI() {};
		NETLIB_RESETI() {};

	public:
		inline nl_double vdd() { return INPANALOG(m_vdd); }
		inline nl_double vss() { return INPANALOG(m_vss); }

		analog_input_t m_vdd;
		analog_input_t m_vss;
	};

	} //namespace devices
} // namespace netlist

#endif /* NLID_CMOS_H_ */
