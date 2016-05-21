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

#include "nl_base.h"
#include "analog/nld_twoterm.h"

#define NE555(_name)                                                        \
		NET_REGISTER_DEV(NE555, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(NE555)
{
	NETLIB_CONSTRUCTOR(NE555)
	, m_R1(*this, "R1")
	, m_R2(*this, "R2")
	, m_R3(*this, "R3")
	, m_RDIS(*this, "RDIS")
	, m_last_out(false)
	, m_ff(false)
	{

		register_subalias("GND",  m_R3.m_N);    // Pin 1
		enregister("TRIG",    m_TRIG);      // Pin 2
		enregister("OUT",    m_OUT);       // Pin 3
		enregister("RESET",   m_RESET);     // Pin 4
		register_subalias("CONT", m_R1.m_N);    // Pin 5
		enregister("THRESH",  m_THRES);     // Pin 6
		register_subalias("DISCH", m_RDIS.m_P); // Pin 7
		register_subalias("VCC",  m_R1.m_P);    // Pin 8

		connect_late(m_R1.m_N, m_R2.m_P);
		connect_late(m_R2.m_N, m_R3.m_P);
		connect_late(m_RDIS.m_N, m_R3.m_N);

		save(NLNAME(m_last_out));
		save(NLNAME(m_ff));
	}

	NETLIB_UPDATEI();
	NETLIB_RESETI();

protected:
	NETLIB_SUB(R) m_R1;
	NETLIB_SUB(R) m_R2;
	NETLIB_SUB(R) m_R3;
	NETLIB_SUB(R) m_RDIS;

	logic_input_t m_RESET;
	analog_input_t m_THRES;
	analog_input_t m_TRIG;
	analog_output_t m_OUT;

private:
	bool m_last_out;
	bool m_ff;

	inline nl_double clamp(const nl_double v, const nl_double a, const nl_double b);

};

#define NE555_DIP(_name)                                                         \
		NET_REGISTER_DEV(NE555_DIP, _name)

NETLIB_DEVICE_DERIVED_PURE(NE555_dip, NE555);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_NE555_H_ */
