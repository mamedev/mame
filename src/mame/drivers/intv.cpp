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
#include "cpu/m6502/m6502.h"
#include "cpu/cp1610/cp1610.h"
#include "includes/intv.h"
#include "sound/ay8910.h"
#include "softlist.h"

#ifndef VERBOSE
#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif
#endif

static const unsigned char intv_colors[] =
{
	0x00, 0x00, 0x00, /* BLACK */
	0x00, 0x2D, 0xFF, /* BLUE */
	0xFF, 0x3D, 0x10, /* RED */
	0xC9, 0xCF, 0xAB, /* TAN */
	0x38, 0x6B, 0x3F, /* DARK GREEN */
	0x00, 0xA7, 0x56, /* GREEN */
	0xFA, 0xEA, 0x50, /* YELLOW */
	0xFF, 0xFC, 0xFF, /* WHITE */
	0xBD, 0xAC, 0xC8, /* GRAY */
	0x24, 0xB8, 0xFF, /* CYAN */
	0xFF, 0xB4, 0x1F, /* ORANGE */
	0x54, 0x6E, 0x00, /* BROWN */
	0xFF, 0x4E, 0x57, /* PINK */
	0xA4, 0x96, 0xFF, /* LIGHT BLUE */
	0x75, 0xCC, 0x80, /* YELLOW GREEN */
	0xB5, 0x1A, 0x58  /* PURPLE */
};

PALETTE_INIT_MEMBER(intv_state, intv)
{
	int k = 0;
	UINT8 r, g, b;
	/* Two copies of everything (why?) */

	for (int i = 0; i < 16; i++)
	{
		r = intv_colors[i * 3 + 0];
		g = intv_colors[i * 3 + 1];
		b = intv_colors[i * 3 + 2];
		palette.set_indirect_color(i, rgb_t(r, g, b));
		palette.set_indirect_color(i + 16, rgb_t(r, g, b));
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

static GFXDECODE_START( intvkbd )
	GFXDECODE_ENTRY( "gfx1", 0x0000, intvkbd_charlayout, 0, 256 )
GFXDECODE_END

static INPUT_PORTS_START( intv )

	/* Left Player Controller */
	PORT_START("KEYPAD1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/Clear") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left/Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left/Upper") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Left/Lower-Left") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Left/Lower-Right") PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DISC1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Left/Up") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Up-Up-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Up-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Right-Up-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Left/Right") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Right-Down-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Down-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Down-Down-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Left/Down") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Down-Down-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Down-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Left-Down-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Left/Left") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Left-Up-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Up-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left/Up-Up-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)

	PORT_START("DISCX1")
	PORT_BIT( 0xff, 0x50, IPT_AD_STICK_X ) PORT_NAME("Left/X") PORT_MINMAX(0x00,0x9f) PORT_SENSITIVITY(100) PORT_KEYDELTA(0x50) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x01)

	PORT_START("DISCY1")
	PORT_BIT( 0xff, 0x50, IPT_AD_STICK_Y ) PORT_NAME("Left/Y") PORT_MINMAX(0x00,0x9f) PORT_SENSITIVITY(100) PORT_KEYDELTA(0x50) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x01)

	/* Right Player Controller */
	PORT_START("KEYPAD2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/1")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/2")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/3")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/4")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/5")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/6")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/7")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/8")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/9")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/Clear")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/0")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right/Enter")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Right/Upper") PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right/Lower-Left") PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Right/Lower-Right") PORT_PLAYER(2)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DISC2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Right/Up") PORT_PLAYER(2) PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Up-Up-Right") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Up-Right") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Right-Up-Right") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right/Right") PORT_PLAYER(2) PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Right-Down-Right") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Down-Right") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Down-Down-Right") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Right/Down") PORT_PLAYER(2) PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Down-Down-Left") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Down-Left") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Left-Down-Left") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Right/Left") PORT_PLAYER(2) PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Left-Up-Left") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Up-Left") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right/Up-Up-Left") PORT_CONDITION("OPTIONS",0x02,EQUALS,0x00)

	PORT_START("DISCX2")
	PORT_BIT( 0xff, 0x50, IPT_AD_STICK_X ) PORT_NAME("Right/X") PORT_MINMAX(0x00,0x9f) PORT_SENSITIVITY(100) PORT_KEYDELTA(0x50) PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_G) PORT_PLAYER(2) PORT_CONDITION("OPTIONS",0x02,EQUALS,0x02)

	PORT_START("DISCY2")
	PORT_BIT( 0xff, 0x50, IPT_AD_STICK_Y ) PORT_NAME("Right/Y") PORT_MINMAX(0x00,0x9f) PORT_SENSITIVITY(100) PORT_KEYDELTA(0x50) PORT_CODE_DEC(KEYCODE_R) PORT_CODE_INC(KEYCODE_F) PORT_PLAYER(2) PORT_CONDITION("OPTIONS",0x02,EQUALS,0x02)

	PORT_START("OPTIONS")
	PORT_CONFNAME( 0x01, 0x00, "Left Disc" )
	PORT_CONFSETTING(    0x00, "Digital" )
	PORT_CONFSETTING(    0x01, "Analog" )
	PORT_CONFNAME( 0x02, 0x00, "Right Disc" )
	PORT_CONFSETTING(    0x00, "Digital" )
	PORT_CONFSETTING(    0x02, "Analog" )
INPUT_PORTS_END


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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)   PORT_CHAR('_') PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)       PORT_CHAR('O') PORT_CHAR(')')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('\xA2')
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

	/* 2008-05 FP: I include here the controller inputs to make happy the read_handler.
	Please remove this (and re-tag accordingly the inputs above) if intv_right_control_r
	is supposed to scan the keyboard inputs when the Keyboard Component is connected */
	PORT_INCLUDE( intv )
INPUT_PORTS_END

static ADDRESS_MAP_START( intv_mem, AS_PROGRAM, 16, intv_state )
	AM_RANGE(0x0000, 0x003f) AM_READWRITE(intv_stic_r, intv_stic_w)
	AM_RANGE(0x0100, 0x01ef) AM_READWRITE(intv_ram8_r, intv_ram8_w)
	AM_RANGE(0x01f0, 0x01ff) AM_DEVREADWRITE8("ay8914", ay8914_device, read, write, 0x00ff)
	AM_RANGE(0x0200, 0x035f) AM_READWRITE(intv_ram16_r, intv_ram16_w)
	AM_RANGE(0x0400, 0x04ff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom04)
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("maincpu", 0x1000 << 1)   // Exec ROM, 10-bits wide
	AM_RANGE(0x2000, 0x2fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom20)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREAD("stic", stic_device, grom_read) // GROM,     8-bits wide
	AM_RANGE(0x3800, 0x39ff) AM_READWRITE(intv_gram_r, intv_gram_w)     // GRAM,     8-bits wide
	AM_RANGE(0x3a00, 0x3bff) AM_READWRITE(intv_gram_r, intv_gram_w)     // GRAM Alias, 8-bits wide
	AM_RANGE(0x4000, 0x47ff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom40)
	AM_RANGE(0x4800, 0x4fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom48)
	AM_RANGE(0x5000, 0x5fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom50)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom60)
	AM_RANGE(0x7000, 0x7fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom70)
	AM_RANGE(0x8000, 0x8fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom80)
	AM_RANGE(0x9000, 0x9fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom90)
	AM_RANGE(0xa000, 0xafff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_roma0)
	AM_RANGE(0xb000, 0xbfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romb0)
	AM_RANGE(0xc000, 0xcfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romc0)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romd0)
	AM_RANGE(0xe000, 0xefff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rome0)
	AM_RANGE(0xf000, 0xffff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romf0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( intvoice_mem, AS_PROGRAM, 16, intv_state )
	AM_RANGE(0x0000, 0x003f) AM_READWRITE(intv_stic_r, intv_stic_w)
	AM_RANGE(0x0080, 0x0081) AM_DEVREADWRITE("voice", intv_voice_device, read_speech, write_speech) // Intellivoice
	AM_RANGE(0x0100, 0x01ef) AM_READWRITE(intv_ram8_r, intv_ram8_w)
	AM_RANGE(0x01f0, 0x01ff) AM_DEVREADWRITE8("ay8914", ay8914_device, read, write, 0x00ff)
	AM_RANGE(0x0200, 0x035f) AM_READWRITE(intv_ram16_r, intv_ram16_w)
	AM_RANGE(0x0400, 0x04ff) AM_DEVREAD("voice", intv_voice_device, read_rom04)
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("maincpu", 0x1000 << 1)   // Exec ROM, 10-bits wide
	AM_RANGE(0x2000, 0x2fff) AM_DEVREAD("voice", intv_voice_device, read_rom20)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREAD("stic", stic_device, grom_read) // GROM,     8-bits wide
	AM_RANGE(0x3800, 0x39ff) AM_READWRITE(intv_gram_r, intv_gram_w)     // GRAM,     8-bits wide
	AM_RANGE(0x3a00, 0x3bff) AM_READWRITE(intv_gram_r, intv_gram_w)     // GRAM Alias, 8-bits wide
	AM_RANGE(0x4000, 0x47ff) AM_DEVREAD("voice", intv_voice_device, read_rom40)
	AM_RANGE(0x4800, 0x4fff) AM_DEVREAD("voice", intv_voice_device, read_rom48)
	AM_RANGE(0x5000, 0x5fff) AM_DEVREAD("voice", intv_voice_device, read_rom50)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREAD("voice", intv_voice_device, read_rom60)
	AM_RANGE(0x7000, 0x7fff) AM_DEVREAD("voice", intv_voice_device, read_rom70)
	AM_RANGE(0x8000, 0x8fff) AM_DEVREAD("voice", intv_voice_device, read_rom80)
	AM_RANGE(0x9000, 0x9fff) AM_DEVREAD("voice", intv_voice_device, read_rom90)
	AM_RANGE(0xa000, 0xafff) AM_DEVREAD("voice", intv_voice_device, read_roma0)
	AM_RANGE(0xb000, 0xbfff) AM_DEVREAD("voice", intv_voice_device, read_romb0)
	AM_RANGE(0xc000, 0xcfff) AM_DEVREAD("voice", intv_voice_device, read_romc0)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREAD("voice", intv_voice_device, read_romd0)
	AM_RANGE(0xe000, 0xefff) AM_DEVREAD("voice", intv_voice_device, read_rome0)
	AM_RANGE(0xf000, 0xffff) AM_DEVREAD("voice", intv_voice_device, read_romf0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( intv2_mem , AS_PROGRAM, 16, intv_state )
	AM_RANGE(0x0000, 0x003f) AM_READWRITE(intv_stic_r, intv_stic_w)
	AM_RANGE(0x0100, 0x01ef) AM_READWRITE(intv_ram8_r, intv_ram8_w)
	AM_RANGE(0x01f0, 0x01ff) AM_DEVREADWRITE8("ay8914", ay8914_device, read, write, 0x00ff)
	AM_RANGE(0x0200, 0x035f) AM_READWRITE(intv_ram16_r, intv_ram16_w)
	AM_RANGE(0x0400, 0x04ff) AM_ROM AM_REGION("maincpu", 0x400 << 1)    // Exec ROM, 10-bits wide
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("maincpu", 0x1000 << 1)   // Exec ROM, 10-bits wide
	AM_RANGE(0x2000, 0x2fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom20)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREAD("stic", stic_device, grom_read) // GROM,     8-bits wide
	AM_RANGE(0x3800, 0x39ff) AM_READWRITE(intv_gram_r, intv_gram_w) // GRAM,     8-bits wide
	AM_RANGE(0x3a00, 0x3bff) AM_READWRITE(intv_gram_r, intv_gram_w) // GRAM Alias, 8-bits wide
	AM_RANGE(0x4000, 0x47ff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom40)
	AM_RANGE(0x4800, 0x4fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom48)
	AM_RANGE(0x5000, 0x5fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom50)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom60)
	AM_RANGE(0x7000, 0x7fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom70)
	AM_RANGE(0x8000, 0x8fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom80)
	AM_RANGE(0x9000, 0x9fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom90)
	AM_RANGE(0xa000, 0xafff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_roma0)
	AM_RANGE(0xb000, 0xbfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romb0)
	AM_RANGE(0xc000, 0xcfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romc0)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romd0)
	AM_RANGE(0xe000, 0xefff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rome0)
	AM_RANGE(0xf000, 0xffff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romf0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( intvecs_mem , AS_PROGRAM, 16, intv_state )
	AM_RANGE(0x0000, 0x003f) AM_READWRITE(intv_stic_r, intv_stic_w)
	AM_RANGE(0x0080, 0x0081) AM_DEVREADWRITE("speech", sp0256_device, spb640_r, spb640_w) /* Intellivoice */
	// AM_RANGE(0x00E0, 0x00E3) AM_READWRITE( intv_ecs_uart_r, intv_ecs_uart_w )
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE("ecs", intv_ecs_device, read_ay, write_ay) /* ecs psg */
	AM_RANGE(0x0100, 0x01ef) AM_READWRITE(intv_ram8_r, intv_ram8_w)
	AM_RANGE(0x01f0, 0x01ff) AM_DEVREADWRITE8("ay8914", ay8914_device, read, write, 0x00ff)
	AM_RANGE(0x0200, 0x035f) AM_READWRITE(intv_ram16_r, intv_ram16_w)
	AM_RANGE(0x0400, 0x04ff) AM_DEVREAD("ecs", intv_ecs_device, read_rom04)
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("maincpu", 0x1000<<1) /* Exec ROM, 10-bits wide */
	AM_RANGE(0x2000, 0x2fff) AM_DEVREADWRITE("ecs", intv_ecs_device, read_rom20, write_rom20)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREAD("stic", stic_device, grom_read) /* GROM,     8-bits wide */
	AM_RANGE(0x3800, 0x39ff) AM_READWRITE(intv_gram_r, intv_gram_w)       /* GRAM,     8-bits wide */
	AM_RANGE(0x3a00, 0x3bff) AM_READWRITE(intv_gram_r, intv_gram_w)       /* GRAM Alias,     8-bits wide */
	AM_RANGE(0x4000, 0x47ff) AM_DEVREADWRITE("ecs", intv_ecs_device, read_ram, write_ram)
	AM_RANGE(0x4800, 0x4fff) AM_DEVREAD("ecs", intv_ecs_device, read_rom48)
	AM_RANGE(0x5000, 0x5fff) AM_DEVREAD("ecs", intv_ecs_device, read_rom50)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREAD("ecs", intv_ecs_device, read_rom60)
	AM_RANGE(0x7000, 0x7fff) AM_DEVREADWRITE("ecs", intv_ecs_device, read_rom70, write_rom70)
	AM_RANGE(0x8000, 0x8fff) AM_DEVREAD("ecs", intv_ecs_device, read_rom80)
	AM_RANGE(0x9000, 0x9fff) AM_DEVREAD("ecs", intv_ecs_device, read_rom90)
	AM_RANGE(0xa000, 0xafff) AM_DEVREAD("ecs", intv_ecs_device, read_roma0)
	AM_RANGE(0xb000, 0xbfff) AM_DEVREAD("ecs", intv_ecs_device, read_romb0)
	AM_RANGE(0xc000, 0xcfff) AM_DEVREAD("ecs", intv_ecs_device, read_romc0)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREAD("ecs", intv_ecs_device, read_romd0)
	AM_RANGE(0xe000, 0xefff) AM_DEVREADWRITE("ecs", intv_ecs_device, read_rome0, write_rome0)
	AM_RANGE(0xf000, 0xffff) AM_DEVREADWRITE("ecs", intv_ecs_device, read_romf0, write_romf0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( intvkbd_mem , AS_PROGRAM, 16, intv_state )
	AM_RANGE(0x0000, 0x003f) AM_READWRITE(intv_stic_r, intv_stic_w)
	AM_RANGE(0x0100, 0x01ef) AM_READWRITE(intv_ram8_r, intv_ram8_w)
	AM_RANGE(0x01f0, 0x01ff) AM_DEVREADWRITE8("ay8914", ay8914_device, read, write, 0x00ff)
	AM_RANGE(0x0200, 0x035f) AM_READWRITE(intv_ram16_r, intv_ram16_w)
	AM_RANGE(0x0400, 0x04ff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom04)
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("maincpu", 0x1000<<1) /* Exec ROM, 10-bits wide */
	AM_RANGE(0x2000, 0x2fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom20)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREAD("stic", stic_device, grom_read) /* GROM,     8-bits wide */
	AM_RANGE(0x3800, 0x39ff) AM_READWRITE(intv_gram_r, intv_gram_w)       /* GRAM,     8-bits wide */
	AM_RANGE(0x3a00, 0x3bff) AM_READWRITE(intv_gram_r, intv_gram_w)       /* GRAM Alias,     8-bits wide */
	AM_RANGE(0x4000, 0x47ff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom40)
	AM_RANGE(0x4800, 0x4fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom48)
	AM_RANGE(0x5000, 0x5fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom50)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rom60)
	AM_RANGE(0x7000, 0x7fff) AM_ROM AM_REGION("maincpu", 0x7000<<1) /* Keyboard ROM */
	AM_RANGE(0x8000, 0xbfff) AM_RAM_WRITE(intvkbd_dualport16_w) AM_SHARE("dualport_ram")  /* Dual-port RAM */
	AM_RANGE(0xc000, 0xcfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romc0)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romd0)
	AM_RANGE(0xe000, 0xefff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_rome0)
	AM_RANGE(0xf000, 0xffff) AM_DEVREAD("cartslot", intv_cart_slot_device, read_romf0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( intvkbd2_mem , AS_PROGRAM, 8, intv_state )
	ADDRESS_MAP_UNMAP_HIGH  /* Required because of probing */
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(intvkbd_dualport8_lsb_r, intvkbd_dualport8_lsb_w)  /* Dual-port RAM */
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE(intvkbd_dualport8_msb_r, intvkbd_dualport8_msb_w)  /* Dual-port RAM */
	AM_RANGE(0xb7f8, 0xb7ff) AM_RAM    /* ??? */
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_SHARE("videoram") /* Text Display */
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_READ(intvkb_iocart_r)
ADDRESS_MAP_END

void intv_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_INTV_INTERRUPT2_COMPLETE:
		intv_interrupt2_complete(ptr, param);
		break;
	case TIMER_INTV_INTERRUPT_COMPLETE:
		intv_interrupt_complete(ptr, param);
		break;
	case TIMER_INTV_BTB_FILL:
		intv_btb_fill(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in intv_state::device_timer");
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


static MACHINE_CONFIG_START( intv, intv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CP1610, XTAL_3_579545MHz/4)        /* Colorburst/4 */
	MCFG_CPU_PROGRAM_MAP(intv_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", intv_state,  intv_interrupt)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_STIC_ADD("stic")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.92)
	//MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2400)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(intv_state, screen_update_intv)
	MCFG_SCREEN_SIZE((STIC_OVERSCAN_LEFT_WIDTH+STIC_BACKTAB_WIDTH*STIC_CARD_WIDTH-1+STIC_OVERSCAN_RIGHT_WIDTH)*STIC_X_SCALE*INTV_X_SCALE, (STIC_OVERSCAN_TOP_HEIGHT+STIC_BACKTAB_HEIGHT*STIC_CARD_HEIGHT+STIC_OVERSCAN_BOTTOM_HEIGHT)*STIC_Y_SCALE*INTV_Y_SCALE)
	MCFG_SCREEN_VISIBLE_AREA(0, (STIC_OVERSCAN_LEFT_WIDTH+STIC_BACKTAB_WIDTH*STIC_CARD_WIDTH-1+STIC_OVERSCAN_RIGHT_WIDTH)*STIC_X_SCALE*INTV_X_SCALE-1, 0, (STIC_OVERSCAN_TOP_HEIGHT+STIC_BACKTAB_HEIGHT*STIC_CARD_HEIGHT+STIC_OVERSCAN_BOTTOM_HEIGHT)*STIC_Y_SCALE*INTV_Y_SCALE-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(intv_state, intv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8914", AY8914, XTAL_3_579545MHz/2)
	MCFG_AY8910_PORT_A_READ_CB(READ8(intv_state, intv_right_control_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(intv_state, intv_left_control_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	/* cartridge */
	MCFG_INTV_CARTRIDGE_ADD("cartslot", intv_cart, nullptr)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "intv")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("ecs_list", "intvecs")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( intv2, intv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(intv2_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( intvoice, intv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(intvoice_mem)

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_DEVICE_ADD("voice", INTV_ROM_VOICE, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( intvecs, intv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(intvecs_mem)

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_DEVICE_ADD("ecs", INTV_ROM_ECS, 0)

	MCFG_SOUND_ADD("speech", SP0256, 3120000)
	/* The Intellivoice uses a speaker with its own volume control so the relative volumes to use are subjective */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* cassette */
	//MCFG_CASSETTE_ADD( "cassette" )

	/* software lists */
	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_DEVICE_REMOVE("ecs_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "intvecs")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("intv_list", "intv")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( intvkbd, intv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(intvkbd_mem)

	MCFG_CPU_ADD("keyboard", M6502, XTAL_3_579545MHz/2) /* Colorburst/2 */
	MCFG_CPU_PROGRAM_MAP(intvkbd2_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", intv_state,  intv_interrupt2)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", intvkbd)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(intv_state, intv)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE((STIC_OVERSCAN_LEFT_WIDTH+STIC_BACKTAB_WIDTH*STIC_CARD_WIDTH-1+STIC_OVERSCAN_RIGHT_WIDTH)*STIC_X_SCALE*INTVKBD_X_SCALE, (STIC_OVERSCAN_TOP_HEIGHT+STIC_BACKTAB_HEIGHT*STIC_CARD_HEIGHT+STIC_OVERSCAN_BOTTOM_HEIGHT)*STIC_Y_SCALE*INTVKBD_Y_SCALE)
	MCFG_SCREEN_VISIBLE_AREA(0, (STIC_OVERSCAN_LEFT_WIDTH+STIC_BACKTAB_WIDTH*STIC_CARD_WIDTH-1+STIC_OVERSCAN_RIGHT_WIDTH)*STIC_X_SCALE*INTVKBD_X_SCALE-1, 0, (STIC_OVERSCAN_TOP_HEIGHT+STIC_BACKTAB_HEIGHT*STIC_CARD_HEIGHT+STIC_OVERSCAN_BOTTOM_HEIGHT)*STIC_Y_SCALE*INTVKBD_Y_SCALE-1)
	MCFG_SCREEN_UPDATE_DRIVER(intv_state, screen_update_intvkbd)

	/* I/O cartslots for BASIC */
	MCFG_GENERIC_CARTSLOT_ADD("ioslot1", generic_plain_slot, "intbasic_cart")
	MCFG_GENERIC_CARTSLOT_ADD("ioslot2", generic_plain_slot, "intbasic_cart")
MACHINE_CONFIG_END

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

DRIVER_INIT_MEMBER(intv_state,intv)
{
	m_stic->set_x_scale(INTV_X_SCALE);
	m_stic->set_y_scale(INTV_Y_SCALE);
	m_is_keybd = 0;
}

DRIVER_INIT_MEMBER(intv_state,intvkbd)
{
	m_stic->set_x_scale(INTVKBD_X_SCALE);
	m_stic->set_y_scale(INTVKBD_Y_SCALE);
	m_is_keybd = 1;
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY     FULLNAME */
CONS( 1979, intv,       0,      0,      intv,       intv,       intv_state,    intv,       "Mattel", "Intellivision", MACHINE_SUPPORTS_SAVE )
CONS( 1981, intvsrs,    intv,   0,      intv,       intv,       intv_state,    intv,       "Sears",  "Super Video Arcade", MACHINE_SUPPORTS_SAVE )
COMP( 1981, intvkbd,    intv,   0,      intvkbd,    intvkbd,    intv_state,    intvkbd,    "Mattel", "Intellivision Keyboard Component (Unreleased)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
CONS( 1982, intv2,      intv,   0,      intv2,      intv,       intv_state,    intv,       "Mattel", "Intellivision II", MACHINE_SUPPORTS_SAVE )

// made up, user friendlier machines with pre-mounted passthu expansions
COMP( 1982, intvoice,   intv,   0,      intvoice,   intv,       intv_state,    intv,       "Mattel", "Intellivision w/IntelliVoice expansion", MACHINE_SUPPORTS_SAVE )
COMP( 1983, intvecs,    intv,   0,      intvecs,    intv,       intv_state,    intv,       "Mattel", "Intellivision w/Entertainment Computer System + Intellivoice expansions", MACHINE_SUPPORTS_SAVE )
