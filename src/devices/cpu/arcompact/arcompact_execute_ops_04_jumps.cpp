// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

uint32_t arcompact_device::handle_jump_to_addr(bool delay, bool link, uint32_t address, uint32_t next_addr)
{
	if (delay)
	{
		m_delayactive = true;
		m_delayjump = address;
		if (link)
			m_delaylinks = true;
		else m_delaylinks = false;
		return next_addr;
	}
	else
	{
		if (link)
			m_regs[REG_BLINK] = next_addr;
		return address;
	}
}

uint32_t arcompact_device::handle_jump_to_register(bool delay, bool link, uint32_t reg, uint32_t next_addr, int flag)
{
	if ((reg == REG_ILINK1) || (reg == REG_ILINK2))
	{
		if (flag)
		{
			if (reg == REG_ILINK1) m_status32 = m_status32_l1;
			if (reg == REG_ILINK2) m_status32 = m_status32_l2;
			uint32_t target = m_regs[reg];
			return handle_jump_to_addr(delay, link, target, next_addr);
		}
		else
		{
			fatalerror("illegal jump to ILINK1/ILINK2 not supported"); // FLAG bit must be set
			return next_addr;
		}
	}
	else
	{
		if (flag)
		{
			fatalerror("illegal jump (flag bit set)"); // FLAG bit must NOT be set
			return next_addr;
		}
		else
		{
			uint32_t target = m_regs[reg];
			return handle_jump_to_addr(delay, link, target, next_addr);
		}
	}
	return 0;
}

uint32_t arcompact_device::handleop32_Jcc_f_a_b_c_helper(uint32_t op, bool delay, bool link)
{
	uint8_t creg = common32_get_creg(op);
	int size = check_limm(creg);
	return handle_jump_to_register(delay, link, creg, m_pc + size, common32_get_F(op));
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_c_helper(uint32_t op, bool delay, bool link)
{
	uint8_t creg = common32_get_creg(op);
	int size = check_limm(creg);
	if (!check_condition(common32_get_condition(op)))
		return m_pc + size;
	return handle_jump_to_register(delay, link, creg, m_pc + size, common32_get_F(op));
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc [c]                         0010 0RRR 1110 0000   0RRR CCCC CC0Q QQQQ
// Jcc limm                        0010 0RRR 1110 0000   0RRR 1111 100Q QQQQ (+ Limm)
// Jcc u6                          0010 0RRR 1110 0000   0RRR uuuu uu1Q QQQQ
// Jcc.F [ilink1]                  0010 0RRR 1110 0000   1RRR 0111 010Q QQQQ
// Jcc.F [ilink2]                  0010 0RRR 1110 0000   1RRR 0111 100Q QQQQ
//                                 IIII I      SS SSSS
// J [c]                           0010 0RRR 0010 0000   0RRR CCCC CCRR RRRR
// J.F [ilink1]                    0010 0RRR 0010 0000   1RRR 0111 01RR RRRR
// J.F [ilink2]                    0010 0RRR 0010 0000   1RRR 0111 10RR RRRR
// J limm                          0010 0RRR 0010 0000   0RRR 1111 10RR RRRR (+ Limm)
// J u6                            0010 0RRR 0110 0000   0RRR uuuu uuRR RRRR
// J s12                           0010 0RRR 1010 0000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc.D u6                        0010 0RRR 1110 0001   0RRR uuuu uu1Q QQQQ
// Jcc.D [c]                       0010 0RRR 1110 0001   0RRR CCCC CC0Q QQQQ
//
// J.D [c]                         0010 0RRR 0010 0001   0RRR CCCC CCRR RRRR
// J.D u6                          0010 0RRR 0110 0001   0RRR uuuu uuRR RRRR
// J.D s12                         0010 0RRR 1010 0001   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc [c]                        0010 0RRR 1110 0010   0RRR CCCC CC0Q QQQQ
// JLcc limm                       0010 0RRR 1110 0010   0RRR 1111 100Q QQQQ (+ Limm)
// JLcc u6                         0010 0RRR 1110 0010   0RRR uuuu uu1Q QQQQ
// JL [c]                          0010 0RRR 0010 0010   0RRR CCCC CCRR RRRR
// JL limm                         0010 0RRR 0010 0010   0RRR 1111 10RR RRRR (+ Limm)
// JL u6                           0010 0RRR 0110 0010   0RRR uuuu uuRR RRRR
// JL s12                          0010 0RRR 1010 0010   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc.D u6                       0010 0RRR 1110 0011   0RRR uuuu uu1Q QQQQ
// JLcc.D [c]                      0010 0RRR 1110 0011   0RRR CCCC CC0Q QQQQ
// JL.D [c]                        0010 0RRR 0010 0011   0RRR CCCC CCRR RRRR
// JL.D u6                         0010 0RRR 0110 0011   0RRR uuuu uuRR RRRR
// JL.D s12                        0010 0RRR 1010 0011   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_J(uint32_t op, bool delay, bool link)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00: return handleop32_Jcc_f_a_b_c_helper(op, delay, link);
	case 0x01:
	{
		int size = 4;
		fatalerror("unimplemented Jump (delay %d link %d) (u6 type) %08x", delay ? 1:0, link? 1:0, op);
		return m_pc + size;
	}
	case 0x02:
	{
		int size = 4;
		fatalerror("unimplemented Jump (delay %d link %d) (u12 type) %08x", delay ? 1:0, link? 1:0, op);
		return m_pc + size;
	}
	case 0x03:
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00: return handleop32_Jcc_cc_f_b_b_c_helper(op, delay, link);
		case 0x01:
		{
			int size = 4;
			fatalerror("unimplemented Jump (delay %d link %d) (CC, u6 type) %08x", delay ? 1:0, link? 1:0, op);
			return m_pc + size;
		}
		}
		return 0;
	}
	return 0;
}
