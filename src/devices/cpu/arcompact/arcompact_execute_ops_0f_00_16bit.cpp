// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S [b]                         0111 1bbb 0000 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_J_S_b(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	return m_regs[breg];
}

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S.D [b]                       0111 1bbb 0010 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_J_S_D_b(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	m_delayactive = true;
	m_delayjump = m_regs[breg];
	m_delaylinks = false;
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S [b]                        0111 1bbb 0100 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_JL_S_b(uint16_t op) // JL_S
{
	uint8_t breg = common16_get_and_expand_breg(op);
	m_regs[REG_BLINK] = m_pc + 2;
	return m_regs[breg];
}

// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S.D [b]                      0111 1bbb 0110 0000
// #######################################################################################################################

uint32_t arcompact_device::handleop_JL_S_D_b(uint16_t op) // JL_S.D
{
	uint8_t breg = common16_get_and_expand_breg(op);
	m_delayactive = true;
	m_delayjump = m_regs[breg];
	m_delaylinks = true;
	return m_pc + 2;
}

// #######################################################################################################################
// SUB_S.NE b,b,b                  0111 1bbb 1100 0000
//                                 IIII I    sssS SSSS
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_NE_b_b_b(uint16_t op)
{
	// opcode clears a register if 'z' is 0
	if (condition_NE())
	{
		uint8_t breg = common16_get_and_expand_breg(op);
		//m_regs[breg] = m_regs[breg] - m_regs[breg];
		m_regs[breg] = 0;
	}
	return m_pc + 2;
}
