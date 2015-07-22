// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7425.c
 *
 */

#include "nld_7425.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(7425_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);

	register_subalias("1", m_1.m_I[0]);
	register_subalias("2", m_1.m_I[1]);

	//register_subalias("3", ); X1 ==> NC

	register_subalias("4", m_1.m_I[2]);
	register_subalias("5", m_1.m_I[3]);
	register_subalias("6", m_1.m_Q[0]);

	register_subalias("8", m_2.m_Q[0]);
	register_subalias("9", m_2.m_I[0]);
	register_subalias("10", m_2.m_I[1]);

	//register_subalias("11", ); X2 ==> NC

	register_subalias("12", m_2.m_I[2]);
	register_subalias("13", m_2.m_I[3]);

}

NETLIB_UPDATE(7425_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_RESET(7425_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
