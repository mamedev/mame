// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74175.c
 *
 */

#include "nld_74175.h"

NETLIB_NAMESPACE_DEVICES_START()

static const netlist_time delay[2] = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };
static const netlist_time delay_clear[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };

NETLIB_START(74175_sub)
{
	register_input("CLK",   m_CLK);

	register_output("Q1",   m_Q[0]);
	register_output("Q1Q",  m_QQ[0]);
	register_output("Q2",   m_Q[1]);
	register_output("Q2Q",  m_QQ[1]);
	register_output("Q3",   m_Q[2]);
	register_output("Q3Q",  m_QQ[2]);
	register_output("Q4",   m_Q[3]);
	register_output("Q4Q",  m_QQ[3]);

	save(NLNAME(m_clrq));
	save(NLNAME(m_data));
}

NETLIB_RESET(74175_sub)
{
	m_CLK.set_state(logic_t::STATE_INP_LH);
	m_clrq = 0;
	m_data = 0xFF;
}

NETLIB_UPDATE(74175_sub)
{
	if (m_clrq)
	{
		for (int i=0; i<4; i++)
		{
			UINT8 d = (m_data >> i) & 1;
			OUTLOGIC(m_Q[i], d, delay[d]);
			OUTLOGIC(m_QQ[i], d ^ 1, delay[d ^ 1]);
		}
		m_CLK.inactivate();
	}
}

NETLIB_UPDATE(74175)
{
	UINT8 d = 0;
	for (int i=0; i<4; i++)
	{
		d |= (INPLOGIC(m_D[i]) << i);
	}
	m_sub.m_clrq = INPLOGIC(m_CLRQ);
	if (!m_sub.m_clrq)
	{
		for (int i=0; i<4; i++)
		{
			OUTLOGIC(m_sub.m_Q[i], 0, delay_clear[0]);
			OUTLOGIC(m_sub.m_QQ[i], 1, delay_clear[1]);
		}
		m_sub.m_data = 0;
	} else if (d != m_sub.m_data)
	{
		m_sub.m_data = d;
		m_sub.m_CLK.activate_lh();
	}
}

NETLIB_START(74175)
{
	register_sub("sub", m_sub);

	register_subalias("CLK",   m_sub.m_CLK);

	register_input("CLRQ",  m_CLRQ);

	register_input("D1",    m_D[0]);
	register_subalias("Q1",   m_sub.m_Q[0]);
	register_subalias("Q1Q",  m_sub.m_QQ[0]);

	register_input("D2",    m_D[1]);
	register_subalias("Q2",   m_sub.m_Q[1]);
	register_subalias("Q2Q",  m_sub.m_QQ[1]);

	register_input("D3",    m_D[2]);
	register_subalias("Q3",   m_sub.m_Q[2]);
	register_subalias("Q3Q",  m_sub.m_QQ[2]);

	register_input("D4",    m_D[3]);
	register_subalias("Q4",   m_sub.m_Q[3]);
	register_subalias("Q4Q",  m_sub.m_QQ[3]);

}

NETLIB_RESET(74175)
{
	m_sub.do_reset();
}

NETLIB_START(74175_dip)
{
	register_sub("sub", m_sub);

	register_subalias("9", m_sub.m_CLK);
	register_input("1",  m_CLRQ);

	register_input("4",    m_D[0]);
	register_subalias("2",   m_sub.m_Q[0]);
	register_subalias("3",  m_sub.m_QQ[0]);

	register_input("5",    m_D[1]);
	register_subalias("7",   m_sub.m_Q[1]);
	register_subalias("6",  m_sub.m_QQ[1]);

	register_input("12",    m_D[2]);
	register_subalias("10",   m_sub.m_Q[2]);
	register_subalias("11",  m_sub.m_QQ[2]);

	register_input("13",    m_D[3]);
	register_subalias("15",   m_sub.m_Q[3]);
	register_subalias("14",  m_sub.m_QQ[3]);

}

NETLIB_RESET(74175_dip)
{
	NETLIB_NAME(74175)::reset();
}

NETLIB_UPDATE(74175_dip)
{
	NETLIB_NAME(74175)::update();
}

NETLIB_NAMESPACE_DEVICES_END()
