// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"



uint32_t arcompact_device::handleop32_ASL_multiple_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ASL_multiple_p11_m0(op);
		case 0x01: return handleop32_ASL_multiple_p11_m1(op);
	}

	return 0;
}


uint32_t arcompact_device::handleop32_LSR_multiple_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_LSR_multiple_p11_m0(op);
		case 0x01: return handleop32_LSR_multiple_p11_m1(op);
	}

	return 0;
}





uint32_t arcompact_device::handleop32_ASL_multiple_p00(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if ((breg == LIMM_REG) || (creg == LIMM_REG))
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b << (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p01(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b << (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p10(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b << (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ASL_multiple_p11_m0 (ASL)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p11_m1(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
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


uint32_t arcompact_device::handleop32_LSR_multiple_p00(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if ((breg == LIMM_REG) || (creg == LIMM_REG))
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p01(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p10(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LSR_multiple_p11_m0 (LSR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p11_m1(uint32_t op)
{
	int size = 4;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	b = m_regs[breg];
	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
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
		case 0x00: return handleop32_LSR_multiple_p00(op);
		case 0x01: return handleop32_LSR_multiple_p01(op);
		case 0x02: return handleop32_LSR_multiple_p10(op);
		case 0x03: return handleop32_LSR_multiple_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ASL_multiple(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ASL_multiple_p00(op);
		case 0x01: return handleop32_ASL_multiple_p01(op);
		case 0x02: return handleop32_ASL_multiple_p10(op);
		case 0x03: return handleop32_ASL_multiple_p11(op);
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

