// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74192.c
 *
 */

#define MAXCNT 9

#include "nld_74192.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(74192)
{
	register_sub("subABCD", m_ABCD);
	register_subalias("A", m_ABCD.m_A);
	register_subalias("B", m_ABCD.m_B);
	register_subalias("C", m_ABCD.m_C);
	register_subalias("D", m_ABCD.m_D);
	register_input("CLEAR",  m_CLEAR);
	register_input("LOADQ",  m_LOADQ);
	register_input("CU", m_CU);
	register_input("CD", m_CD);

	register_output("QA", m_Q[0]);
	register_output("QB", m_Q[1]);
	register_output("QC", m_Q[2]);
	register_output("QD", m_Q[3]);
	register_output("BORROWQ", m_BORROWQ);
	register_output("CARRYQ", m_CARRYQ);

	save(NLNAME(m_cnt));
	save(NLNAME(m_last_CU));
	save(NLNAME(m_last_CD));

}

NETLIB_RESET(74192)
{
	m_cnt = 0;
	m_last_CU = 0;
	m_last_CD = 0;
}

// FIXME: Timing
/* static */ const netlist_time delay[4] =
{
		NLTIME_FROM_NS(40),
		NLTIME_FROM_NS(40),
		NLTIME_FROM_NS(40),
		NLTIME_FROM_NS(40)
};

NETLIB_UPDATE(74192)
{
	int tCarry = 1;
	int tBorrow = 1;
	if (INPLOGIC(m_CLEAR))
	{
		m_cnt = 0;
	}
	else if (!INPLOGIC(m_LOADQ))
	{
		m_cnt = m_ABCD.read_ABCD();
	}
	else
	{
		if (INPLOGIC(m_CD) && !m_last_CU && INPLOGIC(m_CU))
		{
			m_cnt++;
			if (m_cnt > MAXCNT)
				m_cnt = 0;
		}
		if (INPLOGIC(m_CU) && !m_last_CD && INPLOGIC(m_CD))
		{
			if (m_cnt > 0)
				m_cnt--;
			else
				m_cnt = MAXCNT;
		}
	}

	if (!INPLOGIC(m_CU) && (m_cnt == MAXCNT))
		tCarry = 0;

	if (!INPLOGIC(m_CD) && (m_cnt == 0))
		tBorrow = 0;

	m_last_CD = INPLOGIC(m_CD);
	m_last_CU = INPLOGIC(m_CU);

	for (int i=0; i<4; i++)
		OUTLOGIC(m_Q[i], (m_cnt >> i) & 1, delay[i]);

	OUTLOGIC(m_BORROWQ, tBorrow, NLTIME_FROM_NS(20)); //FIXME
	OUTLOGIC(m_CARRYQ, tCarry, NLTIME_FROM_NS(20)); //FIXME
}

NETLIB_FUNC_VOID(74192, update_outputs, (void))
{
}

NETLIB_START(74192_dip)
{
	NETLIB_NAME(74192)::start();

	register_subalias("1", m_ABCD.m_B);
	register_subalias("2", m_Q[1]);
	register_subalias("3", m_Q[0]);
	register_subalias("4", m_CD);
	register_subalias("5", m_CU);
	register_subalias("6", m_Q[2]);
	register_subalias("7", m_Q[3]);

	register_subalias("9", m_ABCD.m_D);
	register_subalias("10", m_ABCD.m_C);
	register_subalias("11", m_LOADQ);
	register_subalias("12", m_CARRYQ);
	register_subalias("13", m_BORROWQ);
	register_subalias("14", m_CLEAR);
	register_subalias("15", m_ABCD.m_A);

}

NETLIB_UPDATE(74192_dip)
{
	NETLIB_NAME(74192)::update();
}

NETLIB_RESET(74192_dip)
{
	NETLIB_NAME(74192)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
