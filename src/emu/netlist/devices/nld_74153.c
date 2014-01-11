/*
 * nld_74153.c
 *
 */

#include "nld_74153.h"

NETLIB_START(nic74153)
{
	register_input("C0", m_C[0]);
	register_input("C1", m_C[1]);
	register_input("C2", m_C[2]);
	register_input("C3", m_C[3]);
	register_input("A", m_A);
	register_input("B", m_B);
	register_input("G", m_G);

	register_output("AY", m_Y); //FIXME: Change netlists
}

NETLIB_RESET(nic74153)
{
}

/* FIXME: timing is not 100% accurate, Strobe and Select inputs have a
 *        slightly longer timing.
 *        Convert this to sub-devices at some time.
 */

NETLIB_UPDATE(nic74153)
{
	const netlist_time delay[2] = { NLTIME_FROM_NS(23), NLTIME_FROM_NS(18) };
	if (!INPLOGIC(m_G))
	{
		UINT8 chan = (INPLOGIC(m_A) | (INPLOGIC(m_B)<<1));
		UINT8 t = INPLOGIC(m_C[chan]);
		OUTLOGIC(m_Y, t, delay[t] );
	}
	else
	{
		OUTLOGIC(m_Y, 0, delay[0]);
	}
}
