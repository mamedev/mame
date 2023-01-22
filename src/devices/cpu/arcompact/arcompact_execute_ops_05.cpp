// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"



uint32_t arcompact_device::handleop32_ASL_multiple_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ASL_multiple_cc_f_b_b_c(op);
		case 0x01: return handleop32_ASL_multiple_cc_f_b_b_u6(op);
	}

	return 0;
}


uint32_t arcompact_device::handleop32_LSR_multiple_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_LSR_multiple_cc_f_b_b_c(op);
		case 0x01: return handleop32_LSR_multiple_cc_f_b_b_u6(op);
	}

	return 0;
}





uint32_t arcompact_device::handleop32_ASL_multiple_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b << (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b << (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);


	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b << (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ASL_multiple_cc_f_b_b_c (ASL)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_cc_f_b_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);


	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);


	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint8_t condition = common32_get_condition(op);
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b << (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);


	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LSR_multiple_cc_f_b_b_c (LSR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_cc_f_b_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);


	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint8_t condition = common32_get_condition(op);
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_LSR_multiple_f_a_b_c(op);
		case 0x01: return handleop32_LSR_multiple_f_a_b_u6(op);
		case 0x02: return handleop32_LSR_multiple_f_b_b_s12(op);
		case 0x03: return handleop32_LSR_multiple_cc(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ASL_multiple(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ASL_multiple_f_a_b_c(op);
		case 0x01: return handleop32_ASL_multiple_f_a_b_u6(op);
		case 0x02: return handleop32_ASL_multiple_f_b_b_s12(op);
		case 0x03: return handleop32_ASL_multiple_cc(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ASLS(uint32_t op)  { return arcompact_handle04_helper(op, "ASLS", 0,0); }
uint32_t arcompact_device::handleop32_ASRS(uint32_t op)  { return arcompact_handle04_helper(op, "ASRS", 0,0); }

uint32_t arcompact_device::handleop32_ADDSDW(uint32_t op)  { return arcompact_handle04_helper(op, "ADDSDW", 0,0); }
uint32_t arcompact_device::handleop32_SUBSDW(uint32_t op)  { return arcompact_handle04_helper(op, "SUBSDW", 0,0); }

uint32_t arcompact_device::handleop32_ASR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ASR", 0,0); }
uint32_t arcompact_device::handleop32_ROR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ROR", 0,0); }
uint32_t arcompact_device::handleop32_MUL64(uint32_t op)  { return arcompact_handle04_helper(op, "MUL64", 2,0); } // special
uint32_t arcompact_device::handleop32_MULU64(uint32_t op)  { return arcompact_handle04_helper(op, "MULU64", 2,0);} // special
uint32_t arcompact_device::handleop32_ADDS(uint32_t op)  { return arcompact_handle04_helper(op, "ADDS", 0,0); }
uint32_t arcompact_device::handleop32_SUBS(uint32_t op)  { return arcompact_handle04_helper(op, "SUBS", 0,0); }
uint32_t arcompact_device::handleop32_DIVAW(uint32_t op)  { return arcompact_handle04_helper(op, "DIVAW", 0,0); }

