// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Excalibur 64 kit computer, designed and sold in Australia by BGR Computers.
The official schematics have a LOT of errors and omissions.

Skeleton driver created on 2014-12-09.

Chips: Z80A, 8251, 8253, 8255, 6845
We have Basic 1.1. Other known versions are 1.01, 2.1
There are 2 versions of the colour prom, which have different palettes.
We have the later version.

Notes:
- Control W then Enter will switch between 40 and 80 characters per line.
- Control V turns cursor on
- Graphics commands such as LINE, CIRCLE, HGRCLS, HGRSET etc only work with disk basic

ToDo:
- Colours are approximate.
- Hardware supports 20cm and 13cm floppies, but we only support 13cm as this
  is the only software that exists.
- The schematic shows the audio counter connected to 2MHz, but this produces
  sounds that are too high. Connected to 1MHz for now.
- Serial
- Pasting can sometimes drop a character.

****************************************************************************/


#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/74123.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/rescap.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/excali64_dsk.h"


class excali64_state : public driver_device
{
public:
	excali64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_cass(*this, "cassette")
		, m_crtc(*this, "crtc")
		, m_io_keyboard(*this, "KEY.%u", 0)
		, m_dma(*this, "dma")
		, m_u12(*this, "u12")
		, m_centronics(*this, "centronics")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	void excali64(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	void excali64_palette(palette_device &palette);
	DECLARE_WRITE8_MEMBER(ppib_w);
	DECLARE_READ8_MEMBER(ppic_r);
	DECLARE_WRITE8_MEMBER(ppic_w);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port50_r);
	DECLARE_WRITE8_MEMBER(port70_w);
	DECLARE_WRITE8_MEMBER(porte4_w);
	DECLARE_READ8_MEMBER(porte8_r);
	DECLARE_WRITE8_MEMBER(portec_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_WRITE_LINE_MEMBER(cent_busy_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	MC6845_UPDATE_ROW(update_row);
	DECLARE_WRITE_LINE_MEMBER(crtc_hs);
	DECLARE_WRITE_LINE_MEMBER(crtc_vs);
	DECLARE_WRITE_LINE_MEMBER(motor_w);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	uint8_t *m_p_videoram;
	uint8_t *m_p_hiresram;
	uint8_t m_sys_status;
	uint8_t m_kbdrow;
	bool m_crtc_vs;
	bool m_crtc_hs;
	bool m_motor;
	bool m_centronics_busy;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_device<cassette_image_device> m_cass;
	required_device<mc6845_device> m_crtc;
	required_ioport_array<8> m_io_keyboard;
	required_device<z80dma_device> m_dma;
	required_device<ttl74123_device> m_u12;
	required_device<centronics_device> m_centronics;
	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

void excali64_state::mem_map(address_map &map)
{
	map(0x0000, 0x1FFF).bankr("bankr1").bankw("bankw1");
	map(0x2000, 0x2FFF).bankr("bankr2").bankw("bankw2");
	map(0x3000, 0x3FFF).bankr("bankr3").bankw("bankw3");
	map(0x4000, 0xBFFF).bankr("bankr4").bankw("bankw4");
	map(0xC000, 0xFFFF).ram().region("rambank", 0xC000);
}

void excali64_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).r(FUNC(excali64_state::port00_r));
	map(0x10, 0x11).mirror(0x0e).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x23).mirror(0x0c).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x30, 0x30).mirror(0x0e).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x31, 0x31).mirror(0x0e).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x50, 0x5f).r(FUNC(excali64_state::port50_r));
	map(0x60, 0x63).mirror(0x0c).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x70, 0x7f).w(FUNC(excali64_state::port70_w));
	map(0xe0, 0xe3).rw(m_dma, FUNC(z80dma_device::bus_r), FUNC(z80dma_device::bus_w));
	map(0xe4, 0xe7).w(FUNC(excali64_state::porte4_w));
	map(0xe8, 0xeb).r(FUNC(excali64_state::porte8_r));
	map(0xec, 0xef).w(FUNC(excali64_state::portec_w));
	map(0xf0, 0xf3).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
}


static INPUT_PORTS_START( excali64 )
	PORT_START("KEY.0")    /* line 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPSLOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)

	PORT_START("KEY.1")    /* line 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // space
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // F1

	PORT_START("KEY.2")    /* line 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) //B
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) //N

	PORT_START("KEY.3")    /* line 3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x27) PORT_CHAR(0x22) PORT_CHAR(0x27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)

	PORT_START("KEY.4")    /* line 4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x7f) PORT_CHAR(0x7f) PORT_CHAR(0x1f)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)

	PORT_START("KEY.5")    /* line 5 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHAR(0x1b)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)

	PORT_START("KEY.6")    /* line 6 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) //Z

	PORT_START("KEY.7")    /* line 7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
INPUT_PORTS_END

WRITE_LINE_MEMBER( excali64_state::cent_busy_w )
{
	m_centronics_busy = state;
}

FLOPPY_FORMATS_MEMBER( excali64_state::floppy_formats )
	FLOPPY_EXCALI64_FORMAT
FLOPPY_FORMATS_END

static void excali64_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

// pulses from port E4 bit 5 restart the 74123. After 3.6 secs without a pulse, the motor gets turned off.
WRITE_LINE_MEMBER( excali64_state::motor_w )
{
	m_motor = state;
	m_floppy1->get_device()->mon_w(!m_motor);
	m_floppy0->get_device()->mon_w(!m_motor);
}

READ8_MEMBER( excali64_state::porte8_r )
{
	return 0xfc | (uint8_t)m_motor;
}

WRITE8_MEMBER( excali64_state::porte4_w )
{
	floppy_image_device *floppy = nullptr;
	if (BIT(data, 0))
		floppy = m_floppy0->get_device();

	if (BIT(data, 1))
		floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	m_u12->b_w(BIT(data, 5)); // motor pulse
}

/*
d0 = precomp (selectable by jumper)
d1 = size select
d2 = density select (0 = double)
*/
WRITE8_MEMBER( excali64_state::portec_w )
{
	m_fdc->enmf_w(BIT(data, 1));
	m_fdc->dden_w(BIT(data, 2));
}

WRITE_LINE_MEMBER( excali64_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state); // tell dma that bus has been granted
}

READ8_MEMBER( excali64_state::memory_read_byte )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER( excali64_state::memory_write_byte )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

READ8_MEMBER( excali64_state::io_read_byte )
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER( excali64_state::io_write_byte )
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

WRITE8_MEMBER( excali64_state::ppib_w )
{
	m_kbdrow = data;
}

READ8_MEMBER( excali64_state::ppic_r )
{
	uint8_t data = 0xf4; // READY line must be low to print
	data |= (uint8_t)m_centronics_busy;
	data |= (m_cass->input() > 0.1) << 3;
	return data;
}

WRITE8_MEMBER( excali64_state::ppic_w )
{
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
	m_centronics->write_strobe(BIT(data, 4));
}

READ8_MEMBER( excali64_state::port00_r )
{
	uint8_t data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_kbdrow, i))
			data &= m_io_keyboard[i]->read();
	}

	return data;
}

/*
d0 : /rom ; screen
d1 : ram on
d2 : /low ; high res
d3 : 2nd colour set (previously, dispen, which is a mistake in hardware and schematic)
d4 : vsync
d5 : rombank
*/
READ8_MEMBER( excali64_state::port50_r )
{
	uint8_t data = m_sys_status & 0x2f;
	bool csync = m_crtc_hs | m_crtc_vs;
	data |= (uint8_t)csync << 4;
	return data;
}

/*
d0,1,2,3,5 : same as port50
(schematic wrongly says d7 used for 2nd colour set)
*/
WRITE8_MEMBER( excali64_state::port70_w )
{
	m_sys_status = data;
	m_crtc->set_unscaled_clock(BIT(data, 2) ? 2e6 : 1e6);
	if (BIT(data, 1))
	{
		// select 64k ram
		membank("bankr1")->set_entry(0);
		membank("bankr2")->set_entry(0);
		membank("bankr3")->set_entry(0);
		membank("bankr4")->set_entry(0);
		membank("bankw2")->set_entry(0);
		membank("bankw3")->set_entry(0);
		membank("bankw4")->set_entry(0);
	}
	else if (BIT(data, 0))
	{
		// select videoram and hiresram
		membank("bankr1")->set_entry(1);
		membank("bankr2")->set_entry(2);
		membank("bankr3")->set_entry(2);
		membank("bankw2")->set_entry(2);
		membank("bankw3")->set_entry(2);
		membank("bankr4")->set_entry(2);
		membank("bankw4")->set_entry(2);
	}
	else
	{
		// select rom, videoram, and main ram
		membank("bankr1")->set_entry(1);
		membank("bankr2")->set_entry(1);
		membank("bankr3")->set_entry(1);
		membank("bankw2")->set_entry(2);
		membank("bankw3")->set_entry(2);
		membank("bankr4")->set_entry(0);
		membank("bankw4")->set_entry(0);
	}

	// other half of ROM_1
	if ((data & 0x22) == 0x20)
		membank("bankr1")->set_entry(2);
}

void excali64_state::machine_reset()
{
	membank("bankr1")->set_entry(1); // read from ROM
	membank("bankr2")->set_entry(1); // read from ROM
	membank("bankr3")->set_entry(1); // read from ROM
	membank("bankr4")->set_entry(0); // read from RAM
	membank("bankw1")->set_entry(0); // write to RAM
	membank("bankw2")->set_entry(2); // write to videoram
	membank("bankw3")->set_entry(2); // write to videoram hires pointers
	membank("bankw4")->set_entry(0); // write to RAM
	m_maincpu->reset();
}

WRITE_LINE_MEMBER( excali64_state::crtc_hs )
{
	m_crtc_hs = state;
}

WRITE_LINE_MEMBER( excali64_state::crtc_vs )
{
	m_crtc_vs = state;
}

/* F4 Character Displayer */
static const gfx_layout excali64_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_excali64 )
	GFXDECODE_ENTRY( "chargen", 0x0000, excali64_charlayout, 0, 1 )
GFXDECODE_END

// The prom, the schematic, and the manual all contradict each other,
// so the colours can only be described as wild guesses. Further, the 38
// colour-load resistors are missing labels and values.
void excali64_state::excali64_palette(palette_device &palette)
{
	// do this here because driver_init hasn't run yet
	m_p_videoram = memregion("videoram")->base();
	m_p_hiresram = m_p_videoram + 0x2000;
	uint8_t *main = memregion("roms")->base();
	uint8_t *ram = memregion("rambank")->base();

	// main ram (cp/m mode)
	membank("bankr1")->configure_entry(0, &ram[0x0000]);
	membank("bankr2")->configure_entry(0, &ram[0x2000]);
	membank("bankr3")->configure_entry(0, &ram[0x3000]);
	membank("bankr4")->configure_entry(0, &ram[0x4000]);//boot
	membank("bankw1")->configure_entry(0, &ram[0x0000]);//boot
	membank("bankw2")->configure_entry(0, &ram[0x2000]);
	membank("bankw3")->configure_entry(0, &ram[0x3000]);
	membank("bankw4")->configure_entry(0, &ram[0x4000]);//boot
	// rom_1
	membank("bankr1")->configure_entry(1, &main[0x0000]);//boot
	membank("bankr1")->configure_entry(2, &main[0x2000]);
	// rom_2
	membank("bankr2")->configure_entry(1, &main[0x4000]);//boot
	membank("bankr3")->configure_entry(1, &main[0x5000]);//boot
	// videoram
	membank("bankr2")->configure_entry(2, &m_p_videoram[0x0000]);
	membank("bankw2")->configure_entry(2, &m_p_videoram[0x0000]);//boot
	// hiresram
	membank("bankr3")->configure_entry(2, &m_p_videoram[0x1000]);
	membank("bankw3")->configure_entry(2, &m_p_videoram[0x1000]);//boot
	membank("bankr4")->configure_entry(2, &m_p_hiresram[0x0000]);
	membank("bankw4")->configure_entry(2, &m_p_hiresram[0x0000]);

	// Set up foreground colours
	for (uint8_t i = 0; i < 32; i++)
	{
		uint8_t const code = m_p_chargen[0x1000+i];
		uint8_t const r = (BIT(code, 0) ? 38 : 0) + (BIT(code, 1) ? 73 : 0) + (BIT(code, 2) ? 144 : 0);
		uint8_t const b = (BIT(code, 3) ? 38 : 0) + (BIT(code, 4) ? 73 : 0) + (BIT(code, 5) ? 144 : 0);
		uint8_t const g = (BIT(code, 6) ? 85 : 0) + (BIT(code, 7) ? 170 : 0);
		palette.set_pen_color(i, r, g, b);
	}

	// Background
	palette.set_pen_color(32, 0x00, 0x00, 0x00);  //  0 Black
	palette.set_pen_color(33, 0xff, 0x00, 0x00);  //  1 Red
	palette.set_pen_color(34, 0x00, 0x00, 0xff);  //  2 Blue
	palette.set_pen_color(35, 0xff, 0x00, 0xff);  //  3 Magenta
	palette.set_pen_color(36, 0x00, 0xff, 0x00);  //  4 Green
	palette.set_pen_color(37, 0xff, 0xff, 0x00);  //  5 Yellow
	palette.set_pen_color(38, 0x00, 0xff, 0xff);  //  6 Cyan
	palette.set_pen_color(39, 0xff, 0xff, 0xff);  //  7 White
}

MC6845_UPDATE_ROW( excali64_state::update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx,col,bg,fg;
	uint16_t mem,x;
	uint8_t col_base = BIT(m_sys_status, 3) ? 16 : 0;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		col = m_p_videoram[mem+0x800];
		fg = col_base + (col >> 4);
		bg = 32 + ((col >> 1) & 7);

		if (BIT(col, 0))
		{
			uint8_t h = m_p_videoram[mem+0x1000] - 4;
			if (h > 5)
				h = 0; // keep us in bounds
			// hires definition - pixels are opposite order to characters
			gfx = bitswap<8>(m_p_hiresram[(h << 12) | (chr<<4) | ra], 0, 1, 2, 3, 4, 5, 6, 7);
		}
		else
			gfx = m_p_chargen[(chr<<4) | ra]; // normal character

		gfx ^= (x == cursor_x) ? 0xff : 0;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 0) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
	}
}

void excali64_state::excali64(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &excali64_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &excali64_state::io_map);

	I8251(config, "uart", 0);
	//uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	//uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(16_MHz_XTAL / 16); /* Timer 0: tone gen for speaker */
	pit.out_handler<0>().set("speaker", FUNC(speaker_sound_device::level_w));
	//pit.set_clk<1>(16_MHz_XTAL / 16); /* Timer 1: baud rate gen for 8251 */
	//pit.out_handler<1>().set(FUNC(excali64_state::write_uart_clock));
	//pit.set_clk<2>(16_MHz_XTAL / 16); /* Timer 2: not used */

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set("cent_data_out", FUNC(output_latch_device::bus_w)); // parallel port
	ppi.out_pb_callback().set(FUNC(excali64_state::ppib_w));
	ppi.in_pc_callback().set(FUNC(excali64_state::ppic_r));
	ppi.out_pc_callback().set(FUNC(excali64_state::ppic_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(80*8, 24*12);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(excali64_state::excali64_palette), 40);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_excali64);

	MC6845(config, m_crtc, 16_MHz_XTAL / 16); // 1MHz for lowres; 2MHz for highres
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(excali64_state::update_row), this);
	m_crtc->out_hsync_callback().set(FUNC(excali64_state::crtc_hs));
	m_crtc->out_vsync_callback().set(FUNC(excali64_state::crtc_vs));

	/* Devices */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	WD2793(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", excali64_floppies, "525qd", excali64_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", excali64_floppies, "525qd", excali64_state::floppy_formats).enable_sound(true);

	Z80DMA(config, m_dma, 16_MHz_XTAL / 4);
	m_dma->out_busreq_callback().set(FUNC(excali64_state::busreq_w));
	m_dma->in_mreq_callback().set(FUNC(excali64_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(excali64_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(excali64_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(excali64_state::io_write_byte));

	TTL74123(config, m_u12, 0);
	m_u12->set_connection_type(TTL74123_GROUNDED);  /* Hook up type (no idea what this means) */
	m_u12->set_resistor_value(RES_K(100));          /* resistor connected between RCext & 5v */
	m_u12->set_capacitor_value(CAP_U(100));         /* capacitor connected between Cext and RCext */
	m_u12->set_a_pin_value(0);                      /* A pin - grounded */
	m_u12->set_b_pin_value(1);                      /* B pin - driven by port e4 bit 5 */
	m_u12->set_clear_pin_value(1);                  /* Clear pin - pulled high */
	m_u12->out_cb().set(FUNC(excali64_state::motor_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(excali64_state::cent_busy_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}

/* ROM definition */
ROM_START( excali64 )
	ROM_REGION(0x6000, "roms", 0)
	ROM_LOAD( "rom_1.ic17", 0x0000, 0x4000, CRC(e129a305) SHA1(e43ec7d040c2b2e548d22fd6bbc7df8b45a26e5a) )
	ROM_LOAD( "rom_2.ic24", 0x4000, 0x2000, CRC(916d9f5a) SHA1(91c527cce963481b7bebf077e955ca89578bb553) )
	// fix a bug that causes screen to be filled with 'p'
	ROM_FILL(0x4ee, 1, 0)
	ROM_FILL(0x4ef, 1, 8)
	ROM_FILL(0x4f6, 1, 0)
	ROM_FILL(0x4f7, 1, 8)
	// patch out the protection
	ROM_FILL(0x3ce7, 1, 0)

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASE00)
	ROM_REGION(0xA000, "videoram", ROMREGION_ERASE00)

	ROM_REGION(0x1020, "chargen", 0)
	ROM_LOAD( "genex_3.ic43", 0x0000, 0x1000, CRC(b91619a9) SHA1(2ced636cb7b94ba9d329868d7ecf79963cefe9d9) )
	ROM_LOAD( "hm7603.ic55",  0x1000, 0x0020, CRC(c74f47dc) SHA1(331ff3c913846191ddd97cacb80bd19438c1ff71) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY          FULLNAME        FLAGS
COMP( 1984, excali64, 0,      0,      excali64, excali64, excali64_state, empty_init, "BGR Computers", "Excalibur 64", 0 )
