// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9310.c
 *
 */

#include "nld_9310.h"

#define MAXCNT 9

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_RESET(9310)
{
	sub.do_reset();
	subABCD.do_reset();
}

NETLIB_RESET(9310_subABCD)
{
#if 0
	m_A.inactivate();
	m_B.inactivate();
	m_C.inactivate();
	m_D.inactivate();
#endif
}

NETLIB_RESET(9310_sub)
{
	m_CLK.set_state(logic_t::STATE_INP_LH);
	m_cnt = 0;
	m_loadq = 1;
	m_ent = 1;
}

NETLIB_UPDATE(9310_sub)
{
	if (m_loadq)
	{
#if 0
		m_cnt = (m_cnt < MAXCNT) ? m_cnt + 1 : 0;
		update_outputs(m_cnt);
		OUTLOGIC(m_RC, m_ent & (m_cnt == MAXCNT), NLTIME_FROM_NS(20));
#else
		switch (m_cnt)
		{
			case MAXCNT - 1:
				m_cnt = MAXCNT;
				OUTLOGIC(m_RC, m_ent, NLTIME_FROM_NS(20));
				OUTLOGIC(m_QA, 1, NLTIME_FROM_NS(20));
				break;
			case MAXCNT:
				OUTLOGIC(m_RC, 0, NLTIME_FROM_NS(20));
				m_cnt = 0;
				update_outputs_all(m_cnt, NLTIME_FROM_NS(20));
				break;
			default:
				m_cnt++;
				update_outputs(m_cnt);
		}
#endif
	}
	else
	{
		m_cnt = m_ABCD->read_ABCD();
		update_outputs_all(m_cnt, NLTIME_FROM_NS(22));
		OUTLOGIC(m_RC, m_ent & (m_cnt == MAXCNT), NLTIME_FROM_NS(27));
	}
}

NETLIB_UPDATE(9310)
{
	sub.m_loadq = INPLOGIC(m_LOADQ);
	sub.m_ent = INPLOGIC(m_ENT);
	const netlist_sig_t clrq = INPLOGIC(m_CLRQ);

	if ((!sub.m_loadq || (sub.m_ent & INPLOGIC(m_ENP))) && clrq)
	{
		sub.m_CLK.activate_lh();
		OUTLOGIC(sub.m_RC, sub.m_ent & (sub.m_cnt == MAXCNT), NLTIME_FROM_NS(27));
	}
	else
	{
		sub.m_CLK.inactivate();
		if (!clrq && (sub.m_cnt>0))
		{
			sub.update_outputs_all(0, NLTIME_FROM_NS(36));
			sub.m_cnt = 0;
			//return;
		}
		OUTLOGIC(sub.m_RC, sub.m_ent & (sub.m_cnt == MAXCNT), NLTIME_FROM_NS(27));
	}
}

inline NETLIB_FUNC_VOID(9310_sub, update_outputs_all, (const UINT8 cnt, const netlist_time out_delay))
{
	OUTLOGIC(m_QA, (cnt >> 0) & 1, out_delay);
	OUTLOGIC(m_QB, (cnt >> 1) & 1, out_delay);
	OUTLOGIC(m_QC, (cnt >> 2) & 1, out_delay);
	OUTLOGIC(m_QD, (cnt >> 3) & 1, out_delay);
}

inline NETLIB_FUNC_VOID(9310_sub, update_outputs, (const UINT8 cnt))
{
	/* static */ const netlist_time out_delay = NLTIME_FROM_NS(20);
#if 0
//    for (int i=0; i<4; i++)
//        OUTLOGIC(m_Q[i], (cnt >> i) & 1, delay[i]);
	OUTLOGIC(m_QA, (cnt >> 0) & 1, out_delay);
	OUTLOGIC(m_QB, (cnt >> 1) & 1, out_delay);
	OUTLOGIC(m_QC, (cnt >> 2) & 1, out_delay);
	OUTLOGIC(m_QD, (cnt >> 3) & 1, out_delay);
#else
	if ((cnt & 1) == 1)
		OUTLOGIC(m_QA, 1, out_delay);
	else
	{
		OUTLOGIC(m_QA, 0, out_delay);
		switch (cnt)
		{
		case 0x00:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 0, out_delay);
			OUTLOGIC(m_QD, 0, out_delay);
			break;
		case 0x02:
		case 0x06:
		case 0x0A:
		case 0x0E:
			OUTLOGIC(m_QB, 1, out_delay);
			break;
		case 0x04:
		case 0x0C:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 1, out_delay);
			break;
		case 0x08:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 0, out_delay);
			OUTLOGIC(m_QD, 1, out_delay);
			break;
		}

	}
#endif
}

NETLIB_NAMESPACE_DEVICES_END()
