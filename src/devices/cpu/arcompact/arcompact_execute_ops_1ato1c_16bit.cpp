// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// #######################################################################################################################
//                                 IIII I
// LD_S b,[pcl,u10]                1101 0bbb uuuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_b_pcl_u10(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint32_t u = common16_get_u8(op);
	m_regs[breg] = READ32(m_regs[REG_PCL] + (u << 2));
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// MOV_S b,u8                      1101 1bbb uuuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_MOV_S_b_u8(uint16_t op) // MOV_S b, u8
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint32_t u = common16_get_u8(op);
	m_regs[breg] = u;
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    s
// ADD_S b,b,u7                    1110 0bbb 0uuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_b_b_u7(uint16_t op) // ADD_S b, b, u7
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint32_t u = common16_get_u7(op);
	m_regs[breg] = m_regs[breg] + u;
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    s
// CMP_S b,u7                      1110 0bbb 1uuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_CMP_S_b_u7(uint16_t op) // CMP b, u7
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint32_t u = common16_get_u7(op);
	// flag setting ALWAYS occurs on CMP operations, even 16-bit ones even without a .F opcode type
	uint32_t result = m_regs[breg] - u;
	do_flags_sub(result, m_regs[breg], u);
	return m_pc + 2;
}
