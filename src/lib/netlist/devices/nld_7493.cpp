// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7493.c
 *
 */

#include "nld_7493.h"
#include "nl_setup.h"

namespace netlist
{
	namespace devices
	{


NETLIB_RESET(7493ff)
{
	m_reset = 1;
	m_state = 0;
	m_I.set_state(logic_t::STATE_INP_HL);
}

NETLIB_UPDATE(7493ff)
{
	const netlist_time out_delay = NLTIME_FROM_NS(18);
	if (m_reset != 0)
	{
		m_state ^= 1;
		OUTLOGIC(m_Q, m_state, out_delay);
	}
}

NETLIB_UPDATE(7493)
{
	const netlist_sig_t r = INPLOGIC(m_R1) & INPLOGIC(m_R2);

	if (r)
	{
		A.m_I.inactivate();
		B.m_I.inactivate();
		OUTLOGIC(A.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(B.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(C.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(D.m_Q, 0, NLTIME_FROM_NS(40));
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 0;
		A.m_state = B.m_state = C.m_state = D.m_state = 0;
	}
	else
	{
		A.m_I.activate_hl();
		B.m_I.activate_hl();
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 1;
	}
}

	} //namespace devices
} // namespace netlist
