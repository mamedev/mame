// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Tiki 100

    12/05/2009 Skeleton driver.

    http://www.djupdal.org/tiki/

****************************************************************************/

/*

    TODO:

    - 3 expansion slots
    - palette RAM should be written during HBLANK
    - DART clocks
    - winchester hard disk
    - analog/digital I/O
    - light pen
    - 8088 CPU card
    - 360KB floppy format

*/

#include "includes/tiki100.h"

/* Memory Banking */

READ8_MEMBER( tiki100_state::read )
{
	bool mdis = 1;

	UINT8 data = m_exp->mrq_r(space, offset, 0xff, mdis);

	offs_t prom_addr = mdis << 5 | m_vire << 4 | m_rome << 3 | (offset >> 13);
	UINT8 prom = m_prom->base()[prom_addr] ^ 0xff;

	if (prom & ROM0)
	{
		data = m_rom->base()[offset & 0x3fff];
	}

	if (prom & ROM1)
	{
		data = m_rom->base()[0x2000 | (offset & 0x3fff)];
	}

	if (prom & VIR)
	{
		UINT16 addr = (offset + (m_scroll << 7)) & TIKI100_VIDEORAM_MASK;

		data = m_video_ram[addr];
	}

	if (prom & RAM)
	{
		data = m_ram->pointer()[offset];
	}

	return data;
}

WRITE8_MEMBER( tiki100_state::write )
{
	bool mdis = 1;
	offs_t prom_addr = mdis << 5 | m_vire << 4 | m_rome << 3 | (offset >> 13);
	UINT8 prom = m_prom->base()[prom_addr] ^ 0xff;

	if (prom & VIR)
	{
		UINT16 addr = (offset + (m_scroll << 7)) & TIKI100_VIDEORAM_MASK;

		m_video_ram[addr] = data;
	}

	if (prom & RAM)
	{
		m_ram->pointer()[offset] = data;
	}

	m_exp->mrq_w(space, offset, data);
}

/* Read/Write Handlers */

READ8_MEMBER( tiki100_state::keyboard_r )
{
	UINT8 data = 0xff;

	switch (m_keylatch)
	{
	case 0: data = m_y1->read(); break;
	case 1: data = m_y2->read(); break;
	case 2: data = m_y3->read(); break;
	case 3: data = m_y4->read(); break;
	case 4: data = m_y5->read(); break;
	case 5: data = m_y6->read(); break;
	case 6: data = m_y7->read(); break;
	case 7: data = m_y8->read(); break;
	case 8: data = m_y9->read(); break;
	case 9: data = m_y10->read(); break;
	case 10: data = m_y11->read(); break;
	case 11: data = m_y12->read(); break;
	}

	m_keylatch++;
	if (m_keylatch == 12) m_keylatch = 0;

	return data;
}

WRITE8_MEMBER( tiki100_state::keyboard_w )
{
	m_keylatch = 0;
}

WRITE8_MEMBER( tiki100_state::video_mode_w )
{
	/*

	    bit     description

	    0       palette entry bit 0
	    1       palette entry bit 1
	    2       palette entry bit 2
	    3       palette entry bit 3
	    4       mode select bit 0
	    5       mode select bit 1
	    6       unused
	    7       write color during HBLANK

	*/

	m_mode = data;

	if (BIT(data, 7))
	{
		int color = data & 0x0f;
		UINT8 colordata = ~m_palette_val;

		m_palette->set_pen_color(color, pal3bit(colordata >> 5), pal3bit(colordata >> 2), pal2bit(colordata >> 0));
	}
}

WRITE8_MEMBER( tiki100_state::palette_w )
{
	/*

	    bit     description

	    0       blue intensity bit 0
	    1       blue intensity bit 1
	    2       green intensity bit 0
	    3       green intensity bit 1
	    4       green intensity bit 2
	    5       red intensity bit 0
	    6       red intensity bit 1
	    7       red intensity bit 2

	*/

	m_palette_val = data;
}

WRITE8_MEMBER( tiki100_state::system_w )
{
	/*

	    bit     signal  description

	    0       DRIS0   drive select 0
	    1       DRIS1   drive select 1
	    2       _ROME   enable ROM at 0000-3fff
	    3       VIRE    enable video RAM at 0000-7fff
	    4       SDEN    single density select (0=DD, 1=SD)
	    5       _LMP0   GRAFIKK key led
	    6       MOTON   floppy motor
	    7       _LMP1   LOCK key led

	*/

	// drive select
	floppy_image_device *floppy = NULL;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	// density select
	m_fdc->dden_w(BIT(data, 4));

	// floppy motor
	if (floppy) floppy->mon_w(!BIT(data, 6));

	/* GRAFIKK key led */
	set_led_status(machine(), 1, BIT(data, 5));

	/* LOCK key led */
	set_led_status(machine(), 2, BIT(data, 7));

	/* bankswitch */
	m_rome = BIT(data, 2);
	m_vire = BIT(data, 3);
}

/* Memory Maps */

static ADDRESS_MAP_START( tiki100_mem, AS_PROGRAM, 8, tiki100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tiki100_io, AS_IO, 8, tiki100_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x03) AM_READWRITE(keyboard_r, keyboard_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE(Z80DART_TAG, z80dart_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0x03) AM_WRITE(video_mode_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(FD1797_TAG, fd1797_t, read, write)
	AM_RANGE(0x14, 0x14) AM_MIRROR(0x01) AM_WRITE(palette_w)
	AM_RANGE(0x16, 0x16) AM_DEVWRITE(AY8912_TAG, ay8910_device, address_w)
	AM_RANGE(0x17, 0x17) AM_DEVREADWRITE(AY8912_TAG, ay8910_device, data_r, data_w)
	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x1c, 0x1c) AM_MIRROR(0x03) AM_WRITE(system_w)
//  AM_RANGE(0x20, 0x27) AM_NOP // winchester controller
//  AM_RANGE(0x60, 0x6f) analog I/O (SINTEF)
//  AM_RANGE(0x60, 0x67) digital I/O (RVO)
//  AM_RANGE(0x70, 0x77) analog/digital I/O
//  AM_RANGE(0x78, 0x7b) light pen
//  AM_RANGE(0x7e, 0x7f) 8088/87 processor w/128KB RAM
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( tiki100 )
/*
        |    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    |
    ----+---------+---------+---------+---------+---------+---------+---------+---------+
      1 | CTRL    | SHIFT   | BRYT    | RETUR   | MLMROM  | / (num) | SLETT   |         |
      2 | GRAFIKK | 1       | ANGRE   | a       | <       | z       | q       | LOCK    |
      3 | 2       | w       | s       | x       | 3       | e       | d       | c       |
      4 | 4       | r       | f       | v       | 5       | t       | g       | b       |
      5 | 6       | y       | h       | n       | 7       | u       | j       | m       |
      6 | 8       | i       | k       | ,       | 9       | o       | l       | .       |
      7 | 0       | p       | ?       | -       | +       | ?       | ?       | HJELP   |
      8 | @       | ^       | '       | VENSTRE | UTVID   | F1      | F4      | SIDEOPP |
      9 | F2      | F3      | F5      | F6      | OPP     | SIDENED | VTAB    | NED     |
     10 | + (num) | - (num) | * (num) | 7 (num) | 8 (num) | 9 (num) | % (num) | = (num) |
     11 | 4 (num) | 5 (num) | 6 (num) | HTAB    | 1 (num) | 0 (num) | . (num) |         |
     12 | HJEM    | H?YRE   | 2 (num) | 3 (num) | ENTER   |         |         |         |
    ----+---------+---------+---------+---------+---------+---------+---------+---------+
*/
	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BRYTT") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MLMROM") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad /") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SLETT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAFIKK") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ANGRE") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\xB8 \xC3\x98") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00f8) PORT_CHAR(0x00d8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\xA5 \xC3\x85") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00e5) PORT_CHAR(0x00c5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\xA6 \xC3\x86") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00e6) PORT_CHAR(0x00c6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HJELP")

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('^') PORT_CHAR('|')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UTVID") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SIDEOPP") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SIDENED") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("VTAB")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad %")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad =")

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HTAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB)) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HJEM") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ENTER") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Video */

UINT32 tiki100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT16 addr = (m_scroll << 7);
	int sx, y, pixel, mode = (m_mode >> 4) & 0x03;

	for (y = 0; y < 256; y++)
	{
		for (sx = 0; sx < 128; sx++)
		{
			UINT8 data = m_video_ram[addr & TIKI100_VIDEORAM_MASK];

			switch (mode)
			{
			case 0:
				for (pixel = 0; pixel < 8; pixel++)
				{
					int x = (sx * 8) + pixel;

					bitmap.pix32(y, x) = palette[0];
				}
				break;

			case 1: /* 1024x256x2 */
				for (pixel = 0; pixel < 8; pixel++)
				{
					int x = (sx * 8) + pixel;
					int color = BIT(data, 0);

					bitmap.pix32(y, x) = palette[color];

					data >>= 1;
				}
				break;

			case 2: /* 512x256x4 */
				for (pixel = 0; pixel < 4; pixel++)
				{
					int x = (sx * 8) + (pixel * 2);
					int color = data & 0x03;

					bitmap.pix32(y, x) = palette[color];
					bitmap.pix32(y, x + 1) = palette[color];

					data >>= 2;
				}
				break;

			case 3: /* 256x256x16 */
				for (pixel = 0; pixel < 2; pixel++)
				{
					int x = (sx * 8) + (pixel * 4);
					int color = data & 0x0f;

					bitmap.pix32(y, x) = palette[color];
					bitmap.pix32(y, x + 1) = palette[color];
					bitmap.pix32(y, x + 2) = palette[color];
					bitmap.pix32(y, x + 3) = palette[color];

					data >>= 4;
				}
				break;
			}

			addr++;
		}
	}

	return 0;
}

/* Z80-PIO Interface */

DECLARE_WRITE_LINE_MEMBER( tiki100_state::write_centronics_ack )
{
	m_centronics_ack = state;
}

DECLARE_WRITE_LINE_MEMBER( tiki100_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

DECLARE_WRITE_LINE_MEMBER( tiki100_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

READ8_MEMBER( tiki100_state::pio_pb_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       ACK
	    5       BUSY
	    6       NO PAPER
	    7       UNIT SELECT, tape in

	*/

	UINT8 data = 0;

	// centronics
	data |= m_centronics_ack << 4;
	data |= m_centronics_busy << 5;
	data |= m_centronics_perror << 6;

	// cassette
	data |= (m_cassette->input() > 0.0) << 7;

	return data;
}

WRITE8_MEMBER( tiki100_state::pio_pb_w )
{
	/*

	    bit     description

	    0       STRB
	    1
	    2
	    3
	    4
	    5
	    6       tape out
	    7

	*/

	// centronics
	m_centronics->write_strobe(BIT(data, 0));

	// cassette
	m_cassette->output(BIT(data, 6) ? -1 : 1);
}

/* Z80-CTC Interface */

TIMER_DEVICE_CALLBACK_MEMBER(tiki100_state::ctc_tick)
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);

	m_ctc->trg1(1);
	m_ctc->trg1(0);
}

WRITE_LINE_MEMBER( tiki100_state::ctc_z0_w )
{
	m_ctc->trg2(state);

	m_dart->rxca_w(state);
	m_dart->txca_w(state);
}

WRITE_LINE_MEMBER( tiki100_state::ctc_z2_w )
{
	m_ctc->trg3(state);
}

/* FD1797 Interface */

FLOPPY_FORMATS_MEMBER( tiki100_state::floppy_formats )
	FLOPPY_TIKI100_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( tiki100_floppies )
	SLOT_INTERFACE( "525ssdd", FLOPPY_525_SSDD )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD ) // Tead FD-55A
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // Teac FD-55F
SLOT_INTERFACE_END

/* AY-3-8912 Interface */

WRITE8_MEMBER( tiki100_state::video_scroll_w )
{
	m_scroll = data;
}

/* Z80 Daisy Chain */

static const z80_daisy_config tiki100_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80DART_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};



TIMER_DEVICE_CALLBACK_MEMBER( tiki100_state::tape_tick )
{
	m_pio->port_b_write((m_cassette->input() > 0.0) << 7);
}

/* Machine Start */

void tiki100_state::machine_start()
{
	/* allocate video RAM */
	m_video_ram.allocate(TIKI100_VIDEORAM_SIZE);

	/* register for state saving */
	save_item(NAME(m_rome));
	save_item(NAME(m_vire));
	save_item(NAME(m_scroll));
	save_item(NAME(m_mode));
	save_item(NAME(m_palette_val));
	save_item(NAME(m_keylatch));
}

void tiki100_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	system_w(space, 0, 0);
}

/* Machine Driver */

static MACHINE_CONFIG_START( tiki100, tiki100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(tiki100_mem)
	MCFG_CPU_IO_MAP(tiki100_io)
	MCFG_CPU_CONFIG(tiki100_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(tiki100_state, screen_update)
	MCFG_SCREEN_SIZE(1024, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 256-1)

	MCFG_PALETTE_ADD("palette", 16)
	// pixel clock 20.01782 MHz

	MCFG_TIKI100_BUS_ADD()
	//MCFG_TIKI100_BUS_IRQ_CALLBACK()
	//MCFG_TIKI100_BUS_NMI_CALLBACK()
	MCFG_TIKI100_BUS_SLOT_ADD("slot1", "8088")
	MCFG_TIKI100_BUS_SLOT_ADD("slot2", NULL)
	MCFG_TIKI100_BUS_SLOT_ADD("slot3", NULL)

	/* devices */
	MCFG_Z80DART_ADD(Z80DART_TAG, XTAL_8MHz/4, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_8MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(DEVREAD8("cent_data_in", input_buffer_device, read))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_IN_PB_CB(READ8(tiki100_state, pio_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(tiki100_state, pio_pb_w))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_8MHz/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(tiki100_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxtxcb_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(tiki100_state, ctc_z2_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", tiki100_state, ctc_tick, attotime::from_hz(XTAL_8MHz/4))

	MCFG_FD1797_ADD(FD1797_TAG, XTAL_8MHz/8) // FD1767PL-02 or FD1797-PL
	MCFG_FLOPPY_DRIVE_ADD(FD1797_TAG":0", tiki100_floppies, "525qd", tiki100_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1797_TAG":1", tiki100_floppies, "525qd", tiki100_state::floppy_formats)

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxb_w))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(tiki100_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(tiki100_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(tiki100_state, write_centronics_perror))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD(CASSETTE_TAG)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("tape", tiki100_state, tape_tick, attotime::from_hz(44100))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(AY8912_TAG, AY8912, XTAL_8MHz/4)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(tiki100_state, video_scroll_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "tiki100")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( kontiki )
	ROM_REGION( 0x4000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "tikirom-1.30.u10",  0x0000, 0x2000, CRC(c482dcaf) SHA1(d140706bb7fc8b1fbb37180d98921f5bdda73cf9) )

	ROM_REGION( 0x100, "u4", 0 )
	ROM_LOAD( "53ls140.u4", 0x000, 0x100, CRC(894b756f) SHA1(429e10de0e0e749246895801b18186ff514c12bc) )
ROM_END

ROM_START( tiki100 )
	ROM_REGION( 0x4000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v203w" )

	ROM_SYSTEM_BIOS( 0, "v135", "TIKI ROM v1.35" )
	ROMX_LOAD( "tikirom-1.35.u10",  0x0000, 0x2000, CRC(7dac5ee7) SHA1(14d622fd843833faec346bf5357d7576061f5a3d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v203w", "TIKI ROM v2.03 W" )
	ROMX_LOAD( "tikirom-2.03w.u10", 0x0000, 0x2000, CRC(79662476) SHA1(96336633ecaf1b2190c36c43295ac9f785d1f83a), ROM_BIOS(2) )

	ROM_REGION( 0x100, "u4", 0 )
	ROM_LOAD( "53ls140.u4", 0x000, 0x100, CRC(894b756f) SHA1(429e10de0e0e749246895801b18186ff514c12bc) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY             FULLNAME        FLAGS */
COMP( 1984, kontiki,    0,          0,      tiki100,    tiki100, driver_device, 0,      "Kontiki Data A/S", "KONTIKI 100",  MACHINE_SUPPORTS_SAVE )
COMP( 1984, tiki100,    kontiki,    0,      tiki100,    tiki100, driver_device, 0,      "Tiki Data A/S",    "TIKI 100",     MACHINE_SUPPORTS_SAVE )
