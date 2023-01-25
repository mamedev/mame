// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

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
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);
	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
		return m_regs[LIMM_REG];
	}
	else
	{
		if (F)
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				return handle_jump_to_register(0,0,creg, m_pc + size, F); // delay, no link
			}
			else
			{
				// should not use .F unless jumping to ILINK1/2
				arcompact_fatal ("illegal 1 unimplemented J.F (F should not be set) %08x", op);
			}
		}
		else
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				// should only jumping to ILINK1/2 if .F is set
				arcompact_fatal("illegal 1 unimplemented J (F not set) %08x", op);
			}
			else
			{
				return m_regs[creg];
			}
		}
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_f_a_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);
	uint32_t c;

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
		c = m_regs[LIMM_REG];
	}
	else
	{
		c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + size;

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_fatal ("fatal handleop32_Jcc_cc_f_b_b_c J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			uint32_t realaddress = c;
			return realaddress;
		}
	}
	else
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented handleop32_Jcc_cc_f_b_b_c J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal handleop32_Jcc_cc_f_b_b_c J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);
		}
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_cc_f_b_b_u6 J %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;
	switch (M)
	{
		case 0x00: return handleop32_Jcc_cc_f_b_b_c(op);
		case 0x01: return handleop32_Jcc_cc_f_b_b_u6(op);
	}
	return 0;
}

uint32_t arcompact_device::handleop32_Jcc(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;
	switch (p)
	{
		case 0x00: return handleop32_Jcc_f_a_b_c(op);
		case 0x01: return handleop32_Jcc_f_a_b_u6(op);
		case 0x02: return handleop32_Jcc_f_b_b_s12(op);
		case 0x03: return handleop32_Jcc_cc(op);
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
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);
	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;

		handle_jump_to_addr(1,0,m_regs[LIMM_REG], m_pc + size);
	}
	else
	{
		return handle_jump_to_register(1,0,creg, m_pc + size, F);
	}

	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_f_a_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
	}

	if (!check_condition(condition))
		return m_pc + size;

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, illegal) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, unimplemented) %08x", op);
		}
	}
	else
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, unimplemented) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, illegal) %08x", op);
		}
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_D_cc_f_b_b_u6 J.D %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_Jcc_D_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;
	switch (M)
	{
		case 0x00: return handleop32_Jcc_D_cc_f_b_b_c(op);
		case 0x01: return handleop32_Jcc_D_cc_f_b_b_u6(op);
	}
	return 0;
}

uint32_t arcompact_device::handleop32_Jcc_D(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;
	switch (p)
	{
		case 0x00: return handleop32_Jcc_D_f_a_b_c(op);
		case 0x01: return handleop32_Jcc_D_f_a_b_u6(op);
		case 0x02: return handleop32_Jcc_D_f_b_b_s12(op);
		case 0x03: return handleop32_Jcc_D_cc(op);
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
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);
	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
		return m_regs[LIMM_REG];
	}
	else
	{
		if (F)
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				return handle_jump_to_register(0,1,creg, m_pc + size, F);
			}
			else
			{
				// should not use .F unless jumping to ILINK1/2
				arcompact_fatal ("illegal 1 unimplemented J.F (F should not be set) %08x", op);
			}
		}
		else
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				// should only jumping to ILINK1/2 if .F is set
				arcompact_fatal("illegal 1 unimplemented J (F not set) %08x", op);
			}
			else
			{
				m_regs[REG_BLINK] = m_pc + size;
				return m_regs[creg];
			}
		}
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_f_a_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);
	uint32_t c;

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
		c = m_regs[LIMM_REG];
	}
	else
	{
		c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + size;

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_fatal ("fatal handleop32_JLcc_cc_f_b_b_c J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			uint32_t realaddress = c;
			m_regs[REG_BLINK] = m_pc + size;
			return realaddress;
		}
	}
	else
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented handleop32_JLcc_cc_f_b_b_c J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal handleop32_JLcc_cc_f_b_b_c J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);
		}
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_JLcc_cc_f_b_b_u6 J %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;
	switch (M)
	{
		case 0x00: return handleop32_JLcc_cc_f_b_b_c(op);
		case 0x01: return handleop32_JLcc_cc_f_b_b_u6(op);
	}
	return 0;
}

uint32_t arcompact_device::handleop32_JLcc(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;
	switch (p)
	{
		case 0x00: return handleop32_JLcc_f_a_b_c(op);
		case 0x01: return handleop32_JLcc_f_a_b_u6(op);
		case 0x02: return handleop32_JLcc_f_b_b_s12(op);
		case 0x03: return handleop32_JLcc_cc(op);
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
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);
	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;

		handle_jump_to_addr(1,1,m_regs[LIMM_REG], m_pc + size);
	}
	else
	{
		return handle_jump_to_register(1,1,creg, m_pc + size, F);
	}

	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_f_a_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
	}

	if (!check_condition(condition))
		return m_pc + size;

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented JLcc.D (p11_m0 type, illegal) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented JLcc.D (p11_m0 type, unimplemented) %08x", op);
		}
	}
	else
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2
		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented JLcc.D.F (p11_m0 type, unimplemented) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented JLcc.D.F (p11_m0 type, illegal) %08x", op);
		}
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_JLcc_D_cc_f_b_b_u6 J.D %08x (u6)", op);
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_JLcc_D_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;
	switch (M)
	{
		case 0x00: return handleop32_JLcc_D_cc_f_b_b_c(op);
		case 0x01: return handleop32_JLcc_D_cc_f_b_b_u6(op);
	}
	return 0;
}

uint32_t arcompact_device::handleop32_JLcc_D(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;
	switch (p)
	{
		case 0x00: return handleop32_JLcc_D_f_a_b_c(op);
		case 0x01: return handleop32_JLcc_D_f_a_b_u6(op);
		case 0x02: return handleop32_JLcc_D_f_b_b_s12(op);
		case 0x03: return handleop32_JLcc_D_cc(op);
	}
	return 0;
}
