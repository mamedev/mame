// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74175.c
 *
 */

#include "nld_74175.h"

namespace netlist
{
	namespace devices
	{

static const netlist_time delay[2] = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };
static const netlist_time delay_clear[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };

NETLIB_RESET(74175_sub)
{
	m_CLK.set_state(logic_t::STATE_INP_LH);
	m_clrq = 0;
	m_data = 0xFF;
}

NETLIB_UPDATE(74175_sub)
{
	if (m_clrq)
	{
		for (int i=0; i<4; i++)
		{
			UINT8 d = (m_data >> i) & 1;
			OUTLOGIC(m_Q[i], d, delay[d]);
			OUTLOGIC(m_QQ[i], d ^ 1, delay[d ^ 1]);
		}
		m_CLK.inactivate();
	}
}

NETLIB_UPDATE(74175)
{
	UINT8 d = 0;
	for (int i=0; i<4; i++)
	{
		d |= (INPLOGIC(m_D[i]) << i);
	}
	m_sub.m_clrq = INPLOGIC(m_CLRQ);
	if (!m_sub.m_clrq)
	{
		for (int i=0; i<4; i++)
		{
			OUTLOGIC(m_sub.m_Q[i], 0, delay_clear[0]);
			OUTLOGIC(m_sub.m_QQ[i], 1, delay_clear[1]);
		}
		m_sub.m_data = 0;
	} else if (d != m_sub.m_data)
	{
		m_sub.m_data = d;
		m_sub.m_CLK.activate_lh();
	}
}


NETLIB_RESET(74175)
{
	//m_sub.do_reset();
}

	} //namespace devices
} // namespace netlist
