// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC RX01 skeleton CPU device

***************************************************************************/

#include "emu.h"
#include "rx01.h"
#include "rx01dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(RX01_CPU, rx01_cpu_device, "rx01_cpu", "DEC RX01 CPU")

rx01_cpu_device::rx01_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, RX01_CPU, tag, owner, clock)
	, m_rom_config("program", ENDIANNESS_LITTLE, 8, 12, 0)
	, m_sp_config("scratchpad", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(rx01_cpu_device::scratchpad_map), this))
	, m_rom_cache(nullptr)
	, m_sp_cache(nullptr)
	, m_pc(0)
	, m_cntr(0)
	, m_sr(0)
	, m_spar(0)
	, m_bar(0)
	, m_crc(0)
	, m_icount(0)
{
	m_rom_config.m_is_octal = true;
	m_sp_config.m_is_octal = true;
}

std::unique_ptr<util::disasm_interface> rx01_cpu_device::create_disassembler()
{
	return std::make_unique<rx01_disassembler>();
}

void rx01_cpu_device::scratchpad_map(address_map &map)
{
	map(0, 15).ram().share("scratchpad"); // two 7489 16x4 register files
}

device_memory_interface::space_config_vector rx01_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_rom_config),
		std::make_pair(AS_DATA, &m_sp_config)
	};
}

void rx01_cpu_device::device_start()
{
	m_rom_cache = space(AS_PROGRAM).cache<0, 0, ENDIANNESS_LITTLE>();
	m_sp_cache = space(AS_DATA).cache<0, 0, ENDIANNESS_LITTLE>();

	set_icountptr(m_icount);

	// Debug state registration
	state_add(RX01_PC, "PC", m_pc).mask(07777).formatstr("%04O");
	state_add(STATE_GENPC, "GENPC", m_pc).mask(07777).formatstr("%04O").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(07777).formatstr("%04O").noshow();
	state_add(RX01_CNTR, "CNTR", m_cntr).formatstr("%03O");
	state_add(RX01_SR, "SR", m_sr).formatstr("%03O");
	state_add(RX01_SPAR, "SPAR", m_spar).mask(15).formatstr("%3s");
	u8 *sp = static_cast<u8 *>(memshare("scratchpad")->ptr());
	for (int r = 0; r < 16; r++)
		state_add(RX01_R0 + r, string_format("R%d", r).c_str(), sp[r]).formatstr("%03O");
	state_add(RX01_BAR, "BAR", m_bar).mask(07777).formatstr("%04O");
	state_add(RX01_CRC, "CRC", m_crc).formatstr("%06O");

	// Save state registration
	save_item(NAME(m_pc));
	save_item(NAME(m_cntr));
	save_item(NAME(m_sr));
	save_item(NAME(m_spar));
	save_item(NAME(m_bar));
	save_item(NAME(m_crc));
}

void rx01_cpu_device::device_reset()
{
	// Clear address registers, counters and flags
	m_pc = 0;
	m_bar = 0;
	m_cntr = 0;
	m_sr = 0;
	m_spar = 0;
}

void rx01_cpu_device::execute_run()
{
	// TODO
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}

void rx01_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case RX01_SPAR:
		str = string_format("R%-2d", m_spar);
		break;
	}
}
