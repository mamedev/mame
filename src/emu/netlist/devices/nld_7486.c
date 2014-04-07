/*
 * nld_7486.c
 *
 */

#include "nld_7486.h"

NETLIB_START(7486)
{
	register_input("A", m_A);
	register_input("B", m_B);
	register_output("Q", m_Q);
}

NETLIB_RESET(7486)
{
}

NETLIB_UPDATE(7486)
{
	static const netlist_time delay[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };
	UINT8 t = INPLOGIC(m_A) ^ INPLOGIC(m_B);
	OUTLOGIC(m_Q, t, delay[t]);
}

NETLIB_START(7486_dip)
{
	register_sub(m_1, "1");
	register_sub(m_2, "2");
	register_sub(m_3, "3");
	register_sub(m_4, "4");

	register_subalias("1", m_1.m_A);
	register_subalias("2", m_1.m_B);
	register_subalias("3", m_1.m_Q);

	register_subalias("4", m_2.m_A);
	register_subalias("5", m_2.m_B);
	register_subalias("6", m_2.m_Q);

	register_subalias("9", m_3.m_A);
	register_subalias("10", m_3.m_B);
	register_subalias("8", m_3.m_Q);

	register_subalias("12", m_4.m_A);
	register_subalias("13", m_4.m_B);
	register_subalias("11", m_4.m_Q);
}

NETLIB_UPDATE(7486_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
	m_3.update_dev();
	m_4.update_dev();
}

NETLIB_RESET(7486_dip)
{
	m_1.do_reset();
	m_2.do_reset();
	m_3.do_reset();
	m_4.do_reset();
}
