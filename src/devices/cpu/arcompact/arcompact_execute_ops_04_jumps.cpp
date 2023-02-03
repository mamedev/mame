// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

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
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	bool F = common32_get_F(op);
	if (creg == REG_LIMM)
	{
		get_limm_32bit_opcode();
		size = 8;
	}

	if (delay)
	{
		return handle_jump_to_register(delay, link, creg, m_pc + size, F);
	}
	else
	{
		if (F)
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				return handle_jump_to_register(delay, link, creg, m_pc + size, F);
			}
			else
			{
				// should not use .F unless jumping to ILINK1/2
				fatalerror("illegal unimplemented J(L)(.D).F (F should not be set) %08x", op);
			}
		}
		else
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				// should only jumping to ILINK1/2 if .F is set
				fatalerror("illegal unimplemented J(L)(.D) (F not set) %08x", op);
			}
			else
			{
				if (link)
					m_regs[REG_BLINK] = m_pc + size;

				return m_regs[creg];
			}
		}
	}

	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_c_helper(uint32_t op, bool delay, bool link)
{
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	bool F = common32_get_F(op);
	uint32_t c;

	if (creg == REG_LIMM)
	{
		get_limm_32bit_opcode();
		size = 8;
	}

	c = m_regs[creg];

	if (!check_condition(condition))
		return m_pc + size;

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			fatalerror("fatal handleop32_J(L)cc_cc_(.D)f_b_b_c J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			if (delay)
			{
				fatalerror("unimplemented J(L)cc.D (p11_m0 type, unimplemented) %08x", op);
			}
			else
			{
				if (link)
					m_regs[REG_BLINK] = m_pc + size;

				uint32_t realaddress = c;
				return realaddress;
			}
		}
	}
	else
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			fatalerror("unimplemented handleop32_J(L)cc_(.D)cc_f_b_b_c J %08x (F set)", op);
		}
		else
		{
			if (delay)
			{
				fatalerror("unimplemented J(L)cc.D.F (p11_m0 type, illegal) %08x", op);
			}
			else
			{
				fatalerror("fatal handleop32_J(L)cc_cc_f_b_b_c J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);
			}
		}
	}
	return m_pc + size;
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

uint32_t arcompact_device::handleop32_Jcc_f_a_b_c(uint32_t op)
{
	return handleop32_Jcc_f_a_b_c_helper(op, false, false);
}

uint32_t arcompact_device::handleop32_Jcc_f_a_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented J %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_f_b_b_s12(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented J %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_c(uint32_t op)
{
	return handleop32_Jcc_cc_f_b_b_c_helper(op, false, false);
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented handleop32_Jcc_cc_f_b_b_u6 J %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00: return handleop32_Jcc_f_a_b_c(op);
	case 0x01: return handleop32_Jcc_f_a_b_u6(op);
	case 0x02: return handleop32_Jcc_f_b_b_s12(op);
	case 0x03:
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00: return handleop32_Jcc_cc_f_b_b_c(op);
		case 0x01: return handleop32_Jcc_cc_f_b_b_u6(op);
		}
		return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc.D u6                        0010 0RRR 1110 0001   0RRR uuuu uu1Q QQQQ
// Jcc.D [c]                       0010 0RRR 1110 0001   0RRR CCCC CC0Q QQQQ
//
// J.D [c]                         0010 0RRR 0010 0001   0RRR CCCC CCRR RRRR
// J.D u6                          0010 0RRR 0110 0001   0RRR uuuu uuRR RRRR
// J.D s12                         0010 0RRR 1010 0001   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_Jcc_D_f_a_b_c(uint32_t op)
{
	return handleop32_Jcc_f_a_b_c_helper(op, true, false);
}

uint32_t arcompact_device::handleop32_Jcc_D_f_a_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented J.D (u6 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_f_b_b_s12(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented J.D (s12 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_cc_f_b_b_c(uint32_t op)
{
	return handleop32_Jcc_cc_f_b_b_c_helper(op, true, false);
}

uint32_t arcompact_device::handleop32_Jcc_D_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented handleop32_Jcc_D_cc_f_b_b_u6 J.D %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00: return handleop32_Jcc_D_f_a_b_c(op);
	case 0x01: return handleop32_Jcc_D_f_a_b_u6(op);
	case 0x02: return handleop32_Jcc_D_f_b_b_s12(op);
	case 0x03:
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00: return handleop32_Jcc_D_cc_f_b_b_c(op);
		case 0x01: return handleop32_Jcc_D_cc_f_b_b_u6(op);
		}
		return 0;
	}
	return 0;
}

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

uint32_t arcompact_device::handleop32_JLcc_f_a_b_c(uint32_t op)
{
	return handleop32_Jcc_f_a_b_c_helper(op, false, true);
}

uint32_t arcompact_device::handleop32_JLcc_f_a_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented JL %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_f_b_b_s12(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented JL %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_cc_f_b_b_c(uint32_t op)
{
	return handleop32_Jcc_cc_f_b_b_c_helper(op, false, true);
}

uint32_t arcompact_device::handleop32_JLcc_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented handleop32_JLcc_cc_f_b_b_u6 JL %08x (u6)", op);
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_JLcc(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00: return handleop32_JLcc_f_a_b_c(op);
	case 0x01: return handleop32_JLcc_f_a_b_u6(op);
	case 0x02: return handleop32_JLcc_f_b_b_s12(op);
	case 0x03:
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00: return handleop32_JLcc_cc_f_b_b_c(op);
		case 0x01: return handleop32_JLcc_cc_f_b_b_u6(op);
		}
		return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc.D u6                       0010 0RRR 1110 0011   0RRR uuuu uu1Q QQQQ
// JLcc.D [c]                      0010 0RRR 1110 0011   0RRR CCCC CC0Q QQQQ
// JL.D [c]                        0010 0RRR 0010 0011   0RRR CCCC CCRR RRRR
// JL.D u6                         0010 0RRR 0110 0011   0RRR uuuu uuRR RRRR
// JL.D s12                        0010 0RRR 1010 0011   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_JLcc_D_f_a_b_c(uint32_t op)
{
	return handleop32_Jcc_f_a_b_c_helper(op, true, true);
}

uint32_t arcompact_device::handleop32_JLcc_D_f_a_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented JL.D (u6 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_f_b_b_s12(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented JL.D (s12 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_cc_f_b_b_c(uint32_t op)
{
	return handleop32_Jcc_cc_f_b_b_c_helper(op, true, true);
}

uint32_t arcompact_device::handleop32_JLcc_D_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	fatalerror("unimplemented handleop32_JLcc_D_cc_f_b_b_u6 JL.D %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00: return handleop32_JLcc_D_f_a_b_c(op);
	case 0x01: return handleop32_JLcc_D_f_a_b_u6(op);
	case 0x02: return handleop32_JLcc_D_f_b_b_s12(op);
	case 0x03:
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00: return handleop32_JLcc_D_cc_f_b_b_c(op);
		case 0x01: return handleop32_JLcc_D_cc_f_b_b_u6(op);
		}
		return 0;
	}
	return 0;
}
