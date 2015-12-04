// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Compucolor II

    http://www.compucolor.org/index.html

*/

/*

    TODO:

    - floppy
    - interlaced video
    - add-on ROM
    - add-on RAM
    - add-on unit

*/

#define I8080_TAG   "ua2"
#define TMS5501_TAG "ud2"
#define CRT5027_TAG "uf9"
#define RS232_TAG   "rs232"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "bus/compucolor/floppy.h"
#include "machine/ram.h"
#include "machine/tms5501.h"
#include "video/tms9927.h"
#include "softlist.h"

class compucolor2_state : public driver_device
{
public:
	compucolor2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8080_TAG),
			m_mioc(*this, TMS5501_TAG),
			m_vtac(*this, CRT5027_TAG),
			m_palette(*this, "palette"),
			m_rs232(*this, RS232_TAG),
			m_floppy0(*this, "cd0"),
			m_floppy1(*this, "cd1"),
			m_char_rom(*this, "chargen"),
			m_video_ram(*this, "videoram"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_y8(*this, "Y8"),
			m_y9(*this, "Y9"),
			m_y10(*this, "Y10"),
			m_y11(*this, "Y11"),
			m_y12(*this, "Y12"),
			m_y13(*this, "Y13"),
			m_y14(*this, "Y14"),
			m_y15(*this, "Y15"),
			m_y128(*this, "Y128")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<tms5501_device> m_mioc;
	required_device<crt5027_device> m_vtac;
	required_device<palette_device> m_palette;
	required_device<rs232_port_device> m_rs232;
	required_device<compucolor_floppy_port_device> m_floppy0;
	required_device<compucolor_floppy_port_device> m_floppy1;
	required_memory_region m_char_rom;
	required_shared_ptr<UINT8> m_video_ram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_y11;
	required_ioport m_y12;
	required_ioport m_y13;
	required_ioport m_y14;
	required_ioport m_y15;
	required_ioport m_y128;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( xi_r );
	DECLARE_WRITE8_MEMBER( xo_w );
	DECLARE_WRITE_LINE_MEMBER( xmt_w );

	IRQ_CALLBACK_MEMBER( int_ack );

	UINT8 m_xo;
};

static ADDRESS_MAP_START( compucolor2_mem, AS_PROGRAM, 8, compucolor2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION(I8080_TAG, 0)
	AM_RANGE(0x6000, 0x6fff) AM_MIRROR(0x1000) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( compucolor2_io, AS_IO, 8, compucolor2_state )
	AM_RANGE(0x00, 0x0f) AM_MIRROR(0x10) AM_DEVICE(TMS5501_TAG, tms5501_device, io_map)
	AM_RANGE(0x60, 0x6f) AM_MIRROR(0x10) AM_DEVREADWRITE(CRT5027_TAG, crt5027_device, read, write)
	AM_RANGE(0x80, 0x9f) AM_MIRROR(0x60) AM_ROM AM_REGION("ua1", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( compucolor2 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_ CRT") PORT_CHAR('_')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F15 PLOT ESC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BL/A7 OFF")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BLINK ON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("^ USER") PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F14 CHAR RECT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A7 ON")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BG ON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("m M TERM MODE") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("] BLCK") PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F13 POINT X")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FG ON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("l L LOCL") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\ 45 UP") PORT_CHAR('\\')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F12 POINT Y")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ERASE PAGE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("k K ROLL") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("[ VIS") PORT_CHAR('[')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F11 POINT INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ERASE LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("z Z 45 DW") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10 X BAR XO")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad X") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("y Y TEST") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9 X BAR Y")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("h H HALF") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("x X XMIT") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8 X BAR XM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("g G BELL") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("w W BASIC") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7 X BAR INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LIST")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("v V DEL LINE") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6 Y BAR YO")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("POKE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("e E BSC RST") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("u U INS LINE") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5 Y BAR X")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AUTO") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRINT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("d D DISK FCS") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("t T TEXT EDIT") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4 Y BAR YM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE CHAR")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOAD")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("c C CURSOR X-Y") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("s S ASMB") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3 Y BAR INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SAVE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("b B PLOT") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("r R BAUD RATE") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2 VECT X1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PLOT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("a A PROT BLND") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("q Q INS CHAR") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1 VECT Y1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT CHAR")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PUT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@ NULL") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("p P CPU OP SYS") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F0 VECT INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK ATTN")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("OUT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y128")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CPU RESET")
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPEAT")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END

UINT32 compucolor2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32*8; y++)
	{
		offs_t offset = (y / 8) * 128;

		for (int sx = 0; sx < 64; sx++)
		{
			UINT8 code = m_video_ram[offset++];
			UINT8 attr = m_video_ram[offset++];

			offs_t char_offs = ((code & 0x7f) << 3) | (y & 0x07);
			if (BIT(code, 7)) char_offs = ((code & 0x7f) << 3) | ((y >> 1) & 0x07);

			UINT8 data = m_char_rom->base()[char_offs];

			rgb_t fg = m_palette->pen_color(attr & 0x07);
			rgb_t bg = m_palette->pen_color((attr >> 3) & 0x07);

			for (int x = 0; x < 6; x++)
			{
				bitmap.pix32(y, (sx * 6) + x) = BIT(data, 7) ? fg : bg;

				data <<= 1;
			}
		}
	}

	return 0;
}

READ8_MEMBER( compucolor2_state::xi_r )
{
	UINT8 data = 0xff;

	switch ((m_xo >> 4) & 0x03)
	{
	case 0:
		switch (m_xo & 0x0f)
		{
		case 0: data &= m_y0->read(); break;
		case 1: data &= m_y1->read(); break;
		case 2: data &= m_y2->read(); break;
		case 3: data &= m_y3->read(); break;
		case 4: data &= m_y4->read(); break;
		case 5: data &= m_y5->read(); break;
		case 6: data &= m_y6->read(); break;
		case 7: data &= m_y7->read(); break;
		case 8: data &= m_y8->read(); break;
		case 9: data &= m_y9->read(); break;
		case 10: data &= m_y10->read(); break;
		case 11: data &= m_y11->read(); break;
		case 12: data &= m_y12->read(); break;
		case 13: data &= m_y13->read(); break;
		case 14: data &= m_y14->read(); break;
		case 15: data &= m_y15->read(); break;
		}

		if (BIT(m_xo, 7))
		{
			data = (m_y128->read() & 0xf0) | (data & 0x0f);
		}
		break;

	default:
		data = 0;
	}

	return data;
}

WRITE8_MEMBER( compucolor2_state::xo_w )
{
	/*

	    bit     description

	    0       Keyboard A0, Stepper Phase 0
	    1       Keyboard A1, Stepper Phase 1
	    2       Keyboard A2, Stepper Phase 2
	    3       Keyboard A3, Disk _R/W
	    4       00=Keyboard/Modem, 01=Internal Drive (CD0), 10=External Drive (CD1)
	    5       ^
	    6       LED
	    7       0=Normal Keyboard, 1=SHIFT/CONTROL/REPEAT/CAPS LOCK

	*/

	m_xo = data;

	switch ((m_xo >> 4) & 0x03)
	{
	case 1:
		m_floppy0->select_w(0);
		m_floppy0->rw_w(BIT(data, 3));
		m_floppy0->stepper_w(data & 0x07);

		m_floppy1->select_w(1);
		break;

	case 2:
		m_floppy1->select_w(0);
		m_floppy1->rw_w(BIT(data, 3));
		m_floppy1->stepper_w(data & 0x07);

		m_floppy0->select_w(1);
		break;
	}
}

WRITE_LINE_MEMBER( compucolor2_state::xmt_w )
{
	switch ((m_xo >> 4) & 0x03)
	{
	case 0:
		m_rs232->write_txd(state);
		break;

	case 1:
		m_floppy0->write_txd(state);
		break;

	case 2:
		m_floppy1->write_txd(state);
		break;
	}
}

IRQ_CALLBACK_MEMBER( compucolor2_state::int_ack )
{
	return m_mioc->get_vector();
}

void compucolor2_state::machine_start()
{
	// state saving
	save_item(NAME(m_xo));
}

void compucolor2_state::machine_reset()
{
	m_rs232->write_rts(1);
	m_rs232->write_dtr(1);
}

static MACHINE_CONFIG_START( compucolor2, compucolor2_state )
	// basic machine hardware
	MCFG_CPU_ADD(I8080_TAG, I8080, XTAL_17_9712MHz/9)
	MCFG_CPU_PROGRAM_MAP(compucolor2_mem)
	MCFG_CPU_IO_MAP(compucolor2_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(compucolor2_state,int_ack)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(compucolor2_state, screen_update)
	MCFG_SCREEN_SIZE(64*6, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*6-1, 0, 32*8-1)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_DEVICE_ADD(CRT5027_TAG, CRT5027, XTAL_17_9712MHz/2)
	MCFG_TMS9927_CHAR_WIDTH(6)
	MCFG_TMS9927_VSYN_CALLBACK(DEVWRITELINE(TMS5501_TAG, tms5501_device, sens_w))
	MCFG_VIDEO_SET_SCREEN("screen")

	// devices
	MCFG_DEVICE_ADD(TMS5501_TAG, TMS5501, XTAL_17_9712MHz/9)
	MCFG_TMS5501_IRQ_CALLBACK(INPUTLINE(I8080_TAG, I8085_INTR_LINE))
	MCFG_TMS5501_XMT_CALLBACK(WRITELINE(compucolor2_state, xmt_w))
	MCFG_TMS5501_XI_CALLBACK(READ8(compucolor2_state, xi_r))
	MCFG_TMS5501_XO_CALLBACK(WRITE8(compucolor2_state, xo_w))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(TMS5501_TAG, tms5501_device, rcv_w))

	MCFG_COMPUCOLOR_FLOPPY_PORT_ADD("cd0", compucolor_floppy_port_devices, "floppy")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(TMS5501_TAG, tms5501_device, rcv_w))

	MCFG_COMPUCOLOR_FLOPPY_PORT_ADD("cd1", compucolor_floppy_port_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(TMS5501_TAG, tms5501_device, rcv_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("8K,16K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compclr2_flop")
MACHINE_CONFIG_END

ROM_START( compclr2 )
	ROM_REGION( 0x4000, I8080_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "678", "v6.78" )
	ROMX_LOAD( "v678.rom", 0x0000, 0x4000, BAD_DUMP CRC(5e559469) SHA1(fe308774aae1294c852fe24017e58d892d880cd3), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "879", "v8.79" )
	ROMX_LOAD( "v879.rom", 0x0000, 0x4000, BAD_DUMP CRC(4de8e652) SHA1(e5c55da3ac893b8a5a99c8795af3ca72b1645f3f), ROM_BIOS(2) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "chargen.uf6", 0x000, 0x400, BAD_DUMP CRC(7eef135a) SHA1(be488ef32f54c6e5f551fb84ab12b881aef72dd9) )
	ROM_LOAD( "chargen.uf7", 0x400, 0x400, BAD_DUMP CRC(2bee7cf6) SHA1(808e0fc6f2332b4297de190eafcf84668703e2f4) )

	ROM_REGION( 0x20, "ua1", 0 )
	ROM_LOAD( "82s123.ua1", 0x00, 0x20, BAD_DUMP CRC(27ae54bc) SHA1(ccb056fbc1ec2132f2602217af64d77237494afb) ) // I/O PROM

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s129.ue2", 0x00, 0x20, NO_DUMP ) // Address Decoder/Timer for RAM
	ROM_LOAD( "82s129.uf3", 0x00, 0x20, NO_DUMP ) // System Decoder
	ROM_LOAD( "82s123.ub3", 0x00, 0x20, NO_DUMP ) // ROM/PROM Decoder
	ROM_LOAD( "82s129.uf8", 0x00, 0x20, NO_DUMP ) // CPU & Horizontal Decoder
	ROM_LOAD( "82s129.ug9", 0x00, 0x20, NO_DUMP ) // Scan Decoder
	ROM_LOAD( "82s129.ug5", 0x00, 0x20, NO_DUMP ) // Color PROM
ROM_END

COMP( 1977, compclr2,    0,      0,      compucolor2,        compucolor2, driver_device, 0,      "Intelligent Systems Corporation",  "Compucolor II",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
