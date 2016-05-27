// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7490.c
 *
 */

#include "nld_7490.h"

namespace netlist
{
	namespace devices
	{

NETLIB_RESET(7490)
{
	m_cnt = 0;
	m_last_A = 0;
	m_last_B = 0;
}

static const netlist_time delay[4] =
{
		NLTIME_FROM_NS(18),
		NLTIME_FROM_NS(36) - NLTIME_FROM_NS(18),
		NLTIME_FROM_NS(54) - NLTIME_FROM_NS(18),
		NLTIME_FROM_NS(72) - NLTIME_FROM_NS(18)};

NETLIB_UPDATE(7490)
{
	const netlist_sig_t new_A = INPLOGIC(m_A);
	const netlist_sig_t new_B = INPLOGIC(m_B);

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
	else
	{
		if (m_last_A && !new_A)  // High - Low
		{
			m_cnt ^= 1;
			OUTLOGIC(m_Q[0], m_cnt & 1, delay[0]);
		}
		if (m_last_B && !new_B)  // High - Low
		{
			m_cnt += 2;
			if (m_cnt >= 10)
				m_cnt &= 1; /* Output A is not reset! */
			update_outputs();
		}
	}
	m_last_A = new_A;
	m_last_B = new_B;
}

NETLIB_FUNC_VOID(7490, update_outputs, (void))
{
	for (int i=0; i<4; i++)
		OUTLOGIC(m_Q[i], (m_cnt >> i) & 1, delay[i]);
}

	} //namespace devices
} // namespace netlist
