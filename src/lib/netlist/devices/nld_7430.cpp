// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7430.c
 *
 */

#include "nld_7430.h"

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
nld_7430::truthtable_t nld_7430::m_ttbl;
const char *nld_7430::m_desc[] = {
		"A,B,C,D,E,F,G,H|Q ",
		"0,X,X,X,X,X,X,X|1|22",
		"X,0,X,X,X,X,X,X|1|22",
		"X,X,0,X,X,X,X,X|1|22",
		"X,X,X,0,X,X,X,X|1|22",
		"X,X,X,X,0,X,X,X|1|22",
		"X,X,X,X,X,0,X,X|1|22",
		"X,X,X,X,X,X,0,X|1|22",
		"X,X,X,X,X,X,X,0|1|22",
		"1,1,1,1,1,1,1,1|0|15",
		""
};

#endif


NETLIB_START(7430_dip)
{
	register_sub("1", m_1);

	register_subalias("1", m_1.m_I[0]);
	register_subalias("2", m_1.m_I[1]);
	register_subalias("3", m_1.m_I[2]);
	register_subalias("4", m_1.m_I[3]);
	register_subalias("5", m_1.m_I[4]);
	register_subalias("6", m_1.m_I[5]);

	register_subalias("8", m_1.m_Q[0]);

	register_subalias("11", m_1.m_I[6]);
	register_subalias("12", m_1.m_I[7]);
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

NETLIB_NAMESPACE_DEVICES_END()
