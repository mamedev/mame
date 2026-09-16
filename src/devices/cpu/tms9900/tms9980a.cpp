// license:BSD-3-Clause
// copyright-holders:Michael Zapf
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
    - Whereas the TMS9900 relies on the TMS9904 to generate its four phase
      clocks, the TMS9980A takes a single 6 MHz to 10 MHz clock input and
      divides it by four internally. Only the phase 3 clock is provided as an
      output for peripherals.
    - On the cycle level both TMS9900 and TMS9980A are equal, except for the
      additional cycles needed for memory read and write access. Accordingly,
      the emulation shares the core and the microprograms and redefines the
      memory access and the interrupt handling only.
    - The 9980A has the same external instructions as the TMS9900, but it
      indicates the command via A0, A1, and A13 (instead of A0-A2).

  TMS9981 is almost identical to TMS9980A except for deleting the -5V Vbb input
  and allowing a crystal oscillator to be connected to pins 34 and 33 (OSCOUT).
  The D0-D7, INT0-2 and /PHI3 pins are shifted to accommodate this feature.

    For pin definitions see tms9900.c

   Michael Zapf, 2012
*/

#include "emu.h"
#include "tms9980a.h"
#include "9900dasm.h"

/*
    Logging (also see tms9900.cpp)
*/

#define LOG_OP         (1U << 1)   // Current instruction
#define LOG_MEM        (1U << 2)   // Memory access
#define LOG_INT        (1U << 3)   // Interrupts
#define LOG_ADDRESSBUS (1U << 4)   // Address bus operation

// Minimum log should be warnings
#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

/****************************************************************************
    Constructor
****************************************************************************/
tms9980a_device::tms9980a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: tms99xx_device(mconfig, type, tag, 8, 14, 11, owner, clock)
{
}

tms9980a_device::tms9980a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms9980a_device(mconfig, TMS9980A, tag, owner, clock)
{
}

tms9981_device::tms9981_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms9980a_device(mconfig, TMS9981, tag, owner, clock)
{
}

uint16_t tms9980a_device::read_workspace_register_debug(int reg)
{
	int temp = m_icount;
	int addr = (WP+(reg<<1)) & m_prgaddr_mask;
	uint16_t value = (m_prgspace->read_byte(addr) << 8) | (m_prgspace->read_byte(addr+1) & 0xff);
	m_icount = temp;
	return value;
}

void tms9980a_device::write_workspace_register_debug(int reg, uint16_t data)
{
	int temp = m_icount;
	int addr = (WP+(reg<<1)) & m_prgaddr_mask;
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
	// We model the three lines IC0-IC2 as 8 separate input lines, although we
	// cannot assert more than one at a time. The state value is not needed,
	// as level 7 means to clean all interrupts, but we consider it for the
	// sake of consistency.

	int level = irqline;

	// Just to stay consistent.
	if (state==CLEAR_LINE) level = INT_9980A_CLEAR;

	switch (level)
	{
	case INT_9980A_RESET:
	case 1:
		level = RESET_INT;
		m_reset = true;
		break;
	case INT_9980A_LOAD:
		level = LOAD_INT;
		break;
	case INT_9980A_LEVEL1:
	case INT_9980A_LEVEL2:
	case INT_9980A_LEVEL3:
	case INT_9980A_LEVEL4:
		level = level - 2;
		break;
	case INT_9980A_CLEAR:
		// Clear all interrupts
		m_load_state = false;
		m_irq_state = false;
		LOGMASKED(LOG_INT, "Clear interrupts\n");
		break;
	}

	m_irq_level = level;

	if (m_irq_level != INT_9980A_CLEAR)
	{
		if (m_irq_level == LOAD_INT)
		{
			// Some boards start up with LOAD interrupt, so we clear the reset flag
			m_reset = false;
			m_load_state = true;
		}
		else m_irq_state = true;
		LOGMASKED(LOG_INT, "Interrupt level=%d, ST=%04x\n", m_irq_level, ST);
	}
}

/*****************************************************************************/

/*
    Memory read:
    Clock cycles: 4 + 2W, W = number of wait states
*/
void tms9980a_device::mem_read()
{
	uint8_t value;
	switch (m_mem_phase)
	{
	case 1:
		m_pass = 4;         // make the CPU visit this method more than once
		if (m_setaddr)
			m_setaddr->write_byte(m_address & m_prgaddr_mask, (TMS99xx_BUS_DBIN | (m_iaq? TMS99xx_BUS_IAQ : 0)));
		LOGMASKED(LOG_ADDRESSBUS, "Set address bus %04x\n", m_address & m_prgaddr_mask);
		m_check_ready = true;
		break;
	case 2:
		// Sample the value on the data bus (high byte)
		value = m_prgspace->read_byte(m_address & m_prgaddr_mask);
		LOGMASKED(LOG_MEM, "Memory read high byte %04x -> %02x\n", m_address & m_prgaddr_mask, value);
		m_current_value = (value << 8) & 0xff00;
		break;
	case 3:
		if (m_setaddr)
			m_setaddr->write_byte((m_address & m_prgaddr_mask) | 1, (TMS99xx_BUS_DBIN | (m_iaq? TMS99xx_BUS_IAQ : 0)));
		LOGMASKED(LOG_ADDRESSBUS, "Set address bus %04x\n", (m_address & m_prgaddr_mask) | 1);
		break;
	case 4:
		// Sample the value on the data bus (low byte)
		value = m_prgspace->read_byte((m_address & m_prgaddr_mask) | 1);
		m_current_value = m_current_value | (value & 0x00ff);
		LOGMASKED(LOG_MEM, "Memory read low byte %04x -> %02x -> complete word %04x\n", (m_address & m_prgaddr_mask) | 1, value, m_current_value);
		break;
	}
	pulse_clock(1);
	m_mem_phase = (m_mem_phase % 4) +1;
}


void tms9980a_device::mem_write()
{
	switch (m_mem_phase)
	{
	case 1:
		m_pass = 4;         // make the CPU visit this method once more
		if (m_setaddr)
			m_setaddr->write_byte(m_address & m_prgaddr_mask, TMS99xx_BUS_WRITE);
		LOGMASKED(LOG_ADDRESSBUS, "Set address bus %04x\n", m_address & m_prgaddr_mask);
		m_prgspace->write_byte(m_address & 0x3ffe, (m_current_value >> 8)&0xff);
		LOGMASKED(LOG_MEM, "Memory write high byte %04x <- %02x\n", m_address & m_prgaddr_mask, (m_current_value >> 8)&0xff);
		m_check_ready = true;
		break;
	case 2:
		// no action here, just wait for READY
		break;
	case 3:
		if (m_setaddr)
			m_setaddr->write_byte((m_address & m_prgaddr_mask) | 1, TMS99xx_BUS_WRITE);
		LOGMASKED(LOG_ADDRESSBUS, "Set address bus %04x\n", (m_address & m_prgaddr_mask) | 1);
		m_prgspace->write_byte((m_address & m_prgaddr_mask) | 1, m_current_value & 0xff);
		LOGMASKED(LOG_MEM, "Memory write low byte %04x <- %02x\n", (m_address & m_prgaddr_mask) | 1,  m_current_value & 0xff);
		break;
	case 4:
		// no action here, just wait for READY
		break;
	}
	pulse_clock(1);
	m_mem_phase = (m_mem_phase % 4) +1;
}

void tms9980a_device::acquire_instruction()
{
	if (m_mem_phase == 1)
	{
		m_iaq = true;
		m_address = PC;
		m_first_cycle = m_icount;
	}
	mem_read();

	if (m_mem_phase == 1)  // changed by mem_read and wrapped
	{
		decode(m_current_value);
		LOGMASKED(LOG_OP, "===== Next operation %04x (%s) at %04x =====\n", IR, opname[m_command], PC);
		debugger_instruction_hook(PC);
		PC = (PC + 2) & m_prgaddr_mask;
	}
	// IAQ will be cleared in the main loop
}



/**************************************************************************/

/*
    The minimum number of cycles applies to a command like STWP R0.
*/
uint32_t tms9980a_device::execute_min_cycles() const noexcept
{
	return 12;
}

/*
    The maximum number of cycles applies to a DIV command, depending on the
    data to be divided, and the mode of adressing.
*/
uint32_t tms9980a_device::execute_max_cycles() const noexcept
{
	return 136;
}

// device_disasm_interface overrides

std::unique_ptr<util::disasm_interface> tms9980a_device::create_disassembler()
{
	return std::make_unique<tms9900_disassembler>(TMS9980_ID);
}

DEFINE_DEVICE_TYPE(TMS9980A, tms9980a_device, "tms9980a", "Texas Instruments TMS9980A")
DEFINE_DEVICE_TYPE(TMS9981, tms9981_device, "tms9981", "Texas Instruments TMS9981")
