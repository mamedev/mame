// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::arcompact_handle0f_00_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S [b]                         0111 1bbb 0000 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_J_S_b(uint16_t op)  { return arcompact_handle0f_00_0x_helper(op, "J_S"); }

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S.D [b]                       0111 1bbb 0010 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_J_S_D_b(uint16_t op)  { return arcompact_handle0f_00_0x_helper(op, "J_S.D"); }

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S [b]                        0111 1bbb 0100 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_JL_S_b(uint16_t op) // JL_S
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);

	return m_regs[breg];
}

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S.D [b]                      0111 1bbb 0110 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_JL_S_D_b(uint16_t op) // JL_S.D
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_delayactive = 1;
	m_delayjump = m_regs[breg];
	m_delaylinks = 1;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
// SUB_S.NE b,b,b                  0111 1bbb 1100 0000
//                                 IIII I    sssS SSSS
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_NE_b_b_b(uint16_t op)  { return arcompact_handle0f_00_0x_helper(op, "SUB_S.NE"); }
