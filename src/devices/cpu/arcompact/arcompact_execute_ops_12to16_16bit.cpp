// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::arcompact_handle_ld_helper(uint16_t op, const char* optext, int shift, int swap)
{
	arcompact_log("unimplemented %s %04x (ld/st group %d %d)", optext, op, shift, swap);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// LD_S c,[b,u7]                   1000 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_c_b_u7(uint16_t op)
{ // LD_S c, [b, u7]
	int breg, creg, u;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	u <<= 2; // check
	m_regs[creg] = READ32((m_regs[breg] + u));

	return m_pc + 2;
}
// #######################################################################################################################
//                                 IIII I
// LDB_S c,[b,u5]                  1000 1bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_c_b_u5(uint16_t op)
{
	// LDB_S c, [b, u5]
	int breg, creg, u;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	m_regs[creg] = READ8((m_regs[breg] + u));

	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// LDW_S c,[b,u6]                  1001 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_c_b_u6(uint16_t op)
{
	// LDB_W c, [b, u6]
	int breg, creg, u;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	u <<= 1;
	m_regs[creg] = READ16((m_regs[breg] + u));

	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// LDW_S.X c,[b,u6]                1001 1bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_X_c_b_u6(uint16_t op)
{
	return arcompact_handle_ld_helper(op, "LDW_S.X", 1, 0);
}

// #######################################################################################################################
//                                 IIII I
// ST_S c,[b,u7]                   1010 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ST_S_c_b_u7(uint16_t op) // ST_S c, [b, u7]
{
	int breg, creg, u;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	u <<= 2;

	WRITE32((m_regs[breg] + u), m_regs[creg]);

	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// STB_S c,[b,u5]                  1010 1bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_STB_S_c_b_u5(uint16_t op) // STB_S c. [b, u6]
{
	int breg, creg, u;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

//  u <<= 0;

	WRITE8((m_regs[breg] + u) >> 0, m_regs[creg]);

	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// STW_S c,[b,u6]                  1011 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_STW_S_c_b_u6(uint16_t op) // STW_S c. [b, u6]
{
	int breg, creg, u;

	breg = common16_get_breg(op);
	creg = common16_get_creg(op);
	u = common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	u <<= 1;

	WRITE16((m_regs[breg] + u), m_regs[creg]);

	return m_pc + 2;

}
