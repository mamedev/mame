// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// #######################################################################################################################
//                                 IIII I
// LD_S c,[b,u7]                   1000 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_c_b_u7(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op) << 2;
	m_regs[creg] = READ32((m_regs[breg] + u));
	return m_pc + 2;
}
// #######################################################################################################################
//                                 IIII I
// LDB_S c,[b,u5]                  1000 1bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_c_b_u5(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op);
	m_regs[creg] = READ8((m_regs[breg] + u));
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// LDW_S c,[b,u6]                  1001 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_c_b_u6(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op) << 1;
	m_regs[creg] = READ16((m_regs[breg] + u));
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// LDW_S.X c,[b,u6]                1001 1bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_X_c_b_u6(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op) << 1;
	m_regs[creg] = READ16((m_regs[breg] + u));
	m_regs[creg] = util::sext(m_regs[creg] & 0xffff, 16);

	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// ST_S c,[b,u7]                   1010 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_ST_S_c_b_u7(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op) << 2;
	WRITE32((m_regs[breg] + u), m_regs[creg]);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// STB_S c,[b,u5]                  1010 1bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_STB_S_c_b_u5(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op);
	WRITE8((m_regs[breg] + u), m_regs[creg]);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII I
// STW_S c,[b,u6]                  1011 0bbb cccu uuuu
// #######################################################################################################################

uint32_t arcompact_device::handleop_STW_S_c_b_u6(uint16_t op)
{
	uint8_t breg = common16_get_and_expand_breg(op);
	uint8_t creg = common16_get_and_expand_creg(op);
	uint32_t u = common16_get_u5(op) << 1;
	WRITE16((m_regs[breg] + u), m_regs[creg]);
	return m_pc + 2;
}
