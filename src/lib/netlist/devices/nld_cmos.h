// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_cmos.h
 *
 */

#ifndef NLD_CMOS_H_
#define NLD_CMOS_H_

#include "nl_base.h"
#include "analog/nld_twoterm.h"

NETLIB_NAMESPACE_DEVICES_START()

class NETLIB_NAME(vdd_vss) : public device_t
{
	public:
		NETLIB_CONSTRUCTOR(vdd_vss) { }

		analog_input_t m_vdd;
		analog_input_t m_vss;

	protected:
		ATTR_HOT void update() override {};
		ATTR_HOT void start() override
		{
			enregister("VDD", m_vdd);
			enregister("VSS", m_vss);
		};
		ATTR_HOT void reset() override  {};

public:
	ATTR_HOT inline nl_double vdd() { return INPANALOG(m_vdd); }
	ATTR_HOT inline nl_double vss() { return INPANALOG(m_vss); }
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_CMOS_H_ */
