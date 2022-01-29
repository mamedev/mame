// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    NEC 78K/II series 8-bit single-chip microcontrollers

    Currently these devices are just stubs with no actual execution core.

****************************************************************************/

#include "emu.h"
#include "upd78k2.h"
#include "upd78k2d.h"

// device type definitions
DEFINE_DEVICE_TYPE(UPD78210, upd78210_device, "upd78210", "NEC uPD78210")
DEFINE_DEVICE_TYPE(UPD78213, upd78213_device, "upd78213", "NEC uPD78213")

//**************************************************************************
//  78K/II CORE
//**************************************************************************

//-------------------------------------------------
//  upd78k2_device - constructor
//-------------------------------------------------

upd78k2_device::upd78k2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int iram_bits, address_map_constructor mem_map, address_map_constructor sfr_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, mem_map)
	, m_iram_config("iram", ENDIANNESS_LITTLE, 16, iram_bits, 0, address_map_constructor(FUNC(upd78k2_device::iram_map), this))
	, m_sfr_config("sfr", ENDIANNESS_LITTLE, 16, 8, 0, sfr_map)
	, m_iram_addrmask((offs_t(1) << iram_bits) - 1)
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

void upd78k2_device::iram_map(address_map &map)
{
	map(0, m_iram_addrmask).ram().share("iram");
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd78k2_device::memory_space_config() const
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

inline u8 upd78k2_device::register_base() const noexcept
{
	return ((BIT(m_psw, 5) ? 0xe0 : 0xf0) & m_iram_addrmask) | (~m_psw & 0x08);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd78k2_device::device_start()
{
	// get address spaces and access caches
	space(AS_PROGRAM).specific(m_program_space);
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_DATA).cache(m_iram_cache);
	space(AS_IO).specific(m_sfr_space);

	set_icountptr(m_icount);

	// debug state
	state_add(UPD78K2_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(UPD78K2_PSW, "PSW", m_psw).mask(0xfb);
	state_add(STATE_GENFLAGS, "FLAGS", m_psw).mask(0xfb).formatstr("%9s").noshow();
	state_add<u8>(UPD78K2_RBS, "RBS",
		[this]() { return bitswap<2>(m_psw, 5, 3); },
		[this](u8 data) { m_psw = (m_psw & 0xd7) | (data & 2) << 4 | (data & 1) << 3; }
	).mask(3).noshow();
	state_add(UPD78K2_SP, "SP", m_sp);
	void *iram = memshare("iram")->ptr();
	for (int n = 0; n < 4; n++)
		state_add<u16>(UPD78K2_AX + n, std::array<const char *, 4>{{"AX", "BC", "DE", "HL"}}[n],
			[this, iram, n]() { return static_cast<u16 *>(iram)[(register_base() >> 1) | n]; },
			[this, iram, n](u16 data) { static_cast<u16 *>(iram)[(register_base() >> 1) | n] = data; }
		);
	for (int n = 0; n < 8; n++)
		state_add<u8>(UPD78K2_X + n, std::array<const char *, 8>{{"X", "A", "C", "B", "E", "D", "L", "H"}}[n],
			[this, iram, n]() { return static_cast<u8 *>(iram)[BYTE_XOR_LE(register_base() | n)]; },
			[this, iram, n](u8 data) { static_cast<u8 *>(iram)[BYTE_XOR_LE(register_base() | n)] = data; }
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

void upd78k2_device::device_reset()
{
	// PC will be initialized from vector following reset
	m_psw = 0x02;
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void upd78k2_device::execute_run()
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

void upd78k2_device::state_string_export(const device_state_entry &entry, std::string &str) const
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
//  78K/II SUBSERIES DEVICES
//**************************************************************************

//-------------------------------------------------
//  upd78210_device - constructor
//-------------------------------------------------

upd78210_device::upd78210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd78k2_device(mconfig, UPD78210, tag, owner, clock, 7,
						address_map_constructor(),
						address_map_constructor(FUNC(upd78210_device::sfr_map), this))
{
}


//-------------------------------------------------
//  create_disassembler -
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> upd78210_device::create_disassembler()
{
	return std::make_unique<upd78214_disassembler>();
}


//-------------------------------------------------
//  sfr_map - type-specific SFR map
//-------------------------------------------------

void upd78210_device::sfr_map(address_map &map)
{
	// TODO
}


//-------------------------------------------------
//  upd78213_device - constructor
//-------------------------------------------------

upd78213_device::upd78213_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd78k2_device(mconfig, UPD78213, tag, owner, clock, 8,
						address_map_constructor(FUNC(upd78213_device::mem_map), this),
						address_map_constructor(FUNC(upd78213_device::sfr_map), this))
{
}


//-------------------------------------------------
//  create_disassembler -
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> upd78213_device::create_disassembler()
{
	return std::make_unique<upd78214_disassembler>();
}


//-------------------------------------------------
//  mem_map - type-specific internal memory map
//  (excluding IRAM and SFRs)
//-------------------------------------------------

void upd78213_device::mem_map(address_map &map)
{
	// no ROM
	map(0x0fd00, 0x0fdff).ram(); // PRAM
}


//-------------------------------------------------
//  sfr_map - type-specific SFR map
//-------------------------------------------------

void upd78213_device::sfr_map(address_map &map)
{
	// TODO
}
