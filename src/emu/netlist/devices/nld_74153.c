/*
 * nld_74153.c
 *
 */

#include "nld_74153.h"

NETLIB_START(74153)
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

NETLIB_RESET(74153)
{
}

/* FIXME: timing is not 100% accurate, Strobe and Select inputs have a
 *        slightly longer timing.
 *        Convert this to sub-devices at some time.
 */

NETLIB_UPDATE(74153)
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


NETLIB_START(74153_dip)
{
    register_sub(m_1, "1");
    register_sub(m_2, "2");

    connect(m_1.m_A, m_2.m_A);
    connect(m_1.m_B, m_2.m_B);

    register_subalias("1", m_1.m_G);
    register_subalias("2", m_1.m_B);    // m_2.m_B
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

    register_subalias("14", m_1.m_A);   // m_2.m_B
    register_subalias("15", m_2.m_G);
}

NETLIB_UPDATE(74153_dip)
{
}

NETLIB_RESET(74153_dip)
{
    m_1.do_reset();
    m_2.do_reset();
}

