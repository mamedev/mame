// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    NEC 78K/IV series 16/8-bit single-chip microcontrollers

    Currently these devices are just stubs with no actual execution core.

****************************************************************************/

#include "emu.h"
#include "upd78k4.h"
#include "upd78k4d.h"

// device type definition
DEFINE_DEVICE_TYPE(UPD784031, upd784031_device, "upd784031", "NEC uPD784031")

//**************************************************************************
//  78K/IV CORE
//**************************************************************************

//-------------------------------------------------
//  upd78k4_device - constructor
//-------------------------------------------------

upd78k4_device::upd78k4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor sfr_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, mem_map)
	, m_iram_config("iram", ENDIANNESS_LITTLE, 16, 9, 0, address_map_constructor(FUNC(upd78k4_device::iram_map), this))
	, m_sfr_config("sfr", ENDIANNESS_LITTLE, 16, 8, 0, sfr_map)
	, m_iram(*this, "iram")
	, m_pc(0)
	, m_ppc(0)
	, m_psw(0)
	, m_sp(0)
	, m_exp_reg{0, 0, 0, 0}
	, m_icount(0)
{
}


//-------------------------------------------------
//  iram_map - type-universal IRAM map
//-------------------------------------------------

void upd78k4_device::iram_map(address_map &map)
{
	map(0x000, 0x1ff).ram().share("iram");
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd78k4_device::memory_space_config() const
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

inline u16 upd78k4_device::register_base() const noexcept
{
	return 0x180 | (~m_psw & 0x7000) >> 8;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd78k4_device::device_start()
{
	// get address spaces and access caches
	space(AS_PROGRAM).specific(m_program_space);
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_DATA).cache(m_iram_cache);
	space(AS_IO).specific(m_sfr_space);

	set_icountptr(m_icount);

	// debug state
	state_add(UPD78K4_PC, "PC", m_pc).mask(0xfffff);
	state_add(STATE_GENPC, "GENPC", m_pc).mask(0xfffff).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).mask(0xfffff).noshow();
	state_add(UPD78K4_PSW, "PSW", m_psw).mask(0xf0fd);
	state_add(STATE_GENFLAGS, "FLAGS", m_psw).mask(0xf0fd).formatstr("%12s").noshow();
	state_add<u8>(UPD78K4_PSWL, "PSWL",
		[this]() { return m_psw & 0x00ff; },
		[this](u8 data) { m_psw = (m_psw & 0xff00) | data; }
	).mask(0xfd).noshow();
	state_add<u8>(UPD78K4_PSWH, "PSWH",
		[this]() { return (m_psw & 0xff00) >> 8; },
		[this](u8 data) { m_psw = (m_psw & 0x00ff) | u16(data) << 8; }
	).mask(0xf0).noshow();
	state_add<u8>(UPD78K4_RBS, "RBS",
		[this]() { return BIT(m_psw, 12, 3); },
		[this](u8 data) { m_psw = (m_psw & 0x8fff) | u16(data) << 12; }
	).mask(7).noshow();
	state_add(UPD78K4_SP, "SP", m_sp).mask(0xffffff);
	for (int n = 0; n < 4; n++)
		state_add<u16>(UPD78K4_RP0 + n, string_format("RP%d", n).c_str(),
			[this, n]() { return m_iram[register_base() >> 1 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | n] = data; }
		).formatstr("%9s");
	for (int n = 0; n < 2; n++)
		state_add<u16>(UPD78K4_AX + n, std::array<const char *, 2>{{"AX", "BC"}}[n],
			[this, n]() { return m_iram[register_base() >> 1 | (m_psw & 0x0020) >> 4 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | (m_psw & 0x0020) >> 4 | n] = data; }
		).noshow();
	for (int n = 0; n < 4; n++)
	{
		const char *rgname = std::array<const char *, 4>{{"VVP", "UUP", "TDE", "WHL"}}[n];
		state_add<u32>(UPD78K4_VVP + n, rgname,
			[this, n]() { return u32(m_exp_reg[n]) << 16 | m_iram[register_base() >> 1 | 0x04 | n]; },
			[this, n](u32 data) { m_exp_reg[n] = data >> 16; m_iram[register_base() >> 1 | 0x04 | n] = data & 0xffff; }
		).mask(0xffffff);
		state_add<u16>(UPD78K4_VP + n, rgname + 1,
			[this, n]() { return m_iram[register_base() >> 1 | 0x04 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | 0x04 | n] = data; }
		).noshow();
		state_add<u16>(UPD78K4_RP4 + n, string_format("RP%d", 4 + n).c_str(),
			[this, n]() { return m_iram[register_base() >> 1 | 0x04 | n]; },
			[this, n](u16 data) { m_iram[register_base() >> 1 | 0x04 | n] = data; }
		).noshow();
		state_add(UPD78K4_V + n, std::array<const char *, 4>{{"V", "U", "T", "W"}}[n], m_exp_reg[n]).noshow();
	}
	for (int n = 0; n < 16; n++)
		state_add<u8>(UPD78K4_R0 + n, string_format("R%d", n).c_str(),
			[this, n]() { return reinterpret_cast<u8 *>(&m_iram[0])[BYTE_XOR_LE(register_base() | n)]; },
			[this, n](u8 data) { reinterpret_cast<u8 *>(&m_iram[0])[BYTE_XOR_LE(register_base() | n)] = data; }
		).noshow();
	for (int n = 0; n < 4; n++)
		state_add<u8>(UPD78K4_X + n, std::array<const char *, 4>{{"X", "A", "C", "B"}}[n],
			[this, n]() { return reinterpret_cast<u8 *>(&m_iram[0])[BYTE_XOR_LE(register_base() | (m_psw & 0x0020) >> 3 | n)]; },
			[this, n](u8 data) { reinterpret_cast<u8 *>(&m_iram[0])[BYTE_XOR_LE(register_base() | (m_psw & 0x0020) >> 3 | n)] = data; }
		).noshow();
	for (int n = 0; n < 8; n++)
		state_add<u8>(UPD78K4_VPL + n, std::array<const char *, 8>{{"VPL", "VPH", "UPL", "UPH", "E", "D", "L", "H"}}[n],
			[this, n]() { return reinterpret_cast<u8 *>(&m_iram[0])[BYTE_XOR_LE(register_base() | 0x08 | n)]; },
			[this, n](u8 data) { reinterpret_cast<u8 *>(&m_iram[0])[BYTE_XOR_LE(register_base() | 0x08 | n)] = data; }
		).noshow();

	// save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_sp));
	save_item(NAME(m_exp_reg));
	save_item(NAME(m_psw));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd78k4_device::device_reset()
{
	// PC will be initialized from vector following reset
	m_psw = 0x0000;
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void upd78k4_device::execute_run()
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

void upd78k4_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c RB%d:%c%c%c%c%c%c0%c",
				BIT(m_psw, 15) ? 'U' : '.',
				(m_psw & 0x7000) >> 12,
				BIT(m_psw, 7) ? 'S' : '.',
				BIT(m_psw, 6) ? 'Z' : '.',
				BIT(m_psw, 5) ? 'R' : '.',
				BIT(m_psw, 4) ? 'A' : '.',
				BIT(m_psw, 3) ? 'I' : '.',
				BIT(m_psw, 2) ? 'V' : '.',
				BIT(m_psw, 0) ? 'C' : '.');
		break;

	case UPD78K4_RP0:
		str = string_format("%04X %s", m_iram[register_base() >> 1], BIT(m_psw, 5) ? "    " : "(AX)");
		break;

	case UPD78K4_RP1:
		str = string_format("%04X %s", m_iram[register_base() >> 1 | 1], BIT(m_psw, 5) ? "    " : "(BC)");
		break;

	case UPD78K4_RP2:
		str = string_format("%04X %s", m_iram[register_base() >> 1 | 2], BIT(m_psw, 5) ? "(AX)" : "    ");
		break;

	case UPD78K4_RP3:
		str = string_format("%04X %s", m_iram[register_base() >> 1 | 3], BIT(m_psw, 5) ? "(BC)" : "    ");
		break;
	}
}


//**************************************************************************
//  78K/IV SUBSERIES DEVICES
//**************************************************************************

//-------------------------------------------------
//  upd784031_device - constructor
//-------------------------------------------------

upd784031_device::upd784031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd78k4_device(mconfig, UPD784031, tag, owner, clock,
						address_map_constructor(FUNC(upd784031_device::sfr_map), this),
						address_map_constructor(FUNC(upd784031_device::sfr_map), this))
{
}


//-------------------------------------------------
//  create_disassembler -
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> upd784031_device::create_disassembler()
{
	return std::make_unique<upd784038_disassembler>();
}


//-------------------------------------------------
//  mem_map - type-specific internal memory map
//  (excluding IRAM and SFRs)
//-------------------------------------------------

void upd784031_device::mem_map(address_map &map)
{
	// no ROM
	//map(0x0f700, 0x0fcff).ram(); // PRAM (TODO: LOCATION 0H)
	map(0xff700, 0xffcff).ram(); // PRAM (LOCATION 0FH)
}


//-------------------------------------------------
//  sfr_map - type-specific SFR map
//-------------------------------------------------

void upd784031_device::sfr_map(address_map &map)
{
	// TODO
}
