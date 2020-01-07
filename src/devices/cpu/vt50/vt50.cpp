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
	, m_uart_rd_callback(*this)
	, m_uart_xd_callback(*this)
	, m_ur_flag_callback(*this)
	, m_ut_flag_callback(*this)
	, m_ruf_callback(*this)
	, m_key_up_callback(*this)
	, m_bell_callback(*this)
	, m_bbits(bbits)
	, m_ybits(ybits)
	, m_pc(0)
	, m_rom_page(0)
	, m_mode_ff(false)
	, m_done_ff(false)
	, m_ac(0)
	, m_buffer(0)
	, m_x(0)
	, m_y(0)
	, m_x8(false)
	, m_cursor_ff(false)
	, m_video_process(false)
	, m_ram_do(0)
	, m_t(0)
	, m_write_ff(false)
	, m_flag_test_ff(false)
	, m_m2u_ff(false)
	, m_bell_ff(false)
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

void vt5x_cpu_device::device_resolve_objects()
{
	// resolve callbacks
	m_uart_rd_callback.resolve_safe(0);
	m_uart_xd_callback.resolve_safe();
	m_ur_flag_callback.resolve_safe(0);
	m_ut_flag_callback.resolve_safe(0);
	m_ruf_callback.resolve_safe();
	m_key_up_callback.resolve_safe(1);
	m_bell_callback.resolve_safe();
}

void vt5x_cpu_device::device_start()
{
	// acquire address spaces
	m_rom_cache = space(AS_PROGRAM).cache<0, 0, ENDIANNESS_LITTLE>();
	m_ram_cache = space(AS_DATA).cache<0, 0, ENDIANNESS_LITTLE>();

	set_icountptr(m_icount);

	state_add(VT5X_PC, "PC", m_pc).formatstr("%04O").mask(01777);
	state_add(STATE_GENPC, "GENPC", m_pc).mask(01777).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(01777).noshow();
	state_add<u8>(STATE_GENFLAGS, "CURFLAGS", [this]() {
		return (m_mode_ff ? 1 : 0) | (m_done_ff ? 2 : 0);
	}).formatstr("%7s").noshow();
	state_add(VT5X_PAGE, "PAGE", m_rom_page).mask(3);
	state_add(VT5X_MODE, "MODE", m_mode_ff).noshow();
	state_add(VT5X_DONE, "DONE", m_done_ff).noshow();
	state_add(VT5X_AC, "AC", m_ac).formatstr("%03O").mask(0177);
	state_add(VT5X_B, "B", m_buffer).formatstr(m_bbits > 6 ? "%03O" : "%02O").mask((1 << m_bbits) - 1);
	state_add(VT5X_X, "X", m_x).formatstr("%03O").mask(0177);
	state_add(VT5X_Y, "Y", m_y).formatstr("%02O").mask((1 << m_ybits) - 1);
	state_add(VT5X_X8, "X8", m_x8);
	state_add(VT5X_CFF, "CFF", m_cursor_ff);
	state_add(VT5X_VID, "VID", m_video_process);

	// save state
	save_item(NAME(m_pc));
	save_item(NAME(m_rom_page));
	save_item(NAME(m_mode_ff));
	save_item(NAME(m_done_ff));
	save_item(NAME(m_ac));
	save_item(NAME(m_buffer));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_x8));
	save_item(NAME(m_cursor_ff));
	save_item(NAME(m_video_process));
	save_item(NAME(m_ram_do));
	save_item(NAME(m_t));
	save_item(NAME(m_write_ff));
	save_item(NAME(m_flag_test_ff));
	save_item(NAME(m_m2u_ff));
	save_item(NAME(m_bell_ff));
	save_item(NAME(m_load_pc));
	save_item(NAME(m_qa_e23));
}

void vt5x_cpu_device::device_reset()
{
	m_pc = 0;
	m_rom_page = 0;
	m_video_process = false;

	// CPU is initialized in weird state that does not allow first instruction to fully execute
	m_t = 7;
	m_flag_test_ff = false;
	m_load_pc = false;
	m_qa_e23 = false;
}

offs_t vt5x_cpu_device::translate_xy() const
{
	const u8 x = m_x ^ (m_x8 ? 8 : 0);
	const offs_t y_shifted = (offs_t(m_y) << (10 - m_ybits)) & 01700;
	const offs_t ram_bank = offs_t(m_y & ((1 << (m_ybits - 4)) - 1)) << 10;
	if (BIT(x, 6) || (y_shifted & 01400) == 01400)
		return (x & 0017) | (y_shifted & 01400) >> 4 | y_shifted | 01400 | ram_bank;
	else
		return x | y_shifted | ram_bank;
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
			m_x8 = false;
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
			m_x8 = false;
			break;

		case 0160:
			// M0
			m_mode_ff = false;
			break;
		}
	}

	if (m_m2u_ff)
	{
		m_uart_xd_callback(m_ram_do);
		m_m2u_ff = false;
	}

	m_flag_test_ff = false;
	m_load_pc = false;
	if (!BIT(inst, 7))
		m_done_ff = false;
	else if (!m_mode_ff)
		m_ac = (m_ac + 1) & 0177;
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
			m_rom_page = (m_rom_page + 1) & 3;
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

	// RUF is transparently latched from U2M decode
	m_ruf_callback((inst & 0362) == 0122 ? 0 : 1);
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
			// CBFF
			m_bell_ff = !m_bell_ff;
			m_bell_callback(m_bell_ff);
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
	if (BIT(inst, 7))
		m_write_ff = (m_mode_ff || m_ac >= m_ram_do) && !m_done_ff;
	else
		m_write_ff = (inst & 0162) == 0022 || (inst & 0162) == 0062 || (inst & 0162) == 0122;

	// DONE is set by any RAM write, not just LD
	if (m_write_ff)
		m_done_ff = true;
}

void vt50_cpu_device::execute_tg(u8 inst)
{
	switch (inst & 0362)
	{
	case 0022:
		// A2M
		m_ram_do = m_ac;
		break;

	case 0062:
		// L40M (http://catb.org/jargon/html/O/octal-forty.html)
		m_ram_do = 040;
		break;

	case 0122:
		// U2M
		m_ram_do = m_uart_rd_callback() & 0177;
		break;

	default:
		// LD
		m_ram_do = inst & 0177;
		break;
	}

	m_ram_cache->write_byte(translate_xy(), m_ram_do);
	m_write_ff = false;
}

void vt52_cpu_device::execute_tg(u8 inst)
{
	switch (inst & 0362)
	{
	case 0022:
		// A2M
		m_ram_do = m_ac;
		break;

	case 0062:
		// B2M
		m_ram_do = m_buffer;
		break;

	case 0122:
		// U2M
		m_ram_do = m_uart_rd_callback() & 0177;
		break;

	default:
		// LD
		m_ram_do = inst & 0177;
		break;
	}

	m_ram_cache->write_byte(translate_xy(), m_ram_do);
	m_write_ff = false;
}

void vt5x_cpu_device::execute_th(u8 inst)
{
	if (inst == 0002)
	{
		// M2A
		m_ac = m_ram_do;
	}
	else if (inst == 0042)
	{
		// M2U
		m_m2u_ff = true;
	}
	else if (inst == 0102)
	{
		// M2X
		m_x = m_ram_do;
	}
	else if (inst == 0142)
	{
		// M2B
		m_buffer = m_ram_do & ((1 << m_bbits) - 1);
	}

	if (m_flag_test_ff)
	{
		switch (inst & 0160)
		{
		case 0000:
			// M0: PSCJ (TODO)
			// M1: URJ
			if (m_mode_ff)
				m_load_pc = m_ur_flag_callback();
			break;

		case 0020:
			// M0: TABJ
			// M1: AEMJ
			if (m_mode_ff)
				m_load_pc = m_ac == m_ram_do;
			else
				m_load_pc = (m_ac & 7) == 7;
			break;

		case 0040:
			// M0: KCLJ (TODO)
			// M1: ALMJ
			if (m_mode_ff)
				m_load_pc = m_ac < m_ram_do;
			break;

		case 0060:
			// M0: FRQJ (TODO)
			// M1: ADXJ
			if (m_mode_ff)
				m_load_pc = m_ac != (m_x ^ (m_x8 ? 8 : 0));
			break;

		case 0100:
			// M0: PRQJ (TODO)
			// M1: AEM2J
			if (m_mode_ff)
				m_load_pc = m_ac == m_ram_do;
			break;

		case 0120:
			// TRUJ
			m_load_pc = true;
			break;

		case 0140:
			// M0: UTJ
			// M1: VSCJ (TODO)
			if (!m_mode_ff)
				m_load_pc = !m_ut_flag_callback();
			break;

		case 0160:
			// M0: TOSJ (TODO)
			// M1: KEYJ
			if (m_mode_ff)
				m_load_pc = m_key_up_callback(m_ac) & 1;
			break;
		}
	}

	if ((m_pc & 0377) == 0377)
		m_rom_page = (m_rom_page + 1) & 3;
	m_pc = (m_pc + 1) & 03777;
}

void vt5x_cpu_device::execute_tj(u8 dest)
{
	if (m_load_pc)
		m_pc = u16(m_rom_page) << 8 | dest;
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
			if (!m_write_ff)
				m_ram_do = m_ram_cache->read_byte(translate_xy()) & 0177;
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
