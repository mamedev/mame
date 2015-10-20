// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_legacy.c
 *
 */

#include "nld_legacy.h"
#include "nl_setup.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(nicRSFF)
{
	register_input("S", m_S);
	register_input("R", m_R);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);
}

NETLIB_RESET(nicRSFF)
{
	m_Q.initial(0);
	m_QQ.initial(1);
}

NETLIB_UPDATE(nicRSFF)
{
	if (!INPLOGIC(m_S))
	{
		OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(20));
		OUTLOGIC(m_QQ, 0, NLTIME_FROM_NS(20));
	}
	else if (!INPLOGIC(m_R))
	{
		OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(20));
		OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(20));
	}
}

NETLIB_START(nicDelay)
{
	register_input("1", m_I);
	register_output("2", m_Q);

	register_param("L_TO_H", m_L_to_H, 10);
	register_param("H_TO_L", m_H_to_L, 10);

	save(NLNAME(m_last));

}

NETLIB_RESET(nicDelay)
{
	m_Q.initial(0);
}

NETLIB_UPDATE_PARAM(nicDelay)
{
}

NETLIB_UPDATE(nicDelay)
{
	netlist_sig_t nval = INPLOGIC(m_I);
	if (nval && !m_last)
	{
		// L_to_H
		OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(m_L_to_H.Value()));
	}
	else if (!nval && m_last)
	{
		// H_to_L
		OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(m_H_to_L.Value()));
	}
	m_last = nval;
}

NETLIB_NAMESPACE_DEVICES_END()
