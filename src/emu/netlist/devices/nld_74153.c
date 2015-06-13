// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74153.c
 *
 */

#include "nld_74153.h"

NETLIB_NAMESPACE_DEVICES_START()

/* FIXME: timing is not 100% accurate, Strobe and Select inputs have a
 *        slightly longer timing.
 *        Convert this to sub-devices at some time.
 */

NETLIB_START(74153sub)
{
	register_input("C0", m_C[0]);
	register_input("C1", m_C[1]);
	register_input("C2", m_C[2]);
	register_input("C3", m_C[3]);
	register_input("G", m_G);

	register_output("AY", m_Y); //FIXME: Change netlists

	m_chan = 0;

	save(NLNAME(m_chan));
}

NETLIB_RESET(74153sub)
{
	m_chan = 0;
}

NETLIB_UPDATE(74153sub)
{
	const netlist_time delay[2] = { NLTIME_FROM_NS(23), NLTIME_FROM_NS(18) };
	if (!INPLOGIC(m_G))
	{
		UINT8 t = INPLOGIC(m_C[m_chan]);
		OUTLOGIC(m_Y, t, delay[t] );
	}
	else
	{
		OUTLOGIC(m_Y, 0, delay[0]);
	}
}


NETLIB_START(74153)
{
	register_sub("sub", m_sub);

	register_subalias("C0", m_sub.m_C[0]);
	register_subalias("C1",  m_sub.m_C[1]);
	register_subalias("C2",  m_sub.m_C[2]);
	register_subalias("C3",  m_sub.m_C[3]);
	register_input("A", m_A);
	register_input("B", m_B);
	register_subalias("G",  m_sub.m_G);

	register_subalias("AY",  m_sub.m_Y); //FIXME: Change netlists
}

NETLIB_RESET(74153)
{
}



NETLIB_UPDATE(74153)
{
	m_sub.m_chan = (INPLOGIC(m_A) | (INPLOGIC(m_B)<<1));
	m_sub.update();
}


NETLIB_START(74153_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);

	register_subalias("1", m_1.m_G);
	register_input("2", m_B);    // m_2.m_B
	register_subalias("3", m_1.m_C[3]);
	register_subalias("4", m_1.m_C[2]);
	register_subalias("5", m_1.m_C[1]);
	register_subalias("6", m_1.m_C[0]);
	register_subalias("7", m_1.m_Y);

	register_subalias("9", m_2.m_Y);
	register_subalias("10", m_2.m_C[0]);
	register_subalias("11", m_2.m_C[1]);
	register_subalias("12", m_2.m_C[2]);
	register_subalias("13", m_2.m_C[3]);

	register_input("14", m_A);   // m_2.m_B
	register_subalias("15", m_2.m_G);
}

NETLIB_UPDATE(74153_dip)
{
	m_2.m_chan = m_1.m_chan = (INPLOGIC(m_A) | (INPLOGIC(m_B)<<1));
	m_1.update();
	m_2.update();
}

NETLIB_RESET(74153_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
