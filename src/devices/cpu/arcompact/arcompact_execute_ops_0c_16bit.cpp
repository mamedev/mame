// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

// #######################################################################################################################
//                                 IIII I       S S
// LD_S a,[b,c]                    0110 0bbb ccc0 0aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_a_b_c(uint16_t op)
{
	arcompact_log("unimplemented LD_S %04x (0x0c group)", op);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// LDB_S a,[b,c]                   0110 0bbb ccc0 1aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_a_b_c(uint16_t op)
{
	arcompact_log("unimplemented LDB_S %04x (0x0c group)", op);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// LDW_S a,[b,c]                   0110 0bbb ccc1 0aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_a_b_c(uint16_t op)
{
	arcompact_log("unimplemented LDW_S %04x (0x0c group)", op);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I       S S
// ADD_S a,b,c                     0110 0bbb ccc1 1aaa
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_a_b_c(uint16_t op) // ADD_S a <- b + c
{
	uint8_t areg = expand_reg(common16_get_areg(op));
	uint8_t breg = expand_reg(common16_get_breg(op));
	uint8_t creg = expand_reg(common16_get_creg(op));
	m_regs[areg] = m_regs[breg] + m_regs[creg];
	return m_pc + 2;
}
