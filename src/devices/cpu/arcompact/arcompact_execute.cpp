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
 30    | 0xf0   | IRQ30 (extended)               | ILINK1           | lv. 1 (low)   | L2
 31    | 0xf8   | IRQ31 (extended)               | ILINK1           | lv. 1 (low)   | L1
--------------------------------------------------------------------------------------------------------

*/

void arcompact_device::check_interrupts()
{
	// Mapping of relative priorities to their vector number, with index 0 = highest priority
	constexpr int RELATIVE_PRIORITY[0x20] = {0x00, 0x01, 0x02, 0x07, 0x06, 0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19,
			0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x05, 0x04, 0x03};

	uint32_t level_masked_ints;

	// STATUS32 & 0x04 = level2 ints enabled, & 0x02 = level1 ints enabled
	if (m_status32 & 0x0000'0004 && (m_pending_ints & m_AUX_IRQ_LEV) != 0)
	{
		level_masked_ints = m_pending_ints & (m_AUX_IRQ_LEV);

		m_regs[REG_ILINK2] = m_pc;
		m_status32_l2 = m_status32;
		m_status32 &= 0xffff'fff9;
		m_AUX_IRQ_LV12 |= 0x00000002;
	}
	else if (m_status32 & 0x0000'0002 && (m_pending_ints & ~m_AUX_IRQ_LEV) != 0)
	{
		level_masked_ints = m_pending_ints & ~m_AUX_IRQ_LEV;

		m_regs[REG_ILINK1] = m_pc;
		m_status32_l1 = m_status32;
		m_status32 &= 0xffff'fffd;
		m_AUX_IRQ_LV12 |= 0x00000001;
	}
	else
	{
		return;
	}

	// Rearrange pending interrupts to be positioned based on priority, with MSB = highest priority interrupt (reset)
	// Most bits are already arranged this way, but there are a few distinct groups that have to be moved around
	uint32_t priority_adjusted_ints = (bitswap(level_masked_ints, 0, 1, 2) << 29) | (BIT(level_masked_ints, 6, 2) << 27) |
			(BIT(level_masked_ints, 8, 24) << 3) | BIT(level_masked_ints, 3, 3);
	// Get position of highest priority set interrupt bit and use lookup table to recover vector from it
	int vector = RELATIVE_PRIORITY[std::countl_zero(priority_adjusted_ints)];

	standard_irq_callback(vector, m_pc);

	set_pc(m_INTVECTORBASE + vector * 8);
	m_allow_loop_check = false;
	m_pending_ints &= ~(1 << vector);
	debugreg_clear_ZZ();
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
				m_allow_loop_check = false;
			}
			else
			{
				uint16_t op = READ16((m_pc + 0));
				set_pc(get_instruction(op));
			}

			// hardware loops

			// NOTE: if LPcc condition code fails or a jump happens, the m_PC returned
			// can be m_LP_END which will then cause this check to happen and potentially
			// jump to the start of the loop if LP_COUNT is anything other than 1, this should
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
