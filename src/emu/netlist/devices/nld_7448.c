/*
 * nld_7448.c
 *
 */

#include "nld_7448.h"



NETLIB_START(7448)
{
	register_sub(sub, "sub");

	register_subalias("A", sub.m_A);
	register_subalias("B", sub.m_B);
	register_subalias("C", sub.m_C);
	register_subalias("D", sub.m_D);
	register_input("LTQ", m_LTQ);
	register_input("BIQ", m_BIQ);
	register_subalias("RBIQ",sub.m_RBIQ);

	register_subalias("a", sub.m_a);
	register_subalias("b", sub.m_b);
	register_subalias("c", sub.m_c);
	register_subalias("d", sub.m_d);
	register_subalias("e", sub.m_e);
	register_subalias("f", sub.m_f);
	register_subalias("g", sub.m_g);
}

NETLIB_RESET(7448)
{
	sub.do_reset();
}

NETLIB_UPDATE(7448)
{
	if (INPLOGIC(m_BIQ) && !INPLOGIC(m_LTQ))
	{
		sub.update_outputs(8);
	}
	else if (!INPLOGIC(m_BIQ))
	{
		sub.update_outputs(15);
	}

	if (!INPLOGIC(m_BIQ) || (INPLOGIC(m_BIQ) && !INPLOGIC(m_LTQ)))
	{
		sub.m_A.inactivate();
		sub.m_B.inactivate();
		sub.m_C.inactivate();
		sub.m_D.inactivate();
		sub.m_RBIQ.inactivate();
	} else {
		sub.m_RBIQ.activate();
		sub.m_D.activate();
		sub.m_C.activate();
		sub.m_B.activate();
		sub.m_A.activate();
		sub.update();
	}

}

NETLIB_START(7448_sub)
{
	register_input("A0", m_A);
	register_input("A1", m_B);
	register_input("A2", m_C);
	register_input("A3", m_D);
	register_input("RBIQ", m_RBIQ);

	register_output("a", m_a);
	register_output("b", m_b);
	register_output("c", m_c);
	register_output("d", m_d);
	register_output("e", m_e);
	register_output("f", m_f);
	register_output("g", m_g);

	save(NLNAME(m_state));
}

NETLIB_RESET(7448_sub)
{
	m_state = 0;
}

NETLIB_UPDATE(7448_sub)
{
	UINT8 v;

	v = (INPLOGIC(m_A) << 0) | (INPLOGIC(m_B) << 1) | (INPLOGIC(m_C) << 2) | (INPLOGIC(m_D) << 3);
	if ((!INPLOGIC(m_RBIQ) && (v==0)))
			v = 15;
	update_outputs(v);
}

NETLIB_FUNC_VOID(7448_sub, update_outputs, (UINT8 v))
{
	nl_assert(v<16);
	if (v != m_state)
	{
		// max transfer time is 100 NS */

		OUTLOGIC(m_a, tab7448[v][0], NLTIME_FROM_NS(100));
		OUTLOGIC(m_b, tab7448[v][1], NLTIME_FROM_NS(100));
		OUTLOGIC(m_c, tab7448[v][2], NLTIME_FROM_NS(100));
		OUTLOGIC(m_d, tab7448[v][3], NLTIME_FROM_NS(100));
		OUTLOGIC(m_e, tab7448[v][4], NLTIME_FROM_NS(100));
		OUTLOGIC(m_f, tab7448[v][5], NLTIME_FROM_NS(100));
		OUTLOGIC(m_g, tab7448[v][6], NLTIME_FROM_NS(100));
		m_state = v;
	}
}

const UINT8 NETLIB_NAME(7448_sub)::tab7448[16][7] =
{
		{   1, 1, 1, 1, 1, 1, 0 },  /* 00 - not blanked ! */
		{   0, 1, 1, 0, 0, 0, 0 },  /* 01 */
		{   1, 1, 0, 1, 1, 0, 1 },  /* 02 */
		{   1, 1, 1, 1, 0, 0, 1 },  /* 03 */
		{   0, 1, 1, 0, 0, 1, 1 },  /* 04 */
		{   1, 0, 1, 1, 0, 1, 1 },  /* 05 */
		{   0, 0, 1, 1, 1, 1, 1 },  /* 06 */
		{   1, 1, 1, 0, 0, 0, 0 },  /* 07 */
		{   1, 1, 1, 1, 1, 1, 1 },  /* 08 */
		{   1, 1, 1, 0, 0, 1, 1 },  /* 09 */
		{   0, 0, 0, 1, 1, 0, 1 },  /* 10 */
		{   0, 0, 1, 1, 0, 0, 1 },  /* 11 */
		{   0, 1, 0, 0, 0, 1, 1 },  /* 12 */
		{   1, 0, 0, 1, 0, 1, 1 },  /* 13 */
		{   0, 0, 0, 1, 1, 1, 1 },  /* 14 */
		{   0, 0, 0, 0, 0, 0, 0 },  /* 15 */
};


NETLIB_START(7448_dip)
{
	NETLIB_NAME(7448)::start();

	register_subalias("1", sub.m_B);
	register_subalias("2", sub.m_C);
	register_subalias("3", m_LTQ);
	register_subalias("4", m_BIQ);
	register_subalias("5",sub.m_RBIQ);
	register_subalias("6", sub.m_D);
	register_subalias("7", sub.m_A);

	register_subalias("9", sub.m_e);
	register_subalias("10", sub.m_d);
	register_subalias("11", sub.m_c);
	register_subalias("12", sub.m_b);
	register_subalias("13", sub.m_a);
	register_subalias("14", sub.m_g);
	register_subalias("15", sub.m_f);
}

NETLIB_UPDATE(7448_dip)
{
	NETLIB_NAME(7448)::update();
}

NETLIB_RESET(7448_dip)
{
	NETLIB_NAME(7448)::reset();
}
