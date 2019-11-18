// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_cmos.h
 *
 */

#ifndef NLID_CMOS_H_
#define NLID_CMOS_H_

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"

namespace netlist
{
namespace devices
{
	// FIXME: this needs to be removed
	NETLIB_OBJECT(vdd_vss)
	{
		NETLIB_CONSTRUCTOR(vdd_vss)
		, m_vdd(*this, "VDD")
		, m_vss(*this, "VSS")
		{
		}

		NETLIB_UPDATEI() {}
		NETLIB_RESETI() {}

	public:
		nl_fptype vdd() { return m_vdd(); }
		nl_fptype vss() { return m_vss(); }

		analog_input_t m_vdd;
		analog_input_t m_vss;
	};

} //namespace devices
} // namespace netlist

#endif /* NLID_CMOS_H_ */
