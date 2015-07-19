// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_MM5837.h
 *
 *  MM5837: Digital noise source
 *
 *          +--------+
 *      VDD |1  ++  8| NC
 *      VGG |2      7| NC
 *      OUT |3      6| NC
 *      VSS |4      5| NC
 *          +--------+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_MM5837_H_
#define NLD_MM5837_H_

#include "../nl_base.h"
#include "../analog/nld_twoterm.h"

#define MM5837_DIP(_name)                                                        \
		NET_REGISTER_DEV(MM5837_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_DEVICE(MM5837_dip,

	analog_input_t m_VDD;
	analog_input_t m_VGG;
	analog_input_t m_VSS;

	/* output stage */
	nld_twoterm m_RV;
	analog_output_t m_V0; /* could be gnd as well */

	/* clock stage */
	logic_input_t m_feedback;
	logic_output_t m_Q;
	netlist_time m_inc;

	/* state */
	UINT32 m_shift;

	/* cache */
	bool m_is_timestep;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MM5837_H_ */
