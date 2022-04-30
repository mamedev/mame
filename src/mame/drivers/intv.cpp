// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/************************************************************************
 *  Mattel Intellivision + Keyboard Component Drivers
 *
 *  Frank Palazzolo
 *  Kyle Davis
 *
 *  TBD:
 *          Add tape support (intvkbd)
 *          Add runtime tape loading
 *          Fix memory system workaround
 *            (memory handler stuff in CP1610, debugger, and shared mem)
 *          STIC
 *            reenable dirty support
 *          Cleanup
 *            Separate stic & video better, get rid of *2 for kbd comp
 *          Add better runtime cart loading
 *          Switch to tilemap system
 * Note from kevtris about IntelliVoice Hookup:
<kevtris> the intv uses a special chip
<kevtris> called the SPB640
<kevtris> it is really cool and lame at the same time
<kevtris> it holds 64 10 bit words, which it simply serializes and sends to the SP0256
<kevtris> the SP0256-012 just blindly jumps to 02000h or something when you try to play one of the samples
<kevtris> the SPB640 sits at 2000h and when it sees that address come up, it will send out the 640 bits to the '256
<kevtris> this means you cannot use any gotos/gosubs in your speech data, but that is OK
<kevtris> it's just a single serial stream.  the intv simply watches the buffer state and refills it periodically.  there's enough buffer to keep it full for 1 frame
<kevtris> that's about it
<kevtris> the samples are stored in the game ROMs, and are easy to extract

9502, 9503 and 9504 are the chips used in the intv1 (the sears one may use different chips)
9505 is used in some carts
9503 and 9506 are the chips used in the intv2

Known roms so far:
RO-3-9502-011 = 4KiB(2Kiw) self-decoding mask rom with decoder & bus, (DIP40 containing 1/2 of EXEC, maps at words 0000-0fff or 1000-1fff [unclear which, guess is 0000], i/o maps at ?), located at U5 on intv1
RO-3-9503-003 = 2KiB(1Kiw) self-decoding mask rom with decoder & bus, (DIP40 containing GROM, maps at words 0x3000-0x37FF, i/o maps at ?), located at U21 in intv1, U5 in intv2
RO-3-9504-021 = 4KiB(2Kiw) self-decoding mask rom without decoder or bus, (DIP28 containing 1/2 of EXEC, maps at words 0000-0fff or 1000-1fff [unclear which, guess is 1000]), located at U6 in intv1
RO-3-9506-010 = 8KiB(4Kiw)+512B(256W) self-decoding mask rom with decoder & bus, (DIP40 containing EXEC2, maps at words 0000-1fff and 0400-04FF, i/o maps at ?) located at U6 in intv2

rom types (suspected, not proven yet):
RO-3-9502 = 4KiB (2Kiw) self decoding address mask rom with external address decoder & bus (DIP40)
RO-3-9503 = 2KiB (1Kiw) self decoding address mask rom with external address decoder & bus (DIP40)
RO-3-9504 = 4KiB (2Kiw) self decoding address mask rom without external address decoder or bus (DIP28)
RO-3-9505 = 8KiB (4Kiw) self decoding address mask rom without external address decoder or bus (DIP28)
RO-3-9506 = 8KiB (4Kiw) self decoding address mask rom with external address decoder & bus (DIP40)

 *
 ************************************************************************/


#include "emu.h"
#include "includes/intv.h"

#include "cpu/m6502/m6502.h"
#include "cpu/cp1610/cp1610.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


#ifndef VERBOSE
#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif
#endif

static constexpr rgb_t intv_colors[] =
{
	{ 0x00, 0x00, 0x00 }, // BLACK
	{ 0x00, 0x2d, 0xff }, // BLUE
	{ 0xff, 0x3d, 0x10 }, // RED
	{ 0xc9, 0xcf, 0xab }, // TAN
	{ 0x38, 0x6b, 0x3f }, // DARK GREEN
	{ 0x00, 0xa7, 0x56 }, // GREEN
	{ 0xfa, 0xea, 0x50 }, // YELLOW
	{ 0xff, 0xfc, 0xff }, // WHITE
	{ 0xbd, 0xac, 0xc8 }, // GRAY
	{ 0x24, 0xb8, 0xff }, // CYAN
	{ 0xff, 0xb4, 0x1f }, // ORANGE
	{ 0x54, 0x6e, 0x00 }, // BROWN
	{ 0xff, 0x4e, 0x57 }, // PINK
	{ 0xa4, 0x96, 0xff }, // LIGHT BLUE
	{ 0x75, 0xcc, 0x80 }, // YELLOW GREEN
	{ 0xb5, 0x1a, 0x58 }  // PURPLE
};

void intv_state::intv_palette(palette_device &palette) const
{
	int k = 0;
	// Two copies of everything (why?)

	for (int i = 0; i < 16; i++)
	{
		palette.set_indirect_color(i, intv_colors[i]);
		palette.set_indirect_color(i + 16, intv_colors[i]);
	}

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			palette.set_pen_indirect(k++, i);
			palette.set_pen_indirect(k++, j);
		}
	}

	for (int i = 0; i < 16; i++)
	{
		for (int j = 16; j < 32; j++)
		{
			palette.set_pen_indirect(k++, i);
			palette.set_pen_indirect(k++, j);
		}
	}
}

/* graphics output */

static const gfx_layout intvkbd_charlayout =
{
	8, 8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8
};

static GFXDECODE_START( gfx_intvkbd )
	GFXDECODE_ENTRY( "gfx1", 0x0000, intvkbd_charlayout, 0, 256 )
GFXDECODE_END


/*
        Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0

 Row 0  NC      NC      NC      NC      NC      NC      CTRL    SHIFT
 Row 1  NC      NC      NC      NC      NC      NC      RPT     LOCK
 Row 2  NC      /       ,       N       V       X       NC      SPC
 Row 3  (right) .       M       B       C       Z       NC      CLS
 Row 4  (down)  ;       K       H       F       S       NC      TAB
 Row 5  ]       P       I       Y       R       W       NC      Q
 Row 6  (up)    -       9       7       5       3       NC      1
 Row 7  =       0       8       6       4       2       NC      [
 Row 8  (return)(left)  O       U       T       E       NC      ESC
 Row 9  DEL     '       L       J       G       D       NC      A

2008-05 FP:
The keyboard layout is quite strange, with '[' and ']' at the two ends of the 1st row,
'Esc' in the 2nd row (between 'Tab' and 'Q'), and with Cursor keys and 'Enter' where
you would expect the braces. Moreover, Shift + Cursor keys produce characters.
The emulated layout moves 'Esc', '[' and ']' to their usual position.

Moreover, a small note on natural keyboard support: currently
- "Clear Screen" is mapped to 'Left Alt'
- "Repeat" is mapped to 'F1'
- "Lock" is mapped to 'F2'
*/

static INPUT_PORTS_START( intvkbd )
	PORT_START("ROW0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("ROW1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REPEAT") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("ROW2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)       PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)       PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)       PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)   PORT_CHAR(' ')

	PORT_START("ROW3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('.')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)       PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)       PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)       PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)       PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear Screen") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("ROW4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR('\\')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)   PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)       PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)       PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)       PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)       PORT_CHAR('S')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)     PORT_CHAR('\t')

	PORT_START("ROW5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)       PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)       PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)       PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)       PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)       PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)       PORT_CHAR('Q')

	PORT_START("ROW6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)  PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)   PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)       PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR(0xA2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') // this one would be 1st row, 1st key (at 'Esc' position)

	PORT_START("ROW8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)       PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)       PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)       PORT_CHAR('T')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)       PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)     PORT_CHAR(UCHAR_MAMEKEY(ESC)) // this one would be 2nd row, 2nd key (between 'Tab' and 'Q')

	PORT_START("ROW9")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)   PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)       PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)       PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)       PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)       PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)       PORT_CHAR('A')

	PORT_START("TEST")  /* For tape drive testing... */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
INPUT_PORTS_END

void intv_state::intv_mem(address_map &map)
{
	map(0x0000, 0x003f).rw(FUNC(intv_state::intv_stic_r), FUNC(intv_state::intv_stic_w));
	map(0x0100, 0x01ef).rw(FUNC(intv_state::intv_ram8_r), FUNC(intv_state::intv_ram8_w));
	map(0x01f0, 0x01ff).rw(m_sound, FUNC(ay8914_device::read), FUNC(ay8914_device::write)).umask16(0x00ff);
	map(0x0200, 0x035f).rw(FUNC(intv_state::intv_ram16_r), FUNC(intv_state::intv_ram16_w));
	map(0x0400, 0x04ff).r(m_cart, FUNC(intv_cart_slot_device::read_rom04));
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000 << 1);   // Exec ROM, 10-bits wide
	map(0x2000, 0x2fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom20));
	map(0x3000, 0x37ff).r(m_stic, FUNC(stic_device::grom_read)); // GROM,     8-bits wide
	map(0x3800, 0x39ff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));     // GRAM,     8-bits wide
	map(0x3a00, 0x3bff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));     // GRAM Alias, 8-bits wide
	map(0x4000, 0x47ff).r(m_cart, FUNC(intv_cart_slot_device::read_rom40));
	map(0x4800, 0x4fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom48));
	map(0x5000, 0x5fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom50));
	map(0x6000, 0x6fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom60));
	map(0x7000, 0x7fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom70));
	map(0x8000, 0x8fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom80));
	map(0x9000, 0x9fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom90));
	map(0xa000, 0xafff).r(m_cart, FUNC(intv_cart_slot_device::read_roma0));
	map(0xb000, 0xbfff).r(m_cart, FUNC(intv_cart_slot_device::read_romb0));
	map(0xc000, 0xcfff).r(m_cart, FUNC(intv_cart_slot_device::read_romc0));
	map(0xd000, 0xdfff).r(m_cart, FUNC(intv_cart_slot_device::read_romd0));
	map(0xe000, 0xefff).r(m_cart, FUNC(intv_cart_slot_device::read_rome0));
	map(0xf000, 0xffff).r(m_cart, FUNC(intv_cart_slot_device::read_romf0));
}

void intv_state::intvoice_mem(address_map &map)
{
	map(0x0000, 0x003f).rw(FUNC(intv_state::intv_stic_r), FUNC(intv_state::intv_stic_w));
	map(0x0080, 0x0081).rw("voice", FUNC(intv_voice_device::read_speech), FUNC(intv_voice_device::write_speech)); // Intellivoice
	map(0x0100, 0x01ef).rw(FUNC(intv_state::intv_ram8_r), FUNC(intv_state::intv_ram8_w));
	map(0x01f0, 0x01ff).rw(m_sound, FUNC(ay8914_device::read), FUNC(ay8914_device::write)).umask16(0x00ff);
	map(0x0200, 0x035f).rw(FUNC(intv_state::intv_ram16_r), FUNC(intv_state::intv_ram16_w));
	map(0x0400, 0x04ff).r("voice", FUNC(intv_voice_device::read_rom04));
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000 << 1);   // Exec ROM, 10-bits wide
	map(0x2000, 0x2fff).r("voice", FUNC(intv_voice_device::read_rom20));
	map(0x3000, 0x37ff).r(m_stic, FUNC(stic_device::grom_read)); // GROM,     8-bits wide
	map(0x3800, 0x39ff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));     // GRAM,     8-bits wide
	map(0x3a00, 0x3bff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));     // GRAM Alias, 8-bits wide
	map(0x4000, 0x47ff).r("voice", FUNC(intv_voice_device::read_rom40));
	map(0x4800, 0x4fff).r("voice", FUNC(intv_voice_device::read_rom48));
	map(0x5000, 0x5fff).r("voice", FUNC(intv_voice_device::read_rom50));
	map(0x6000, 0x6fff).r("voice", FUNC(intv_voice_device::read_rom60));
	map(0x7000, 0x7fff).r("voice", FUNC(intv_voice_device::read_rom70));
	map(0x8000, 0x8fff).r("voice", FUNC(intv_voice_device::read_rom80));
	map(0x9000, 0x9fff).r("voice", FUNC(intv_voice_device::read_rom90));
	map(0xa000, 0xafff).r("voice", FUNC(intv_voice_device::read_roma0));
	map(0xb000, 0xbfff).r("voice", FUNC(intv_voice_device::read_romb0));
	map(0xc000, 0xcfff).r("voice", FUNC(intv_voice_device::read_romc0));
	map(0xd000, 0xdfff).r("voice", FUNC(intv_voice_device::read_romd0));
	map(0xe000, 0xefff).r("voice", FUNC(intv_voice_device::read_rome0));
	map(0xf000, 0xffff).r("voice", FUNC(intv_voice_device::read_romf0));
}

void intv_state::intv2_mem(address_map &map)
{
	map(0x0000, 0x003f).rw(FUNC(intv_state::intv_stic_r), FUNC(intv_state::intv_stic_w));
	map(0x0100, 0x01ef).rw(FUNC(intv_state::intv_ram8_r), FUNC(intv_state::intv_ram8_w));
	map(0x01f0, 0x01ff).rw(m_sound, FUNC(ay8914_device::read), FUNC(ay8914_device::write)).umask16(0x00ff);
	map(0x0200, 0x035f).rw(FUNC(intv_state::intv_ram16_r), FUNC(intv_state::intv_ram16_w));
	map(0x0400, 0x04ff).rom().region("maincpu", 0x400 << 1);    // Exec ROM, 10-bits wide
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000 << 1);   // Exec ROM, 10-bits wide
	map(0x2000, 0x2fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom20));
	map(0x3000, 0x37ff).r(m_stic, FUNC(stic_device::grom_read)); // GROM,     8-bits wide
	map(0x3800, 0x39ff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w)); // GRAM,     8-bits wide
	map(0x3a00, 0x3bff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w)); // GRAM Alias, 8-bits wide
	map(0x4000, 0x47ff).r(m_cart, FUNC(intv_cart_slot_device::read_rom40));
	map(0x4800, 0x4fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom48));
	map(0x5000, 0x5fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom50));
	map(0x6000, 0x6fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom60));
	map(0x7000, 0x7fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom70));
	map(0x8000, 0x8fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom80));
	map(0x9000, 0x9fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom90));
	map(0xa000, 0xafff).r(m_cart, FUNC(intv_cart_slot_device::read_roma0));
	map(0xb000, 0xbfff).r(m_cart, FUNC(intv_cart_slot_device::read_romb0));
	map(0xc000, 0xcfff).r(m_cart, FUNC(intv_cart_slot_device::read_romc0));
	map(0xd000, 0xdfff).r(m_cart, FUNC(intv_cart_slot_device::read_romd0));
	map(0xe000, 0xefff).r(m_cart, FUNC(intv_cart_slot_device::read_rome0));
	map(0xf000, 0xffff).r(m_cart, FUNC(intv_cart_slot_device::read_romf0));
}

void intv_state::intvecs_mem(address_map &map)
{
	map(0x0000, 0x003f).rw(FUNC(intv_state::intv_stic_r), FUNC(intv_state::intv_stic_w));
	map(0x0080, 0x0081).rw("speech", FUNC(sp0256_device::spb640_r), FUNC(sp0256_device::spb640_w)); /* Intellivoice */
	// map(0x00e0, 0x00e3).rw(FUNC(intv_state::intv_ecs_uart_r), FUNC(intv_state::intv_ecs_uart_w));
	map(0x00f0, 0x00ff).rw("ecs", FUNC(intv_ecs_device::read_ay), FUNC(intv_ecs_device::write_ay)); /* ecs psg */
	map(0x0100, 0x01ef).rw(FUNC(intv_state::intv_ram8_r), FUNC(intv_state::intv_ram8_w));
	map(0x01f0, 0x01ff).rw(m_sound, FUNC(ay8914_device::read), FUNC(ay8914_device::write)).umask16(0x00ff);
	map(0x0200, 0x035f).rw(FUNC(intv_state::intv_ram16_r), FUNC(intv_state::intv_ram16_w));
	map(0x0400, 0x04ff).r("ecs", FUNC(intv_ecs_device::read_rom04));
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000<<1); /* Exec ROM, 10-bits wide */
	map(0x2000, 0x2fff).rw("ecs", FUNC(intv_ecs_device::read_rom20), FUNC(intv_ecs_device::write_rom20));
	map(0x3000, 0x37ff).r(m_stic, FUNC(stic_device::grom_read)); /* GROM,     8-bits wide */
	map(0x3800, 0x39ff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));       /* GRAM,     8-bits wide */
	map(0x3a00, 0x3bff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));       /* GRAM Alias,     8-bits wide */
	map(0x4000, 0x47ff).rw("ecs", FUNC(intv_ecs_device::read_ram), FUNC(intv_ecs_device::write_ram));
	map(0x4800, 0x4fff).r("ecs", FUNC(intv_ecs_device::read_rom48));
	map(0x5000, 0x5fff).r("ecs", FUNC(intv_ecs_device::read_rom50));
	map(0x6000, 0x6fff).r("ecs", FUNC(intv_ecs_device::read_rom60));
	map(0x7000, 0x7fff).rw("ecs", FUNC(intv_ecs_device::read_rom70), FUNC(intv_ecs_device::write_rom70));
	map(0x8000, 0x8fff).r("ecs", FUNC(intv_ecs_device::read_rom80));
	map(0x9000, 0x9fff).r("ecs", FUNC(intv_ecs_device::read_rom90));
	map(0xa000, 0xafff).r("ecs", FUNC(intv_ecs_device::read_roma0));
	map(0xb000, 0xbfff).r("ecs", FUNC(intv_ecs_device::read_romb0));
	map(0xc000, 0xcfff).r("ecs", FUNC(intv_ecs_device::read_romc0));
	map(0xd000, 0xdfff).r("ecs", FUNC(intv_ecs_device::read_romd0));
	map(0xe000, 0xefff).rw("ecs", FUNC(intv_ecs_device::read_rome0), FUNC(intv_ecs_device::write_rome0));
	map(0xf000, 0xffff).rw("ecs", FUNC(intv_ecs_device::read_romf0), FUNC(intv_ecs_device::write_romf0));
}

void intv_state::intvkbd_mem(address_map &map)
{
	map(0x0000, 0x003f).rw(FUNC(intv_state::intv_stic_r), FUNC(intv_state::intv_stic_w));
	map(0x0100, 0x01ef).rw(FUNC(intv_state::intv_ram8_r), FUNC(intv_state::intv_ram8_w));
	map(0x01f0, 0x01ff).rw(m_sound, FUNC(ay8914_device::read), FUNC(ay8914_device::write)).umask16(0x00ff);
	map(0x0200, 0x035f).rw(FUNC(intv_state::intv_ram16_r), FUNC(intv_state::intv_ram16_w));
	map(0x0400, 0x04ff).r(m_cart, FUNC(intv_cart_slot_device::read_rom04));
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000<<1); /* Exec ROM, 10-bits wide */
	map(0x2000, 0x2fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom20));
	map(0x3000, 0x37ff).r(m_stic, FUNC(stic_device::grom_read)); /* GROM,     8-bits wide */
	map(0x3800, 0x39ff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));       /* GRAM,     8-bits wide */
	map(0x3a00, 0x3bff).rw(FUNC(intv_state::intv_gram_r), FUNC(intv_state::intv_gram_w));       /* GRAM Alias,     8-bits wide */
	map(0x4000, 0x47ff).r(m_cart, FUNC(intv_cart_slot_device::read_rom40));
	map(0x4800, 0x4fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom48));
	map(0x5000, 0x5fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom50));
	map(0x6000, 0x6fff).r(m_cart, FUNC(intv_cart_slot_device::read_rom60));
	map(0x7000, 0x7fff).rom().region("maincpu", 0x7000<<1); /* Keyboard ROM */
	map(0x8000, 0xbfff).ram().w(FUNC(intv_state::intvkbd_dualport16_w)).share("dualport_ram");  /* Dual-port RAM */
	map(0xc000, 0xcfff).r(m_cart, FUNC(intv_cart_slot_device::read_romc0));
	map(0xd000, 0xdfff).r(m_cart, FUNC(intv_cart_slot_device::read_romd0));
	map(0xe000, 0xefff).r(m_cart, FUNC(intv_cart_slot_device::read_rome0));
	map(0xf000, 0xffff).r(m_cart, FUNC(intv_cart_slot_device::read_romf0));
}

void intv_state::intvkbd2_mem(address_map &map)
{
	map.unmap_value_high();  /* Required because of probing */
	map(0x0000, 0x3fff).rw(FUNC(intv_state::intvkbd_dualport8_lsb_r), FUNC(intv_state::intvkbd_dualport8_lsb_w));  /* Dual-port RAM */
	map(0x4000, 0x40bf).rw(FUNC(intv_state::intvkbd_io_r), FUNC(intv_state::intvkbd_io_w));
	map(0x40c0, 0x40cf).rw(m_crtc, FUNC(tms9927_device::read), FUNC(tms9927_device::write));
	map(0x4200, 0x7fff).rw(FUNC(intv_state::intvkbd_dualport8_msb_r), FUNC(intv_state::intvkbd_dualport8_msb_w));  /* Dual-port RAM */
	map(0xb7f8, 0xb7ff).rw(FUNC(intv_state::intvkbd_periph_r), FUNC(intv_state::intvkbd_periph_w));
	map(0xb800, 0xbfff).ram().share("videoram"); /* Text Display */
	map(0xc000, 0xdfff).rom();
	map(0xe000, 0xffff).r(FUNC(intv_state::intvkb_iocart_r));
}

void intv_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_INTV_INTERRUPT2_COMPLETE:
		intv_interrupt2_complete(param);
		break;
	case TIMER_INTV_INTERRUPT_COMPLETE:
		intv_interrupt_complete(param);
		break;
	case TIMER_INTV_BTB_FILL:
		intv_btb_fill(param);
		break;
	default:
		throw emu_fatalerror("Unknown id in intv_state::device_timer");
	}
}


/* This is needed because MAME core does not allow PULSE_LINE.
    The time interval is not critical, although it should be below 1000. */

TIMER_CALLBACK_MEMBER(intv_state::intv_interrupt2_complete)
{
	m_keyboard->set_input_line(0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(intv_state::intv_interrupt2)
{
	m_keyboard->set_input_line(0, ASSERT_LINE);
	timer_set(m_keyboard->cycles_to_attotime(100), TIMER_INTV_INTERRUPT2_COMPLETE);
}

void intv_state::intv(machine_config &config)
{
	/* basic machine hardware */
	cp1610_cpu_device &maincpu(CP1610(config, m_maincpu, XTAL(3'579'545)/4));        /* Colorburst/4 */
	maincpu.set_addrmap(AS_PROGRAM, &intv_state::intv_mem);
	maincpu.set_vblank_int("screen", FUNC(intv_state::intv_interrupt));
	maincpu.iab().set(FUNC(intv_state::iab_r));
	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	STIC(config, m_stic, XTAL(3'579'545));
	m_stic->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.92);
	//screen.set_vblank_time(ATTOSECONDS_IN_USEC(2400)); /* not accurate */
	screen.set_screen_update(FUNC(intv_state::screen_update_intv));
	screen.set_size(stic_device::SCREEN_WIDTH*INTV_X_SCALE, stic_device::SCREEN_HEIGHT*INTV_Y_SCALE);
	screen.set_visarea(0, stic_device::SCREEN_WIDTH*INTV_X_SCALE-1, 0, stic_device::SCREEN_HEIGHT*INTV_Y_SCALE-1);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(intv_state::intv_palette), 0x400, 32);

	INTV_CONTROL_PORT(config, "iopt_right_ctrl", intv_control_port_devices, "handctrl");
	INTV_CONTROL_PORT(config, "iopt_left_ctrl", intv_control_port_devices, "handctrl");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8914(config, m_sound, XTAL(3'579'545)/2);
	m_sound->port_a_read_callback().set("iopt_right_ctrl", FUNC(intv_control_port_device::ctrl_r));
	m_sound->port_b_read_callback().set("iopt_left_ctrl", FUNC(intv_control_port_device::ctrl_r));
	m_sound->add_route(ALL_OUTPUTS, "mono", 0.33);

	/* cartridge */
	INTV_CART_SLOT(config, m_cart, intv_cart, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("intv");
	SOFTWARE_LIST(config, "ecs_list").set_compatible("intvecs");
}

void intv_state::intv2(machine_config &config)
{
	intv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &intv_state::intv2_mem);
}

void intv_state::intvoice(machine_config &config)
{
	intv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &intv_state::intvoice_mem);

	config.device_remove("cartslot");
	INTV_ROM_VOICE(config, "voice", 0);
}

void intv_state::intvecs(machine_config &config)
{
	intv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &intv_state::intvecs_mem);

	config.device_remove("cartslot");
	INTV_ROM_ECS(config, "ecs", 0);

	sp0256_device &speech(SP0256(config, "speech", 3120000));
	/* The Intellivoice uses a speaker with its own volume control so the relative volumes to use are subjective */
	speech.add_route(ALL_OUTPUTS, "mono", 1.00);

	/* cassette */
	//CASSETTE(config, "cassette");

	/* software lists */
	config.device_remove("ecs_list");
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("intvecs");
	SOFTWARE_LIST(config, "intv_list").set_compatible("intv");
}

void intv_state::intvkbd(machine_config &config)
{
	intv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &intv_state::intvkbd_mem);

	M6502(config, m_keyboard, XTAL(7'159'090)/8);
	m_keyboard->set_addrmap(AS_PROGRAM, &intv_state::intvkbd2_mem);
	m_keyboard->set_vblank_int("screen", FUNC(intv_state::intv_interrupt2));

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_intvkbd);

	/* crt controller */
	TMS9927(config, m_crtc, XTAL(7'159'090)/8);
	m_crtc->set_char_width(8);
	m_crtc->set_overscan(
		stic_device::OVERSCAN_LEFT_WIDTH*stic_device::X_SCALE*INTVKBD_X_SCALE,
		stic_device::OVERSCAN_RIGHT_WIDTH*stic_device::X_SCALE*INTVKBD_X_SCALE,
		stic_device::OVERSCAN_TOP_HEIGHT*stic_device::Y_SCALE*INTVKBD_Y_SCALE,
		stic_device::OVERSCAN_BOTTOM_HEIGHT*stic_device::Y_SCALE*INTVKBD_Y_SCALE);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(intv_state::screen_update_intvkbd));

	/* I/O cartslots for BASIC */
	GENERIC_CARTSLOT(config, m_iocart1, generic_plain_slot, "intbasic_cart");
	GENERIC_CARTSLOT(config, m_iocart2, generic_plain_slot, "intbasic_cart");
}

ROM_START(intv) // the intv1 exec rom should be two roms: RO-3-9502-011.U5 and RO-3-9504-021.U6
	ROM_REGION(0x10000<<1,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD( "exec.bin", (0x1000<<1)+0, 0x2000, CRC(cbce86f7) SHA1(5a65b922b562cb1f57dab51b73151283f0e20c7a))
ROM_END

#define rom_intvoice rom_intv

// the later intellivision 2's exec rom is a single ro-3-9506-010 at location ic6 holding 8k plus 512 bytes; the 1st 512 bytes are at 0x400 and the 8k at 0x1000
ROM_START(intv2)
	ROM_REGION(0x10000<<1,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ro-3-9506-010.ic6", (0x400<<1)+0, 0x200, CRC(dd7e1237) SHA1(fb821a643b7714ed4c812553cd3f668766fd44ab))
	ROM_CONTINUE( (0x1000<<1)+0, 0x2000 )
ROM_END

ROM_START(intvsrs) // the intv1 sears exec rom should be two roms: RO-3-9502-???.U5 and RO-3-9504-???.U6 but the correct names are unknown as of yet
	ROM_REGION(0x10000<<1,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD( "searsexc.bin", (0x1000<<1)+0, 0x2000, CRC(ea552a22) SHA1(834339de056d42a35571cae7fd5b04d1344001e9))
ROM_END

ROM_START(intvecs) // the intv1 exec rom should be two roms: RO-3-9502-011.U5 and RO-3-9504-021.U6
	ROM_REGION(0x10000<<1,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD( "exec.bin", (0x1000<<1)+0, 0x2000, CRC(cbce86f7) SHA1(5a65b922b562cb1f57dab51b73151283f0e20c7a))

	ROM_REGION( 0x10000<<1, "speech", 0 )
	/* SP0256-012 Speech chip w/2KiB mask rom */
	ROM_LOAD( "sp0256-012.bin",   0x1000, 0x0800, CRC(0de7579d) SHA1(618563e512ff5665183664f52270fa9606c9d289) )
ROM_END

/*
Intellivision Keyboard Component - Prototype
-------------------------------------------------------------

GI    9333B-0104         0104.U20  4Kx8  Mask ROM, 6502 code
Intel 2732 "CPU 2D"     CPU2D.U21  4Kx8  EPROM, 6502 code

TI    8S030N  1149-0360  0360.U58  32x8  Timing prom?
TI    8S030N  1149-0370  0370.U74  32x8  Timing prom?

GI    RO-3-9502-024       024.U60  2Kx10 Mask ROM+Addr Decoder, CP1600 code
GI    9316B-4D72         4D72.U62  2Kx8  Mask ROM, CP1600 code (upper)
GI    9316B-4D71         4D71.U63  2Kx8  Mask ROM, CP1600 code (lower)
GI    9316B-4C52         4C52.U34  2Kx8  Mask ROM, Alphanumerics

Main board also includes:

    2  2114 DRAMS        1Kx4  Character memory
    10 MM5290J DRAMS    16Kx10 CP1600 memory?
    1  6502
    1  Mystery 40-pin chip (under heat sink)
       (actually a SMC CRT5027 aka TI TMS9927 CRT controller)
*/

ROM_START(intvkbd) // the intv1 exec rom should be two roms: RO-3-9502-011.U5 and RO-3-9504-021.U6
	ROM_REGION(0x10000<<1,"maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD( "exec.bin", 0x1000<<1, 0x2000, CRC(cbce86f7) SHA1(5a65b922b562cb1f57dab51b73151283f0e20c7a))
	ROM_LOAD16_WORD( "024.u60",  0x7000<<1, 0x1000, CRC(4f7998ec) SHA1(ec006d0ae9002e9d56d83a71f5f2eddd6a456a40))
	ROM_LOAD16_BYTE( "4d72.u62", 0x7800<<1, 0x0800, CRC(aa57c594) SHA1(741860d489d90f5882ca53daa3169b6abacdf130))
	ROM_LOAD16_BYTE( "4d71.u63", (0x7800<<1)+1, 0x0800, CRC(069b2f0b) SHA1(070850bb32f8474107cc52c5183cfaa32d640f9a))

	ROM_REGION(0x10000,"keyboard",0)
	ROM_LOAD( "0104.u20",  0xc000, 0x1000, CRC(5c6f1256) SHA1(271931fb354dfae6a1a5697ee888924a89a15ca8))
	ROM_RELOAD( 0xe000, 0x1000 )
	ROM_LOAD("cpu2d.u21",  0xd000, 0x1000, CRC(2c2dba33) SHA1(0db5d177fec3f8ae89abeef2e6900ad4f3460266))
	ROM_RELOAD( 0xf000, 0x1000 )

	ROM_REGION(0x00800,"gfx1",0)
	ROM_LOAD( "4c52.u34",  0x0000, 0x0800, CRC(cbeb2e96) SHA1(f0e17adcd278fb376c9f90833c7fbbb60193dbe3))

	ROM_REGION(0x0100,"proms",0)
	ROM_LOAD( "0360.u58", 0x00, 0x20, CRC(1295528a) SHA1(b35e598891f1185e02cbacb4811d2334357abd79))
	ROM_LOAD( "0370.u74", 0x20, 0x20, CRC(19da5096) SHA1(76af50e4fd29649fc4837120c245321a8fc84cd3))
ROM_END

void intv_state::init_intv()
{
	m_stic->set_x_scale(INTV_X_SCALE);
	m_stic->set_y_scale(INTV_Y_SCALE);
	m_is_keybd = 0;
}

void intv_state::init_intvkbd()
{
	m_stic->set_x_scale(INTVKBD_X_SCALE);
	m_stic->set_y_scale(INTVKBD_Y_SCALE);
	m_is_keybd = 1;
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS       INIT          COMPANY               FULLNAME, FLAGS */
CONS( 1979, intv,     0,      0,      intv,     0,       intv_state, init_intv,    "Mattel Electronics", "Intellivision", MACHINE_SUPPORTS_SAVE )
CONS( 1981, intvsrs,  intv,   0,      intv,     0,       intv_state, init_intv,    "Sears",              "Super Video Arcade", MACHINE_SUPPORTS_SAVE )
COMP( 1981, intvkbd,  intv,   0,      intvkbd,  intvkbd, intv_state, init_intvkbd, "Mattel Electronics", "Intellivision Keyboard Component (Unreleased)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
CONS( 1982, intv2,    intv,   0,      intv2,    0,       intv_state, init_intv,    "Mattel Electronics", "Intellivision II", MACHINE_SUPPORTS_SAVE )

// made up, user friendlier machines with pre-mounted passthu expansions
COMP( 1982, intvoice, intv,   0,      intvoice, 0,       intv_state, init_intv,    "Mattel Electronics", "Intellivision w/IntelliVoice expansion", MACHINE_SUPPORTS_SAVE )
COMP( 1983, intvecs,  intv,   0,      intvecs,  0,       intv_state, init_intv,    "Mattel Electronics", "Intellivision w/Entertainment Computer System + Intellivoice expansions", MACHINE_SUPPORTS_SAVE )
