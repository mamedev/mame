// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.c
 *
 */

#include "nld_7448.h"

NETLIB_NAMESPACE_DEVICES_START()

#if (0 && USE_TRUTHTABLE)
nld_7448::truthtable_t nld_7448::m_ttbl;
const char *nld_7448::m_desc[] = {
		" LTQ,BIQ,RBIQ, A , B , C , D | a, b, c, d, e, f, g",

		"  1,  1,  1,   0,  0,  0,  0 | 1, 1, 1, 1, 1, 1, 0|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  0,  0,  0 | 0, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  1,  0,  0 | 1, 1, 0, 1, 1, 0, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  1,  0,  0 | 1, 1, 1, 1, 0, 0, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  0,  1,  0 | 0, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  0,  1,  0 | 1, 0, 1, 1, 0, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  1,  1,  0 | 0, 0, 1, 1, 1, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  1,  1,  0 | 1, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  0,  0,  1 | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  0,  0,  1 | 1, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  1,  0,  1 | 0, 0, 0, 1, 1, 0, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  1,  0,  1 | 0, 0, 1, 1, 0, 0, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  0,  1,  1 | 0, 1, 0, 0, 0, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  0,  1,  1 | 1, 0, 0, 1, 0, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   0,  1,  1,  1 | 0, 0, 0, 1, 1, 1, 1|100,100,100,100,100,100,100",
		"  1,  1,  X,   1,  1,  1,  1 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100",

		// BI/RBO is input output. In the next case it is used as an output will go low.
		"  1,  1,  0,   0,  0,  0,  0 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100", // RBI

		"  0,  1,  X,   X,  X,  X,  X | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100", // LT

		// This condition has precedence
		"  X,  0,  X,   X,  X,  X,  X | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100", // BI
		""
};

NETLIB_START(7448_dip)
{
	NETLIB_NAME(7448)::start();

	register_subalias("1", m_i[4]); // B
	register_subalias("2", m_i[5]); // C
	register_subalias("3", m_i[0]); // LTQ
	register_subalias("4", m_i[1]); // BIQ
	register_subalias("5", m_i[2]); // RBIQ
	register_subalias("6", m_i[6]); // D
	register_subalias("7", m_i[3]); // A

	register_subalias("9",  m_Q[4]); // e
	register_subalias("10", m_Q[3]); // d
	register_subalias("11", m_Q[2]); // c
	register_subalias("12", m_Q[1]); // b
	register_subalias("13", m_Q[0]); // a
	register_subalias("14", m_Q[6]); // g
	register_subalias("15", m_Q[5]); // f
}

#else
NETLIB_START(7448)
{
	register_sub("sub", sub);

	register_subalias("A", sub.m_A);
	register_subalias("B", sub.m_B);
	register_subalias("C", sub.m_C);
	register_subalias("D", sub.m_D);
	register_input("LTQ", m_LTQ);
	register_input("BIQ", m_BIQ);
	register_subalias("RBIQ",sub.m_RBIQ);

	register_subalias("a", sub.m_Q[0]);
	register_subalias("b", sub.m_Q[1]);
	register_subalias("c", sub.m_Q[2]);
	register_subalias("d", sub.m_Q[3]);
	register_subalias("e", sub.m_Q[4]);
	register_subalias("f", sub.m_Q[5]);
	register_subalias("g", sub.m_Q[6]);

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

	register_output("a", m_Q[0]);
	register_output("b", m_Q[1]);
	register_output("c", m_Q[2]);
	register_output("d", m_Q[3]);
	register_output("e", m_Q[4]);
	register_output("f", m_Q[5]);
	register_output("g", m_Q[6]);

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

		for (int i=0; i<7; i++)
			OUTLOGIC(m_Q[i], tab7448[v][i], NLTIME_FROM_NS(100));
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

	register_subalias("9",  sub.m_Q[4]); // e
	register_subalias("10", sub.m_Q[3]); // d
	register_subalias("11", sub.m_Q[2]); // c
	register_subalias("12", sub.m_Q[1]); // b
	register_subalias("13", sub.m_Q[0]); // a
	register_subalias("14", sub.m_Q[6]); // g
	register_subalias("15", sub.m_Q[5]); // f
}

#endif

NETLIB_UPDATE(7448_dip)
{
	NETLIB_NAME(7448)::update();
}

NETLIB_RESET(7448_dip)
{
	NETLIB_NAME(7448)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
