// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle0d_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (0x0d group)", optext, op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
// ADD_S c,b,u3                    0110 1bbb ccc0 0uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_c_b_u3(uint16_t op)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] + u;
	m_regs[creg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
// SUB_S c,b,u3                    0110 1bbb ccc0 1uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_c_b_u3(uint16_t op)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] - u;
	m_regs[creg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S S
// ASL_S c,b,u3                    0110 1bbb ccc1 0uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASL_S_c_b_u3(uint16_t op)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] << u;
	m_regs[creg] = result;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S S
// ASR_S c,b,u3                    0110 1bbb ccc1 1uuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ASR_S_c_b_u3(uint16_t op)
{
	return arcompact_handle0d_helper(op, "ASR_S");
}
