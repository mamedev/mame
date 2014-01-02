/*
 * nld_7474.c
 *
 */

#include "nld_7474.h"

ATTR_HOT inline void NETLIB_NAME(7474sub)::newstate(const UINT8 state)
{
	static const netlist_time delay[2] = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(40) };
	OUTLOGIC(m_Q, state, delay[state]);
	OUTLOGIC(m_QQ, !state, delay[!state]);
}

NETLIB_UPDATE(7474sub)
{
	//if (!INP_LAST(m_clk) & INP(m_clk))
	{
		newstate(m_nextD);
		m_clk.inactivate();
	}
}

NETLIB_UPDATE(7474)
{
	if (!INPLOGIC(m_preQ))
	{
		sub.newstate(1);
		sub.m_clk.inactivate();
		m_D.inactivate();
	}
	else if (!INPLOGIC(m_clrQ))
	{
		sub.newstate(0);
		sub.m_clk.inactivate();
		m_D.inactivate();
	}
	else
	{
		m_D.activate();
		sub.m_nextD = INPLOGIC(m_D);
		sub.m_clk.activate_lh();
	}
}

NETLIB_START(7474)
{
	register_sub(sub, "sub");

	register_subalias("CLK",    sub.m_clk);
	register_input("D",         m_D);
	register_input("CLRQ",      m_clrQ);
	register_input("PREQ",      m_preQ);

	register_subalias("Q",      sub.m_Q);
	register_subalias("QQ",     sub.m_QQ);

}

NETLIB_START(7474sub)
{
	register_input("CLK",  m_clk, netlist_input_t::STATE_INP_LH);

	register_output("Q",   m_Q);
	register_output("QQ",  m_QQ);

	m_nextD = 0;
	m_Q.initial(1);
	m_QQ.initial(0);

	save(NAME(m_nextD));
}
