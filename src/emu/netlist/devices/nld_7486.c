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

NETLIB_UPDATE(7486)
{
	static const netlist_time delay[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };
	UINT8 t = INPLOGIC(m_A) ^ INPLOGIC(m_B);
	OUTLOGIC(m_Q, t, delay[t]);
}
