// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LP<cc> u7                       0010 0RRR 1110 1000   0RRR uuuu uu1Q QQQQ
// LP s13                          0010 0RRR 1010 1000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LP(uint32_t op) // LPcc (loop setup)
{
	int size = 4;
	int p = common32_get_p(op);
	if (p == 0x00)
	{
		fatalerror("<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		fatalerror("<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		uint32_t S = common32_get_s12(op);
		m_LP_START = m_pc + size;
		m_LP_END = (m_pc & 0xfffffffc) + (S * 2);

		return m_pc + size;
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		uint32_t u = common32_get_u6(op);
		uint8_t condition = common32_get_condition(op);
		// if the loop condition fails then just jump to after the end of the loop, don't set any registers
		if (!check_condition(condition))
		{
			m_allow_loop_check = false; // guard against instantly jumping back
			uint32_t realoffset = (m_pc & 0xfffffffc) + (u * 2);
			return realoffset;
		}
		else
		{
			// otherwise set up the loop positions
			m_LP_START = m_pc + size;
			m_LP_END = (m_pc & 0xfffffffc) + (u * 2);
			return m_pc + size;
		}
	}
	return m_pc + size;
}
