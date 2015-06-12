// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74107.c
 *
 */

#include "nld_74107.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(74107Asub)
{
	register_input("CLK", m_clk);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);

	save(NLNAME(m_Q1));
	save(NLNAME(m_Q2));
	save(NLNAME(m_F));
}

NETLIB_RESET(74107Asub)
{
	m_clk.set_state(logic_t::STATE_INP_HL);
	m_Q.initial(0);
	m_QQ.initial(1);

	m_Q1 = 0;
	m_Q2 = 0;
	m_F = 0;
}

NETLIB_START(74107A)
{
	register_sub("sub", sub);

	register_subalias("CLK", sub.m_clk);
	register_input("J", m_J);
	register_input("K", m_K);
	register_input("CLRQ", m_clrQ);
	register_subalias("Q", sub.m_Q);
	register_subalias("QQ", sub.m_QQ);

}

NETLIB_RESET(74107A)
{
	sub.reset();
}

ATTR_HOT inline void NETLIB_NAME(74107Asub)::newstate(const netlist_sig_t state)
{
	const netlist_time delay[2] = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(40) };

	OUTLOGIC(m_Q, state, delay[state]);
	OUTLOGIC(m_QQ, state ^ 1, delay[state ^ 1]);
}

NETLIB_UPDATE(74107Asub)
{
	const netlist_sig_t t = m_Q.net().as_logic().Q();
	newstate(((t ^ 1) & m_Q1) | (t & m_Q2) | m_F);
	if (m_Q1 ^ 1)
		m_clk.inactivate();
}

NETLIB_UPDATE(74107A)
{
	const UINT8 JK = (INPLOGIC(m_J) << 1) | INPLOGIC(m_K);

	switch (JK)
	{
		case 0:
			sub.m_Q1 = 0;
			sub.m_Q2 = 1;
			sub.m_F  = 0;
			sub.m_clk.inactivate();
			break;
		case 1:             // (!INPLOGIC(m_J) & INPLOGIC(m_K))
			sub.m_Q1 = 0;
			sub.m_Q2 = 0;
			sub.m_F  = 0;
			break;
		case 2:             // (INPLOGIC(m_J) & !INPLOGIC(m_K))
			sub.m_Q1 = 0;
			sub.m_Q2 = 0;
			sub.m_F  = 1;
			break;
		case 3:             // (INPLOGIC(m_J) & INPLOGIC(m_K))
			sub.m_Q1 = 1;
			sub.m_Q2 = 0;
			sub.m_F  = 0;
			break;
		default:
			break;
	}

	if (!INPLOGIC(m_clrQ))
	{
		sub.m_clk.inactivate();
		sub.newstate(0);
	}
	else if (!sub.m_Q2)
		sub.m_clk.activate_hl();
}

NETLIB_START(74107_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);

	register_subalias("1", m_1.m_J);
	register_subalias("2", m_1.sub.m_QQ);
	register_subalias("3", m_1.sub.m_Q);

	register_subalias("4", m_1.m_K);
	register_subalias("5", m_2.sub.m_Q);
	register_subalias("6", m_2.sub.m_QQ);

	// register_subalias("7", ); ==> GND

	register_subalias("8", m_2.m_J);
	register_subalias("9", m_2.sub.m_clk);
	register_subalias("10", m_2.m_clrQ);

	register_subalias("11", m_2.m_K);
	register_subalias("12", m_1.sub.m_clk);
	register_subalias("13", m_1.m_clrQ);

	// register_subalias("14", ); ==> VCC
}

NETLIB_RESET(74107_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_UPDATE(74107_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_NAMESPACE_DEVICES_END()
