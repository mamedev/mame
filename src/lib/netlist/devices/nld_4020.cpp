// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4020.c
 *
 */

#include "nld_4020.h"

NETLIB_NAMESPACE_DEVICES_START()


NETLIB_UPDATE(CD4020_sub)
{
	UINT8 cnt = m_cnt;
	cnt = ( cnt + 1) & 0x3fff;
	update_outputs(cnt);
	m_cnt = cnt;
}

NETLIB_UPDATE(CD4020)
{
	if (INPLOGIC(m_RESET))
	{
		m_sub.m_cnt = 0;
		m_sub.m_IP.inactivate();
		/* static */ const netlist_time reset_time = netlist_time::from_nsec(140);
		OUTLOGIC(m_sub.m_Q[0], 0, reset_time);
		for (int i=3; i<14; i++)
			OUTLOGIC(m_sub.m_Q[i], 0, reset_time);
	}
	else
		m_sub.m_IP.activate_hl();
}

inline NETLIB_FUNC_VOID(CD4020_sub, update_outputs, (const UINT16 cnt))
{
	/* static */ const netlist_time out_delayQn[14] = {
			NLTIME_FROM_NS(180), NLTIME_FROM_NS(280),
			NLTIME_FROM_NS(380), NLTIME_FROM_NS(480),
			NLTIME_FROM_NS(580), NLTIME_FROM_NS(680),
			NLTIME_FROM_NS(780), NLTIME_FROM_NS(880),
			NLTIME_FROM_NS(980), NLTIME_FROM_NS(1080),
			NLTIME_FROM_NS(1180), NLTIME_FROM_NS(1280),
			NLTIME_FROM_NS(1380), NLTIME_FROM_NS(1480),
	};

	OUTLOGIC(m_Q[0], 0, out_delayQn[0]);
	for (int i=3; i<14; i++)
		OUTLOGIC(m_Q[i], (cnt >> i) & 1, out_delayQn[i]);
}

NETLIB_NAMESPACE_DEVICES_END()
