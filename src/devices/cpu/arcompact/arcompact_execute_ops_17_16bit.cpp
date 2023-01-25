// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

// #######################################################################################################################
//                                 IIII I    SSS
// ASL_S b,b,u5                    1011 1bbb 000u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASL_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	m_regs[breg] = m_regs[breg] << (u & 0x1f);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// LSR_S b,b,u5                    1011 1bbb 001u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LSR_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	m_regs[breg] = m_regs[breg] >> (u & 0x1f);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// ASR_S b,b,u5                    1011 1bbb 010u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASR_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	int32_t temp = (int32_t)m_regs[breg];
	m_regs[breg] = temp >> (u&0x1f); // treat it as a signed value, so sign extension occurs during shift
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// SUB_S b,b,u5                    1011 1bbb 011u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	m_regs[breg] = m_regs[breg] - u;
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// BSET_S b,b,u5                   1011 1bbb 100u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_BSET_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	m_regs[breg] = m_regs[breg] | (1 << (u & 0x1f));
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// BCLR_S b,b,u5                   1011 1bbb 101u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_BCLR_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	m_regs[breg] = m_regs[breg] &~ (1 << u);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// BMSK_S b,b,u5                   1011 1bbb 110u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_BMSK_S_b_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	m_regs[breg] = m_regs[breg] & ((1 << (u + 1)) - 1);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I    SSS
// BTST_S b,u5                     1011 1bbb 111u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_BTST_S_b_u5(uint16_t op)
{
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint32_t u = common16_get_u5(op);
	uint32_t result = m_regs[breg] & (1 << u);
	do_flags_nz(result);
	return m_pc + 2;
}
