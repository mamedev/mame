/*
 * nld_7430.c
 *
 */

#include "nld_7430.h"

NETLIB_START(7430_dip)
{
	register_sub(m_1, "1");

	register_subalias("1", m_1.m_i[0]);
	register_subalias("2", m_1.m_i[1]);
	register_subalias("3", m_1.m_i[2]);
	register_subalias("4", m_1.m_i[3]);
	register_subalias("5", m_1.m_i[4]);
	register_subalias("6", m_1.m_i[5]);

	register_subalias("8", m_1.m_Q[0]);

	register_subalias("11", m_1.m_i[6]);
	register_subalias("12", m_1.m_i[7]);
}

NETLIB_UPDATE(7430_dip)
{
	/* only called during startup */
	m_1.update_dev();
}

NETLIB_RESET(7430_dip)
{
	m_1.do_reset();
}
