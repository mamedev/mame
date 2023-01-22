// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"





// #######################################################################################################################
//                                 IIII I    s
// BREQ_S b,0,s8                   1110 1bbb 0sss ssss
// #######################################################################################################################


uint32_t arcompact_device::handleop_BREQ_S_b_0_s8(uint16_t op) // BREQ_S b,0,s8
{
	int breg;
	breg = common16_get_breg(op);
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

// #######################################################################################################################
//                                 IIII I    s
// BRNE_S b,0,s8                   1110 1bbb 1sss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BRNE_S_b_0_s8(uint16_t op) // BRNE_S b,0,s8
{
	int breg;
	breg = common16_get_breg(op);
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

// #######################################################################################################################
//                                 IIII ISS
// B_S s10                         1111 000s ssss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_B_S_s10(uint16_t op) // B_S s10  (branch always)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);
	uint32_t realaddress = PC_ALIGNED32 + (s * 2);
	//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
	return realaddress;
}

// #######################################################################################################################
//                                 IIII ISS
// BEQ_S s10                       1111 001s ssss ssss
// #######################################################################################################################

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

// #######################################################################################################################
//                                 IIII ISS
// BNE_S s10                       1111 010s ssss ssss
// #######################################################################################################################

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

// #######################################################################################################################
//                                 IIII ISSs ss
// BGT_S s7                        1111 0110 00ss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BGT_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BGT_S"); }

// #######################################################################################################################
//                                 IIII ISSs ss
// BGE_S s7                        1111 0110 01ss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BGE_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BGE_S"); }

// #######################################################################################################################
//                                 IIII ISSs ss
// BLT_S s7                        1111 0110 10ss ssss
// #######################################################################################################################

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

// #######################################################################################################################
//                                 IIII ISSs ss
// BLE_S s7                        1111 0110 11ss ssss
// #######################################################################################################################

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

// #######################################################################################################################
//                                 IIII ISSs ss
// BHI_S s7                        1111 0111 00ss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BHI_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BHI_S"); }

// #######################################################################################################################
//                                 IIII ISSs ss
// BHS_S s7                        1111 0111 01ss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BHS_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BHS_S"); }

// #######################################################################################################################
//                                 IIII ISSs ss
// BLO_S s7                        1111 0111 10ss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BLO_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BLO_S"); }

// #######################################################################################################################
//                                 IIII ISSs ss
// BLS_S s7                        1111 0111 11ss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BLS_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BLS_S"); }

// #######################################################################################################################
//                                 IIII I
// BL_S s13                        1111 1sss ssss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_BL_S_s13(uint16_t op) // BL_S s13
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	uint32_t realaddress = PC_ALIGNED32 + (s * 4);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);
	return realaddress;
}
