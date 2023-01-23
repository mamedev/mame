// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle0e_0x_helper(uint16_t op, const char* optext, int revop)
{
	int h;// , breg;
	int size;

	h = group_0e_get_h(op);

	if (h == LIMM_REG)
	{
		//uint32_t limm;
		//GET_LIMM;
		size = 6;
	}
	else
	{
	}

	arcompact_log("unimplemented %s %04x (0x0e_0x group)", optext, op);

	return m_pc+ (size>>0);

}

// #######################################################################################################################
//                                 IIII I       S S
// ADD_S b,b,h                     0111 0bbb hhh0 0HHH
// ADD_S b,b,limm                  0111 0bbb 1100 0111 (+ Limm)
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_b_b_h_or_limm(uint16_t op) // ADD_s b, b, h
{
	int h,breg;
	int size = 2;

	h = group_0e_get_h(op);
	breg = common16_get_breg(op);
	breg = expand_reg(breg);

	if (h == LIMM_REG)
	{
		get_limm_16bit_opcode();
		size = 6;
	}

	m_regs[breg] = m_regs[breg] + m_regs[h];

	return m_pc+ (size>>0);
}

// #######################################################################################################################
//                                 IIII I       S S
// MOV_S b,h                       0111 0bbb hhh0 1HHH
// MOV_S b,limm                    0111 0bbb 1100 1111 (+ Limm)
// #######################################################################################################################

// 16-bit MOV with extended register range
uint32_t arcompact_device::handleop_MOV_S_b_h_or_limm(uint16_t op) // MOV_S b <- h
{
	int h,breg;
	int size = 2;

	h = group_0e_get_h(op);
	breg = common16_get_breg(op);
	breg = expand_reg(breg);

	if (h == LIMM_REG)
	{
		// opcode        iiii ibbb hhhI Ihhh
		// MOV_S b, limm 0111 0bbb 1100 1111 [LIMM]   (h == LIMM)
		get_limm_16bit_opcode();
		size = 6;

		m_regs[breg] = m_regs[h];

	}
	else
	{
		// opcode        iiii ibbb hhhI Ihhh
		// MOV_S b,h     0111 0bbb hhh0 1HHH
		m_regs[breg] = m_regs[h];
	}

	return m_pc+ (size>>0);
}

// #######################################################################################################################
//                                 IIII I       S S
// CMP_S b,h                       0111 0bbb hhh1 0HHH
// CMP_S b,limm                    0111 0bbb 1101 0111 (+ Limm)
// #######################################################################################################################

uint32_t arcompact_device::handleop_CMP_S_b_h_or_limm(uint16_t op)
{
	return arcompact_handle0e_0x_helper(op, "CMP_S", 0);
}

// #######################################################################################################################
//                                 IIII I       S S
// MOV_S hob                       0111 0bbb hhh1 1HHH
// #######################################################################################################################

uint32_t arcompact_device::handleop_MOV_S_hob(uint16_t op) // MOV_S h <- b
{
	int h,breg;
	int size = 2;

	h = group_0e_get_h(op);
	breg = common16_get_breg(op);
	breg = expand_reg(breg);

	if (h == LIMM_REG) // no result..
	{
	}

	m_regs[h] = m_regs[breg];

	return m_pc+ (size>>0);
}
