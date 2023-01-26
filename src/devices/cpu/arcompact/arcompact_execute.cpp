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
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		// make sure CPU isn't in 'SLEEP' mode
		if (!debugreg_check_ZZ())
		{
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
				if (m_regs[REG_LP_COUNT] > 0)
				{
					m_pc = m_LP_START;
					m_regs[REG_LP_COUNT]--;
				}
			}
		}
		m_icount--;
	}
}



int arcompact_device::check_condition(uint8_t condition)
{
	switch (condition & 0x1f)
	{
		case 0x00: return condition_AL();
		case 0x01: return condition_EQ();
		case 0x02: return condition_NE();
		case 0x03: return condition_PL();
		case 0x04: return condition_MI();
		case 0x05: return condition_CS();
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


void arcompact_device::do_flags_overflow(uint32_t result, uint32_t b, uint32_t c)
{
	if ((b & 0x80000000) == (c & 0x80000000))
	{
		if ((result & 0x80000000) != (b & 0x80000000))
		{
			status32_set_v();
		}
		else
		{
			status32_clear_v();
		}
	}
}

void arcompact_device::do_flags_add(uint32_t result, uint32_t b, uint32_t c)
{
	do_flags_nz(result);
	do_flags_overflow(result, b, c);

	if (result < b)
	{
		status32_set_c();
	}
	else
	{
		status32_clear_c();
	}
}

void arcompact_device::do_flags_sub(uint32_t result, uint32_t b, uint32_t c)
{
	do_flags_nz(result);
	do_flags_overflow(result, b, c);

	if (result > b)
	{
		status32_set_c();
	}
	else
	{
		status32_clear_c();
	}
}

void arcompact_device::do_flags_nz(uint32_t result)
{
	if (result & 0x80000000) { status32_set_n(); }
	else { status32_clear_n(); }
	if (result == 0x00000000) { status32_set_z(); }
	else { status32_clear_z(); }
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
