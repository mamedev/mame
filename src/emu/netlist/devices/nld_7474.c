// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7474.c
 *
 */

#include "nld_7474.h"

NETLIB_NAMESPACE_DEVICES_START()

ATTR_HOT inline void NETLIB_NAME(7474sub)::newstate(const UINT8 stateQ, const UINT8 stateQQ)
{
	// 0: High-to-low 40 ns, 1: Low-to-high 25 ns
	const netlist_time delay[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };
	OUTLOGIC(m_Q, stateQ, delay[stateQ]);
	OUTLOGIC(m_QQ, stateQQ, delay[stateQQ]);
}

NETLIB_UPDATE(7474sub)
{
	//if (INP_LH(m_CLK))
	{
		newstate(m_nextD, !m_nextD);
		m_CLK.inactivate();
	}
}

NETLIB_UPDATE(7474)
{
	if (INPLOGIC(m_PREQ) && INPLOGIC(m_CLRQ))
	{
		m_D.activate();
		sub.m_nextD = INPLOGIC(m_D);
		sub.m_CLK.activate_lh();
	}
	else if (!INPLOGIC(m_PREQ))
	{
		sub.newstate(1, 0);
		sub.m_CLK.inactivate();
		m_D.inactivate();
	}
	else if (!INPLOGIC(m_CLRQ))
	{
		sub.newstate(0, 1);
		sub.m_CLK.inactivate();
		m_D.inactivate();
	}
	else
	{
		sub.newstate(1, 1);
		sub.m_CLK.inactivate();
		m_D.inactivate();
	}
}

NETLIB_START(7474)
{
	register_sub("sub", sub);

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

	save(NLNAME(m_nextD));
}

NETLIB_RESET(7474sub)
{
	m_CLK.set_state(logic_t::STATE_INP_LH);

	m_nextD = 0;
}

NETLIB_START(7474_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);

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
	register_subalias("13", m_2.m_CLRQ);
	// register_subalias("14", ); ==> VCC
}

NETLIB_RESET(7474_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_UPDATE(7474_dip)
{
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_NAMESPACE_DEVICES_END()
