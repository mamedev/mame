/*
 * nld_7404.c
 *
 */

#include "nld_7404.h"

NETLIB_START(nic7404)
{
	register_input("A", m_I);
	register_output("Q", m_Q);
	m_Q.initial(1);
}

NETLIB_UPDATE(nic7404)
{
	static const netlist_time delay[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };
	UINT8 t = (INPLOGIC(m_I)) ^ 1;
	OUTLOGIC(m_Q, t, delay[t]);
}
