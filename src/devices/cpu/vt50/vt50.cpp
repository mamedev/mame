// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VT50/VT52 microprocessor emulation

    The principal components of this custom TTL-based processor are
    inelegantly divided between two PCBs: ROM, UART & Timing (RUT) and
    Data Paths, Memory & Decoders. The UART present on the former board
    is not included in this CPU emulation, which uses callbacks instead
    (as for the keyboard, which is a separate component in the same case).
    Opcodes may contain up to four instructions each, which are executed
    sequentially during defined phases of the instruction cycle.

    The machine cycle time (each instruction takes two cycles) is also the
    time it takes to display one character. RAM addresses are determined
    by the contents of the X and Y registers (plus one XOR gate applied to
    bit 3 of the X output) for both displayed characters and programmed
    data transfers. During non-blanked portions of of horizontal lines, X
    (but not Y) is automatically incremented as each character is latched,
    with the lowest 3 bits of the accumulator defining the character scan
    line. The firmware uses the tail end of RAM as its scratchpad area.

    The accumulator, X and Y registers are mostly implemented as 74193
    up/down counters. There is no proper ALU, only a 7485 magnitude
    comparator and an 8242 equality checker whose output is also used to
    establish the position of the underline cursor.

    RAM is 7 bits wide, even though the VT50's character generator can
    only accept 6 bits. Most of the registers are also effectively 7 bits
    wide, though unused eighth bits are physically present. PC is also
    physically 12 bits wide, though only up to 10 bits are usable. Y is
    only 4 bits wide on the VT50, which has a 12-line display; in order
    to double the quantity of addressable RAM to allow for 24 lines, the
    VT52 adds an extra flip-flop stage to Y and rejumpers the address
    encoding circuit.

    The mode flip-flop changes the meanings of the jump conditions and the
    function of the constant load instruction, whose execution in mode 0
    is conditioned on equality with the preincremented accumulator. Jumps,
    if taken, load the highest two bits of the destination (which define
    the ROM page) from a ripple counter whose value may be incremented by
    the IROM instruction.

    The done flip-flop is set any time data is committed to memory. Its
    purpose is to ensure that only one in a sequence of consecutive
    load instructions in the firmware's keyboard lookup routine is
    actually executed.

    While horizontal blanking is defined in hardware as 20 characters out
    of every 100, vertical blanking periods are arbitrarily determined by
    when the firmware decides to deactivate the video flip-flop, which
    necessitates an awkward workaround since MAME's screen emulation
    expects a definite value. The vertical and horizontal synchronization
    pulses are also generated without regard to each other, which causes
    the screen refresh period to be 256 lines in 60 Hz mode and 307.2
    lines in 50 Hz mode. The unorthodox split structure of the timing
    chain permits it to double as a baud rate generator.

***************************************************************************/

#include "emu.h"
#include "vt50.h"
#include "vt50dasm.h"
#include "screen.h"

#define FIND_FIRST_LINE 0

// device type definitions
DEFINE_DEVICE_TYPE(VT50_CPU, vt50_cpu_device, "vt50_cpu", "DEC VT50 CPU")
DEFINE_DEVICE_TYPE(VT52_CPU, vt52_cpu_device, "vt52_cpu", "DEC VT52 CPU")

vt5x_cpu_device::vt5x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bbits, int ybits)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_rom_config("program", ENDIANNESS_LITTLE, 8, 10, 0)
	, m_ram_config("data", ENDIANNESS_LITTLE, 8, 6 + ybits, 0) // actually 7 bits wide
	, m_baud_9600_callback(*this)
	, m_vert_count_callback(*this)
	, m_uart_rd_callback(*this)
	, m_uart_xd_callback(*this)
	, m_ur_flag_callback(*this)
	, m_ut_flag_callback(*this)
	, m_ruf_callback(*this)
	, m_key_up_callback(*this)
	, m_kclk_callback(*this)
	, m_frq_callback(*this)
	, m_bell_callback(*this)
	, m_cen_callback(*this)
	, m_csf_callback(*this)
	, m_ccf_callback(*this)
	, m_char_data_callback(*this)
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
	, m_cursor_active(false)
	, m_video_process(false)
	, m_ram_do(0)
	, m_t(0)
	, m_write_ff(false)
	, m_flag_test_ff(false)
	, m_m2u_ff(false)
	, m_bell_ff(false)
	, m_load_pc(false)
	, m_icount(0)
	, m_horiz_count(0)
	, m_vert_count(0)
	, m_top_of_screen(false)
	, m_current_line(0)
	, m_first_line(~0)
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
	, m_graphic_callback(*this)
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

void vt5x_cpu_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(vt5x_cpu_device::screen_update));

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock(), 900, 128, 848, 256, 4, 244); // 60 Hz default parameters
}

void vt5x_cpu_device::device_resolve_objects()
{
	// resolve callbacks
	m_baud_9600_callback.resolve_safe();
	m_vert_count_callback.resolve_safe();
	m_uart_rd_callback.resolve_safe(0);
	m_uart_xd_callback.resolve_safe();
	m_ur_flag_callback.resolve_safe(0);
	m_ut_flag_callback.resolve_safe(0);
	m_ruf_callback.resolve_safe();
	m_key_up_callback.resolve_safe(1);
	m_kclk_callback.resolve_safe(1);
	m_frq_callback.resolve_safe(1);
	m_bell_callback.resolve_safe();
	m_cen_callback.resolve_safe();
	m_csf_callback.resolve_safe(1);
	m_ccf_callback.resolve_safe(1);
	m_char_data_callback.resolve_safe(0177);
}

void vt52_cpu_device::device_resolve_objects()
{
	vt5x_cpu_device::device_resolve_objects();

	m_graphic_callback.resolve_safe();
}

void vt5x_cpu_device::device_start()
{
	// acquire address spaces
	space(AS_PROGRAM).cache(m_rom_cache);
	space(AS_DATA).cache(m_ram_cache);

	screen().register_screen_bitmap(m_bitmap);
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
	state_add<u16>(VT5X_XYAD, "XYAD", [this]() { return translate_xy(); }).formatstr("%04O").mask((1 << (6 + m_ybits)) - 1);
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
	save_item(NAME(m_cursor_active));
	save_item(NAME(m_video_process));
	save_item(NAME(m_ram_do));
	save_item(NAME(m_t));
	save_item(NAME(m_write_ff));
	save_item(NAME(m_flag_test_ff));
	save_item(NAME(m_m2u_ff));
	save_item(NAME(m_bell_ff));
	save_item(NAME(m_load_pc));
	save_item(NAME(m_horiz_count));
	save_item(NAME(m_vert_count));
	save_item(NAME(m_top_of_screen));
	save_item(NAME(m_current_line));
#if FIND_FIRST_LINE
	save_item(NAME(m_first_line));
#else
	(void)m_first_line;
#endif
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

	m_horiz_count = 0;
	m_vert_count = 0;
	m_top_of_screen = true;
	m_current_line = 0;

	m_baud_9600_callback(0);
	m_vert_count_callback(0);
}

u32 vt5x_cpu_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void vt5x_cpu_device::draw_char_line()
{
	if (m_current_line < screen().visible_area().top() || m_current_line > screen().visible_area().bottom())
		return;

	u8 hc = (u8(m_horiz_count) >> 4) * 10 + (m_horiz_count & 15);
	unsigned xc = ((hc >= 22 ? hc : hc + 100) - 22) * 9 + screen().visible_area().left();
	if (xc > screen().visible_area().right() - 8)
		return;

	u32 *pix = &m_bitmap.pix(m_current_line, xc);
	if (m_video_process && m_cursor_ff && m_cursor_active)
		std::fill_n(pix, 9, rgb_t::white());
	else if (!m_video_process || m_cursor_ff)
		std::fill_n(pix, 9, rgb_t::black());
	else
	{
		// CD6 is first shifted out; CD0 is last out
		u8 vsr = m_char_data_callback(u16(m_ram_do) << 3 | (m_ac & 7)) | 0200;
		for (int i = 0; i < 9; i++)
		{
			*pix++ = BIT(vsr, 7) ? rgb_t::black() : rgb_t::white();
			vsr = (vsr << 1) | 1;
		}
	}
}

offs_t vt5x_cpu_device::translate_xy() const
{
	//                              A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
	// Screen RAM, columns 0–63:    y3 y2 y1 y0 x5 x4 x3 x2 x1 x0
	// Screen RAM, columns 64–79:    1  1 y1 y0 y3 y2 x3 x2 x1 x0
	// Scratchpad (not displayed):   1  1 y1 y0  1  1 x3 x2 x1 x0
	const u8 x = m_x ^ (m_x8 ? 8 : 0);
	const offs_t y_shifted = (offs_t(m_y) << (10 - m_ybits)) & 01700;
	const offs_t page_sel = offs_t(m_y & ((1 << (m_ybits - 4)) - 1)) << 10;
	if (BIT(x, 6) || (y_shifted & 01400) == 01400)
		return (x & 0017) | (y_shifted & 01400) >> 4 | y_shifted | 01400 | page_sel;
	else
		return x | y_shifted | page_sel;
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
			// EPR
			m_cen_callback(1);
			break;

		case 0160:
			// HPR!ZY
			m_cen_callback(0);
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

void vt52_cpu_device::execute_tw(u8 inst)
{
	vt5x_cpu_device::execute_tw(inst);

	// ZCAV also borrows from the upper half of AC on the VT52
	if (inst == 0100)
		m_ac = (m_ac - 020) & 0177;
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
		// LD (TODO: B/C masking in mode 0 is determined by optional jumpers)
		m_ram_do = inst & (!m_mode_ff && m_cursor_ff ? 0037 : 0177);
		break;
	}

	m_ram_cache.write_byte(translate_xy(), m_ram_do);
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
		// LD (TODO: B/C masking in mode 0 is determined by optional jumpers)
		m_ram_do = inst & (!m_mode_ff && m_cursor_ff ? 0037 : 0177);
		break;
	}

	m_ram_cache.write_byte(translate_xy(), m_ram_do);
	m_write_ff = false;
}

void vt5x_cpu_device::execute_th(u8 inst)
{
	switch (inst & 0362)
	{
	case 0002:
		// M2A
		m_ac = m_ram_do;
		break;

	case 0042:
		// M2U
		m_m2u_ff = true;
		break;

	case 0102:
		// M2X
		m_x = m_ram_do;
		break;

	case 0142:
		// M2B
		m_buffer = m_ram_do & ((1 << m_bbits) - 1);
		break;
	}

	if (m_flag_test_ff)
	{
		switch (inst & 0160)
		{
		case 0000:
			// M0: PSCJ
			// M1: URJ
			if (m_mode_ff)
				m_load_pc = m_ur_flag_callback();
			else
				m_load_pc = m_csf_callback();
			break;

		case 0020:
			// M0: TABJ (jump on 74H10 NAND of AC0–2; documentation incorrectly suggests the opposite)
			// M1: AEMJ
			if (m_mode_ff)
				m_load_pc = m_ac == m_ram_do;
			else
				m_load_pc = (m_ac & 7) != 7;
			break;

		case 0040:
			// M0: KCLJ
			// M1: ALMJ
			if (m_mode_ff)
				m_load_pc = m_ac < m_ram_do;
			else
				m_load_pc = m_kclk_callback();
			break;

		case 0060:
			// M0: FRQJ
			// M1: ADXJ
			if (m_mode_ff)
				m_load_pc = m_ac != (m_x ^ (m_x8 ? 8 : 0));
			else
				m_load_pc = m_frq_callback();
			break;

		case 0100:
			// M0: PRQJ
			// M1: AEM2J
			if (m_mode_ff)
				m_load_pc = m_ac == m_ram_do;
			else
				m_load_pc = m_ccf_callback();
			break;

		case 0120:
			// M0: COPJ (TODO?)
			// M1: TRUJ
			m_load_pc = true;
			break;

		case 0140:
			// M0: UTJ
			// M1: VSCJ
			if (m_mode_ff)
				m_load_pc = m_horiz_count >= 8;
			else
				m_load_pc = !m_ut_flag_callback();
			break;

		case 0160:
			// M0: TOSJ
			// M1: KEYJ
			if (m_mode_ff)
				m_load_pc = m_key_up_callback(m_ac) & 1;
			else
				m_load_pc = !m_top_of_screen;
			break;
		}
	}

	if ((m_pc & 0377) == 0377)
		m_rom_page = (m_rom_page + 1) & 3;
	m_pc = (m_pc + 1) & 03777;
}

void vt52_cpu_device::execute_th(u8 inst)
{
	// not actually synchronized to TH (but may be gated externally with EN CYCLE)
	if ((inst & 0362) == 0162)
		m_graphic_callback(m_ram_do);

	vt5x_cpu_device::execute_th(inst);
}

void vt5x_cpu_device::execute_tj(u8 dest)
{
	if (m_load_pc)
		m_pc = u16(m_rom_page) << 8 | dest;
	else
	{
		// Hardware bug: the ROM page counter will not increment on the second byte of a jump not taken.
		m_pc = (m_pc + 1) & 03777;
	}
}

void vt5x_cpu_device::clock_video_counters()
{
	if ((m_horiz_count & 9) == 9)
	{
		m_horiz_count = (m_horiz_count & (15 * 16)) + 16;
		if (m_vert_count == 07777)
		{
			m_vert_count = m_frq_callback() ? 03000 : 02000;
			m_horiz_count = 0;
			m_top_of_screen = true;
			m_vert_count_callback(0);
		}
		else
		{
			m_vert_count++;
			if (m_horiz_count == 10 * 16)
				m_horiz_count = 0;
			m_vert_count_callback(m_vert_count & 0177);
		}
	}
	else
	{
		m_horiz_count++;
		if (m_horiz_count == 8)
		{
			if (m_top_of_screen)
			{
				m_top_of_screen = false;

				// This calculates the number of visible lines, which is actually firmware-defined.
				bool is_60hz = BIT(m_vert_count, 9);
				unsigned first_line = is_60hz ? 4 : 32;
				screen().configure(
					900,
					(010000 - m_vert_count) / 10,
					rectangle(128, 847, first_line, 24 * (is_60hz ? 10 : 11) + first_line - 1),
					clocks_to_attotime((010000 - m_vert_count) * 90).as_attoseconds()
				);
				screen().reset_origin();
				m_current_line = 0;
#if FIND_FIRST_LINE
				m_first_line = ~0;
#endif
			}
			else
				m_current_line++;
			m_baud_9600_callback(0);
		}
		else if (m_horiz_count == 4)
			m_baud_9600_callback(1);
	}
}

void vt5x_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		bool en_cycle = BIT(m_horiz_count, 0);
		switch (m_t)
		{
		case 0:
			if (!en_cycle)
				debugger_instruction_hook(m_pc);
			m_t = 1;
			break;

		case 1:
			if (!en_cycle)
				execute_te(m_rom_cache.read_byte(m_pc));
			m_t = 2;
			break;

		case 2:
			if (!en_cycle)
				execute_tf(m_rom_cache.read_byte(m_pc));
			if (!m_write_ff)
				m_ram_do = m_ram_cache.read_byte(translate_xy()) & 0177;
			m_cursor_active = m_ac == (m_x ^ (m_x8 ? 8 : 0));
			if (u8(m_horiz_count - 2) >= 2 * 16)
			{
				if (m_video_process)
				{
#if FIND_FIRST_LINE
					if (m_first_line > m_current_line)
						m_first_line = m_current_line;
#endif
					m_x = (m_x + 1) & 0177;
				}
				draw_char_line();
			}
			m_t = 3;
			break;

		case 3:
			if (en_cycle && m_write_ff)
				execute_tg(m_rom_cache.read_byte(m_pc));
			m_t = 4;
			break;

		case 4:
			if (en_cycle)
				execute_th(m_rom_cache.read_byte(m_pc));
			m_t = 5;
			break;

		case 5:
			m_t = 6;
			break;

		case 6:
			if (en_cycle && m_flag_test_ff)
				execute_tj(m_rom_cache.read_byte(m_pc));
			m_t = 7;
			break;

		case 7:
			if (!en_cycle)
				execute_tw(m_rom_cache.read_byte(m_pc));
			m_t = 8;
			break;

		case 8:
			m_t = 0;
			clock_video_counters();
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
