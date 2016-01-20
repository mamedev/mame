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
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"
//#include "machine/clock.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "sound/speaker.h"
#include "machine/z80dma.h"
#include "machine/rescap.h"
#include "machine/74123.h"
#include "machine/wd_fdc.h"
#include "formats/excali64_dsk.h"

class excali64_state : public driver_device
{
public:
	excali64_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_crtc(*this, "crtc")
		, m_io_keyboard(*this, "KEY")
		, m_dma(*this, "dma")
		, m_u12(*this, "u12")
		, m_centronics(*this, "centronics")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	DECLARE_PALETTE_INIT(excali64);
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
	DECLARE_WRITE8_MEMBER(motor_w);
	DECLARE_MACHINE_RESET(excali64);
	required_device<palette_device> m_palette;

private:
	const UINT8 *m_p_chargen;
	UINT8 *m_p_videoram;
	UINT8 *m_p_hiresram;
	UINT8 m_sys_status;
	UINT8 m_kbdrow;
	bool m_crtc_vs;
	bool m_crtc_hs;
	bool m_motor;
	bool m_centronics_busy;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<mc6845_device> m_crtc;
	required_ioport_array<8> m_io_keyboard;
	required_device<z80dma_device> m_dma;
	required_device<ttl74123_device> m_u12;
	required_device<centronics_device> m_centronics;
	required_device<wd2793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

static ADDRESS_MAP_START(excali64_mem, AS_PROGRAM, 8, excali64_state)
	AM_RANGE(0x0000, 0x1FFF) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
	AM_RANGE(0x2000, 0x2FFF) AM_READ_BANK("bankr2") AM_WRITE_BANK("bankw2")
	AM_RANGE(0x3000, 0x3FFF) AM_READ_BANK("bankr3") AM_WRITE_BANK("bankw3")
	AM_RANGE(0x4000, 0xBFFF) AM_READ_BANK("bankr4") AM_WRITE_BANK("bankw4")
	AM_RANGE(0xC000, 0xFFFF) AM_RAM AM_REGION("rambank", 0xC000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(excali64_io, AS_IO, 8, excali64_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_READ(port00_r)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_DEVREADWRITE("uart",i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0x0c) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0x0e) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x0e) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x50, 0x5f) AM_READ(port50_r)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0x0c) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0x70, 0x7f) AM_WRITE(port70_w)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE("dma", z80dma_device, read, write)
	AM_RANGE(0xe4, 0xe7) AM_WRITE(porte4_w)
	AM_RANGE(0xe8, 0xeb) AM_READ(porte8_r)
	AM_RANGE(0xec, 0xef) AM_WRITE(portec_w)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("fdc", wd2793_t, read, write)
ADDRESS_MAP_END


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

static SLOT_INTERFACE_START( excali64_floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_525_QD )
	SLOT_INTERFACE( "drive1", FLOPPY_525_QD )
SLOT_INTERFACE_END

// pulses from port E4 bit 5 restart the 74123. After 3.6 secs without a pulse, the motor gets turned off.
WRITE8_MEMBER( excali64_state::motor_w )
{
	m_motor = BIT(data, 0);
	m_floppy1->get_device()->mon_w(!m_motor);
	m_floppy0->get_device()->mon_w(!m_motor);
}

READ8_MEMBER( excali64_state::porte8_r )
{
	return 0xfc | (UINT8)m_motor;
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

	m_u12->b_w(space,offset, BIT(data, 5)); // motor pulse
}

/*
d0 = precomp (selectable by jumper)
d1 = size select (we only support 13cm)
d2 = density select (0 = double)
*/
WRITE8_MEMBER( excali64_state::portec_w )
{
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
	UINT8 data = 0xf4; // READY line must be low to print
	data |= (UINT8)m_centronics_busy;
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
	UINT8 data = 0xff;

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
	UINT8 data = m_sys_status & 0x2f;
	bool csync = m_crtc_hs | m_crtc_vs;
	data |= (UINT8)csync << 4;
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
	if BIT(data, 1)
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
	else
	if BIT(data, 0)
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

MACHINE_RESET_MEMBER( excali64_state, excali64 )
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

static GFXDECODE_START( excali64 )
	GFXDECODE_ENTRY( "chargen", 0x0000, excali64_charlayout, 0, 1 )
GFXDECODE_END

// The prom, the schematic, and the manual all contradict each other,
// so the colours can only be described as wild guesses. Further, the 38
// colour-load resistors are missing labels and values.
PALETTE_INIT_MEMBER( excali64_state, excali64 )
{
	// do this here because driver_init hasn't run yet
	m_p_videoram = memregion("videoram")->base();
	m_p_chargen = memregion("chargen")->base();
	m_p_hiresram = m_p_videoram + 0x2000;
	UINT8 *main = memregion("roms")->base();
	UINT8 *ram = memregion("rambank")->base();

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
	UINT8 r,g,b,i,code;
	for (i = 0; i < 32; i++)
	{
		code = m_p_chargen[0x1000+i];
		r = (BIT(code, 0) ? 38 : 0) + (BIT(code, 1) ? 73 : 0) + (BIT(code, 2) ? 144 : 0);
		b = (BIT(code, 3) ? 38 : 0) + (BIT(code, 4) ? 73 : 0) + (BIT(code, 5) ? 144 : 0);
		g = (BIT(code, 6) ? 85 : 0) + (BIT(code, 7) ? 170 : 0);
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
	UINT8 chr,gfx,col,bg,fg;
	UINT16 mem,x;
	UINT8 col_base = BIT(m_sys_status, 3) ? 16 : 0;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		col = m_p_videoram[mem+0x800];
		fg = col_base + (col >> 4);
		bg = 32 + ((col >> 1) & 7);

		if BIT(col, 0)
		{
			UINT8 h = m_p_videoram[mem+0x1000] - 4;
			if (h > 5)
				h = 0; // keep us in bounds
			// hires definition - pixels are opposite order to characters
			gfx = BITSWAP8(m_p_hiresram[(h << 12) | (chr<<4) | ra], 0, 1, 2, 3, 4, 5, 6, 7);
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

static MACHINE_CONFIG_START( excali64, excali64_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(excali64_mem)
	MCFG_CPU_IO_MAP(excali64_io)

	MCFG_MACHINE_RESET_OVERRIDE(excali64_state, excali64)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	//MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz / 16) /* Timer 0: tone gen for speaker */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("speaker", speaker_sound_device, level_w))
	//MCFG_PIT8253_CLK1(XTAL_16MHz / 16) /* Timer 1: baud rate gen for 8251 */
	//MCFG_PIT8253_OUT1_HANDLER(WRITELINE(excali64_state, write_uart_clock))
	//MCFG_PIT8253_CLK2(XTAL_16MHz / 16) /* Timer 2: not used */

	MCFG_DEVICE_ADD("ppi", I8255A, 0 )
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write)) // parallel port
	MCFG_I8255_OUT_PORTB_CB(WRITE8(excali64_state, ppib_w))
	MCFG_I8255_IN_PORTC_CB(READ8(excali64_state, ppic_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(excali64_state, ppic_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 24*12)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*12-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_ADD("palette", 40)
	MCFG_PALETTE_INIT_OWNER(excali64_state, excali64)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", excali64)
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_16MHz / 16) // 1MHz for lowres; 2MHz for highres
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(excali64_state, update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(excali64_state, crtc_hs))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(excali64_state, crtc_vs))

	/* Devices */
	MCFG_CASSETTE_ADD( "cassette" )

	MCFG_WD2793_ADD("fdc", XTAL_16MHz / 16)
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("dma", z80dma_device, rdy_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", excali64_floppies, "drive0", excali64_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", excali64_floppies, "drive1", excali64_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_DEVICE_ADD("dma", Z80DMA, XTAL_16MHz/4)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(excali64_state, busreq_w))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(excali64_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(excali64_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(excali64_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(excali64_state, io_write_byte))

	MCFG_DEVICE_ADD("u12", TTL74123, 0)
	MCFG_TTL74123_CONNECTION_TYPE(TTL74123_GROUNDED)    /* Hook up type (no idea what this means) */
	MCFG_TTL74123_RESISTOR_VALUE(RES_K(100))               /* resistor connected between RCext & 5v */
	MCFG_TTL74123_CAPACITOR_VALUE(CAP_U(100))               /* capacitor connected between Cext and RCext */
	MCFG_TTL74123_A_PIN_VALUE(0)                  /* A pin - grounded */
	MCFG_TTL74123_B_PIN_VALUE(1)                  /* B pin - driven by port e4 bit 5 */
	MCFG_TTL74123_CLEAR_PIN_VALUE(1)                  /* Clear pin - pulled high */
	MCFG_TTL74123_OUTPUT_CHANGED_CB(WRITE8(excali64_state, motor_w))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(excali64_state, cent_busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

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

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     CLASS         INIT        COMPANY         FULLNAME        FLAGS */
COMP( 1984, excali64, 0,      0,       excali64,  excali64, driver_device, 0,  "BGR Computers", "Excalibur 64", 0 )
