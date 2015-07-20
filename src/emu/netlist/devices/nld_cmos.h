// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_cmos.h
 *
 */

#ifndef NLD_CMOS_H_
#define NLD_CMOS_H_

#include "../nl_base.h"
#include "../analog/nld_twoterm.h"

NETLIB_NAMESPACE_DEVICES_START()

class nld_vdd_vss : public device_t
{
	public:
		nld_vdd_vss ()
		: device_t()
			{ }

		analog_input_t m_vdd;
		analog_input_t m_vss;

	protected:
		ATTR_HOT void update() {};
		ATTR_HOT void start()
		{
			register_input("VDD", m_vdd);
			register_input("VSS", m_vss);
		};
		ATTR_HOT void reset()  {};

public:
	ATTR_HOT inline nl_double vdd() { return INPANALOG(m_vdd); }
	ATTR_HOT inline nl_double vss() { return INPANALOG(m_vss); }
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_CMOS_H_ */
