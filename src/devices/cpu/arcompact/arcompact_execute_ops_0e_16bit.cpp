// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// #######################################################################################################################
//                                 IIII I       S S
// ADD_S b,b,h                     0111 0bbb hhh0 0HHH
// ADD_S b,b,limm                  0111 0bbb 1100 0111 (+ Limm)
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_b_b_h_or_limm(uint16_t op) // ADD_s b, b, h
{
	uint8_t h = group_0e_get_h(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	int size = check_limm16(h);
	m_regs[breg] = m_regs[breg] + m_regs[h];
	return m_pc + size;
}

// #######################################################################################################################
//                                 IIII I       S S
// MOV_S b,h                       0111 0bbb hhh0 1HHH
// MOV_S b,limm                    0111 0bbb 1100 1111 (+ Limm)
// #######################################################################################################################

// 16-bit MOV with extended register range
uint32_t arcompact_device::handleop_MOV_S_b_h_or_limm(uint16_t op) // MOV_S b <- h
{
	uint8_t h = group_0e_get_h(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	int size = check_limm16(h);
	m_regs[breg] = m_regs[h];
	return m_pc + size;
}

// #######################################################################################################################
//                                 IIII I       S S
// CMP_S b,h                       0111 0bbb hhh1 0HHH
// CMP_S b,limm                    0111 0bbb 1101 0111 (+ Limm)
// #######################################################################################################################

uint32_t arcompact_device::handleop_CMP_S_b_h_or_limm(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t h = group_0e_get_h(op);
	int size = check_limm16(h);
	// flag setting ALWAYS occurs on CMP operations, even 16-bit ones even without a .F opcode type
	uint32_t result = m_regs[breg] - m_regs[h];
	do_flags_sub(result, m_regs[breg], m_regs[h]);
	return m_pc + size;
}

// #######################################################################################################################
//                                 IIII I       S S
// MOV_S h,b                       0111 0bbb hhh1 1HHH
// #######################################################################################################################

uint32_t arcompact_device::handleop_MOV_S_h_b(uint16_t op) // MOV_S h <- b
{
	uint8_t h = group_0e_get_h(op);
	uint8_t breg = common16_get_and_expand_breg(op);
	m_regs[h] = m_regs[breg];
	return m_pc + 2;
}
