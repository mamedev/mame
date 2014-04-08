// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_NE555.h
 *
 *  NE555: PRECISION TIMERS
 *
 *          +--------+
 *      GND |1  ++  8| VCC
 *     TRIG |2      7| DISCH
 *      OUT |3      6| THRES
 *    RESET |4      5| CONT
 *          +--------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_NE555_H_
#define NLD_NE555_H_

#include "../nl_base.h"
#include "../analog/nld_twoterm.h"

#define NE555(_name)                                                        \
		NET_REGISTER_DEV(NE555, _name)


NETLIB_DEVICE(NE555,
	NETLIB_NAME(R) m_R1;
	NETLIB_NAME(R) m_R2;
	NETLIB_NAME(R) m_R3;
	NETLIB_NAME(R) m_RDIS;

	netlist_logic_input_t m_RESET;
	netlist_analog_input_t m_THRES;
	netlist_analog_input_t m_TRIG;
	netlist_analog_output_t m_OUT;

	bool m_last_out;

	inline double clamp(const double v, const double a, const double b);

);

#define NE555_DIP(_name)                                                         \
		NET_REGISTER_DEV(NE555_dip, _name)

NETLIB_DEVICE_DERIVED(NE555_dip, NE555,
);


#endif /* NLD_NE555_H_ */
