// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle04_2f_helper(uint32_t op, const char* optext)
{
	int size;

	COMMON32_GET_p;
	//uint32_t breg = common32_get_breg(op);

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			//uint32_t limm;
			//GET_LIMM_32;
			size = 8;
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented %s %08x (type 04_2f)", optext, op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_ASL_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASL"); } // ASL
uint32_t arcompact_device::handleop32_ASR_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASR"); } // ASR

uint32_t arcompact_device::handleop32_RRC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCC"); } // RCC
uint32_t arcompact_device::handleop32_SEXB(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXB"); } // SEXB
uint32_t arcompact_device::handleop32_SEXW(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXW"); } // SEXW


uint32_t arcompact_device::handleop32_ABS(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ABS"); } // ABS
uint32_t arcompact_device::handleop32_NOT(uint32_t op)  { return arcompact_handle04_2f_helper(op, "NOT"); } // NOT
uint32_t arcompact_device::handleop32_RLC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCL"); } // RLC
uint32_t arcompact_device::handleop32_EX(uint32_t op)  { return arcompact_handle04_2f_helper(op, "EX"); } // EX



uint32_t arcompact_device::handleop32_LSR_single_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_LSR_single_p10 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_LSR_single_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (LSR1)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_LSR_single_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_LSR_single_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (LSR1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p00(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c >> 1;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p01(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c >> 1;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_LSR_single_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_LSR_single_p11_m0(op);
		case 0x01: return handleop32_LSR_single_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_LSR_single(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_LSR_single_p00(op);
		case 0x01: return handleop32_LSR_single_p01(op);
		case 0x02: return handleop32_LSR_single_p10(op);
		case 0x03: return handleop32_LSR_single_p11(op);
	}

	return 0;
}



uint32_t arcompact_device::handleop32_ROR_single_p00(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	int shift = 1; uint32_t mask = (1 << (shift)) - 1; mask <<= (32-shift); uint32_t result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p01(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	int shift = 1; uint32_t mask = (1 << (shift)) - 1; mask <<= (32-shift); uint32_t result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_p10 (ares bits already used as opcode select, can't be used as s12) (ROR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (ROR)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_ROR_single_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (ROR)\n");
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ROR_single_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ROR_single_p11_m0(op);
		case 0x01: return handleop32_ROR_single_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ROR_single(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ROR_single_p00(op);
		case 0x01: return handleop32_ROR_single_p01(op);
		case 0x02: return handleop32_ROR_single_p10(op);
		case 0x03: return handleop32_ROR_single_p11(op);
	}

	return 0;
}


uint32_t arcompact_device::handleop32_EXTW_p00(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x0000ffff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTW (EXTW) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p01(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x0000ffff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTW (EXTW) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_p10 (ares bits already used as opcode select, can't be used as s12) (EXTW)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (EXTW)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_EXTW_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (EXTW)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_EXTW_p11_m0(op);
		case 0x01: return handleop32_EXTW_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_EXTW(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_EXTW_p00(op);
		case 0x01: return handleop32_EXTW_p01(op);
		case 0x02: return handleop32_EXTW_p10(op);
		case 0x03: return handleop32_EXTW_p11(op);
	}

	return 0;
}




uint32_t arcompact_device::handleop32_EXTB_p00(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x000000ff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTB (EXTB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p01(uint32_t op)
{
	int size = 4;

	uint32_t breg = common32_get_breg(op);
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x000000ff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTB (EXTB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_p10 (ares bits already used as opcode select, can't be used as s12) (EXTB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (EXTB)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_EXTB_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (EXTB)\n");
	return m_pc + (size >> 0);
}




uint32_t arcompact_device::handleop32_EXTB_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_EXTB_p11_m0(op);
		case 0x01: return handleop32_EXTB_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_EXTB(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_EXTB_p00(op);
		case 0x01: return handleop32_EXTB_p01(op);
		case 0x02: return handleop32_EXTB_p10(op);
		case 0x03: return handleop32_EXTB_p11(op);
	}

	return 0;
}


