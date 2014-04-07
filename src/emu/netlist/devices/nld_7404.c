/*
 * nld_7404.c
 *
 */

#include "nld_7404.h"

NETLIB_START(7404)
{
	register_input("A", m_I);
	register_output("Q", m_Q);
}

NETLIB_RESET(7404)
{
}

NETLIB_UPDATE(7404)
{
	static const netlist_time delay[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };
	UINT8 t = (INPLOGIC(m_I)) ^ 1;
	OUTLOGIC(m_Q, t, delay[t]);
}

NETLIB_START(7404_dip)
{
	register_sub(m_1, "1");
	register_sub(m_2, "2");
	register_sub(m_3, "3");
	register_sub(m_4, "4");
	register_sub(m_5, "5");
	register_sub(m_6, "6");

	register_subalias("1", m_1.m_I);
	register_subalias("2", m_1.m_Q);

	register_subalias("3", m_2.m_I);
	register_subalias("4", m_2.m_Q);

	register_subalias("5", m_3.m_I);
	register_subalias("6", m_3.m_Q);

	register_subalias("8", m_4.m_Q);
	register_subalias("9", m_4.m_I);

	register_subalias("10", m_5.m_Q);
	register_subalias("11", m_5.m_I);

	register_subalias("12", m_6.m_Q);
	register_subalias("13", m_6.m_I);
}

NETLIB_UPDATE(7404_dip)
{
	/* only called during startup */

	m_1.update_dev();
	m_2.update_dev();
	m_3.update_dev();
	m_4.update_dev();
	m_5.update_dev();
	m_6.update_dev();
}

NETLIB_RESET(7404_dip)
{
	m_1.do_reset();
	m_2.do_reset();
	m_3.do_reset();
	m_4.do_reset();
	m_5.do_reset();
	m_6.do_reset();
}
