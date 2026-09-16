// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

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

/*

Vector | Offset | Default Source                 | Link Reg         | Default Pri   | Relative Priority
--------------------------------------------------------------------------------------------------------
 0     | 0x00   | Reset (Special)                | (none)           | high (always) | H1
 1     | 0x08   | Memory Error (Special)         | ILINK2 (always)  | high (always) | H2
 2     | 0x10   | Instruction Error (Special)    | ILINK2 (always)  | high (always) | H3
--------------------------------------------------------------------------------------------------------
 3     | 0x18   | Timer 0 IRQ                    | ILINK1           | lv. 1 (low)   | L27
 4     | 0x20   | XY Memory IRQ                  | ILINK1           | lv. 1 (low)   | L26
 5     | 0x28   | UART IRQ                       | ILINK1           | lv. 1 (low)   | L25
 6     | 0x30   | EMAC IRQ                       | ILINK2           | lv. 2 (med)   | M2
 7     | 0x38   | Timer 1 IRQ                    | ILINK2           | lv. 2 (med)   | M1
--------------------------------------------------------------------------------------------------------
 8     | 0x40   | IRQ8                           | ILINK1           | lv. 1 (low)   | L24
 9     | 0x48   | IRQ9                           | ILINK1           | lv. 1 (low)   | L23
....
 14    | 0x70   | IRQ14                          | ILINK1           | lv. 1 (low)   | L18
 15    | 0x78   | IRQ15                          | ILINK1           | lv. 1 (low)   | L17
--------------------------------------------------------------------------------------------------------
 16    | 0x80   | IRQ16 (extended)               | ILINK1           | lv. 1 (low)   | L16
 17    | 0x87   | IRQ17 (extended)               | ILINK1           | lv. 1 (low)   | L15
....
 16    | 0xf0   | IRQ30 (extended)               | ILINK1           | lv. 1 (low)   | L2
 17    | 0xf8   | IRQ31 (extended)               | ILINK1           | lv. 1 (low)   | L1
--------------------------------------------------------------------------------------------------------

*/

// currently causes the Leapster to put an unhandled interrupt exception string in RAM
// at 0x03000800
void arcompact_device::check_interrupts()
{
	int vector = 8;

	if (vector < 3)
	{
		fatalerror("check_interrupts called for vector < 3 (these are special exceptions)");
	}

	if (m_irq_pending)
	{
		if (m_status32 & 0x00000002) // & 0x04 = level2, & 0x02 = level1
		{
			int level = ((m_AUX_IRQ_LEV >> vector) & 1) + 1;

			logerror("HACK/TEST IRQ\n");

			standard_irq_callback(level, m_pc);
			if (level == 1)
			{
				m_regs[REG_ILINK1] = m_pc;
				m_status32_l1 = m_status32;
				m_AUX_IRQ_LV12 |= 0x00000001;
			}
			else if (level == 2)
			{
				m_regs[REG_ILINK2] = m_pc;
				m_status32_l2 = m_status32;
				m_AUX_IRQ_LV12 |= 0x00000002;
			}
			else
			{
				fatalerror("illegal IRQ level\n");
			}

			set_pc(m_INTVECTORBASE + vector * 8);
			m_irq_pending = 0;
			debugreg_clear_ZZ();
		}
	}
}

void arcompact_device::execute_run()
{
	while (m_icount > 0)
	{
		if (!m_delayactive)
		{
			check_interrupts();
		}

		// make sure CPU isn't in 'SLEEP' mode
		if (debugreg_check_ZZ())
			debugger_wait_hook();
		else
		{
			debugger_instruction_hook(m_pc);

			if (m_delayactive)
			{
				uint16_t op = READ16((m_pc + 0));
				set_pc(get_instruction(op));
				if (m_delaylinks) m_regs[REG_BLINK] = m_pc;

				set_pc(m_delayjump);

				m_delayactive = false;
				m_delaylinks = false;
			}
			else
			{
				uint16_t op = READ16((m_pc + 0));
				set_pc(get_instruction(op));
			}

			// hardware loops

			// NOTE: if LPcc condition code fails, the m_PC returned will be m_LP_END
			// which will then cause this check to happen and potentially jump back to
			// the start of the loop if LP_COUNT is anything other than 1, this should
			// not happen.  It could be our PC handling needs to be better?  Either way
			// guard against it!
			if (m_allow_loop_check)
			{
				if (m_pc == m_LP_END)
				{
					// NOTE: this behavior should differ between ARC models
					if ((m_regs[REG_LP_COUNT] != 1))
					{
						set_pc(m_LP_START);
					}
					m_regs[REG_LP_COUNT]--;
				}
			}
			else
			{
				m_allow_loop_check = true;
			}
		}
		m_icount--;
	}
}


/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers                                                                                            *
*                                                                                                                                   *
************************************************************************************************************************************/

uint32_t arcompact_device::arcompact_handle_reserved(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op) { logerror("<reserved 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + 4; }

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint16_t op) { logerror("<illegal 0x%02x_%02x> (%04x)\n", param1, param2, op); fatalerror("<illegal op>");  return m_pc + 2; }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x> (%04x)\n", param1, param2, param3, op); fatalerror("<illegal op>");  return m_pc + 2; }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%04x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>");  return m_pc + 2; }

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint32_t op) { logerror("<illegal 0x%02x_%02x> (%08x)\n", param1, param2, op); fatalerror("<illegal op>"); return m_pc + 4; }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op) { logerror("<illegal 0x%02x_%02x_%02x> (%08x)\n", param1, param2, param3, op); fatalerror("<illegal op>"); return m_pc + 4; }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op) { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + 4; }
