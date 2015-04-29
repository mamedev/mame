/*
 * nld_74175.c
 *
 */

#include "nld_74175.h"

static const netlist_time delay[2] = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };
static const netlist_time delay_clear[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };

NETLIB_UPDATE(74175)
{
	if (!INPLOGIC(m_CLRQ))
	{
		for (int i=0; i<4; i++)
		{
			OUTLOGIC(m_Q[i], 0, delay_clear[0]);
			OUTLOGIC(m_QQ[i], 1, delay_clear[1]);
		}
	}
	else if (!m_last && INPLOGIC(m_CLK))
	{
		for (int i=0; i<4; i++)
		{
			UINT8 d = INPLOGIC(m_D[i]);
			OUTLOGIC(m_Q[i], d, delay[d]);
			OUTLOGIC(m_QQ[i], d ^ 1, delay[d ^ 1]);
		}
	}
	m_last = INPLOGIC(m_CLK);
}

NETLIB_START(74175)
{
	register_input("CLK",   m_CLK);
	register_input("CLRQ",  m_CLRQ);

	register_input("D1",    m_D[0]);
	register_output("Q1",   m_Q[0]);
	register_output("Q1Q",  m_QQ[0]);

	register_input("D2",    m_D[1]);
	register_output("Q2",   m_Q[1]);
	register_output("Q2Q",  m_QQ[1]);

	register_input("D3",    m_D[2]);
	register_output("Q3",   m_Q[2]);
	register_output("Q3Q",  m_QQ[2]);

	register_input("D4",    m_D[3]);
	register_output("Q4",   m_Q[3]);
	register_output("Q4Q",  m_QQ[3]);

	save(NLNAME(m_last.ref()));
}

NETLIB_RESET(74175)
{
}

NETLIB_START(74175_dip)
{
	register_input("9", m_CLK);
	register_input("1",  m_CLRQ);

	register_input("4",    m_D[0]);
	register_output("2",   m_Q[0]);
	register_output("3",  m_QQ[0]);

	register_input("5",    m_D[1]);
	register_output("7",   m_Q[1]);
	register_output("6",  m_QQ[1]);

	register_input("12",    m_D[2]);
	register_output("10",   m_Q[2]);
	register_output("11",  m_QQ[2]);

	register_input("13",    m_D[3]);
	register_output("15",   m_Q[3]);
	register_output("14",  m_QQ[3]);

	save(NLNAME(m_last.ref()));
}

NETLIB_RESET(74175_dip)
{
	NETLIB_NAME(74175)::reset();
}

NETLIB_UPDATE(74175_dip)
{
	NETLIB_NAME(74175)::update();
}
