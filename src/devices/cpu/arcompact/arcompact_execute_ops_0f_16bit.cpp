// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"



uint32_t arcompact_device::arcompact_handle0f_0x_helper(uint16_t op, const char* optext, int nodst)
{
	arcompact_log("unimplemented %s %04x (0xf_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// SUB_S b,b,c                     0111 1bbb ccc0 0010
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] - m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// AND_S b,b,c                     0111 1bbb ccc0 0100
// #######################################################################################################################

uint32_t arcompact_device::handleop_AND_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] & m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// OR_S b,b,c                      0111 1bbb ccc0 0101
// #######################################################################################################################

uint32_t arcompact_device::handleop_OR_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] | m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// BIC_S b,b,c                     0111 1bbb ccc0 0110
// #######################################################################################################################

uint32_t arcompact_device::handleop_BIC_S_b_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "BIC_S",0);  }

// #######################################################################################################################
//                                 IIII I       S SSSS
// XOR_S b,b,c                     0111 1bbb ccc0 0111
// #######################################################################################################################

uint32_t arcompact_device::handleop_XOR_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] ^ m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// TST_S b,c                       0111 1bbb ccc0 1011
// #######################################################################################################################

uint32_t arcompact_device::handleop_TST_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "TST_S",1);  }

// #######################################################################################################################
//                                 IIII I       S SSSS
// MUL64_S <0,>b,c                 0111 1bbb ccc0 1100
// #######################################################################################################################

uint32_t arcompact_device::handleop_MUL64_S_0_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "MUL64_S",2);  } // actual destination is special multiply registers

// #######################################################################################################################
//                                 IIII I       S SSSS
// SEXB_S b,c                      0111 1bbb ccc0 1101
// #######################################################################################################################

uint32_t arcompact_device::handleop_SEXB_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "SEXB_S",0); }

// #######################################################################################################################
//                                 IIII I       S SSSS
// SEXW_S b,c                      0111 1bbb ccc0 1110
// #######################################################################################################################

uint32_t arcompact_device::handleop_SEXW_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "SEXW_S",0); }

// #######################################################################################################################
//                                 IIII I       S SSSS
// EXTB_S b,c                      0111 1bbb ccc0 1111
// #######################################################################################################################

uint32_t arcompact_device::handleop_EXTB_S_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] & 0x000000ff;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// EXTW_S b,c                      0111 1bbb ccc1 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_EXTW_S_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] & 0x0000ffff;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ABS_S b,c                       0111 1bbb ccc1 0001
// #######################################################################################################################

uint32_t arcompact_device::handleop_ABS_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ABS_S",0);  }

// #######################################################################################################################
//                                 IIII I       S SSSS
// NOT_S b,c                       0111 1bbb ccc1 0010
// #######################################################################################################################

uint32_t arcompact_device::handleop_NOT_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "NOT_S",0);  }

// #######################################################################################################################
//                                 IIII I       S SSSS
// NEG_S b,c                       0111 1bbb ccc1 0011
// #######################################################################################################################

uint32_t arcompact_device::handleop_NEG_S_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = 0 - m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD1_S b,b,c                    0111 1bbb ccc1 0100
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD1_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<1);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD2_S b,b,c                    0111 1bbb ccc1 0101
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD2_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<2);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD3_S b,b,c                    0111 1bbb ccc1 0110
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD3_S_b_b_c(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<3);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ASL_S b,b,c                     0111 1bbb ccc1 1000
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASL_S_b_b_c_multiple(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ASL_S",0);  }

// #######################################################################################################################
//                                 IIII I       S SSSS
// LSR_S b,b,c                     0111 1bbb ccc1 1001
// #######################################################################################################################

uint32_t arcompact_device::handleop_LSR_S_b_b_c_multiple(uint16_t op)
{
	int breg, creg;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] >> (m_regs[creg]&0x1f);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ASR_S b,b,c                     0111 1bbb ccc1 1010
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASR_S_b_b_c_multiple(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ASR_S",0);  }

// #######################################################################################################################
//                                 IIII I       S SSSS
// ASL_S b,c                       0111 1bbb ccc1 1011
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASL_S_b_c_single(uint16_t op)
{
	int breg = common16_get_breg(op);
	int creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] << 1;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// ASR_S b,c                       0111 1bbb ccc1 1100
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASR_S_b_c_single(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ASR1_S",0); }

// #######################################################################################################################
//                                 IIII I       S SSSS
// LSR_S b,c                       0111 1bbb ccc1 1101
// #######################################################################################################################

uint32_t arcompact_device::handleop_LSR_S_b_c_single(uint16_t op)
{
	int breg = common16_get_breg(op);
	int creg = common16_get_creg(op);

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] >> 1;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S SSSS
// TRAP_S u6                       0111 1uuu uuu1 1110
// #######################################################################################################################

uint32_t arcompact_device::handleop_TRAP_S_u6(uint16_t op)  // special
{
	arcompact_log("unimplemented TRAP_S %04x",  op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII Isss sssS SSSS
// BRK_S                           0111 1111 1111 1111
// #######################################################################################################################

uint32_t arcompact_device::handleop_BRK_S(uint16_t op)  // special
{
	arcompact_log("unimplemented BRK_S %04x",  op);
	return m_pc + (2 >> 0);
}
