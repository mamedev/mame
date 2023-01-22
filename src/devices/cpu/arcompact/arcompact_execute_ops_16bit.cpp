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



















uint32_t arcompact_device::handleop_BREQ_S_b_0_s8(uint16_t op) // BREQ_S b,0,s8
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (!m_regs[breg])
	{
		int s = (op & 0x007f) >> 0; op &= ~0x007f;
		if (s & 0x40) s = -0x40 + (s & 0x3f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_BRNE_S_b_0_s8(uint16_t op) // BRNE_S b,0,s8
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (m_regs[breg])
	{
		int s = (op & 0x007f) >> 0; op &= ~0x007f;
		if (s & 0x40) s = -0x40 + (s & 0x3f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::arcompact_handle1e_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (1e_0x type)", optext, op);
	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop_B_S_s10(uint16_t op) // B_S s10  (branch always)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);
	uint32_t realaddress = PC_ALIGNED32 + (s * 2);
	//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
	return realaddress;
}

uint32_t arcompact_device::handleop_BEQ_S_s10(uint16_t op) // BEQ_S s10 (branch is zero bit is set)
{
	if (STATUS32_CHECK_Z)
	{
		int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
		if (s & 0x100) s = -0x100 + (s & 0xff);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BNE_S_s10(uint16_t op) // BNE_S s10  (branch if zero bit isn't set)
{
	if (!STATUS32_CHECK_Z)
	{
		int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
		if (s & 0x100) s = -0x100 + (s & 0xff);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::arcompact_handle1e_03_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BGT_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BGT_S"); }
uint32_t arcompact_device::handleop_BGE_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BGE_S"); }

uint32_t arcompact_device::handleop_BLT_S_s7(uint16_t op) // BLT_S
{
	if (CONDITION_LT)
	{
		int s = (op & 0x003f) >> 0; op &= ~0x003f;
		if (s & 0x020) s = -0x20 + (s & 0x1f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BLE_S_s7(uint16_t op) // BLE_S
{
	if (CONDITION_LE)
	{
		int s = (op & 0x003f) >> 0; op &= ~0x003f;
		if (s & 0x020) s = -0x20 + (s & 0x1f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BHI_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BHI_S"); }
uint32_t arcompact_device::handleop_BHS_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BHS_S"); }
uint32_t arcompact_device::handleop_BLO_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BLO_S"); }
uint32_t arcompact_device::handleop_BLS_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BLS_S"); }

uint32_t arcompact_device::handleop_BL_S_s13(uint16_t op) // BL_S s13
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	uint32_t realaddress = PC_ALIGNED32 + (s * 4);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);
	return realaddress;
}







// #######################################################################################################################
//                                 IIII I       S SSSS
// SUB_S b,b,c                     0111 1bbb ccc0 0010
// #######################################################################################################################


uint32_t arcompact_device::handleop_SUB_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

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


uint32_t arcompact_device::handleop_LSR_S_b_c_single(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "LSR1_S",0); }

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

