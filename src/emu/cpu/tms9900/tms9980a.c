/*
    Cycle-precise implementation of the TMS9980A.
    Subclassed from tms99xx_device in tms9900.c.

                    +----------------+
              /HOLD | 1     \/     40| /MEMEN
              HOLDA | 2            39| READY
                IAQ | 3            38| /WE
  LSB +- A13,CRUOUT | 4            37| CRUCLK
      |         A12 | 5            36| Vdd
      |         A11 | 6            35| Vss
      |         A10 | 7            34| CKIN
   Address       A9 | 8            33| D7      --+
     bus         A8 | 9            32| D6        |
      |          A7 |10            31| D5       Data
    16KiB        A6 |11            30| D4       bus
      |          A5 |12            29| D3        |
      |          A4 |13            28| D2       2 * 8 bit
      |          A3 |14            27| D1        |
      |          A2 |15            26| D0      --+
      |          A1 |16            25| INT0    --+
  MSB +--        A0 |17            24| INT1      | Interrupt levels
               DBIN |18            23| INT2    --+
              CRUIN |19            22| /PHI3
                Vcc |20            21| Vbb
                    +----------------+

  The TMS9980A is similar to the TMS9900, with the following differences:

    - Address bus is only 14 bit wide (16 KiB)
    - Data bus is 16 bit wide and multiplexed on 8 lines (2 bytes per access)
    - CRU space is limited to 2048 bits (due to fewer address lines)
    - Only three interrupt level lines, for a maximum of 8 levels.
    - No INTREQ, RESET, and LOAD lines. All interrupts are signaled via INT0 -
      INT2. Reset=00x, Load=010, Level1=011, Level2=100, Level3=101, Level4=110,
      all interrupts cleared=111.
    - Memory accesses are always 2 bytes (even address byte, odd address byte)
      even for byte operations. Thus the 9980A, like the TMS9900, needs to
      pre-fetch the word at the destination before overwriting it.
    - On the cycle level both TMS9900 and TMS9980A are equal, except for the
      additional cycles needed for memory read and write access. Accordingly,
      the emulation shares the core and the microprograms and redefines the
      memory access and the interrupt handling only.
    - The 9980A has the same external instructions as the TMS9900, but it
      indicates the command via A0, A1, and A13 (instead of A0-A2).

    For pin definitions see tms9900.c

   Michael Zapf, 2012
*/

#include "tms9980a.h"

#define LOG logerror
#define VERBOSE 1

/****************************************************************************
    Constructor
****************************************************************************/

tms9980a_device::tms9980a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms99xx_device(mconfig, TMS9980A, "TMS9980A", tag, 8, 14, 11, owner, clock)
{
}

UINT16 tms9980a_device::read_workspace_register_debug(int reg)
{
	int temp = m_icount;
	int addr = (WP+(reg<<1)) & 0xfffe & m_prgaddr_mask;
	UINT16 value = (m_prgspace->read_byte(addr) << 8) | (m_prgspace->read_byte(addr+1) & 0xff);
	m_icount = temp;
	return value;
}

void tms9980a_device::write_workspace_register_debug(int reg, UINT16 data)
{
	int temp = m_icount;
	int addr = (WP+(reg<<1)) & 0xfffe & m_prgaddr_mask;
	m_prgspace->write_byte(addr, data>>8);
	m_prgspace->write_byte(addr+1, data & 0xff);
	m_icount = temp;
}

/*
    Interrupt input. Keep in mind that the TMS9980A does not have any INTREQ
    line but signals interrupts via IC0-IC2 only. Thus we cannot take down any
    single interrupt; only all interrupts can be cleared at once using level 7.
    The state parameter is actually not needed.
*/
void tms9980a_device::execute_set_input(int irqline, int state)
{
	m_irq_level = get_intlevel(state);

	if (m_irq_level != 7)
	{
		if (m_irq_level == LOAD_INT)
		{
			// Clearing m_reset is a hack to prevent an initial RESET.
			// Should fix that in tms99xx
			m_reset = false;
			m_load_state = true;
		}
		else m_irq_state = true;
		if (VERBOSE>6) LOG("tms9980a: interrupt level=%d, ST=%04x\n", m_irq_level, ST);
	}
}

int tms9980a_device::get_intlevel(int state)
{
	int level = m_get_intlevel(0) & 0x0007;

	// Just to stay consistent.
	if (state==CLEAR_LINE) level = 7;

	switch (level)
	{
	case 0:
	case 1:
		level = RESET_INT;
		m_reset = true;
		break;
	case 2:
		level = LOAD_INT;
		break;
	case 3:
	case 4:
	case 5:
	case 6:
		level = level - 2;
		break;
	case 7:
		// Clear all interrupts
		m_load_state = false;
		m_irq_state = false;
		if (VERBOSE>6) LOG("tms9980a: clear interrupts\n");
		break;
	}
	return level;
}

/*****************************************************************************/

/*
    Memory read:
    Clock cycles: 4 + 2W, W = number of wait states
*/
void tms9980a_device::mem_read()
{
	UINT8 value;
	if (m_lowbyte)
	{
		value = m_prgspace->read_byte((m_address & m_prgaddr_mask) | 1);
		m_current_value = m_current_value | (value & 0x00ff);
		if (VERBOSE>7) LOG("tms9980a: memory read low byte %04x -> complete word %04x\n", (m_address & m_prgaddr_mask) | 1, m_current_value);
		m_lowbyte = false;
	}
	else
	{
		value = m_prgspace->read_byte(m_address & 0x3ffe);
		if (VERBOSE>7) LOG("tms9980a: memory read high byte %04x -> %02x\n", m_address & m_prgaddr_mask, value);
		m_current_value = (value << 8) & 0xff00;
		m_lowbyte = true;
		m_pass = 2;         // make the CPU visit this method once more
	}
	pulse_clock(2);
	m_check_ready = true;
}

void tms9980a_device::mem_write()
{
	if (m_lowbyte)
	{
		m_prgspace->write_byte((m_address & 0x3ffe) | 1, m_current_value & 0xff);
		if (VERBOSE>7) LOG("tms9980a: memory write low byte %04x <- %02x\n", (m_address & m_prgaddr_mask) | 1, m_current_value & 0xff);
		m_lowbyte = false;
	}
	else
	{
		m_prgspace->write_byte(m_address & 0x3ffe, (m_current_value >> 8)&0xff);
		if (VERBOSE>7) LOG("tms9980a: memory write high byte %04x <- %02x\n", m_address & m_prgaddr_mask, (m_current_value >> 8)&0xff);
		m_lowbyte = true;
		m_pass = 2;         // make the CPU visit this method once more
	}
	pulse_clock(2);
	m_check_ready = true;
}

void tms9980a_device::acquire_instruction()
{
	if (!m_lowbyte)
	{
		m_iaq_line(ASSERT_LINE);
		m_address = PC;
		m_first_cycle = m_icount;
		mem_read();
	}
	else
	{
		mem_read();
		decode(m_current_value);
		if (VERBOSE>3) LOG("tms9980a: ===== Next operation %04x (%s) at %04x =====\n", IR, opname[m_command], PC);
		debugger_instruction_hook(this, PC);
		PC = (PC + 2) & 0xfffe & m_prgaddr_mask;
	}
	// IAQ will be cleared in the main loop
}

/**************************************************************************/
UINT32 tms9980a_device::execute_min_cycles() const
{
	return 2;
}

// TODO: Compute this value, just a wild guess for the average
UINT32 tms9980a_device::execute_max_cycles() const
{
	return 10;
}

UINT32 tms9980a_device::execute_input_lines() const
{
	return 1;
}

// clocks to cycles, cycles to clocks = id
// execute_default_irq_vector = 0
// execute_burn = nop

// device_disasm_interface overrides
UINT32 tms9980a_device::disasm_min_opcode_bytes() const
{
	return 2;
}

UINT32 tms9980a_device::disasm_max_opcode_bytes() const
{
	return 6;
}

offs_t tms9980a_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return Dasm9900(buffer, pc, TMS9980_ID, oprom, opram);
}

const device_type TMS9980A = &device_creator<tms9980a_device>;
