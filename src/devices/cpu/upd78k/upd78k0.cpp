// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    NEC 78K/0 series 8-bit single-chip microcontrollers

    Currently these devices are just stubs with no actual execution core.

****************************************************************************/

#include "emu.h"
#include "upd78k0.h"
#include "upd78k0d.h"

// device type definition
DEFINE_DEVICE_TYPE(UPD78053, upd78053_device, "upd78053", "NEC uPD78053")

//**************************************************************************
//  78K/0 CORE
//**************************************************************************

//-------------------------------------------------
//  upd78k0_device - constructor
//-------------------------------------------------

upd78k0_device::upd78k0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 iram_size, address_map_constructor mem_map, address_map_constructor sfr_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, mem_map)
	, m_iram_config("iram", ENDIANNESS_LITTLE, 16, iram_size > 0x200 ? 10 : iram_size > 0x100 ? 9 : 8, 0,
					address_map_constructor(FUNC(upd78k0_device::iram_map), this))
	, m_sfr_config("sfr", ENDIANNESS_LITTLE, 16, 8, 0, sfr_map)
	, m_iram_size(iram_size)
	, m_subclock(0)
	, m_pc(0)
	, m_ppc(0)
	, m_psw(0)
	, m_sp(0)
	, m_icount(0)
{
}


//-------------------------------------------------
//  iram_map - type-universal IRAM map
//-------------------------------------------------

void upd78k0_device::iram_map(address_map &map)
{
	if (m_iram_size > 0x200)
		map(0x400 - m_iram_size, 0x3ff).ram().share("iram");
	else if (m_iram_size > 0x100)
		map(0x200 - m_iram_size, 0x1ff).ram().share("iram");
	else
		map(0x00, 0xff).ram().share("iram");
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd78k0_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_iram_config),
		std::make_pair(AS_IO, &m_sfr_config)
	};
}


//-------------------------------------------------
//  register_base - determine current base of
//  register file in IRAM
//-------------------------------------------------

inline u16 upd78k0_device::register_base() const noexcept
{
	return (BIT(m_psw, 5) ? 0x3e0 : 0x3f0) | (~m_psw & 0x08);
}


//-------------------------------------------------
//  debug_register_base - determine current base of
//  register file relative to start of IRAM
//-------------------------------------------------

inline u16 upd78k0_device::debug_register_base() const noexcept
{
	return (m_iram_size - 0x20) | (BIT(m_psw, 5) ? 0 : 0x10) | (~m_psw & 0x08);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd78k0_device::device_start()
{
	// get address spaces and access caches
	space(AS_PROGRAM).specific(m_program_space);
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_DATA).cache(m_iram_cache);
	space(AS_IO).specific(m_sfr_space);

	set_icountptr(m_icount);

	// debug state
	state_add(UPD78K0_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(UPD78K0_PSW, "PSW", m_psw).mask(0xfb);
	state_add(STATE_GENFLAGS, "FLAGS", m_psw).mask(0xfb).formatstr("%9s").noshow();
	state_add<u8>(UPD78K0_RBS, "RBS",
		[this]() { return bitswap<2>(m_psw, 5, 3); },
		[this](u8 data) { m_psw = (m_psw & 0xd7) | (data & 2) << 4 | (data & 1) << 3; }
	).mask(3).noshow();
	state_add(UPD78K0_SP, "SP", m_sp);
	u16 *iram = static_cast<u16 *>(memshare("iram")->ptr());
	for (int n = 0; n < 4; n++)
		state_add<u16>(UPD78K0_AX + n, std::array<const char *, 4>{{"AX", "BC", "DE", "HL"}}[n],
			[this, iram, n]() { return iram[(debug_register_base() >> 1) | n]; },
			[this, iram, n](u16 data) { iram[(debug_register_base() >> 1) | n] = data; }
		);
	for (int n = 0; n < 8; n++)
		state_add<u8>(UPD78K0_X + n, std::array<const char *, 8>{{"X", "A", "C", "B", "E", "D", "L", "H"}}[n],
			[this, iram, n]() { return util::little_endian_cast<const u8>(iram)[debug_register_base() | n]; },
			[this, iram, n](u8 data) { util::little_endian_cast<u8>(iram)[debug_register_base() | n] = data; }
		).noshow();

	// save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_sp));
	save_item(NAME(m_psw));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd78k0_device::device_reset()
{
	// PC will be initialized from vector following reset
	m_psw = 0x02;
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void upd78k0_device::execute_run()
{
	m_pc = m_program_cache.read_word(0);
	m_ppc = m_pc;
	debugger_instruction_hook(m_pc);

	// TODO
	m_icount = 0;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//-------------------------------------------------

void upd78k0_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("RB%d:%c%c%c%c%c",
				(BIT(m_psw, 5) ? 2 : 0) | (BIT(m_psw, 3) ? 1 : 0),
				BIT(m_psw, 7) ? 'I' : '.',
				BIT(m_psw, 6) ? 'Z' : '.',
				BIT(m_psw, 4) ? 'A' : '.',
				BIT(m_psw, 1) ? 'P' : '.',
				BIT(m_psw, 0) ? 'C' : '.');
		break;
	}
}


//**************************************************************************
//  78K/0 SUBSERIES DEVICES
//**************************************************************************

//-------------------------------------------------
//  upd78053_device - constructor
//-------------------------------------------------

upd78053_device::upd78053_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd78k0_device(mconfig, UPD78053, tag, owner, clock, 0x400,
						address_map_constructor(FUNC(upd78053_device::mem_map), this),
						address_map_constructor(FUNC(upd78053_device::sfr_map), this))
{
}


//-------------------------------------------------
//  create_disassembler -
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> upd78053_device::create_disassembler()
{
	return std::make_unique<upd78054_disassembler>();
}


//-------------------------------------------------
//  mem_map - type-specific internal memory map
//  (excluding high-speed RAM and SFRs)
//-------------------------------------------------

void upd78053_device::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region(DEVICE_SELF, 0); // 24K mask ROM
	map(0xfa80, 0xfabf).unmaprw(); // reserved
	map(0xfac0, 0xfadf).ram().share("buffer"); // buffer RAM
	map(0xfae0, 0xfaff).unmaprw(); // reserved
}


//-------------------------------------------------
//  sfr_map - type-specific SFR map
//-------------------------------------------------

void upd78053_device::sfr_map(address_map &map)
{
	// TODO
}
