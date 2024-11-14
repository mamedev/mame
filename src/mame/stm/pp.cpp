// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Preliminary driver for STM (Semi-Tech Microelectronics) Pied Piper.

****************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8279.h"
#include "machine/input_merger.h"
#include "machine/output_latch.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/scn2674.h"
#include "screen.h"
#include "speaker.h"


namespace {

class pp_state : public driver_device
{
public:
	pp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_int4(*this, "int4")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_pvtc(*this, "pvtc")
		, m_speaker(*this, "speaker")
		, m_printer(*this, "printer")
		, m_bios(*this, "bios")
		, m_chargen(*this, "chargen")
		, m_kbd_rows(*this, "ROW%u", 0U)
		, m_modifiers(*this, "MODIFIERS")
		, m_kbd_leds(*this, "led%u", 1U)
		, m_rom_enabled(false)
		, m_int_pending(0)
		, m_int_status(0xf)
		, m_kbd_scan(0)
		, m_kbd_release(false)
		, m_tmi_enable(false)
		, m_printer_busy(false)
		, m_dr(0)
		, m_mode(0)
	{
	}

	void pp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(display_char);

	template<int Line> void int_w(int state);
	TIMER_CALLBACK_MEMBER(int_update);
	IRQ_CALLBACK_MEMBER(intak_cb);

	void kbd_scan_w(u8 data);
	u8 kbd_cols_r();
	int cntl_r();
	int shift_r();
	void printer_busy_w(int state);

	u8 memory_r(offs_t offset);
	void memory_w(offs_t offset, u8 data);
	u8 stat_r();
	void co_w(u8 data);
	void hld_w(int state);
	void mode_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void display_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<input_merger_device> m_int4;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<scn2672_device> m_pvtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_printer;
	required_region_ptr<u8> m_bios;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<8> m_kbd_rows;
	required_ioport m_modifiers;
	output_finder<3> m_kbd_leds;

	std::unique_ptr<u8[]> m_ram;
	bool m_rom_enabled;
	u8 m_int_pending;
	u8 m_int_status;
	u8 m_kbd_scan;
	bool m_kbd_release;
	bool m_tmi_enable;
	bool m_printer_busy;
	u8 m_dr;
	u8 m_mode;
};

void pp_state::machine_start()
{
	m_kbd_leds.resolve();
	m_kbd_leds[0] = 1; // power LED (green) is always on

	m_fdc->dden_w(0);

	// 64K of dynamic RAM (8x 4864)
	m_ram = make_unique_clear<u8[]>(0x10000);
	save_pointer(NAME(m_ram), 0x10000);

	save_item(NAME(m_rom_enabled));
	save_item(NAME(m_int_pending));
	save_item(NAME(m_int_status));
	save_item(NAME(m_kbd_scan));
	save_item(NAME(m_kbd_release));
	save_item(NAME(m_tmi_enable));
	save_item(NAME(m_printer_busy));
	save_item(NAME(m_dr));
	save_item(NAME(m_mode));
}

void pp_state::machine_reset()
{
	mode_w(0);
	m_rom_enabled = true;
}


template <int Line>
void pp_state::int_w(int state)
{
	if (BIT(m_int_pending, Line) == state)
		return;

	if (state)
		m_int_pending |= 1 << Line;
	else
		m_int_pending &= ~(1 << Line);

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pp_state::int_update), this));
}

TIMER_CALLBACK_MEMBER(pp_state::int_update)
{
	if (m_int_pending == 0)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		m_int_status = 0xf;
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

		u8 priority = 7;
		while (priority != 0 && !BIT(m_int_pending, priority))
			priority--;
		m_int_status = (priority ^ 7) << 1;
	}
}

IRQ_CALLBACK_MEMBER(pp_state::intak_cb)
{
	return m_int_status;
}


SCN2672_DRAW_CHARACTER_MEMBER(pp_state::display_char)
{
	u8 dots = m_chargen[(charcode & 0x7f) << 4 | linecount];

	// reverse video logic
	if (cursor ^ (BIT(charcode, 7) && BIT(m_mode, 1)) ^ BIT(m_mode, 5))
		dots = ~dots;

	// highlight logic
	rgb_t fg = cursor || (BIT(charcode, 7) && BIT(m_mode, 2)) ? rgb_t::white() : rgb_t(0xc0, 0xc0, 0xc0);

	for (int i = 0; i < 7; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 7) ? fg : rgb_t::black();
		dots <<= 1;
	}
	if (!BIT(m_mode, 6))
		bitmap.pix(y, x++) = BIT(dots, 7) ? fg : rgb_t::black();
}


void pp_state::kbd_scan_w(u8 data)
{
	// SL3 = AUTORPT (also clocks timer interrupt)
	if (!BIT(m_kbd_scan, 3) && BIT(data, 3) && m_tmi_enable)
		int_w<3>(1);
	m_kbd_scan = data;
}

u8 pp_state::kbd_cols_r()
{
	if (m_kbd_release)
		return 0xff;
	else
		return m_kbd_rows[m_kbd_scan & 7]->read();
}

int pp_state::cntl_r()
{
	// pin 1 of J13 connector
	return (m_modifiers->read() & 0x5) == 0x5;
}

int pp_state::shift_r()
{
	// pin 3 of J13 connector
	return (m_modifiers->read() & 0x6) == 0x6;
}

void pp_state::printer_busy_w(int state)
{
	m_printer_busy = state;
}


u8 pp_state::memory_r(offs_t offset)
{
	if (m_rom_enabled && !BIT(offset, 15))
	{
		// ROM accesses have 1 wait state
		if (!machine().side_effects_disabled())
			m_maincpu->adjust_icount(-1);

		return m_bios[offset & 0xfff];
	}
	else
		return m_ram[offset];
}

void pp_state::memory_w(offs_t offset, u8 data)
{
	if (m_rom_enabled && !BIT(offset, 15))
	{
		// ROM accesses have 1 wait state
		if (!machine().side_effects_disabled())
			m_maincpu->adjust_icount(-1);

		logerror("%s: Writing %02Xh to ROM at %04Xh\n", machine().describe_context(), data, offset);
	}
	else
		m_ram[offset] = data;
}

u8 pp_state::stat_r()
{
	if (!machine().side_effects_disabled())
		m_rom_enabled = !BIT(m_mode, 4);

	bool index_pulse = (BIT(m_dr, 0) && m_floppy[0]->get_device() != nullptr && m_floppy[0]->get_device()->idx_r())
			|| (BIT(m_dr, 1) && m_floppy[1]->get_device() != nullptr && m_floppy[1]->get_device()->idx_r());

	return m_int_status
			| (BIT(m_modifiers->read(), 3) ? 0x10 : 0x00)
			| (m_printer_busy ? 0x20 : 0x00)
			| (index_pulse ? 0x00 : 0x40)
			| (BIT(m_kbd_scan, 3) ? 0x80 : 0x00);
}

void pp_state::co_w(u8 data)
{
	// D0 = DR1, D1 = DR2, D2 = SS
	m_dr = data & 0x07;
	floppy_image_device *floppy = nullptr;
	for (int i = 0; i < 2 && floppy == nullptr; i++)
		if (BIT(data, i))
			floppy = m_floppy[i]->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy != nullptr)
		floppy->ss_w(BIT(data, 2));

	m_kbd_leds[2] = BIT(data, 0); // pin 21 of J13 connector
	m_kbd_leds[1] = BIT(data, 1); // pin 22 of J13 connector

	m_printer->write_strobe(!BIT(data, 3)); // pin 7 of connector

	// KBIEN
	m_int4->in_w<0>(BIT(data, 4));

	// TMIEN
	if (m_tmi_enable && !BIT(data, 5))
		int_w<3>(0);
	m_tmi_enable = BIT(data, 5);

	// TODO: D6 = HDIEN

	// KBREL
	m_kbd_release = BIT(data, 7);
}

void pp_state::hld_w(int state)
{
	for (int i = 0; i < 2; i++)
		if (m_floppy[i]->get_device() != nullptr)
			m_floppy[i]->get_device()->mon_w(!state);
}

void pp_state::mode_w(u8 data)
{
	// D0 = 40COL, D1 = REV, D2 = HIL, D3 = CHAR (TODO), D4 = ROMSEL, D5 = BOW, D6 = DIV7-8/
	m_pvtc->set_unscaled_clock(13_MHz_XTAL / (BIT(data, 0) ? 2 : 1) / (BIT(data, 6) ? 7 : 8));
	m_pvtc->set_character_width(BIT(data, 6) ? 7 : 8);

	m_speaker->level_w(BIT(data, 7));

	m_mode = data;
}

void pp_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(pp_state::memory_r), FUNC(pp_state::memory_w));
}

void pp_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x08, 0x09).mirror(2).rw("pkdi", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x0d, 0x0d).mirror(2).r(m_pvtc, FUNC(scn2672_device::buffer_r)).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x10, 0x13).mirror(4).rw("fdc", FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x18, 0x18).mirror(2).rw(FUNC(pp_state::stat_r), FUNC(pp_state::co_w));
	map(0x1c, 0x1c).mirror(2).w(m_pvtc, FUNC(scn2672_device::buffer_w));
	map(0x1d, 0x1d).mirror(2).w(FUNC(pp_state::mode_w));
	map(0x30, 0x37).unmaprw(); // TODO: hard disk interface
}

void pp_state::display_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).ram(); // 4x MM2114L
}


static INPUT_PORTS_START(pp)
	PORT_START("ROW0") // pin 11 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0xa3) PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE) // 0x60 unshifted internally
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_DEL) // 0x7f internally

	PORT_START("ROW1") // pin 13 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)

	PORT_START("ROW2") // pin 14 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3") // pin 15 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("ROW4") // pin 16 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT)

	PORT_START("ROW5") // pin 17 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW6") // pin 18 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_LEFT) // 0x02/0x10/0x13 internally
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW7") // pin 19 of J13 connector
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space Bar") PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_RIGHT) // 0x06/0x0e/0x18 internally
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIERS")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Funct") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_TOGGLE PORT_CODE(KEYCODE_CAPSLOCK)
INPUT_PORTS_END

static void pp_floppies(device_slot_interface &device)
{
	// Mitsubishi M485X
	device.option_add("525qd", FLOPPY_525_QD);
}

void pp_state::pp(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pp_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pp_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(pp_state::intak_cb));

	INPUT_MERGER_ALL_HIGH(config, m_int4).output_handler().set(FUNC(pp_state::int_w<4>));
	INPUT_MERGER_ANY_HIGH(config, "int6").output_handler().set(FUNC(pp_state::int_w<6>));
	INPUT_MERGER_ANY_HIGH(config, "int7").output_handler().set(FUNC(pp_state::int_w<7>));

	i8279_device &pkdi(I8279(config, "pkdi", 8_MHz_XTAL / 8));
	pkdi.out_irq_callback().set(m_int4, FUNC(input_merger_device::in_w<1>));
	pkdi.out_sl_callback().set(FUNC(pp_state::kbd_scan_w));
	//pkdi.out_disp_callback().set(FUNC(pp_state::disp_w));
	pkdi.in_rl_callback().set(FUNC(pp_state::kbd_cols_r));
	pkdi.in_shift_callback().set(FUNC(pp_state::shift_r));
	pkdi.in_ctrl_callback().set(FUNC(pp_state::cntl_r));

	FD1793(config, m_fdc, 8_MHz_XTAL / 8); // with FDC9216 data separator
	m_fdc->set_force_ready(true);
	m_fdc->intrq_wr_callback().set("int6", FUNC(input_merger_device::in_w<0>));
	m_fdc->drq_wr_callback().set("int7", FUNC(input_merger_device::in_w<0>));
	m_fdc->hld_wr_callback().set(FUNC(pp_state::hld_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], pp_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], pp_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	SCN2672(config, m_pvtc, 13_MHz_XTAL / 8);
	m_pvtc->set_character_width(8); // 8 or 7 depending on mode
	m_pvtc->set_screen("screen");
	m_pvtc->set_addrmap(0, &pp_state::display_map);
	m_pvtc->intr_callback().set(FUNC(pp_state::int_w<5>));
	m_pvtc->set_display_callback(FUNC(pp_state::display_char));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(13_MHz_XTAL, 630, 0, 560, 240, 0, 216);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SPEAKER(config, "mono").front_center(); // audio output on pin 24 of J13 connector
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);

	CENTRONICS(config, m_printer, centronics_devices, nullptr);
	m_printer->busy_handler().set(FUNC(pp_state::printer_busy_w));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_printer->set_output_latch(latch);
}

ROM_START(pp)
	ROM_REGION(0x1000, "bios", 0)
	ROM_LOAD("rom.bin", 0x0000, 0x1000, CRC(0a4c548f) SHA1(ec0c4ae4c17d427046deadb6f7850ddb3ba002c7))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("20000111-00.bin", 0x0000, 0x0800, CRC(17480a9e) SHA1(a874a2f5ceb2884cc1af16132bea2c32f26805c4))
	ROM_CONTINUE(0x0000, 0x0800) // first half blank
	ROM_LOAD("chargen.bin", 0x0000, 0x0800, CRC(111e443a) SHA1(455a573addf274ae3fd41307316d87587d8f5550))
	ROM_CONTINUE(0x0000, 0x0800) // first half blank
ROM_END

} // anonymous namespace


COMP(1983, pp, 0, 0, pp, pp, pp_state, empty_init, "STM Electronics", "Pied Piper Communicator 1", MACHINE_NOT_WORKING)
