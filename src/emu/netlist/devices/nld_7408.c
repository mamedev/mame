// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7408.c
 *
 */

#include "nld_7408.h"

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
nld_7408::truthtable_t nld_7408::m_ttbl;
const char *nld_7408::m_desc[] = {
		"A , B | Q ",
		"X,0|0|15",
		"0,X|0|15",
		"1,1|1|22",
		""
};

#endif

NETLIB_START(7408_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);
	register_sub("3", m_3);
	register_sub("4", m_4);

	register_subalias("1", m_1.m_I[0]);
	register_subalias("2", m_1.m_I[1]);
	register_subalias("3", m_1.m_Q[0]);

	register_subalias("4", m_2.m_I[0]);
	register_subalias("5", m_2.m_I[1]);
	register_subalias("6", m_2.m_Q[0]);

	register_subalias("9", m_3.m_I[0]);
	register_subalias("10", m_3.m_I[1]);
	register_subalias("8", m_3.m_Q[0]);

	register_subalias("12", m_4.m_I[0]);
	register_subalias("13", m_4.m_I[1]);
	register_subalias("11", m_4.m_Q[0]);
}

NETLIB_UPDATE(7408_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
	m_3.update_dev();
	m_4.update_dev();
}

NETLIB_RESET(7408_dip)
{
	m_1.do_reset();
	m_2.do_reset();
	m_3.do_reset();
	m_4.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
