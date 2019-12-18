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
	, m_t(0)
	, m_write_ff(false)
	, m_flag_test_ff(false)
	, m_load_pc(false)
	, m_qa_e23(0)
	, m_icount(0)
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
	save_item(NAME(m_t));
	save_item(NAME(m_write_ff));
	save_item(NAME(m_flag_test_ff));
	save_item(NAME(m_load_pc));
	save_item(NAME(m_qa_e23));
}

void vt5x_cpu_device::device_reset()
{
	m_pc = 0;
	m_rom_bank = 0;
	m_video_process = false;
	m_t = 7;
	m_flag_test_ff = false;
	m_load_pc = false;
	m_qa_e23 = false;
}

void vt5x_cpu_device::execute_te(u8 inst)
{
	if (BIT(inst, 3) && !BIT(inst, 7))
	{
		switch (inst & 0160)
		{
		case 0000:
			// ZXZY
			m_x = 0;
			m_y = 0;
			break;

		case 0020:
			// X8
			m_x8 = !m_x8;
			break;

		case 0040:
			// IXDY
			m_x = (m_x + 1) & 0177;
			m_y = (m_y - 1) & ((1 << m_ybits) - 1);
			break;

		case 0060:
			// IX
			m_x = (m_x + 1) & 0177;
			break;

		case 0100:
			// ZA
			m_ac = 0;
			break;

		case 0120:
			// M1
			m_mode_ff = true;
			break;

		case 0140:
			// ZX
			m_x = 0;
			break;

		case 0160:
			// M0
			m_mode_ff = false;
			break;
		}
	}

	m_flag_test_ff = false;
	m_load_pc = false;
	if (!BIT(inst, 7))
		m_done_ff = false;
}

void vt5x_cpu_device::execute_tf(u8 inst)
{
	if (BIT(inst, 2) && !BIT(inst, 7))
	{
		switch (inst & 0160)
		{
		case 0000:
			// DXDY
			m_x = (m_x - 1) & 0177;
			m_y = (m_y - 1) & ((1 << m_ybits) - 1);
			break;

		case 0020:
		case 0040:
			// IA or IA1
			m_ac = (m_ac + 1) & 0177;
			break;

		case 0060:
			// IY
			m_y = (m_y + 1) & ((1 << m_ybits) - 1);
			break;

		case 0100:
			// DY
			m_y = (m_y - 1) & ((1 << m_ybits) - 1);
			break;

		case 0120:
			// IROM
			m_rom_bank = (m_rom_bank + 1) & 3;
			break;

		case 0140:
			// DX
			m_x = (m_x - 1) & 0177;
			break;

		case 0160:
			// DA
			m_ac = (m_ac - 1) & 0177;
			break;
		}
	}
}

void vt5x_cpu_device::execute_tw(u8 inst)
{
	if ((inst & 0217) == 0)
	{
		switch (inst & 0160)
		{
		case 0000:
			// SCFF
			m_cursor_ff = true;
			break;

		case 0020:
			// SVID
			m_video_process = true;
			break;

		case 0040:
			// B2Y
			m_y = m_buffer & ((1 << m_ybits) - 1);
			break;

		case 0060:
			// CBFF (TODO)
			break;

		case 0100:
			// ZCAV
			m_cursor_ff = false;
			m_video_process = false;
			break;

		case 0120:
			// LPB (TODO: load print shift register)
			break;

		case 0140:
			// EPR (TODO: start printer)
			break;

		case 0160:
			// HPR!ZY (TODO: halt printer)
			m_y = 0;
			break;
		}
	}

	if (BIT(inst, 0) && !BIT(inst, 7))
		m_flag_test_ff = true;

	// set FF for instructions that write to RAM
	m_write_ff = (BIT(inst, 7) && !m_done_ff) || inst == 022 || inst == 042 || inst == 062;
	if (m_write_ff)
		m_done_ff = true;
}

void vt5x_cpu_device::execute_tg(u8 inst)
{
	// TODO

	m_write_ff = false;
}

void vt5x_cpu_device::execute_th(u8 inst)
{
	if (m_flag_test_ff)
	{
		switch (inst & 0160)
		{
		case 0000:
			// M0: PSCJ (TODO)
			// M1: URJ (TODO)
			break;

		case 0020:
			// M0: TABJ
			// M1: AEMJ (TODO)
			if (!m_mode_ff)
				m_load_pc = (m_ac & 7) == 7;
			break;

		case 0040:
			// M0: KCLJ (TODO)
			// M1: ALMJ (TODO)
			break;

		case 0060:
			// M0: FRQJ (TODO)
			// M1: ADXJ
			if (m_mode_ff)
				m_load_pc = m_ac != m_x;
			break;

		case 0100:
			// M0: PRQJ (TODO)
			// M1: AEM2J (TODO)
			break;

		case 0120:
			// TRUJ
			m_load_pc = true;
			break;

		case 0140:
			// M0: UTJ (TODO)
			// M1: VSCJ (TODO)
			break;

		case 0160:
			// M0: TOSJ (TODO)
			// M1: KEYJ (TODO)
			break;
		}
	}

	if ((m_pc & 0377) == 0377)
		m_rom_bank = (m_rom_bank + 1) & 3;
	m_pc = (m_pc + 1) & 03777;
}

void vt5x_cpu_device::execute_tj(u8 dest)
{
	if (m_load_pc)
		m_pc = u16(m_rom_bank) << 8 | dest;
	else
		m_pc = (m_pc + 1) & 03777;
}

void vt5x_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		switch (m_t)
		{
		case 0:
			if (!m_qa_e23)
				debugger_instruction_hook(m_pc);
			m_t = 1;
			break;

		case 1:
			if (!m_qa_e23)
				execute_te(m_rom_cache->read_byte(m_pc));
			m_t = 2;
			break;

		case 2:
			if (!m_qa_e23)
				execute_tf(m_rom_cache->read_byte(m_pc));
			m_t = 3;
			break;

		case 3:
			if (m_qa_e23 && m_write_ff)
				execute_tg(m_rom_cache->read_byte(m_pc));
			m_t = 4;
			break;

		case 4:
			if (m_qa_e23)
				execute_th(m_rom_cache->read_byte(m_pc));
			m_t = 5;
			break;

		case 5:
			m_t = 6;
			break;

		case 6:
			if (m_qa_e23 && m_flag_test_ff)
				execute_tj(m_rom_cache->read_byte(m_pc));
			m_t = 7;
			break;

		case 7:
			if (!m_qa_e23)
				execute_tw(m_rom_cache->read_byte(m_pc));
			m_t = 8;
			break;

		case 8:
			m_t = 0;
			m_qa_e23 = !m_qa_e23;
			break;
		}

		m_icount--;
	}
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
