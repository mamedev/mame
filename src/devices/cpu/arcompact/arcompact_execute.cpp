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
// at 0x03000000
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
			int level = ((m_AUX_IRQ_LEV >> vector) & 1)+1;

			logerror("HACK/TEST IRQ\n");

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
			standard_irq_callback_member(*this, 0);
		}
	}
}

void arcompact_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		if (!m_delayactive)
		{
			check_interrupts();
		}

		// make sure CPU isn't in 'SLEEP' mode
		if (!debugreg_check_ZZ())
		{
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

void arcompact_device::arcompact_handle_ld_helper(uint32_t op, uint8_t areg, uint8_t breg, uint32_t s, uint8_t X, uint8_t Z, uint8_t a)
{
	// writeback / increment
	if (a == 1)
	{
		if (breg == REG_LIMM)
			fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	uint32_t address = m_regs[breg];

	// address manipulation
	if (a == 0)
	{
		address = address + s;
	}
	else if (a == 3)
	{
		if (Z == 0)
		{
			address = address + (s << 2);
		}
		else if (Z == 2)
		{
			address = address + (s << 1);
		}
		else // Z == 1 and Z == 3 are invalid here
		{
			fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a);
		}
	}

	uint32_t readdata = 0;

	// read data
	if (Z == 0)
	{
		readdata = READ32(address);
		m_regs[areg] = readdata;
		if (X) // sign extend is not supported for long reads
			fatalerror("illegal LD helper %08x (data size %d mode %d with X)", op, Z, a);
	}
	else if (Z == 1)
	{
		readdata = READ8(address);

		if (X)
		{
			if (readdata & 0x80)
				readdata |= 0xffffff00;

			m_regs[areg] = readdata;
		}
		else
		{
			m_regs[areg] = readdata;
		}
	}
	else if (Z == 2)
	{
		readdata = READ16(address);

		if (X)
		{
			if (readdata & 0x8000)
				readdata |= 0xffff0000;

			m_regs[areg] = readdata;
		}
		else
		{
			m_regs[areg] = readdata;
		}
	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a);
	}

	// writeback / increment
	if (a == 2)
	{
		if (breg == REG_LIMM)
			fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}
}

uint32_t arcompact_device::handleop32_general(uint32_t op, ophandler32 ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		m_regs[common32_get_areg(op)] = ophandler(this, m_regs[breg], m_regs[creg], common32_get_F(op));
		return m_pc + size;
	}

	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		m_regs[common32_get_areg(op)] = ophandler(this, m_regs[breg], common32_get_u6(op), common32_get_F(op));
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		m_regs[breg] = ophandler(this, m_regs[breg], common32_get_s12(op), common32_get_F(op));
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			if (check_condition(common32_get_condition(op)))
				m_regs[breg] = ophandler(this, m_regs[breg], m_regs[creg], common32_get_F(op));
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			if (check_condition(common32_get_condition(op)))
				m_regs[breg] = ophandler(this, m_regs[breg], common32_get_u6(op), common32_get_F(op));
			return m_pc + size;
		}
		}
	}
	}
	return 0;
}

uint32_t arcompact_device::handleop32_general_MULx64(uint32_t op, ophandler32_mul ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		ophandler(this, m_regs[breg], m_regs[creg]);
		return m_pc + size;
	}

	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(this, m_regs[breg], common32_get_u6(op));
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(this, m_regs[breg], common32_get_s12(op));
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			if (!check_condition(common32_get_condition(op)))
				return m_pc + size;
			ophandler(this, m_regs[breg], m_regs[creg]);
			return m_pc + size;
		}

		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			if (!check_condition(common32_get_condition(op)))
				return m_pc + size;
			ophandler(this, m_regs[breg], common32_get_u6(op));
			return m_pc + size;
		}
		}
	}
	}

	return 0;
}

uint32_t arcompact_device::handleop32_general_nowriteback_forced_flag(uint32_t op, ophandler32_ff ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		ophandler(this, m_regs[breg], m_regs[creg]);
		return m_pc + size;
	}
	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(this, m_regs[breg], common32_get_u6(op));
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(this, m_regs[breg], common32_get_s12(op));
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			if (check_condition(common32_get_condition(op)))
				ophandler(this, m_regs[breg], m_regs[creg]);
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			if (check_condition(common32_get_condition(op)))
				ophandler(this, m_regs[breg], common32_get_u6(op));
			return m_pc + size;
		}
		}
	}
	}

	return 0;
}

uint32_t arcompact_device::handleop32_general_SOP_group(uint32_t op, ophandler32_sop ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			m_regs[breg] = ophandler(this, m_regs[creg], common32_get_F(op));
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			m_regs[breg] = ophandler(this, common32_get_u6(op), common32_get_F(op));
			return m_pc + size;
		}
		case 0x02:
		case 0x03:
			fatalerror("SOP Group: illegal mode 02/03 specifying use of bits already assigned to opcode select: opcode %04x\n", op);
			return 0;
	}
	return 0;
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
