/*
 * nld_7400.c
 *
 */

#include "nld_7400.h"

NETLIB_START(7400_dip)
{
	register_sub(m_1, "1");
	register_sub(m_2, "2");
	register_sub(m_3, "3");
	register_sub(m_4, "4");

	register_subalias("1", m_1.m_i[0]);
	register_subalias("2", m_1.m_i[1]);
	register_subalias("3", m_1.m_Q);

	register_subalias("4", m_2.m_i[0]);
	register_subalias("5", m_2.m_i[1]);
	register_subalias("6", m_2.m_Q);

	register_subalias("9", m_3.m_i[0]);
	register_subalias("10", m_3.m_i[1]);
	register_subalias("8", m_3.m_Q);

	register_subalias("12", m_4.m_i[0]);
	register_subalias("13", m_4.m_i[1]);
	register_subalias("11", m_4.m_Q);
}

NETLIB_UPDATE(7400_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
	m_3.update_dev();
	m_4.update_dev();
}

NETLIB_RESET(7400_dip)
{
	m_1.do_reset();
	m_2.do_reset();
	m_3.do_reset();
	m_4.do_reset();
}
