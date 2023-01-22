// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle0c_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (0x0c group)", optext, op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I       S S
// LD_S a,[b,c]                    0110 0bbb ccc0 0aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_a_b_c(uint16_t op)
{
	return arcompact_handle0c_helper(op, "LD_S");
}

// #######################################################################################################################
//                                 IIII I       S S
// LDB_S a,[b,c]                   0110 0bbb ccc0 1aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_a_b_c(uint16_t op)
{
	return arcompact_handle0c_helper(op, "LDB_S");
}

// #######################################################################################################################
//                                 IIII I       S S
// LDW_S a,[b,c]                   0110 0bbb ccc1 0aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_a_b_c(uint16_t op)
{
	return arcompact_handle0c_helper(op, "LDW_S");
}

// #######################################################################################################################
//                                 IIII I       S S
// ADD_S a,b,c                     0110 0bbb ccc1 1aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_a_b_c(uint16_t op) // ADD_S a <- b + c
{
	int areg, breg, creg;

	COMMON16_GET_areg;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(areg);
	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	m_regs[areg] = m_regs[breg] + m_regs[creg];

	return m_pc + (2 >> 0);
}
