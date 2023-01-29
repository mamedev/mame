// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

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
	m_delayactive = 1;
	m_delayjump = m_regs[breg];
	m_delaylinks = 0;
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
	m_delayactive = 1;
	m_delayjump = m_regs[breg];
	m_delaylinks = 1;
	return m_pc + 2;
}

// #######################################################################################################################
// SUB_S.NE b,b,b                  0111 1bbb 1100 0000
//                                 IIII I    sssS SSSS
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_NE_b_b_b(uint16_t op)
{
	arcompact_log("unimplemented SUB_S.NE %04x", op);
	return m_pc + 2;
}
