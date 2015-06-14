// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.c
 *
 */

#include "nld_7450.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(7450)
{
	register_input("A", m_A);
	register_input("B", m_B);
	register_input("C", m_C);
	register_input("D", m_D);
	register_output("Q", m_Q);
}

NETLIB_RESET(7450)
{
}

NETLIB_UPDATE(7450)
{
	m_A.activate();
	m_B.activate();
	m_C.activate();
	m_D.activate();
	UINT8 t1 = INPLOGIC(m_A) & INPLOGIC(m_B);
	UINT8 t2 = INPLOGIC(m_C) & INPLOGIC(m_D);

	const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };

	UINT8 res = 0;
	if (t1 ^ 1)
	{
		if (t2 ^ 1)
		{
			res = 1;
		}
		else
		{
			m_A.inactivate();
			m_B.inactivate();
		}
	} else {
		if (t2 ^ 1)
		{
			m_C.inactivate();
			m_D.inactivate();
		}
	}
	OUTLOGIC(m_Q, res, times[1 - res]);// ? 22000 : 15000);
}


NETLIB_START(7450_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);

	register_subalias("1", m_1.m_A);
	register_subalias("2", m_2.m_A);
	register_subalias("3", m_2.m_B);
	register_subalias("4", m_2.m_C);
	register_subalias("5", m_2.m_D);
	register_subalias("6", m_2.m_Q);
	//register_subalias("7",);  GND

	register_subalias("8", m_1.m_Q);
	register_subalias("9", m_1.m_C);
	register_subalias("10", m_1.m_D);
	//register_subalias("11", m_1.m_X1);
	//register_subalias("12", m_1.m_X1Q);
	register_subalias("13", m_1.m_B);
	//register_subalias("14",);  VCC
}

NETLIB_UPDATE(7450_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_RESET(7450_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
