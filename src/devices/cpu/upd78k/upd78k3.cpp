// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    NEC 78K/III series 16/8-bit single-chip microcontrollers

    Currently these devices are just stubs with no actual execution core.

****************************************************************************/

#include "emu.h"
#include "upd78k3.h"
#include "upd78k3d.h"

// device type definitions
DEFINE_DEVICE_TYPE(UPD78310, upd78310_device, "upd78310", "NEC uPD78310")
DEFINE_DEVICE_TYPE(UPD78312, upd78312_device, "upd78312", "NEC uPD78312")

//**************************************************************************
//  78K/III CORE
//**************************************************************************

//-------------------------------------------------
//  upd78k3_device - constructor
//-------------------------------------------------

upd78k3_device::upd78k3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor sfr_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, mem_map)
	, m_iram_config("iram", ENDIANNESS_LITTLE, 16, 8, 0, address_map_constructor(FUNC(upd78k3_device::iram_map), this))
	, m_sfr_config("sfr", ENDIANNESS_LITTLE, 16, 8, 0, sfr_map)
	, m_iram(*this, "iram")
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

void upd78k3_device::iram_map(address_map &map)
{
	map(0x00, 0xff).ram().share("iram");
}


//-------------------------------------------------
//  iram_byte_r - read one byte from IRAM
//-------------------------------------------------

u8 upd78k3_device::iram_byte_r(offs_t offset)
{
	if (BIT(offset, 0))
		return (m_iram[offset >> 1] & 0xff00) >> 8;
	else
		return m_iram[offset >> 1] & 0x00ff;
}


//-------------------------------------------------
//  iram_byte_w - write one byte to IRAM
//-------------------------------------------------

void upd78k3_device::iram_byte_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_iram[offset >> 1] = (m_iram[offset >> 1] & 0x00ff) | u16(data) << 8;
	else
		m_iram[offset >> 1] = (m_iram[offset >> 1] & 0xff00) | data;
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd78k3_device::memory_space_config() const
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

inline u8 upd78k3_device::register_base() const noexcept
{
	return 0x80 | (~m_psw & 0x7000) >> 8;
}


//-------------------------------------------------
//  state_add_psw - overridable method for PSW
//  state registration
//-------------------------------------------------

void upd78k3_device::state_add_psw()
{
	state_add(UPD78K3_PSW, "PSW", m_psw).mask(0xf0fd);
	state_add(STATE_GENFLAGS, "FLAGS", m_psw).mask(0xf0fd).formatstr("%12s").noshow();
	state_add<u8>(UPD78K3_PSWL, "PSWL",
		[this]() { return m_psw & 0x00ff; },
		[this](u8 data) { m_psw = (m_psw & 0xff00) | data; }
	).mask(0xfd).noshow();
	state_add<u8>(UPD78K3_PSWH, "PSWH",
		[this]() { return (m_psw & 0xff00) >> 8; },
		[this](u8 data) { m_psw = (m_psw & 0x00ff) | u16(data) << 8; }
	).mask(0xf0).noshow();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd78k3_device::device_start()
{
	// get address spaces and access caches
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_PROGRAM).specific(m_program_space);
	space(AS_DATA).cache(m_iram_cache);
	space(AS_IO).specific(m_sfr_space);

	set_icountptr(m_icount);

	// debug state
	state_add(UPD78K3_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add_psw();
	state_add<u8>(UPD78K3_RBS, "RBS",
		[this]() { return (m_psw & 7000) >> 12; },
		[this](u8 data) { m_psw = (m_psw & 0x8fff) | u16(data) << 12; }
	).mask(7).noshow();
	state_add(UPD78K3_SP, "SP", m_sp);
	for (int n = 0; n < 4; n++)
		state_add<u16>(UPD78K3_RP0 + n, string_format("RP%d", n).c_str(),
			[this, n]() { return m_iram[register_base() >> 1 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | n] = data; }
		).formatstr("%9s");
	for (int n = 0; n < 2; n++)
		state_add<u16>(UPD78K3_AX + n, std::array<const char *, 2>{{"AX", "BC"}}[n],
			[this, n]() { return m_iram[register_base() >> 1 | (m_psw & 0x0020) >> 4 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | (m_psw & 0x0020) >> 4 | n] = data; }
		).noshow();
	for (int n = 0; n < 4; n++)
	{
		state_add<u16>(UPD78K3_VP + n, std::array<const char *, 4>{{"VP", "UP", "DE", "HL"}}[n],
			[this, n]() { return m_iram[register_base() >> 1 | 0x04 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | 0x04 | n] = data; }
		);
		state_add<u16>(UPD78K3_RP4 + n, string_format("RP%d", 4 + n).c_str(),
			[this, n]() { return m_iram[register_base() >> 1 | 0x04 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | 0x04 | n] = data; }
		).noshow();
	}
	for (int n = 0; n < 16; n++)
		state_add<u8>(UPD78K3_R0 + n, string_format("R%d", n).c_str(),
			[this, n]() { return iram_byte_r(register_base() | n); },
			[this, n](u8 data) { iram_byte_w(register_base() | n, data); }
		).noshow();
	for (int n = 0; n < 4; n++)
		state_add<u8>(UPD78K3_X + n, std::array<const char *, 4>{{"X", "A", "C", "B"}}[n],
			[this, n]() { return iram_byte_r(register_base() | (m_psw & 0x0020) >> 3 | n); },
			[this, n](u8 data) { iram_byte_w(register_base() | (m_psw & 0x0020) >> 3 | n, data); }
		).noshow();
	for (int n = 0; n < 8; n++)
		state_add<u8>(UPD78K3_VPL + n, std::array<const char *, 8>{{"VPL", "VPH", "UPL", "UPH", "E", "D", "L", "H"}}[n],
			[this, n]() { return iram_byte_r(register_base() | 0x08 | n); },
			[this, n](u8 data) { iram_byte_w(register_base() | 0x08 | n, data); }
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

void upd78k3_device::device_reset()
{
	// PC will be initialized from vector following reset
	m_psw = 0x0000;
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void upd78k3_device::execute_run()
{
	m_pc = m_program_cache.read_word(0);
	m_ppc = m_pc;
	debugger_instruction_hook(m_pc);

	// TODO
	m_icount = 0;
}


//-------------------------------------------------
//  state_string_export -
//-------------------------------------------------

void upd78k3_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("RB%d:%c%c%c%c%c%c%c%c",
				(m_psw & 0x7000) >> 12,
				BIT(m_psw, 15) ? 'U' : '.',
				BIT(m_psw, 7) ? 'S' : '.',
				BIT(m_psw, 6) ? 'Z' : '.',
				BIT(m_psw, 5) ? 'R' : '.',
				BIT(m_psw, 4) ? 'A' : '.',
				BIT(m_psw, 3) ? 'I' : '.',
				BIT(m_psw, 2) ? 'V' : '.',
				BIT(m_psw, 0) ? 'C' : '.');
		break;

	case UPD78K3_RP0:
		str = string_format("%04X %s", m_iram[register_base() >> 1], BIT(m_psw, 5) ? "    " : "(AX)");
		break;

	case UPD78K3_RP1:
		str = string_format("%04X %s", m_iram[register_base() >> 1 | 1], BIT(m_psw, 5) ? "    " : "(BC)");
		break;

	case UPD78K3_RP2:
		str = string_format("%04X %s", m_iram[register_base() >> 1 | 2], BIT(m_psw, 5) ? "(AX)" : "    ");
		break;

	case UPD78K3_RP3:
		str = string_format("%04X %s", m_iram[register_base() >> 1 | 3], BIT(m_psw, 5) ? "(BC)" : "    ");
		break;
	}
}


//**************************************************************************
//  78K/III SUBSERIES DEVICES
//**************************************************************************

//-------------------------------------------------
//  upd78312_device - constructor
//-------------------------------------------------

upd78312_device::upd78312_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd78312_device(mconfig, UPD78312, tag, owner, clock, address_map_constructor(FUNC(upd78312_device::mem_map), this))
{
}

upd78312_device::upd78312_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: upd78k3_device(mconfig, type, tag, owner, clock, map,
						address_map_constructor(FUNC(upd78312_device::sfr_map), this))
{
}


//-------------------------------------------------
//  upd78310_device - constructor
//-------------------------------------------------

upd78310_device::upd78310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd78312_device(mconfig, UPD78310, tag, owner, clock, address_map_constructor())
{
}


//-------------------------------------------------
//  create_disassembler -
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> upd78312_device::create_disassembler()
{
	return std::make_unique<upd78312_disassembler>();
}


//-------------------------------------------------
//  mem_map - type-specific internal memory map
//  (excluding IRAM and SFRs)
//-------------------------------------------------

void upd78312_device::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region(DEVICE_SELF, 0); // 8K mask ROM
	map(0xfe00, 0xfeff).rw(FUNC(upd78312_device::iram_byte_r), FUNC(upd78312_device::iram_byte_w));
}


//-------------------------------------------------
//  sfr_map - type-specific SFR map
//-------------------------------------------------

void upd78312_device::sfr_map(address_map &map)
{
	// TODO
}


//-------------------------------------------------
//  state_add_psw - overridable method for PSW
//  state registration
//-------------------------------------------------

void upd78312_device::state_add_psw()
{
	state_add(UPD78K3_PSW, "PSW", m_psw).mask(0x72ff);
	state_add(STATE_GENFLAGS, "FLAGS", m_psw).mask(0x72ff).formatstr("%13s").noshow();
	state_add<u8>(UPD78K3_PSWL, "PSWL",
		[this]() { return m_psw & 0x00ff; },
		[this](u8 data) { m_psw = (m_psw & 0xff00) | data; }
	).noshow();
	state_add<u8>(UPD78K3_PSWH, "PSWH",
		[this]() { return (m_psw & 0xff00) >> 8; },
		[this](u8 data) { m_psw = (m_psw & 0x00ff) | u16(data) << 8; }
	).mask(0x72).noshow();
}


//-------------------------------------------------
//  state_string_export -
//-------------------------------------------------

void upd78312_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("RB%d:%c%c%c%c%c%c%c%c%c",
				(m_psw & 0x7000) >> 12,
				BIT(m_psw, 9) ? 'I' : '.',
				BIT(m_psw, 7) ? 'S' : '.',
				BIT(m_psw, 6) ? 'Z' : '.',
				BIT(m_psw, 5) ? 'R' : '.',
				BIT(m_psw, 4) ? 'A' : '.',
				BIT(m_psw, 3) ? 'I' : '.',
				BIT(m_psw, 2) ? 'V' : '.',
				BIT(m_psw, 1) ? '-' : '+',
				BIT(m_psw, 0) ? 'C' : '.');
		break;

	default:
		upd78k3_device::state_string_export(entry, str);
		break;
	}
}
