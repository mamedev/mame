// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


// #######################################################################################################################
//                                 IIII I
// LD_S b,[pcl,u10]                1101 0bbb uuuu uuuu
// #######################################################################################################################


uint32_t arcompact_device::handleop_LD_S_b_pcl_u10(uint16_t op)
{
	arcompact_log("unimplemented MOV_S x, [PCL, x] %04x",  op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I
// MOV_S b,u8                      1101 1bbb uuuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_MOV_S_b_u8(uint16_t op) // MOV_S b, u8
{
	int breg;
	uint32_t u;
	breg = common16_get_breg(op);
	u = common16_get_u8(op);
	breg = expand_reg(breg);

	m_regs[breg] = u;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    s
// ADD_S b,b,u7                    1110 0bbb 0uuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_b_b_u7(uint16_t op) // ADD_S b, b, u7
{
	int breg;
	uint32_t u;
	breg = common16_get_breg(op);
	u = common16_get_u7(op);
	breg = expand_reg(breg);

	m_regs[breg] = m_regs[breg] + u;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    s
// CMP_S b,u7                      1110 0bbb 1uuu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_CMP_S_b_u7(uint16_t op) // CMP b, u7
{
	int breg;
	uint32_t u;
	breg = common16_get_breg(op);
	u = common16_get_u7(op);
	breg = expand_reg(breg);

	// flag setting ALWAYS occurs on CMP operations, even 16-bit ones even without a .F opcode type

	// TODO: verify this flag setting logic

	// unsigned checks
	if (m_regs[breg] == u)
	{
		STATUS32_SET_Z;
	}
	else
	{
		STATUS32_CLEAR_Z;
	}

	if (m_regs[breg] < u)
	{
		STATUS32_SET_C;
	}
	else
	{
		STATUS32_CLEAR_C;
	}
	// signed checks
	int32_t temp = (int32_t)m_regs[breg] - (int32_t)u;

	if (temp < 0)
	{
		STATUS32_SET_N;
	}
	else
	{
		STATUS32_CLEAR_N;
	}

	// if signs of source values don't match, and sign of result doesn't match the first source value, then we've overflowed?
	if ((m_regs[breg] & 0x80000000) != (u & 0x80000000))
	{
		if ((m_regs[breg] & 0x80000000) != (temp & 0x80000000))
		{
			STATUS32_SET_V;
		}
		else
		{
			STATUS32_CLEAR_V;
		}
	}

	// only sets flags, no result written

	return m_pc + (2 >> 0);
}
