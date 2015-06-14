// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74193.c
 *
 */

#define MAXCNT 15

#include "nld_74193.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(74193)
{
	register_input("A", m_A);
	register_input("B", m_B);
	register_input("C", m_C);
	register_input("D", m_D);
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

NETLIB_RESET(74193)
{
	m_cnt = 0;
	m_last_CU = 0;
	m_last_CD = 0;
}

// FIXME: Timing
static const netlist_time delay[4] =
{
		NLTIME_FROM_NS(40),
		NLTIME_FROM_NS(40),
		NLTIME_FROM_NS(40),
		NLTIME_FROM_NS(40)
};

NETLIB_UPDATE(74193)
{
	int tCarry = 1;
	int tBorrow = 1;
	if (INPLOGIC(m_CLEAR))
	{
		m_cnt = 0;
	}
	else if (!INPLOGIC(m_LOADQ))
	{
		m_cnt = (INPLOGIC(m_D) << 3) | (INPLOGIC(m_C) << 2)
				| (INPLOGIC(m_B) << 1) | (INPLOGIC(m_A) << 0);
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

NETLIB_FUNC_VOID(74193, update_outputs, (void))
{
}

NETLIB_START(74193_dip)
{
	NETLIB_NAME(74193)::start();

	register_subalias("1", m_B);
	register_subalias("2", m_Q[1]);
	register_subalias("3", m_Q[0]);
	register_subalias("4", m_CD);
	register_subalias("5", m_CU);
	register_subalias("6", m_Q[2]);
	register_subalias("7", m_Q[3]);

	register_subalias("9", m_D);
	register_subalias("10", m_C);
	register_subalias("11", m_LOADQ);
	register_subalias("12", m_CARRYQ);
	register_subalias("13", m_BORROWQ);
	register_subalias("14", m_CLEAR);
	register_subalias("15", m_A);

}

NETLIB_UPDATE(74193_dip)
{
	NETLIB_NAME(74193)::update();
}

NETLIB_RESET(74193_dip)
{
	NETLIB_NAME(74193)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
