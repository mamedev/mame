// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Specialist driver by Miodrag Milanovic

        2008-03-15 Preliminary driver.
        2008-03-20 Cassette support

****************************************************************************/

#include "emu.h"
#include "includes/special.h"

#include "sound/volt_reg.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


/* Address maps */
void special_state::specialist_mem(address_map &map)
{
	map(0x0000, 0x2fff).bankrw("bank1"); // First bank
	map(0x3000, 0x8fff).ram();  // RAM
	map(0x9000, 0xbfff).ram().share("videoram"); // Video RAM
	map(0xc000, 0xefff).rom();  // System ROM
	map(0xf800, 0xf803).mirror(0x7fc).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void special_state::specialp_mem(address_map &map)
{
	map(0x0000, 0x2fff).bankrw("bank1"); // First bank
	map(0x3000, 0x7fff).ram();  // RAM
	map(0x8000, 0xbfff).ram().share("videoram"); // Video RAM
	map(0xc000, 0xefff).rom();  // System ROM
	map(0xf800, 0xf803).mirror(0x7fc).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void special_state::erik_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x8fff).bankrw("bank2");
	map(0x9000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xefff).bankrw("bank4");
	map(0xf000, 0xf7ff).bankrw("bank5");
	map(0xf800, 0xffff).bankrw("bank6");
}

void special_state::erik_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf1, 0xf1).rw(FUNC(special_state::erik_rr_reg_r), FUNC(special_state::erik_rr_reg_w));
	map(0xf2, 0xf2).rw(FUNC(special_state::erik_rc_reg_r), FUNC(special_state::erik_rc_reg_w));
	map(0xf3, 0xf3).rw(FUNC(special_state::erik_disk_reg_r), FUNC(special_state::erik_disk_reg_w));
	map(0xf4, 0xf7).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
}

void special_state::specimx_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x8fff).bankrw("bank1");
	map(0x9000, 0xbfff).bankrw("bank2");
	map(0xc000, 0xffbf).bankrw("bank3");
	map(0xffc0, 0xffdf).bankrw("bank4");
	map(0xffe0, 0xffe3).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xffe4, 0xffe7).ram(); //external 8255
	map(0xffe8, 0xffeb).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0xffec, 0xffef).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xfff0, 0xfff3).rw(FUNC(special_state::specimx_disk_ctrl_r), FUNC(special_state::specimx_disk_ctrl_w));
	map(0xfff8, 0xfffb).rw(FUNC(special_state::specimx_video_color_r), FUNC(special_state::specimx_video_color_w));
	map(0xfffc, 0xffff).w(FUNC(special_state::specimx_select_bank));
}

/* Input ports */

/* Inputs will need to be adapted when/if support for non-latin keys is added to natural keyboard:
these systems have different keys to switch among alphabets!
*/
static INPUT_PORTS_START( special )
/* Alt switches between Latin and Cyrillic alphabets */
	PORT_START("LINE0")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("LINE1")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))

	PORT_START("LINE2")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("LINE3")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("LINE4")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("LINE5")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("LINE6")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("LINE7")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE8")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("LINE9")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("LINE10")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("LINE11")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lat / Cyr") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

static INPUT_PORTS_START( specialp )
	PORT_INCLUDE( special )

	/* Shift itself switches between Latin and Cyrillic alphabets */
	PORT_MODIFY("LINE0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')

	PORT_MODIFY("LINE11")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
INPUT_PORTS_END

static INPUT_PORTS_START( lik )
	PORT_INCLUDE( special )

	/* 2009-05 FP: Shift + Numbers produces still numbers: does it work differently outside monitor? */
	PORT_MODIFY("LINE0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_MODIFY("LINE2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')// PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')// PORT_CHAR(')')

	PORT_MODIFY("LINE3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')// PORT_CHAR('(')

	PORT_MODIFY("LINE4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')// PORT_CHAR('\'')

	PORT_MODIFY("LINE5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')// PORT_CHAR('&')

	PORT_MODIFY("LINE6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')// PORT_CHAR('%')

	PORT_MODIFY("LINE7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')// PORT_CHAR('$')

	PORT_MODIFY("LINE8")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')// PORT_CHAR('#')

	PORT_MODIFY("LINE9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')// PORT_CHAR('"')

	PORT_MODIFY("LINE10")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')// PORT_CHAR('!')
INPUT_PORTS_END


static INPUT_PORTS_START( specimx )
/* Alt switches between Latin and Cyrillic alphabets */
	PORT_START("LINE0")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("LINE1")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("LINE2")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("LINE3")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("LINE4")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("LINE5")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE6")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("LINE7")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0xA4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("LINE8")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("LINE9")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE10")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("LINE11")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("LINE12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER( special_state::specimx_floppy_formats )
	FLOPPY_SMX_FORMAT
FLOPPY_FORMATS_END

static void specimx_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

/* Machine driver */
void special_state::special(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &special_state::specialist_mem);

	MCFG_MACHINE_RESET_OVERRIDE(special_state, special )

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(384, 256);
	screen.set_visarea(0, 384-1, 0, 256-1);
	screen.set_screen_update(FUNC(special_state::screen_update_special));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.0625);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* Devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(special_state::specialist_8255_porta_r));
	m_ppi->out_pa_callback().set(FUNC(special_state::specialist_8255_porta_w));
	m_ppi->in_pb_callback().set(FUNC(special_state::specialist_8255_portb_r));
	m_ppi->out_pb_callback().set(FUNC(special_state::specialist_8255_portb_w));
	m_ppi->in_pc_callback().set(FUNC(special_state::specialist_8255_portc_r));
	m_ppi->out_pc_callback().set(FUNC(special_state::specialist_8255_portc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(rks_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cassette->set_interface("special_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("special_cass");
}

void special_state::specialp(machine_config &config)
{
	special(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &special_state::specialp_mem);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_screen_update(FUNC(special_state::screen_update_specialp));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
}

void special_state::specialm(machine_config &config)
{
	special(config);
	m_ppi->in_pa_callback().set(FUNC(special_state::specialist_8255_porta_r));
	m_ppi->out_pa_callback().set(FUNC(special_state::specialist_8255_porta_w));
	m_ppi->in_pb_callback().set(FUNC(special_state::specimx_8255_portb_r));
	m_ppi->out_pb_callback().set(FUNC(special_state::specialist_8255_portb_w));
	m_ppi->in_pc_callback().set(FUNC(special_state::specialist_8255_portc_r));
	m_ppi->out_pc_callback().set(FUNC(special_state::specialist_8255_portc_w));
}

void special_state::specimx(machine_config &config)
{
	special(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &special_state::specimx_mem);

	MCFG_MACHINE_START_OVERRIDE (special_state, specimx )
	MCFG_MACHINE_RESET_OVERRIDE (special_state, specimx )

	/* video hardware */
	subdevice<screen_device>("screen")->set_screen_update(FUNC(special_state::screen_update_specimx));

	MCFG_VIDEO_START_OVERRIDE(special_state,specimx)

	m_palette->set_init(FUNC(special_state::specimx_palette));
	m_palette->set_entries(16);

	/* audio hardware */
	SPECIMX_SND(config, "custom", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);

	/* Devices */
	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(2000000);
	m_pit->out_handler<0>().set("custom", FUNC(specimx_sound_device::set_input_ch0));
	m_pit->set_clk<1>(2000000);
	m_pit->out_handler<1>().set("custom", FUNC(specimx_sound_device::set_input_ch1));
	m_pit->set_clk<2>(2000000);
	m_pit->out_handler<2>().set("custom", FUNC(specimx_sound_device::set_input_ch2));

	m_ppi->in_pa_callback().set(FUNC(special_state::specialist_8255_porta_r));
	m_ppi->out_pa_callback().set(FUNC(special_state::specialist_8255_porta_w));
	m_ppi->in_pb_callback().set(FUNC(special_state::specimx_8255_portb_r));
	m_ppi->out_pb_callback().set(FUNC(special_state::specialist_8255_portb_w));
	m_ppi->in_pc_callback().set(FUNC(special_state::specialist_8255_portc_r));
	m_ppi->out_pc_callback().set(FUNC(special_state::specialist_8255_portc_w));

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->drq_wr_callback().set(FUNC(special_state::fdc_drq));
	FLOPPY_CONNECTOR(config, "fd0", specimx_floppies, "525qd", special_state::specimx_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd1", specimx_floppies, "525qd", special_state::specimx_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("special_flop");

	/* internal ram */
	RAM(config, m_ram).set_default_size("128K").set_default_value(0x00);
}

void special_state::erik(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &special_state::erik_mem);
	m_maincpu->set_addrmap(AS_IO, &special_state::erik_io_map);

	MCFG_MACHINE_RESET_OVERRIDE(special_state, erik )

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(384, 256);
	screen.set_visarea(0, 384-1, 0, 256-1);
	screen.set_screen_update(FUNC(special_state::screen_update_erik));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(special_state::erik_palette), 8);

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.0625);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* Devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(rks_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cassette->set_interface("special_cass");

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(special_state::specialist_8255_porta_r));
	m_ppi->out_pa_callback().set(FUNC(special_state::specialist_8255_porta_w));
	m_ppi->in_pb_callback().set(FUNC(special_state::specialist_8255_portb_r));
	m_ppi->out_pb_callback().set(FUNC(special_state::specialist_8255_portb_w));
	m_ppi->in_pc_callback().set(FUNC(special_state::specialist_8255_portc_r));
	m_ppi->out_pc_callback().set(FUNC(special_state::specialist_8255_portc_w));

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->drq_wr_callback().set(FUNC(special_state::fdc_drq));
	FLOPPY_CONNECTOR(config, "fd0", specimx_floppies, "525qd", special_state::specimx_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd1", specimx_floppies, "525qd", special_state::specimx_floppy_formats);

	/* internal ram */
	RAM(config, m_ram).set_default_size("192K").set_default_value(0x00);
}

/* ROM definition */
ROM_START( special )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "2nd", "2nd rev.")
	ROMX_LOAD( "monitor2_1.rom", 0xc000, 0x0800, CRC(52abde77) SHA1(66ba2ef9eac14a5c0df510224ea25fd5745399cd), ROM_BIOS(0))
	ROMX_LOAD( "monitor2_2.rom", 0xc800, 0x0800, CRC(c425f719) SHA1(1c322591b4e5c8b01b81362c6801aa6fd9fc1492), ROM_BIOS(0))
	ROMX_LOAD( "monitor2_3.rom", 0xd000, 0x0800, CRC(d804aeba) SHA1(1585f354719c25e1f59c7cb8b3a3f5d309a7e8fb), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2rom", "2nd rev. rom disk")
	ROMX_LOAD( "monitor2_1.rom",  0xc000, 0x0800, CRC(52abde77) SHA1(66ba2ef9eac14a5c0df510224ea25fd5745399cd), ROM_BIOS(1))
	ROMX_LOAD( "m2_rom-disk.rom", 0xc800, 0x0800, CRC(7bd3d476) SHA1(232341755ae794f8aab4f6181c8d499a66016af2), ROM_BIOS(1))
	ROMX_LOAD( "monitor2_3.rom",  0xd000, 0x0800, CRC(d804aeba) SHA1(1585f354719c25e1f59c7cb8b3a3f5d309a7e8fb), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "1b", "1st rev. + BASIC")
	ROMX_LOAD( "root.rom", 0xc000, 0x0800, CRC(62de741d) SHA1(6c6a29d4340b0b1230c708f9c04ff2ed1c012a76), ROM_BIOS(2))
	ROMX_LOAD( "pzu2.rom", 0xc800, 0x0800, CRC(49937e13) SHA1(872ae5a7c3496d4404cdd577caa2236424016c66), ROM_BIOS(2))
	ROMX_LOAD( "pzu3.rom", 0xd000, 0x0800, CRC(dc817a08) SHA1(8101fe924386f38c10ef929e6dea5a83fcf34600), ROM_BIOS(2))
	ROMX_LOAD( "pzu4.rom", 0xd800, 0x0800, CRC(6793ba23) SHA1(3a27b5dbc6561ea7af5fb30513dc83ec64f1d94c), ROM_BIOS(2))
	ROMX_LOAD( "pzu5.rom", 0xe000, 0x0800, CRC(13a1a0dc) SHA1(3a0818cb8f36c2c5a9b9916669538ad1702a7710), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "1st", "1st rev.")
	ROMX_LOAD( "special1.rom", 0xc000, 0x1000, CRC(217414bd) SHA1(345cd1410fbca8f75421d12d1419f27f81cd35d6), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "2col", "2nd rev. color")
	ROMX_LOAD( "col_mon2.rom",   0xc000, 0x0800, CRC(8cebb1b5) SHA1(0c912a25220de8c5135e16c443e4796e6bb6f805), ROM_BIOS(4))
	ROMX_LOAD( "monitor2_2.rom", 0xc800, 0x0800, CRC(c425f719) SHA1(1c322591b4e5c8b01b81362c6801aa6fd9fc1492), ROM_BIOS(4))
	ROMX_LOAD( "monitor2_3.rom", 0xd000, 0x0800, CRC(d804aeba) SHA1(1585f354719c25e1f59c7cb8b3a3f5d309a7e8fb), ROM_BIOS(4))
ROM_END

ROM_START( specialm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pzu1-m.rom", 0xc000, 0x0800, CRC(61e94485) SHA1(dbe212dcd377cb8cd16000516cd4b3b760ce779e))
	ROM_LOAD( "pzu2-m.rom", 0xc800, 0x0800, CRC(83d76815) SHA1(9d139eaa6ca6b241fa9fac94ede72abb56d83674))
	ROM_LOAD( "pzu3-m.rom", 0xd000, 0x0800, CRC(2121ad65) SHA1(b43811d058d818f8c1cee59b405b515f96958656))
	ROM_LOAD( "pzu4-m.rom", 0xd800, 0x0800, CRC(8bf8218e) SHA1(bd1174c384b04a40dfcaa35aa373c5070304f37b))
ROM_END

ROM_START( specialp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "special6.rom", 0xc000, 0x1000, CRC(f0c5a0ac) SHA1(50b53bd7c05117930aa84653a9ea0fc0c6f0f496) )
ROM_END

ROM_START( lik )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1st", "1st rev.")
	ROMX_LOAD( "lik.rom",   0xc000, 0x3000, CRC(705bb3a0) SHA1(f90b009ec9d3303bbda228714dd24de057e744b6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2nd", "2nd rev.")
	ROMX_LOAD( "lik2.rom",  0xc000, 0x3000, CRC(71820e43) SHA1(a85b4fc33b1ea96a1b8fe0c791f1aab8e967bb44), ROM_BIOS(1))
ROM_END

ROM_START( specimx )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "fos", "ROM FOS")
	ROMX_LOAD( "specimx.rom", 0x10000, 0xb800,  CRC(db68f9b1) SHA1(c79888449f8a605267ec3e10dcc8e6e6f43b3a95), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "nc", "NC")
	ROMX_LOAD( "ncrdy.rom",   0x10000, 0x10000, CRC(5d04c522) SHA1(d7daa7fe14cd8e0c6f87fd6453ec3e94ea2c259f) ,ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ramfos", "RAMFOS")
	ROMX_LOAD( "ramfos.rom",  0x10000, 0x3000, CRC(83e19df4) SHA1(20e5e53eb45729a24c1c7c63e114dbd14e3c4184) ,ROM_BIOS(2))
ROM_END

ROM_START( erik )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "erik.bin", 0x10000, 0x10000, CRC(6f3208f4) SHA1(41f6e2763ef60d3c7214c98893e580d25346fa2d))
ROM_END

ROM_START( pioner )
	// Ukranian clone with 16KB RAM
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pioner.rf2", 0xc000, 0x0800, CRC(d6250ab2) SHA1(b953517d883c64857e63139fed52436f77d371cb))
ROM_END

/* Driver */

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS          INIT          COMPANY      FULLNAME                    FLAGS
COMP( 1985, special,  0,       0,      special,  special,  special_state, init_special, "<unknown>", "Specialist",               0 )
COMP( 1985, specialm, special, 0,      specialm, special,  special_state, init_special, "<unknown>", "Specialist M",             0 )
COMP( 1985, pioner,   special, 0,      special,  special,  special_state, init_special, "<unknown>", "Pioner",                   MACHINE_NOT_WORKING )
COMP( 1985, specialp, special, 0,      specialp, specialp, special_state, init_special, "<unknown>", "Specialist + hires graph", MACHINE_NOT_WORKING )
COMP( 1985, lik,      special, 0,      special,  lik,      special_state, init_special, "<unknown>", "Lik",                      0 )
COMP( 1985, specimx,  special, 0,      specimx,  specimx,  special_state, empty_init,   "<unknown>", "Specialist MX",            0 )
COMP( 1994, erik,     special, 0,      erik,     special,  special_state, init_erik,    "<unknown>", "Erik",                     0 )
