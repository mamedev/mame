// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton CPU device.

***************************************************************************/

#include "emu.h"
#include "edsp.h"
#include "edspdasm.h"

DEFINE_DEVICE_TYPE(EMG2000A, emg2000a_device, "emg2000a", "Elan eMG2000A TV Game Processor")

edsp_device::edsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program_map, address_map_constructor data_map, address_map_constructor io_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, -1, program_map)
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 16, -1, data_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 7, -1, io_map)
	, m_sp(0)
	, m_r{0, 0, 0, 0, 0, 0, 0, 0}
{
}

emg2000a_device::emg2000a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: edsp_device(mconfig, EMG2000A, tag, owner, clock,
		address_map_constructor(FUNC(emg2000a_device::program_map), this),
		address_map_constructor(FUNC(emg2000a_device::data_map), this),
		address_map_constructor(FUNC(emg2000a_device::io_map), this))
{
}

void emg2000a_device::program_map(address_map &map)
{
	// TODO: internal PRAM
	//map(0x007000, 0x007fff).ram();
}

void emg2000a_device::data_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); // 2KW WRAM
	map(0x2000, 0x2fff).ram(); // 4KW BGA
	map(0x3000, 0x33ff).ram(); // 1KW SPA
	map(0x3400, 0x73ff).ram(); // 16KW VRAM
	//map(0x8000, 0x8fff); // PPU Register
	//map(0x9000, 0x9fff); // APU Register
	map(0xe000, 0xe3ff).ram(); // Palette table
}

void emg2000a_device::io_map(address_map &map)
{
	// TODO: everything
}

device_memory_interface::space_config_vector edsp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> edsp_device::create_disassembler()
{
	return std::make_unique<edsp_disassembler>();
}

void edsp_device::device_start()
{
	set_icountptr(m_icount);

	state_add(EDSP_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_pc).noshow();
	state_add(EDSP_SP, "SP", m_sp);
	state_add(EDSP_RC, "RC", m_rcr);
	state_add(EDSP_LC, "LC", m_lcr);
	state_add(EDSP_LSA, "LSA", m_lsa);
	state_add(EDSP_LEA, "LEA", m_lea);
	state_add(EDSP_SR, "SR", m_sr);
	for (int n = 0; n < 8; n++)
		state_add(EDSP_R0 + n, util::string_format("R%d", n).c_str(), m_r[n]);

	save_item(NAME(m_pc));
	save_item(NAME(m_sp));
	save_item(NAME(m_rcr));
	save_item(NAME(m_lcr));
	save_item(NAME(m_lsa));
	save_item(NAME(m_lea));
	save_item(NAME(m_sr));
	save_item(NAME(m_r));
}

void edsp_device::device_reset()
{
	m_pc = 0;
	m_rcr = 0;
	m_lcr = 0;
	m_lsa = 0;
	m_lea = 0;
	m_sr = 0;
}

void edsp_device::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}
