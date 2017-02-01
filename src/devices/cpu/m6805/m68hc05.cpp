// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "m68hc05.h"
#include "m6805defs.h"

/****************************************************************************
 * Configurable logging
 ****************************************************************************/

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"


namespace {

std::pair<u16, char const *> const m68705c4_syms[] = {
	{ 0x0000, "PORTA" }, { 0x0001, "PORTB" }, { 0x0002, "PORTC" }, { 0x0003, "PORTD" },
	{ 0x0004, "DDRA"  }, { 0x0005, "DDRB"  }, { 0x0006, "DDRC"  },
	{ 0x000a, "SPCR"  }, { 0x000b, "SPSR"  }, { 0x000c, "SPDR"  },
	{ 0x000d, "BAUD"  }, { 0x000e, "SCCR1" }, { 0x000f, "SCCR2" }, { 0x0010, "SCSR"  }, { 0x0011, "SCDR"  },
	{ 0x0012, "TCR"   }, { 0x0013, "TSR"   },
	{ 0x0014, "ICRH"  }, { 0x0015, "ICRL"  }, { 0x0016, "OCRH"  }, { 0x0017, "OCRL"  },
	{ 0x0018, "TRH"   }, { 0x0019, "TRL"   }, { 0x001a, "ATRH"  }, { 0x001b, "ATRL"  } };

} // anonymous namespace


/****************************************************************************
 * Global variables
 ****************************************************************************/

device_type const M68HC05C4 = &device_creator<m68hc05c4_device>;


/****************************************************************************
 * M68HC05 base device
 ****************************************************************************/

m68hc05_device::m68hc05_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock,
		device_type type,
		char const *name,
		address_map_delegate internal_map,
		char const *shortname,
		char const *source)
	: m6805_base_device(
			mconfig,
			tag,
			owner,
			clock,
			type,
			name,
			{ s_hc_ops, s_hc_cycles, 13, 0x00ff, 0x00c0, 0xfffc },
			internal_map,
			shortname,
			source)
{
}

void m68hc05_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	default:
		if (m_irq_state[inputnum] != state)
		{
			m_irq_state[inputnum] = (state == ASSERT_LINE) ? ASSERT_LINE : CLEAR_LINE;

			if (state != CLEAR_LINE)
				m_pending_interrupts |= 1 << inputnum;
		}
	}
}

uint64_t m68hc05_device::execute_clocks_to_cycles(uint64_t clocks) const
{
	return (clocks + 1) / 2;
}

uint64_t m68hc05_device::execute_cycles_to_clocks(uint64_t cycles) const
{
	return cycles * 2;
}

offs_t m68hc05_device::disasm_disassemble(
		std::ostream &stream,
		offs_t pc,
		const uint8_t *oprom,
		const uint8_t *opram,
		uint32_t options)
{
	return CPU_DISASSEMBLE_NAME(m68hc05)(this, stream, pc, oprom, opram, options);
}


/****************************************************************************
 * MC68HC05C4 device
 ****************************************************************************/

DEVICE_ADDRESS_MAP_START( c4_map, 8, m68hc05c4_device )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	ADDRESS_MAP_UNMAP_HIGH

	// 0x0000 PORTA
	// 0x0001 PORTB
	// 0x0002 PORTC
	// 0x0003 PORTD
	// 0x0004 DDRA
	// 0x0005 DDRB
	// 0x0006 DDRC
	// 0x0007-0x0009 unused
	// 0x000a SPCR
	// 0x000b SPSR
	// 0x000c SPDR
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	// 0x0012 TCR
	// 0x0013 TDR
	// 0x0014 ICRH
	// 0x0015 ICRL
	// 0x0016 OCRH
	// 0x0017 OCRL
	// 0x0018 TRH
	// 0x0019 TRL
	// 0x001a ATRH
	// 0x001b ATRL
	// 0x001c-0x001f unused
	AM_RANGE(0x0020, 0x004f) AM_ROM // user ROM
	AM_RANGE(0x0050, 0x00ff) AM_RAM // RAM/stack
	AM_RANGE(0x0100, 0x10ff) AM_ROM // user ROM
	// 0x1100-0x1eff unused
	AM_RANGE(0x1f00, 0x1fef) AM_ROM // self-check
	// 0x1ff0-0x1ff3 unused
	AM_RANGE(0x1ff4, 0x1fff) AM_ROM // user vectors
ADDRESS_MAP_END

m68hc05c4_device::m68hc05c4_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68hc05_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC05C4,
			"MC68HC05C4",
			address_map_delegate(FUNC(m68hc05c4_device::c4_map), this),
			"m68hc05c4",
			__FILE__)
{
}

offs_t m68hc05c4_device::disasm_disassemble(
		std::ostream &stream,
		offs_t pc,
		const uint8_t *oprom,
		const uint8_t *opram,
		uint32_t options)
{
	return CPU_DISASSEMBLE_NAME(m68hc05)(this, stream, pc, oprom, opram, options, m68705c4_syms);
}
