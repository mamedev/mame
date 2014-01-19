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
		m_CLK.inactivate();
	}
}

NETLIB_UPDATE(7474)
{
	if (!INPLOGIC(m_PREQ))
	{
		sub.newstate(1);
		sub.m_CLK.inactivate();
		m_D.inactivate();
	}
	else if (!INPLOGIC(m_CLRQ))
	{
		sub.newstate(0);
		sub.m_CLK.inactivate();
		m_D.inactivate();
	}
	else
	{
		m_D.activate();
		sub.m_nextD = INPLOGIC(m_D);
		sub.m_CLK.activate_lh();
	}
}

NETLIB_START(7474)
{
	register_sub(sub, "sub");

	register_subalias("CLK",    sub.m_CLK);
	register_input("D",         m_D);
	register_input("CLRQ",      m_CLRQ);
	register_input("PREQ",      m_PREQ);

	register_subalias("Q",      sub.m_Q);
	register_subalias("QQ",     sub.m_QQ);

}

NETLIB_RESET(7474)
{
    sub.do_reset();
}

NETLIB_START(7474sub)
{
	register_input("CLK",  m_CLK);

	register_output("Q",   m_Q);
	register_output("QQ",  m_QQ);

	save(NAME(m_nextD));
}

NETLIB_RESET(7474sub)
{
    m_CLK.set_state(netlist_input_t::STATE_INP_LH);

    m_nextD = 0;
    m_Q.initial(1);
    m_QQ.initial(0);
}

NETLIB_START(7474_dip)
{
    register_sub(m_1, "1");
    register_sub(m_2, "2");

    register_subalias("1", m_1.m_CLRQ);
    register_subalias("2", m_1.m_D);
    register_subalias("3", m_1.sub.m_CLK);
    register_subalias("4", m_1.m_PREQ);
    register_subalias("5", m_1.sub.m_Q);
    register_subalias("6", m_1.sub.m_QQ);
    // register_subalias("7", ); ==> GND

    register_subalias("8", m_2.sub.m_QQ);
    register_subalias("9", m_2.sub.m_Q);
    register_subalias("10", m_2.m_PREQ);
    register_subalias("11", m_2.sub.m_CLK);
    register_subalias("12", m_2.m_D);
    register_subalias("13", m_1.m_CLRQ);
    // register_subalias("14", ); ==> VCC
}

NETLIB_RESET(7474_dip)
{
    m_1.do_reset();
    m_2.do_reset();
}

NETLIB_UPDATE(7474_dip)
{
}
