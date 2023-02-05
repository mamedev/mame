// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// #######################################################################################################################
//                                 IIII I       S S
// LD_S a,[b,c]                    0110 0bbb ccc0 0aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_a_b_c(uint16_t op)
{
	uint8_t areg = common16_get_and_expand_areg(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	m_regs[areg] = READ32(m_regs[breg] + m_regs[creg]);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// LDB_S a,[b,c]                   0110 0bbb ccc0 1aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_a_b_c(uint16_t op)
{
	uint8_t areg = common16_get_and_expand_areg(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	m_regs[areg] = READ8(m_regs[breg] + m_regs[creg]);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// LDW_S a,[b,c]                   0110 0bbb ccc1 0aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_a_b_c(uint16_t op)
{
	uint8_t areg = common16_get_and_expand_areg(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	m_regs[areg] = READ16(m_regs[breg] + m_regs[creg]);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// ADD_S a,b,c                     0110 0bbb ccc1 1aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_a_b_c(uint16_t op) // ADD_S a <- b + c
{
	uint8_t areg = common16_get_and_expand_areg(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	m_regs[areg] = m_regs[breg] + m_regs[creg];
	return m_pc + 2;
}
