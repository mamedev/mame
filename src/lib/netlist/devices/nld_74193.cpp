// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74193.c
 *
 */

#define MAXCNT 15

#include "nld_74193.h"

NETLIB_NAMESPACE_DEVICES_START()

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


NETLIB_NAMESPACE_DEVICES_END()
