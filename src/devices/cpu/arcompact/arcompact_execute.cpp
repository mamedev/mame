// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"



void arcompact_device::execute_run()
{
	//uint32_t lres;
	//lres = 0;

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

//      printf("new pc %04x\n", m_pc);

		if (m_delayactive)
		{
			uint16_t op = READ16((m_pc + 0) >> 1);
			m_pc = get_instruction(op);
			if (m_delaylinks) m_regs[REG_BLINK] = m_pc;

			m_pc = m_delayjump;
			m_delayactive = 0; m_delaylinks = 0;
		}
		else
		{
			uint16_t op = READ16((m_pc + 0) >> 1);
			m_pc = get_instruction(op);
		}

		// hardware loops
		if (m_pc == m_LP_END)
		{
			if (m_regs[REG_LP_COUNT] != 1)
			{
				m_pc = m_LP_START;
			}
			m_regs[REG_LP_COUNT]--;

		}

		m_icount--;
	}

}



int arcompact_device::check_condition(uint8_t condition)
{
	switch (condition & 0x1f)
	{
		case 0x00: return 1; // AL
		case 0x01: return CONDITION_EQ;
		case 0x02: return !CONDITION_EQ; // NE
		case 0x03: fatalerror("unhandled condition check %s", arcompact_disassembler::conditions[condition]); return -1;
		case 0x04: return CONDITION_MI; // MI (N)
		case 0x05: return CONDITION_CS; // CS (Carry Set / Lower than)
		default: fatalerror("unhandled condition check %s", arcompact_disassembler::conditions[condition]); return -1;
	}

	return -1;
}

// handlers

uint32_t arcompact_device::handle_jump_to_addr(int delay, int link, uint32_t address, uint32_t next_addr)
{
	if (delay)
	{
		m_delayactive = 1;
		m_delayjump = address;
		if (link) m_delaylinks = 1;
		else m_delaylinks = 0;
		return next_addr;
	}
	else
	{
		if (link) m_regs[REG_BLINK] = next_addr;
		return address;
	}

}

uint32_t arcompact_device::handle_jump_to_register(int delay, int link, uint32_t reg, uint32_t next_addr, int flag)
{
	if (reg == LIMM_REG)
		arcompact_fatal("handle_jump_to_register called with LIMM register, call handle_jump_to_addr instead");

	if ((reg == REG_ILINK1) || (reg == REG_ILINK2))
	{
		if (flag)
		{
			arcompact_fatal("jump to ILINK1/ILINK2 not supported");
			return next_addr;
		}
		else
		{
			arcompact_fatal("illegal jump to ILINK1/ILINK2 not supported"); // FLAG bit must be set
			return next_addr;
		}
	}
	else
	{
		if (flag)
		{
			arcompact_fatal("illegal jump (flag bit set)"); // FLAG bit must NOT be set
			return next_addr;
		}
		else
		{
			//arcompact_fatal("jump not supported");
			uint32_t target = m_regs[reg];
			return handle_jump_to_addr(delay, link, target, next_addr);
		}
	}

	return 0;
}



uint32_t arcompact_device::arcompact_handle04_helper(uint32_t op, const char* optext, int ignore_dst, int b_reserved)
{
	int size;
	//uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_p;
	COMMON32_GET_breg;

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			//GET_LIMM_32;
			size = 8;
			got_limm = 1;
		}
		else
		{
		}
	}
	else
	{
	}


	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			if (!got_limm)
			{
				//GET_LIMM_32;
				size = 8;
			}
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
		int M = (op & 0x00000020) >> 5;

		if (M == 0)
		{
			COMMON32_GET_creg

			if (creg == LIMM_REG)
			{
				if (!got_limm)
				{
					//GET_LIMM_32;
					size = 8;
				}
			}
			else
			{
			}

		}
		else if (M == 1)
		{
		}

	}

	arcompact_log("unimplemented %s %08x (04 type helper)", optext, op);

	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_ADD_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD_p11_m0 (ADD)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB_p11_m0 (SUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & c;
	if (areg != LIMM_REG) { m_regs[areg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & c;
	if (areg != LIMM_REG) { m_regs[areg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & c;
	if (breg != LIMM_REG) { m_regs[breg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_AND_p11_m0 (AND)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b & c;
	if (breg != LIMM_REG) { m_regs[breg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_OR_p11_m0 (OR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b | c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & (~c);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & (~c);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & (~c);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BIC_p11_m0 (BIC)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b & (~c);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b ^ c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b ^ c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b ^ c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_XOR_p11_m0 (XOR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b ^ c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p10(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_MOV_p11_m0 (MOV)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p11_m1(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c - b;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c - b;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c - b;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_RSUB_p11_m0 (RSUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = c - b;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BSET_p11_m0 (BSET)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BMSK_p11_m0 (BMSK)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD1_p11_m0 (ADD1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD2_p11_m0 (ADD2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD3_p11_m0 (ADD3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB1_p11_m0 (SUB1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB2_p11_m0 (SUB2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB3_p11_m0 (SUB3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p10(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LR_p11_m0 (LR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p11_m1(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SR_p11_m0 (SR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
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
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

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
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

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
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

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
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
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
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

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
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

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
	uint32_t limm = 0;
/*  int got_limm = 0; */

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
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

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


uint32_t arcompact_device::handleop32_LSR_single_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
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
/*  int got_limm = 0; */

	COMMON32_GET_breg;
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


uint32_t arcompact_device::handleop32_ROR_single_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
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
/*  int got_limm = 0; */

	COMMON32_GET_breg;
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


uint32_t arcompact_device::handleop32_EXTB_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
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
/*  int got_limm = 0; */

	COMMON32_GET_breg;
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


uint32_t arcompact_device::handleop32_EXTW_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
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
/*  int got_limm = 0; */

	COMMON32_GET_breg;
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


uint32_t arcompact_device::handleop_SUB_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] - m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_AND_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] & m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_OR_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] | m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_XOR_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] ^ m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_EXTB_S_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] & 0x000000ff;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_EXTW_S_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] & 0x0000ffff;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_NEG_S_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = 0 - m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ADD1_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<1);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ADD2_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<2);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ADD3_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<3);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_LSR_S_b_b_c_multiple(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] >> (m_regs[creg]&0x1f);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASL_S_b_c_single(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] << 1;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASL_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] << (u&0x1f);

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_LSR_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] >> (u&0x1f);

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASR_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	int32_t temp = (int32_t)m_regs[breg]; m_regs[breg] = temp >> (u&0x1f); // treat it as a signed value, so sign extension occurs during shift

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_SUB_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] - u;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_BSET_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] | (1 << (u & 0x1f));

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_BMSK_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] | ((1 << (u + 1)) - 1);

	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop32_ADC(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x01], /*"ADC"*/ 0,0);
}



uint32_t arcompact_device::handleop32_SBC(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x03], /*"SBC"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MAX(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x08], /*"MAX"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MIN(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x09], /*"MIN"*/ 0,0);
}



uint32_t arcompact_device::handleop32_TST(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0b], /*"TST"*/ 1,0);
}

uint32_t arcompact_device::handleop32_CMP(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0c], /*"CMP"*/ 1,0);
}

uint32_t arcompact_device::handleop32_RCMP(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0d], /*"RCMP"*/ 1,0);
}



uint32_t arcompact_device::handleop32_BCLR(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x10], /*"BCLR"*/ 0,0);
}

uint32_t arcompact_device::handleop32_BTST(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x11], /*"BTST"*/ 0,0);
}

uint32_t arcompact_device::handleop32_BXOR(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x12], /*"BXOR"*/ 0,0);
}






uint32_t arcompact_device::handleop32_MPY(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1a], /*"MPY"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYH(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1b], /*"MPYH"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYHU(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1c], /*"MPYHU"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYU(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1d], /*"MPYU"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_Jcc_p00(uint32_t op)
{
	int size;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_F

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J limm          0010 0RRR 0010 0000 0RRR 1111 10RR RRRR  [LIMM]  (creg = LIMM)

		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		return limm;
	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J [c]           0010 0RRR 0010 0000 0RRR CCCC CCRR RRRR
		// J.F [ilink1]    0010 0RRR 0010 0000 1RRR 0111 01RR RRRR  (creg = ILINK1, FLAG must be set)
		// J.F [ilink2]    0010 0RRR 0010 0000 1RRR 0111 10RR RRRR  (creg = ILINE2, FLAG must be set)

		if (F)
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				arcompact_log("1 unimplemented J.F %08x", op);
			}
			else
			{
				// should not use .F unless jumping to ILINK1/2
				arcompact_fatal ("illegal 1 unimplemented J.F (F should not be set) %08x", op);
			}

		}
		else
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				// should only jumping to ILINK1/2 if .F is set
				arcompact_fatal("illegal 1 unimplemented J (F not set) %08x", op);
			}
			else
			{
				return m_regs[creg];
			}
		}
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p01(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uu-- ----
	// J u6            0010 0RRR 0110 0000 0RRR uuuu uuRR RRRR
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p10(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- ssss ssSS SSSS
	// J s12           0010 0RRR 1010 0000 0RRR ssss ssSS SSSS
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_p11_m0(uint32_t op) // Jcc   (no link, no delay)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_CONDITION;
	COMMON32_GET_F

	uint32_t c;

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc limm        0010 0RRR 1110 0000 0RRR 1111 100Q QQQQ  [LIUMM]
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		c = limm;

	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc [c]         0010 0RRR 1110 0000 0RRR CCCC CC0Q QQQQ
		// no conditional links to ILINK1, ILINK2?

		c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + (size>>0);

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_fatal ("fatal handleop32_Jcc_p11_m0 J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			uint32_t realaddress = c;
			return realaddress;
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented handleop32_Jcc_p11_m0 J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal handleop32_Jcc_p11_m0 J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);

		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p11_m1(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uumq qqqq
	// Jcc u6          0010 0RRR 1110 0000 0RRR uuuu uu1Q QQQQ
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_p11_m1 J %08x (u6)", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_F

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		handle_jump_to_addr(1,0,limm, m_pc + (size>>0));
	}
	else
	{
		return handle_jump_to_register(1,0,creg, m_pc + (size>>0), F); // delay, no link
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p01(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p10(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_p11_m0(uint32_t op) // Jcc.D   (no link, delay)
{
	int size = 4;
	[[maybe_unused]] uint32_t limm;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_CONDITION;
	COMMON32_GET_F

	//uint32_t c = 0;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

	//  c = limm;

	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc [c]         0010 0RRR 1110 0000 0RRR CCCC CC0Q QQQQ
		// no conditional links to ILINK1, ILINK2?

	//  c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + (size>>0);

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, illegal) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, unimplemented) %08x", op);
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, unimplemented) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, illegal) %08x", op);
		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_D_p11_m1 J.D %08x (u6)", op);
	return m_pc + (size>>0);
}



uint32_t arcompact_device::handleop32_JLcc(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x22], /*"JL"*/ 1,1);
}

uint32_t arcompact_device::handleop32_JLcc_D(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x23], /*"JL.D"*/ 1,1);
}




uint32_t arcompact_device::handleop32_LP(uint32_t op) // LPcc (loop setup)
{
	int size = 4;
//  COMMON32_GET_breg; // breg is reserved
	COMMON32_GET_p;

	if (p == 0x00)
	{
		arcompact_fatal("<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		arcompact_fatal("<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		COMMON32_GET_s12
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		arcompact_fatal("Lp unconditional not supported %d", S);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		COMMON32_GET_u6
		COMMON32_GET_CONDITION
		//arcompact_fatal("Lp conditional %s not supported %d", arcompact_disassembler::conditions[condition], u);

		// if the loop condition fails then just jump to after the end of the loop, don't set any registers
		if (!check_condition(condition))
		{
			uint32_t realoffset = PC_ALIGNED32 + (u * 2);
			return realoffset;
		}
		else
		{
			// otherwise set up the loop positions
			m_LP_START = m_pc + (size >> 0);
			m_LP_END = PC_ALIGNED32 + (u * 2);
			return m_pc + (size>>0);
		}

	}

	return m_pc + (size>>0);

}








uint32_t arcompact_device::handleop32_ASR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ASR", 0,0); }
uint32_t arcompact_device::handleop32_ROR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ROR", 0,0); }
uint32_t arcompact_device::handleop32_MUL64(uint32_t op)  { return arcompact_handle04_helper(op, "MUL64", 2,0); } // special
uint32_t arcompact_device::handleop32_MULU64(uint32_t op)  { return arcompact_handle04_helper(op, "MULU64", 2,0);} // special
uint32_t arcompact_device::handleop32_ADDS(uint32_t op)  { return arcompact_handle04_helper(op, "ADDS", 0,0); }
uint32_t arcompact_device::handleop32_SUBS(uint32_t op)  { return arcompact_handle04_helper(op, "SUBS", 0,0); }
uint32_t arcompact_device::handleop32_DIVAW(uint32_t op)  { return arcompact_handle04_helper(op, "DIVAW", 0,0); }






/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers                                                                                            *
*                                                                                                                                   *
************************************************************************************************************************************/

uint32_t arcompact_device::arcompact_handle_reserved(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op)  { logerror("<reserved 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint16_t op) { logerror("<illegal 0x%02x_%02x> (%04x)\n", param1, param2, op); fatalerror("<illegal op>");  return m_pc + (2 >> 0); }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x> (%04x)\n", param1, param2, param3, op); fatalerror("<illegal op>");  return m_pc + (2 >> 0); }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%04x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>");  return m_pc + (2 >> 0); }

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint32_t op)  { logerror("<illegal 0x%02x_%02x> (%08x)\n", param1, param2, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op)  { logerror("<illegal 0x%02x_%02x_%02x> (%08x)\n", param1, param2, param3, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op)  { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}
