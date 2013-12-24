/*
 * nld_7490.c
 *
 */

#include "nld_7490.h"

NETLIB_START(7490)
{
	m_cnt = 0;

	register_input("CLK", m_clk);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);
	register_input("R91", m_R91);
	register_input("R92", m_R92);

	register_output("QA", m_Q[0]);
	register_output("QB", m_Q[1]);
	register_output("QC", m_Q[2]);
	register_output("QD", m_Q[3]);

	save(NAME(m_cnt));

}

NETLIB_UPDATE(7490)
{
	if (INPLOGIC(m_R91) & INPLOGIC(m_R92))
	{
		m_cnt = 9;
		update_outputs();
	}
	else if (INPLOGIC(m_R1) & INPLOGIC(m_R2))
	{
		m_cnt = 0;
		update_outputs();
	}
	else if (INP_HL(m_clk))
	{
		m_cnt++;
		if (m_cnt >= 10)
			m_cnt = 0;
		update_outputs();
	}
}

NETLIB_FUNC_VOID(7490, update_outputs, (void))
{
	const netlist_time delay[4] = { NLTIME_FROM_NS(18), NLTIME_FROM_NS(36), NLTIME_FROM_NS(54), NLTIME_FROM_NS(72) };
	for (int i=0; i<4; i++)
		OUTLOGIC(m_Q[i], (m_cnt >> i) & 1, delay[i]);
}
