// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4020.c
 *
 */

#include "nld_4020.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(CD4020)
{
	register_sub("sub", sub);
	register_sub("supply", m_supply);

	register_input("RESET", m_RESET);
	register_subalias("IP", sub.m_IP);
	register_subalias("Q1", sub.m_Q[0]);
	register_subalias("Q4", sub.m_Q[3]);
	register_subalias("Q5", sub.m_Q[4]);
	register_subalias("Q6", sub.m_Q[5]);
	register_subalias("Q7", sub.m_Q[6]);
	register_subalias("Q8", sub.m_Q[7]);
	register_subalias("Q9", sub.m_Q[8]);
	register_subalias("Q10", sub.m_Q[9]);
	register_subalias("Q11", sub.m_Q[10]);
	register_subalias("Q12", sub.m_Q[11]);
	register_subalias("Q13", sub.m_Q[12]);
	register_subalias("Q14", sub.m_Q[13]);
	register_subalias("VDD", m_supply.m_vdd);
	register_subalias("VSS", m_supply.m_vss);
}

NETLIB_RESET(CD4020)
{
	sub.do_reset();
}


NETLIB_START(CD4020_sub)
{
	register_input("IP", m_IP);

	register_output("Q1", m_Q[0]);
	register_output("Q4", m_Q[3]);
	register_output("Q5", m_Q[4]);
	register_output("Q6", m_Q[5]);
	register_output("Q7", m_Q[6]);
	register_output("Q8", m_Q[7]);
	register_output("Q9", m_Q[8]);
	register_output("Q10", m_Q[9]);
	register_output("Q11", m_Q[10]);
	register_output("Q12", m_Q[11]);
	register_output("Q13", m_Q[12]);
	register_output("Q14", m_Q[13]);

	save(NLNAME(m_cnt));
}

NETLIB_RESET(CD4020_sub)
{
	m_IP.set_state(logic_t::STATE_INP_HL);
	m_cnt = 0;
}

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
		sub.m_cnt = 0;
		sub.m_IP.inactivate();
		/* static */ const netlist_time reset_time = netlist_time::from_nsec(140);
		OUTLOGIC(sub.m_Q[0], 0, reset_time);
		for (int i=3; i<14; i++)
			OUTLOGIC(sub.m_Q[i], 0, reset_time);
	}
	else
		sub.m_IP.activate_hl();
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
