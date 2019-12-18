// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VT50/VT52 CPU skeleton

***************************************************************************/

#include "emu.h"
#include "vt50.h"
#include "vt50dasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(VT50_CPU, vt50_cpu_device, "vt50_cpu", "DEC VT50 CPU")
DEFINE_DEVICE_TYPE(VT52_CPU, vt52_cpu_device, "vt52_cpu", "DEC VT52 CPU")

vt5x_cpu_device::vt5x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bbits, int ybits)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_rom_config("program", ENDIANNESS_LITTLE, 8, 10, 0)
	, m_ram_config("data", ENDIANNESS_LITTLE, 8, 6 + ybits, 0) // actually 7 bits wide
	, m_rom_cache(nullptr)
	, m_ram_cache(nullptr)
	, m_bbits(bbits)
	, m_ybits(ybits)
	, m_pc(0)
	, m_rom_bank(0)
	, m_mode_ff(false)
	, m_done_ff(false)
	, m_ac(0)
	, m_buffer(0)
	, m_x(0)
	, m_y(0)
	, m_x8(false)
	, m_cursor_ff(false)
	, m_video_process(false)
{
	m_rom_config.m_is_octal = true;
	m_ram_config.m_is_octal = true;
}

vt50_cpu_device::vt50_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: vt5x_cpu_device(mconfig, VT50_CPU, tag, owner, clock, 4, 4)
{
}

vt52_cpu_device::vt52_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: vt5x_cpu_device(mconfig, VT52_CPU, tag, owner, clock, 7, 5)
{
}

std::unique_ptr<util::disasm_interface> vt50_cpu_device::create_disassembler()
{
	return std::make_unique<vt50_disassembler>();
}

std::unique_ptr<util::disasm_interface> vt52_cpu_device::create_disassembler()
{
	return std::make_unique<vt52_disassembler>();
}

device_memory_interface::space_config_vector vt5x_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_rom_config),
		std::make_pair(AS_DATA, &m_ram_config)
	};
}

void vt5x_cpu_device::device_start()
{
	m_rom_cache = space(AS_PROGRAM).cache<0, 0, ENDIANNESS_LITTLE>();
	m_ram_cache = space(AS_DATA).cache<0, 0, ENDIANNESS_LITTLE>();

	set_icountptr(m_icount);

	state_add(VT5X_PC, "PC", m_pc).formatstr("%04O").mask(01777);
	state_add(STATE_GENPC, "GENPC", m_pc).mask(01777).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(01777).noshow();
	state_add<u8>(STATE_GENFLAGS, "CURFLAGS", [this]() {
		return (m_mode_ff ? 1 : 0) | (m_done_ff ? 2 : 0);
	}).formatstr("%7s").noshow();
	state_add(VT5X_BANK, "BANK", m_rom_bank).mask(3);
	state_add(VT5X_MODE, "MODE", m_mode_ff).noshow();
	state_add(VT5X_DONE, "DONE", m_done_ff).noshow();
	state_add(VT5X_AC, "AC", m_ac).formatstr("%03O").mask(0177);
	state_add(VT5X_B, "B", m_buffer).formatstr(m_bbits > 6 ? "%03O" : "%02O").mask((1 << m_bbits) - 1);
	state_add(VT5X_X, "X", m_x).formatstr("%03O").mask(0177);
	state_add(VT5X_Y, "Y", m_y).formatstr("%02O").mask((1 << m_ybits) - 1);
	state_add(VT5X_X8, "X8", m_x8);
	state_add(VT5X_CFF, "CFF", m_cursor_ff);
	state_add(VT5X_VID, "VID", m_video_process);

	save_item(NAME(m_pc));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_mode_ff));
	save_item(NAME(m_done_ff));
	save_item(NAME(m_ac));
	save_item(NAME(m_buffer));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_x8));
	save_item(NAME(m_cursor_ff));
	save_item(NAME(m_video_process));
}

void vt5x_cpu_device::device_reset()
{
	m_pc = 0;
	m_rom_bank = 0;
	m_video_process = false;
}

void vt5x_cpu_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void vt5x_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("M%d %4s", m_mode_ff ? 1 : 0, m_done_ff ? "DONE" : "");
		break;
	}
}
