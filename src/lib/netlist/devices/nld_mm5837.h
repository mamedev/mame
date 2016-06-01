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

#include "nl_base.h"
#include "analog/nld_twoterm.h"

#define MM5837_DIP(name)                                                        \
		NET_REGISTER_DEV(MM5837_DIP, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(MM5837_dip)
{
	NETLIB_CONSTRUCTOR(MM5837_dip)
	,m_RV(*this, "RV")
	{
		/* clock */
		enregister("Q", m_Q);
		enregister("FB", m_feedback);
		m_inc = netlist_time::from_hz(56000);
		connect_late(m_feedback, m_Q);

		/* output */
		enregister("_RV1", m_RV.m_P);
		enregister("_RV2", m_RV.m_N);
		enregister("_Q", m_V0);
		connect_late(m_RV.m_N, m_V0);

		/* device */
		enregister("1", m_VDD);
		enregister("2", m_VGG);
		register_subalias("3", m_RV.m_P);
		enregister("4", m_VSS);

		save(NLNAME(m_shift));

	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

protected:
	NETLIB_SUB(twoterm) m_RV;
	analog_input_t m_VDD;
	analog_input_t m_VGG;
	analog_input_t m_VSS;

	/* output stage */
	analog_output_t m_V0; /* could be gnd as well */

	/* clock stage */
	logic_input_t m_feedback;
	logic_output_t m_Q;
	netlist_time m_inc;

	/* state */
	UINT32 m_shift;

	/* cache */
	bool m_is_timestep;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MM5837_H_ */
