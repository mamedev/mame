// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::arcompact_handle18_0x_helper(uint16_t op, const char* optext, int st)
{
	arcompact_log("unimplemented %s %04x (0x18_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    SSS
// LD_S b,[sp,u7]                  1100 0bbb 000u uuuu
// #######################################################################################################################


uint32_t arcompact_device::handleop_LD_S_b_sp_u7(uint16_t op)   // LD_S b, [SP, u7]
{
	int breg;
	uint32_t u;

	breg = common16_get_breg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);

	uint32_t address = m_regs[REG_SP] + (u << 2);

	m_regs[breg] = READ32(address);

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    SSS
// LDB_S b,[sp,u7]                 1100 0bbb 001u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_b_sp_u7(uint16_t op)
{
	return arcompact_handle18_0x_helper(op, "LDB_S (SP)", 0);
}

// #######################################################################################################################
//                                 IIII I    SSS
// ST_S b,[sp,u7]                  1100 0bbb 010u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ST_S_b_sp_u7(uint16_t op)  // ST_S b, [SP, u7]
{
	int breg;
	uint32_t u;

	breg = common16_get_breg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);

	uint32_t address = m_regs[REG_SP] + (u << 2);

	WRITE32(address, m_regs[breg]);

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    SSS
// STB_S b,[sp,u7]                 1100 0bbb 011u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_STB_S_b_sp_u7(uint16_t op)
{
	return arcompact_handle18_0x_helper(op, "STB_S (SP)", 1);
}

// #######################################################################################################################
//                                 IIII I    SSS
// ADD_S b,sp,u7                   1100 0bbb 100u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_b_sp_u7(uint16_t op)  // ADD_S b, SP, u7
{
	int breg;
	uint32_t u;

	breg = common16_get_breg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);

	m_regs[breg] = m_regs[REG_SP] + (u << 2);

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII Isss SSS
// ADD_S sp,sp,u7                  1100 0000 101u uuuu
// #######################################################################################################################

// op bits remaining for 0x18_05_xx subgroups 0x001f
uint32_t arcompact_device::handleop_ADD_S_sp_sp_u7(uint16_t op)
{
	int u;
	u = common16_get_u5(op);

	m_regs[REG_SP] = m_regs[REG_SP] + (u << 2);

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII Isss SSS
// SUB_S sp,sp,u7                  1100 0001 101u uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_SUB_S_sp_sp_u7(uint16_t op)
{
	int u;
	u = common16_get_u5(op);

	m_regs[REG_SP] = m_regs[REG_SP] - (u << 2);

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    SSSs ssss
// POP_S b                         1100 0bbb 1100 0001
// #######################################################################################################################

// op bits remaining for 0x18_06_xx subgroups 0x0700
uint32_t arcompact_device::handleop_POP_S_b(uint16_t op) // POP_S b
{
	int breg;
	breg = common16_get_breg(op);
	breg = expand_reg(breg);

	m_regs[breg] = READ32(m_regs[REG_SP]);
	m_regs[REG_SP] += 4;

	return m_pc + (2 >> 0);
}


// #######################################################################################################################
//                                 IIII I    SSSs ssss
// POP_S blink                     1100 0RRR 1101 0001
// #######################################################################################################################

uint32_t arcompact_device::handleop_POP_S_blink(uint16_t op) // POP_S blink
{
	// breg bits are reserved
	m_regs[REG_BLINK] = READ32(m_regs[REG_SP]);
	m_regs[REG_SP] += 4;

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    SSSs ssss
// PUSH_S b                        1100 0bbb 1110 0001
// #######################################################################################################################

// op bits remaining for 0x18_07_xx subgroups 0x0700
uint32_t arcompact_device::handleop_PUSH_S_b(uint16_t op) // PUSH_S b
{
	int breg;
	breg = common16_get_breg(op);
	breg = expand_reg(breg);

	m_regs[REG_SP] -= 4;

	WRITE32(m_regs[REG_SP], m_regs[breg]);

	return m_pc + (2 >> 0);
}

// #######################################################################################################################
//                                 IIII I    SSSs ssss
// PUSH_S blink                    1100 0RRR 1111 0001
// #######################################################################################################################


uint32_t arcompact_device::handleop_PUSH_S_blink(uint16_t op) // PUSH_S [blink]
{
	// breg bits are reserved

	m_regs[REG_SP] -= 4;

	WRITE32(m_regs[REG_SP], m_regs[REG_BLINK]);

	return m_pc + (2 >> 0);
}
