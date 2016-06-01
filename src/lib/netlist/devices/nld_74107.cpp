// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74107.c
 *
 */

#include "nld_74107.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_RESET(74107Asub)
{
	m_clk.set_state(logic_t::STATE_INP_HL);
	//m_Q.initial(0);
	//m_QQ.initial(1);

	m_Q1 = 0;
	m_Q2 = 0;
	m_F = 0;
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
			m_sub.m_Q1 = 0;
			m_sub.m_Q2 = 1;
			m_sub.m_F  = 0;
			m_sub.m_clk.inactivate();
			break;
		case 1:             // (!INPLOGIC(m_J) & INPLOGIC(m_K))
			m_sub.m_Q1 = 0;
			m_sub.m_Q2 = 0;
			m_sub.m_F  = 0;
			break;
		case 2:             // (INPLOGIC(m_J) & !INPLOGIC(m_K))
			m_sub.m_Q1 = 0;
			m_sub.m_Q2 = 0;
			m_sub.m_F  = 1;
			break;
		case 3:             // (INPLOGIC(m_J) & INPLOGIC(m_K))
			m_sub.m_Q1 = 1;
			m_sub.m_Q2 = 0;
			m_sub.m_F  = 0;
			break;
		default:
			break;
	}

	if (!INPLOGIC(m_clrQ))
	{
		m_sub.m_clk.inactivate();
		m_sub.newstate(0);
	}
	else if (!m_sub.m_Q2)
		m_sub.m_clk.activate_hl();
}


NETLIB_NAMESPACE_DEVICES_END()
