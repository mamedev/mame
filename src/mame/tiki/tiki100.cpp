// license:BSD-3-Clause
// copyright-holders:Curt Coder, Frode van der Meeren
/***************************************************************************

    Tiki 100

    12/05/2009 Skeleton driver.

    Most of the driver is written by Curt Coder.

    2020-06-02:
       Screen-drawing redone by Frode van der Meeren.
       There is still some work that needs to get this
       even more accurate, but it's mostly down to
       triggering update at the right times.

****************************************************************************/

/*

    TODO:

    - winchester hard disk
    - analog/digital I/O
    - light pen
    - 8088 CPU card

*/

#include "emu.h"
#include "tiki100.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


/* Memory Banking */

uint8_t tiki100_state::mrq_r(offs_t offset)
{
	bool mdis = 1;

	uint8_t data = m_exp->mrq_r(offset, 0xff, mdis);

	offs_t prom_addr = mdis << 5 | m_vire << 4 | m_rome << 3 | (offset >> 13);
	uint8_t prom = m_prom->base()[prom_addr] ^ 0xff;

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
		uint16_t addr = (offset + (m_scroll << 7)) & TIKI100_VIDEORAM_MASK;

		data = m_video_ram[addr];
	}

	if (prom & RAM0)
	{
		data = m_ram->pointer()[offset];
	}

	return data;
}

void tiki100_state::mrq_w(offs_t offset, uint8_t data)
{
	bool mdis = 1;
	offs_t prom_addr = mdis << 5 | m_vire << 4 | m_rome << 3 | (offset >> 13);
	uint8_t prom = m_prom->base()[prom_addr] ^ 0xff;

	if (prom & VIR)
	{
		uint16_t addr = (offset + (m_scroll << 7)) & TIKI100_VIDEORAM_MASK;

		m_video_ram[addr] = data;
	}

	if (prom & RAM0)
	{
		m_ram->pointer()[offset] = data;
	}

	m_exp->mrq_w(offset, data);
}

uint8_t tiki100_state::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset, 0xff);

	switch ((offset & 0xff) >> 2)
	{
	case 0x00: // KEYS
		data = keyboard_r();
		break;

	case 0x01: // SERS
		data = m_dart->cd_ba_r(offset & 0x03);
		break;

	case 0x02: // PARS
		data = m_pio->read(offset & 0x03);
		break;

	case 0x04: // FLOP
		data = m_fdc->read(offset & 0x03);
		break;

	case 0x05: // VIPS
		switch (offset & 0x03)
		{
		case 3:
			data = m_psg->data_r();
			break;
		}
		break;

	case 0x06: // TIMS
		data = m_ctc->read(offset & 0x03);
		break;
	}

	return data;
}

void tiki100_state::iorq_w(offs_t offset, uint8_t data)
{
	m_exp->iorq_w(offset, data);

	switch ((offset & 0xff) >> 2)
	{
	case 0x00: // KEYS
		keyboard_w(data);
		break;

	case 0x01: // SERS
		m_dart->cd_ba_w(offset & 0x03, data);
		break;

	case 0x02: // PARS
		m_pio->write(offset & 0x03, data);
		break;

	case 0x03: // VIPB
		video_mode_w(data);
		break;

	case 0x04: // FLOP
		m_fdc->write(offset & 0x03, data);
		break;

	case 0x05: // VIPS
		switch (offset & 0x03)
		{
		case 0: case 1:
			palette_w(data);
			break;

		case 2:
			m_psg->address_w(data);
			break;

		case 3:
			m_psg->data_w(data);
			break;
		}
		break;

	case 0x06: // TIMS
		m_ctc->write(offset & 0x03, data);
		break;

	case 0x07: // SYL
		system_w(data);
		break;
	}
}

/* Read/Write Handlers */

uint8_t tiki100_state::keyboard_r()
{
	uint8_t data = 0xff;

	if (m_keylatch < 13)
	{
		data = m_y[m_keylatch]->read();
	}

	m_keylatch++;
	if (m_keylatch == 16) m_keylatch = 0;   // Column selected by a 4-bit counter

	return data;
}

void tiki100_state::keyboard_w(uint8_t data)
{
	m_keylatch = 0;
}

void tiki100_state::video_mode_w(uint8_t data)
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
}

void tiki100_state::palette_w(uint8_t data)
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

void tiki100_state::system_w(uint8_t data)
{
	/*

	    bit     signal  description

	    0       DRIS0   drive select 0
	    1       DRIS1   drive select 1
	    2       _ROME   enable ROM at 0000-3fff
	    3       VIRE    enable video RAM at 0000-7fff or 4000-bfff
	    4       SDEN    single density select (0=DD, 1=SD)
	    5       _LMP0   GRAFIKK key led
	    6       MOTON   floppy motor
	    7       _LMP1   LOCK key led

	*/

	// drive select
	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	// density select
	m_fdc->dden_w(BIT(data, 4));

	// floppy motor
	if (floppy) floppy->mon_w(!BIT(data, 6));

	/* GRAFIKK key led */
	m_leds[0] = BIT(data, 5);

	/* LOCK key led */
	m_leds[1] = BIT(data, 7);

	/* bankswitch */
	m_rome = BIT(data, 2);
	m_vire = BIT(data, 3);
}

/* Memory Maps */

void tiki100_state::tiki100_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(tiki100_state::mrq_r), FUNC(tiki100_state::mrq_w));
}

void tiki100_state::tiki100_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(tiki100_state::iorq_r), FUNC(tiki100_state::iorq_w));
}

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
     13 |         |         |         |         |         |         |         |         |
    ----+---------+---------+---------+---------+---------+---------+---------+---------+

    Unused bits may return some fixed values, an undocumented system call will read and return them.
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

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ST")
	PORT_CONFNAME( 0x01, 0x01, "DART TxCA")
	PORT_CONFSETTING( 0x00, "BAR0" )
	PORT_CONFSETTING( 0x01, "BAR2" )
INPUT_PORTS_END

/* Video */

TIMER_DEVICE_CALLBACK_MEMBER( tiki100_state::scanline_start )
{
	// TODO: Optional assertion of VSYNC on one of the PIO-pins, as used by some demos
	//int scanline = param;

	// TODO: use TIMER to trigger this every 16-pixel chunk (20MHz/16) instead of once at the end of the scanline
	m_screen->update_now();

	// This point is by the end of a line. Changes in palette are applied here.
	if (BIT(m_mode, 7))
	{
		int color = m_mode & 0x0f;
		uint8_t colordata = ~m_palette_val;

		m_palette->set_pen_color(color, pal3bit(colordata >> 5), pal3bit(colordata >> 2), pal2bit(colordata >> 0));
	}
}

uint32_t tiki100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//
	//  Emulates the pixel-shifter and row/column counters from a
	//  starting point on screen to an ending point on screen.
	//
	//  For the most accurate results, this should be run once for
	//  every 16-pixel chunk, but per the spring of 2020 there is
	//  only one demo that require this degree of accuracy.
	//
	//

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	for (int vaddr = cliprect.min_y; vaddr <= cliprect.max_y; vaddr++)
	{
		// This is at the start of a line
		int haddr = (cliprect.min_x>>4);
		int const haddr_end = (cliprect.max_x>>4);
		for (; haddr <= haddr_end; haddr++)
		{
			// This is at the start of a 16-dot cluster. Changes in m_scroll and m_video_ram come into effect here.
			uint16_t addr = ((vaddr+m_scroll)<<7) | (haddr<<1);
			uint16_t data = (m_video_ram[(addr+1) & TIKI100_VIDEORAM_MASK]<<8) | m_video_ram[addr & TIKI100_VIDEORAM_MASK];
			for(int dot = 0; dot < 16; dot++)
			{
				// This is at the start of a dot. Changes in m_mode are applied at this point.
				int mode = (m_mode >> 4) & 0x03;
				if((mode == 1) || (!(dot&0x01) && mode == 2) || (!(dot&0x03) && mode == 3))
				{
					// This is the point when a pixel is latched from the dot-shifter.
					//
					//   For the sake of readability:
					//   mode == 0: keep the old pixel
					//   mode == 1: latch new pixel once per dot
					//   mode == 2: latch new pixel once per 2 dots
					//   mode == 3: latch new pixel once per 4 dots
					//
					m_current_pixel = data&0x0F;
				}
				bitmap.pix(vaddr, (haddr<<4) + dot) = palette[m_current_pixel&0x0F];

				// This will run the dot-shifter and add the TEST-pattern to the upper bits (usually hidden by palette).
				data = (data>>1)|((haddr&0x02)<<14);
			}
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

uint8_t tiki100_state::pio_pb_r()
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

	uint8_t data = 0;

	// centronics
	data |= m_centronics_ack << 4;
	data |= m_centronics_busy << 5;
	data |= m_centronics_perror << 6;

	// cassette
	data |= (m_cassette->input() > 0.0) << 7;

	return data;
}

void tiki100_state::pio_pb_w(uint8_t data)
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

WRITE_LINE_MEMBER( tiki100_state::bar0_w )
{
	m_ctc->trg2(state);

	m_dart->rxca_w(state);

	if (!m_st) m_dart->txca_w(state);
}

WRITE_LINE_MEMBER( tiki100_state::bar2_w )
{
	if (m_st) m_dart->txca_w(state);

	m_ctc->trg3(state);
}

/* FD1797 Interface */

void tiki100_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TIKI100_FORMAT);
}

static void tiki100_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd", FLOPPY_525_DD); // Tead FD-55A
	device.option_add("525qd", FLOPPY_525_QD); // Teac FD-55F
}

/* AY-3-8912 Interface */

void tiki100_state::video_scroll_w(uint8_t data)
{
	m_scroll = data;
}

/* Z80 Daisy Chain */

static const z80_daisy_config tiki100_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80DART_TAG },
	{ Z80PIO_TAG },
	{ "slot1" },
	{ "slot2" },
	{ "slot3" },
	{ nullptr }
};

TIMER_DEVICE_CALLBACK_MEMBER( tiki100_state::tape_tick )
{
	m_pio->port_b_write((m_cassette->input() > 0.0) << 7);
}

WRITE_LINE_MEMBER( tiki100_state::busrq_w )
{
	// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_exp->busak_w(state);
}

/* Machine Start */

void tiki100_state::machine_start()
{
	m_leds.resolve();

	/* register for state saving */
	save_item(NAME(m_rome));
	save_item(NAME(m_vire));
	save_item(NAME(m_scroll));
	save_item(NAME(m_mode));
	save_item(NAME(m_palette_val));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_st));
}

void tiki100_state::machine_reset()
{
	system_w(0);

	m_st = m_st_io->read();
}

/* Machine Driver */

void tiki100_state::tiki100(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tiki100_state::tiki100_mem);
	m_maincpu->set_addrmap(AS_IO, &tiki100_state::tiki100_io);
	m_maincpu->set_daisy_config(tiki100_daisy_chain);

	/* video hardware */
	TIMER(config, "scantimer").configure_scanline(FUNC(tiki100_state::scanline_start), "screen", 0, 1);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20_MHz_XTAL, 1280, 0, 1024, 312, 0, 256);
	m_screen->set_screen_update(FUNC(tiki100_state::screen_update));
	PALETTE(config, m_palette).set_entries(16);

	TIKI100_BUS(config, m_exp, 0);
	m_exp->irq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_exp->nmi_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->busrq_wr_callback().set(FUNC(tiki100_state::busrq_w));
	m_exp->mrq_rd_callback().set(FUNC(tiki100_state::mrq_r));
	m_exp->mrq_wr_callback().set(FUNC(tiki100_state::mrq_w));
	TIKI100_BUS_SLOT(config, "slot1", m_exp, tiki100_cards, "8088");
	TIKI100_BUS_SLOT(config, "slot2", m_exp, tiki100_cards, "hdc");
	TIKI100_BUS_SLOT(config, "slot3", m_exp, tiki100_cards, nullptr);

	/* devices */
	Z80DART(config, m_dart, 8_MHz_XTAL / 4);
	m_dart->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio, 8_MHz_XTAL / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set("cent_data_in", FUNC(input_buffer_device::read));
	m_pio->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio->in_pb_callback().set(FUNC(tiki100_state::pio_pb_r));
	m_pio->out_pb_callback().set(FUNC(tiki100_state::pio_pb_w));

	Z80CTC(config, m_ctc, 8_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(tiki100_state::bar0_w));
	m_ctc->zc_callback<1>().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	m_ctc->zc_callback<2>().set(FUNC(tiki100_state::bar2_w));

	TIMER(config, "ctc").configure_periodic(FUNC(tiki100_state::ctc_tick), attotime::from_hz(8_MHz_XTAL / 4));

	FD1797(config, m_fdc, 8_MHz_XTAL / 8); // FD1767PL-02 or FD1797-PL
	FLOPPY_CONNECTOR(config, FD1797_TAG":0", tiki100_floppies, "525qd", tiki100_state::floppy_formats);
	FLOPPY_CONNECTOR(config, FD1797_TAG":1", tiki100_floppies, "525qd", tiki100_state::floppy_formats);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer("cent_data_in");
	m_centronics->ack_handler().set(FUNC(tiki100_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(tiki100_state::write_centronics_busy));
	m_centronics->perror_handler().set(FUNC(tiki100_state::write_centronics_perror));

	INPUT_BUFFER(config, "cent_data_in");

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_psg, 8_MHz_XTAL / 4);
	m_psg->set_flags(AY8910_SINGLE_OUTPUT);
	m_psg->port_a_write_callback().set(FUNC(tiki100_state::video_scroll_w));
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	TIMER(config, "tape").configure_periodic(FUNC(tiki100_state::tape_tick), attotime::from_hz(44100));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("tiki100");
}

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
	ROMX_LOAD( "tikirom-1.35.u10",  0x0000, 0x2000, CRC(7dac5ee7) SHA1(14d622fd843833faec346bf5357d7576061f5a3d), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v203w", "TIKI ROM v2.03 W" )
	ROMX_LOAD( "tikirom-2.03w.u10", 0x0000, 0x2000, CRC(79662476) SHA1(96336633ecaf1b2190c36c43295ac9f785d1f83a), ROM_BIOS(1) )

	ROM_REGION( 0x100, "u4", 0 )
	ROM_LOAD( "53ls140.u4", 0x000, 0x100, CRC(894b756f) SHA1(429e10de0e0e749246895801b18186ff514c12bc) )
ROM_END

/* System Drivers */

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY             FULLNAME        FLAGS
COMP( 1984, kontiki, 0,       0,      tiki100, tiki100, tiki100_state, empty_init, "Kontiki Data A/S", "KONTIKI 100",  MACHINE_SUPPORTS_SAVE )
COMP( 1984, tiki100, kontiki, 0,      tiki100, tiki100, tiki100_state, empty_init, "Tiki Data A/S",    "TIKI 100",     MACHINE_SUPPORTS_SAVE )
