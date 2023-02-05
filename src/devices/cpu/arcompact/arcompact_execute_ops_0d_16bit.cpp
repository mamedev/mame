// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// #######################################################################################################################
// ADD_S c,b,u3                    0110 1bbb ccc0 0uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_c_b_u3(uint16_t op)
{
	uint32_t u = common16_get_u3(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t result = m_regs[breg] + u;
	m_regs[creg] = result;
	return m_pc + 2;
}

// #######################################################################################################################
// SUB_S c,b,u3                    0110 1bbb ccc0 1uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_c_b_u3(uint16_t op)
{
	uint32_t u = common16_get_u3(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t result = m_regs[breg] - u;
	m_regs[creg] = result;
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// ASL_S c,b,u3                    0110 1bbb ccc1 0uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASL_S_c_b_u3(uint16_t op)
{
	uint32_t u = common16_get_u3(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t result = m_regs[breg] << u;
	m_regs[creg] = result;
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// ASR_S c,b,u3                    0110 1bbb ccc1 1uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASR_S_c_b_u3(uint16_t op)
{
	uint32_t u = common16_get_u3(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t source = m_regs[breg];
	uint32_t result = source >> u;
	if (source & 0x80000000)
		result |= 0xffffffff << (31 - u);
	m_regs[creg] = result;
	return m_pc + 2;
}
