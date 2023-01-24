// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

/*

NOTES:

LIMM use
--------
If "destination register = LIMM" then there is no result stored, only flag updates, however there's no way to read LIMM anyway
as specifying LIMM as a source register just causes the CPU to read the long immediate data instead. For emulation purposes we
therefore just store the result as normal, to the reigster that can't be read.

Likewise, when LIMM is used as a source register, we load the LIMM register with the long immediate when the opcode is decoded
so don't require special logic further in the opcode handler.

It is possible this is how the CPU works internally.

*/


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
			uint16_t op = READ16((m_pc + 0));
			m_pc = get_instruction(op);
			if (m_delaylinks) m_regs[REG_BLINK] = m_pc;

			m_pc = m_delayjump;
			m_delayactive = 0; m_delaylinks = 0;
		}
		else
		{
			uint16_t op = READ16((m_pc + 0));
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
		case 0x00: return condition_AL(); // AL
		case 0x01: return condition_EQ();
		case 0x02: return condition_NE(); // NE
		case 0x03: return condition_PL();
		case 0x04: return condition_MI(); // MI (N)
		case 0x05: return condition_CS(); // CS (Carry Set / Lower than)
		case 0x06: return condition_HS();
		case 0x07: return condition_VS();
		case 0x08: return condition_VC();
		case 0x09: return condition_GT();
		case 0x0a: return condition_GE();
		case 0x0b: return condition_LT();
		case 0x0c: return condition_LE();
		case 0x0d: return condition_HI();
		case 0x0e: return condition_LS();
		case 0x0f: return condition_PNZ();

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
			if ((reg == REG_ILINK1)) m_status32 = m_status32_l1;
			if ((reg == REG_ILINK2)) m_status32 = m_status32_l2;
			uint32_t target = m_regs[reg];
			return handle_jump_to_addr(delay, link, target, next_addr);
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
	int p = common32_get_p(op);
	uint8_t breg = common32_get_breg(op);
	int size = 4;

	if (!b_reserved)
	{
		size = check_b_limm(breg);
	}
	else
	{
	}

	if (p == 0)
	{
		uint8_t creg = common32_get_creg(op);
		size = check_c_limm(creg);
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
			uint8_t creg = common32_get_creg(op);
			size = check_c_limm(creg);
		}
		else if (M == 1)
		{
		}
	}

	arcompact_log("unimplemented %s %08x (04 type helper)", optext, op);

	return m_pc + size;
}




/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers                                                                                            *
*                                                                                                                                   *
************************************************************************************************************************************/

uint32_t arcompact_device::arcompact_handle_reserved(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op)  { logerror("<reserved 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + 4;}

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint16_t op) { logerror("<illegal 0x%02x_%02x> (%04x)\n", param1, param2, op); fatalerror("<illegal op>");  return m_pc + 2; }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x> (%04x)\n", param1, param2, param3, op); fatalerror("<illegal op>");  return m_pc + 2; }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%04x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>");  return m_pc + 2; }

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint32_t op)  { logerror("<illegal 0x%02x_%02x> (%08x)\n", param1, param2, op); fatalerror("<illegal op>"); return m_pc + 4;}
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op)  { logerror("<illegal 0x%02x_%02x_%02x> (%08x)\n", param1, param2, param3, op); fatalerror("<illegal op>"); return m_pc + 4;}
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op)  { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + 4;}
