/***************************************************************************
HAD to change the PORT_ANALOG defs in this file...  please check ;-)

Colour Genie memory map

CPU #1:
0000-3fff ROM basic & bios        R   D0-D7

4000-bfff RAM
c000-dfff ROM dos                 R   D0-D7
e000-efff ROM extra               R   D0-D7
f000-f3ff color ram               W/R D0-D3
f400-f7ff font ram                W/R D0-D7
f800-f8ff keyboard matrix         R   D0-D7
ffe0-ffe3 floppy motor            W   D0-D2
          floppy head select      W   D3
ffec-ffef FDC WD179x              R/W D0-D7
          ffec command            W
          ffec status             R
          ffed track              R/W
          ffee sector             R/W
          ffef data               R/W

Interrupts:
IRQ mode 1
NMI
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/cgenie.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "formats/cgen_cas.h"
#include "machine/ram.h"

static ADDRESS_MAP_START (cgenie_mem, AS_PROGRAM, 8, cgenie_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
//  AM_RANGE(0x4000, 0xbfff) AM_RAM // set up in MACHINE_START
//  AM_RANGE(0xc000, 0xdfff) AM_ROM // installed in cgenie_init_machine
//  AM_RANGE(0xe000, 0xefff) AM_ROM // installed in cgenie_init_machine
	AM_RANGE(0xf000, 0xf3ff) AM_READWRITE(cgenie_colorram_r, cgenie_colorram_w ) AM_SHARE("colorram")
	AM_RANGE(0xf400, 0xf7ff) AM_READWRITE(cgenie_fontram_r, cgenie_fontram_w) AM_SHARE("fontram")
	AM_RANGE(0xf800, 0xf8ff) AM_READ(cgenie_keyboard_r )
	AM_RANGE(0xf900, 0xffdf) AM_NOP
	AM_RANGE(0xffe0, 0xffe3) AM_READWRITE(cgenie_irq_status_r, cgenie_motor_w )
	AM_RANGE(0xffe4, 0xffeb) AM_NOP
	AM_RANGE(0xffec, 0xffec) AM_READWRITE(cgenie_status_r, cgenie_command_w )
	AM_RANGE(0xffe4, 0xffeb) AM_NOP
	AM_RANGE(0xffec, 0xffec) AM_WRITE(cgenie_command_w )
	AM_RANGE(0xffed, 0xffed) AM_READWRITE(cgenie_track_r, cgenie_track_w )
	AM_RANGE(0xffee, 0xffee) AM_READWRITE(cgenie_sector_r, cgenie_sector_w )
	AM_RANGE(0xffef, 0xffef) AM_READWRITE(cgenie_data_r, cgenie_data_w )
	AM_RANGE(0xfff0, 0xffff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START (cgenie_io, AS_IO, 8, cgenie_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(cgenie_sh_control_port_r, cgenie_sh_control_port_w )
	AM_RANGE(0xf9, 0xf9) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, data_w)
	AM_RANGE(0xfa, 0xfa) AM_READWRITE(cgenie_index_r, cgenie_index_w )
	AM_RANGE(0xfb, 0xfb) AM_READWRITE(cgenie_register_r, cgenie_register_w )
	AM_RANGE(0xff, 0xff) AM_READWRITE(cgenie_port_ff_r, cgenie_port_ff_w )
ADDRESS_MAP_END

static INPUT_PORTS_START( cgenie )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "Floppy Disc Drives")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "CG-DOS ROM C000-DFFF")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Extension  E000-EFFF")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Video Display accuracy") PORT_CODE(KEYCODE_F5) PORT_TOGGLE
	PORT_DIPSETTING(    0x10, "TV set" )
	PORT_DIPSETTING(    0x00, "RGB monitor" )
	PORT_BIT(0x0f, 0x0f, IPT_UNUSED)

/**************************************************************************
   +-------------------------------+     +-------------------------------+
   | 0   1   2   3   4   5   6   7 |     | 0   1   2   3   4   5   6   7 |
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
|0 | @ | A | B | C | D | E | F | G |  |0 | ` | a | b | c | d | e | f | g |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|1 | H | I | J | K | L | M | N | O |  |1 | h | i | j | k | l | m | n | o |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|2 | P | Q | R | S | T | U | V | W |  |2 | p | q | r | s | t | u | v | w |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|3 | X | Y | Z | [ |F-1|F-2|F-3|F-4|  |3 | x | y | z | { |F-5|F-6|F-7|F-8|
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |  |4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|5 | 8 | 9 | : | ; | , | - | . | / |  |5 | 8 | 9 | * | + | < | = | > | ? |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|  |6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|7 |SHF|ALT|PUP|PDN|INS|DEL|CTL|END|  |7 |SHF|ALT|PUP|PDN|INS|DEL|CTL|END|
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
***************************************************************************/

	PORT_START("ROW0")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("ROW1")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("ROW2")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("ROW3")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("[  { (Not Connected)") PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F5))
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F6))
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)            PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F7))
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)            PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("ROW4")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("ROW5")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('-') PORT_CHAR('=')
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("ROW6")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)         PORT_CHAR(13)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(UCHAR_MAMEKEY(F10))
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)            PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)          PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)          PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)         PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')

	/* 2008-05 FP: Below we still miss a 'Lock' key, two 'Rst' keys (used in pair, they should restart the
	system) and, I guess, two unused inputs (according to the user manual, there are no other keys on the
	keyboard)  */
	PORT_START("ROW7")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Mod Sel") PORT_CODE(KEYCODE_LALT)  PORT_CHAR(UCHAR_MAMEKEY(F9))
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Unknown 1")
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Rpt") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(F11))
		PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(F12))
		PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Unknown 2")
		PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Unknown 3")
		PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Unknown 4")

	PORT_START("JOY0")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_MINMAX(0x00,0xcf ) PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_MINMAX(0x00,0xcf ) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("JOY2")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_MINMAX(0x00,0xcf ) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_MINMAX(0x00,0xcf ) PORT_PLAYER(2) PORT_REVERSE

	/* Joystick Keypad */
	/* keypads were organized a 3 x 4 matrix and it looked     */
	/* exactly like a our northern telephone numerical pads    */
	/* The addressing was done with a single clear bit 0..6    */
	/* on i/o port A,  while all other bits were set.          */
	/* (e.g. 0xFE addresses keypad1 row 0, 0xEF keypad2 row 1) */

	/*       bit  0   1   2   3   */
	/* FE/F7  0  [3] [6] [9] [#]  */
	/* FD/EF  1  [2] [5] [8] [0]  */
	/* FB/DF  2  [1] [4] [7] [*]  */

	/* 2008-05 FP: temporarily I mapped these as follows:
	    - Joy 1 at Keypad
	    - Joy 2 at a Joystick buttons
	A better mapping would be needed...
	*/

	PORT_START("KP0")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [3]") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [6]") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [9]") PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [#]") PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("KP1")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [2]") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [5]") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [8]") PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [0]") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KP2")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [1]") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [4]") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [7]") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 1 [*]") PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("KP3")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [3]") PORT_CODE(JOYCODE_BUTTON2)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [6]") PORT_CODE(JOYCODE_BUTTON5)
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [9]")
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [#]") PORT_CODE(JOYCODE_BUTTON1)

	PORT_START("KP4")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [2]") PORT_CODE(JOYCODE_BUTTON2)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [5]") PORT_CODE(JOYCODE_BUTTON5)
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [8]")
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [0]")

	PORT_START("KP5")
		PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [1]") PORT_CODE(JOYCODE_BUTTON1)
		PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [4]") PORT_CODE(JOYCODE_BUTTON4)
		PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [7]")
		PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Joy 2 [*]") PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END

static const gfx_layout cgenie_charlayout =
{
	8,8,           /* 8*8 characters */
	384,           /* 256 fixed + 128 defineable characters */
	1,             /* 1 bits per pixel */
	{ 0 },         /* no bitplanes; 1 bit per pixel */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },   /* x offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8            /* every char takes 8 bytes */
};

static const gfx_layout cgenie_gfxlayout =
{
	8,8,            /* 4*8 characters */
	256,            /* 256 graphics patterns */
	2,              /* 2 bits per pixel */
	{ 0, 1 },       /* two bitplanes; 2 bit per pixel */
	{ 0, 0, 2, 2, 4, 4, 6, 6}, /* x offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8             /* every char takes 8 bytes */
};

static GFXDECODE_START( cgenie )
	GFXDECODE_ENTRY( "gfx1", 0, cgenie_charlayout, 0, 3*16 )
	GFXDECODE_ENTRY( "gfx2", 0, cgenie_gfxlayout, 3*16*2, 3*4 )
GFXDECODE_END

static const unsigned char cgenie_colors[] = {
	0x00,0x00,0x00,  /* background   */
	0x5e,0x5e,0x5e,  /* gray         */
	0x11,0xff,0xea,  /* cyan         */
	0xff,0x00,0x5e,  /* red          */
	0xea,0xea,0xea,  /* white        */
	0xff,0xf9,0x00,  /* yellow       */
	0x6e,0xff,0x00,  /* green        */
	0xff,0x52,0x00,  /* orange       */
	0xea,0xff,0x00,  /* light yellow */
	0x02,0x48,0xff,  /* blue         */
	0x8e,0xd4,0xff,  /* light blue   */
	0xff,0x12,0xff,  /* pink         */
	0x88,0x43,0xff,  /* purple       */
	0x8c,0x8c,0x8c,  /* light gray   */
	0x00,0xfb,0x8c,  /* turquoise    */
	0xd2,0x00,0xff,  /* magenta      */
	0xff,0xff,0xff,  /* bright white */


#if 0
	0*4,  0*4,  0*4,  /* background   */

/* this is the 'RGB monitor' version, strong and clean */
	15*4, 15*4, 15*4,  /* gray         */
		0*4, 48*4, 48*4,  /* cyan         */
	60*4,  0*4,  0*4,  /* red          */
	47*4, 47*4, 47*4,  /* white        */
	55*4, 55*4,  0*4,  /* yellow       */
		0*4, 56*4,  0*4,  /* green        */
	42*4, 32*4,  0*4,  /* orange       */
	63*4, 63*4,  0*4,  /* light yellow */
		0*4,  0*4, 48*4,  /* blue         */
		0*4, 24*4, 63*4,  /* light blue   */
	60*4,  0*4, 38*4,  /* pink         */
	38*4,  0*4, 60*4,  /* purple       */
	31*4, 31*4, 31*4,  /* light gray   */
		0*4, 63*4, 63*4,  /* light cyan   */
	58*4,  0*4, 58*4,  /* magenta      */
	63*4, 63*4, 63*4,  /* bright white */
#endif

/* this is the 'TV screen' version, weak and blurred by repeating pixels */
	15*2+80, 15*2+80, 15*2+80,  /* gray         */
		0*2+80, 48*2+80, 48*2+80,   /* cyan         */
	60*2+80,  0*2+80,  0*2+80,  /* red          */
	47*2+80, 47*2+80, 47*2+80,  /* white        */
	55*2+80, 55*2+80,  0*2+80,  /* yellow       */
		0*2+80, 56*2+80,  0*2+80,   /* green        */
	42*2+80, 32*2+80,  0*2+80,  /* orange       */
	63*2+80, 63*2+80,  0*2+80,  /* light yellow */
		0*2+80,  0*2+80, 48*2+80,   /* blue         */
		0*2+80, 24*2+80, 63*2+80,   /* light blue   */
	60*2+80,  0*2+80, 38*2+80,  /* pink         */
	38*2+80,  0*2+80, 60*2+80,  /* purple       */
	31*2+80, 31*2+80, 31*2+80,  /* light gray   */
		0*2+80, 63*2+80, 63*2+80,   /* light cyan   */
	58*2+80,  0*2+80, 58*2+80,  /* magenta      */
	63*2+80, 63*2+80, 63*2+80,  /* bright white */

	15*2+96, 15*2+96, 15*2+96,  /* gray         */
		0*2+96, 48*2+96, 48*2+96,   /* cyan         */
	60*2+96,  0*2+96,  0*2+96,  /* red          */
	47*2+96, 47*2+96, 47*2+96,  /* white        */
	55*2+96, 55*2+96,  0*2+96,  /* yellow       */
		0*2+96, 56*2+96,  0*2+96,   /* green        */
	42*2+96, 32*2+96,  0*2+96,  /* orange       */
	63*2+96, 63*2+96,  0*2+96,  /* light yellow */
		0*2+96,  0*2+96, 48*2+96,   /* blue         */
		0*2+96, 24*2+96, 63*2+96,   /* light blue   */
	60*2+96,  0*2+96, 38*2+96,  /* pink         */
	38*2+96,  0*2+96, 60*2+96,  /* purple       */
	31*2+96, 31*2+96, 31*2+96,  /* light gray   */
		0*2+96, 63*2+96, 63*2+96,   /* light cyan   */
	58*2+96,  0*2+96, 58*2+96,  /* magenta      */
	63*2+96, 63*2+96, 63*2+96,  /* bright white */


};

static const unsigned char cgenienz_colors[] = {
	0x00,0x00,0x00,  /* background   */
	0xff,0xff,0xff,  /* gray         */
	0x12,0xff,0xff,  /* cyan         */
	0xff,0x6f,0xff,  /* red          */
	0x31,0x77,0xff,  /* white        */
	0xff,0xcb,0x00,  /* yellow       */
	0x6e,0xff,0x00,  /* green        */
	0xff,0x52,0x00,  /* orange       */
	0x5e,0x5e,0x5e,  /* light yellow */
	0xea,0xea,0xea,  /* blue         */
	0x00,0xff,0xdd,  /* light blue   */
	0xd2,0x00,0xff,  /* pink         */
	0x02,0x48,0xff,  /* purple       */
	0xff,0xf9,0x00,  /* light gray   */
	0x00,0xda,0x00,  /* light cyan   */
	0xff,0x22,0x00,  /* magenta      */
	0x00,0x00,0x00,  /* bright white */


/* this is the 'TV screen' version, weak and blurred by repeating pixels */
	15*2+80, 15*2+80, 15*2+80,  /* gray         */
		0*2+80, 48*2+80, 48*2+80,   /* cyan         */
	60*2+80,  0*2+80,  0*2+80,  /* red          */
	47*2+80, 47*2+80, 47*2+80,  /* white        */
	55*2+80, 55*2+80,  0*2+80,  /* yellow       */
		0*2+80, 56*2+80,  0*2+80,   /* green        */
	42*2+80, 32*2+80,  0*2+80,  /* orange       */
	63*2+80, 63*2+80,  0*2+80,  /* light yellow */
		0*2+80,  0*2+80, 48*2+80,   /* blue         */
		0*2+80, 24*2+80, 63*2+80,   /* light blue   */
	60*2+80,  0*2+80, 38*2+80,  /* pink         */
	38*2+80,  0*2+80, 60*2+80,  /* purple       */
	31*2+80, 31*2+80, 31*2+80,  /* light gray   */
		0*2+80, 63*2+80, 63*2+80,   /* light cyan   */
	58*2+80,  0*2+80, 58*2+80,  /* magenta      */
	63*2+80, 63*2+80, 63*2+80,  /* bright white */

	15*2+96, 15*2+96, 15*2+96,  /* gray         */
		0*2+96, 48*2+96, 48*2+96,   /* cyan         */
	60*2+96,  0*2+96,  0*2+96,  /* red          */
	47*2+96, 47*2+96, 47*2+96,  /* white        */
	55*2+96, 55*2+96,  0*2+96,  /* yellow       */
		0*2+96, 56*2+96,  0*2+96,   /* green        */
	42*2+96, 32*2+96,  0*2+96,  /* orange       */
	63*2+96, 63*2+96,  0*2+96,  /* light yellow */
		0*2+96,  0*2+96, 48*2+96,   /* blue         */
		0*2+96, 24*2+96, 63*2+96,   /* light blue   */
	60*2+96,  0*2+96, 38*2+96,  /* pink         */
	38*2+96,  0*2+96, 60*2+96,  /* purple       */
	31*2+96, 31*2+96, 31*2+96,  /* light gray   */
		0*2+96, 63*2+96, 63*2+96,   /* light cyan   */
	58*2+96,  0*2+96, 58*2+96,  /* magenta      */
	63*2+96, 63*2+96, 63*2+96,  /* bright white */


};

static const unsigned short cgenie_palette[] =
{
	0, 1, 0, 2, 0, 3, 0, 4, /* RGB monitor set of text colors */
	0, 5, 0, 6, 0, 7, 0, 8,
	0, 9, 0,10, 0,11, 0,12,
	0,13, 0,14, 0,15, 0,16,

	0,17, 0,18, 0,19, 0,20, /* TV set text colors: darker */
	0,21, 0,22, 0,23, 0,24,
	0,25, 0,26, 0,27, 0,28,
	0,29, 0,30, 0,31, 0,32,

	0,33, 0,34, 0,35, 0,36, /* TV set text colors: a bit brighter */
	0,37, 0,38, 0,39, 0,40,
	0,41, 0,42, 0,43, 0,44,
	0,45, 0,46, 0,47, 0,48,

	0,    9,    7,    6,    /* RGB monitor graphics colors */
	0,    25,   23,   22,   /* TV set graphics colors: darker */
	0,    41,   39,   38,   /* TV set graphics colors: a bit brighter */
};

/* Initialise the palette */
PALETTE_INIT_MEMBER(cgenie_state,cgenie)
{
	UINT8 i, r, g, b;

	for ( i = 0; i < 49; i++ )
	{
		r = cgenie_colors[i*3]; g = cgenie_colors[i*3+1]; b = cgenie_colors[i*3+2];
		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for(i=0; i<108; i++)
		palette.set_pen_indirect(i, cgenie_palette[i]);
}

PALETTE_INIT_MEMBER(cgenie_state,cgenienz)
{
	UINT8 i, r, g, b;

	for ( i = 0; i < 49; i++ )
	{
		r = cgenienz_colors[i*3]; g = cgenienz_colors[i*3+1]; b = cgenienz_colors[i*3+2];
		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for(i=0; i<108; i++)
		palette.set_pen_indirect(i, cgenie_palette[i]);
}

// This is currently broken
static LEGACY_FLOPPY_OPTIONS_START(cgenie )
	LEGACY_FLOPPY_OPTION( cgd, "cgd", "Colour Genie disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([10])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface cgenie_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(cgenie),
	NULL
};


// TODO: investigate this! I think it is some sort of expansion of the DOS cart...
DEVICE_IMAGE_LOAD_MEMBER( cgenie_state, cgenie_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}


static MACHINE_CONFIG_START( cgenie_common, cgenie_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_17_73447MHz/8)        /* 2,2168 MHz */
	MCFG_CPU_PROGRAM_MAP(cgenie_mem)
	MCFG_CPU_IO_MAP(cgenie_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cgenie_state,  cgenie_frame_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(cgenie_state, cgenie_timer_interrupt,  40)
	MCFG_QUANTUM_TIME(attotime::from_hz(240))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(48*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1,0*8,32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cgenie_state, screen_update_cgenie)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cgenie )
	MCFG_PALETTE_ADD("palette", 108)
	MCFG_PALETTE_INDIRECT_ENTRIES(49)

	// Actually the video is driven by an HD46505 clocked at XTAL_17_73447MHz/16

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_17_73447MHz/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(cgenie_state, cgenie_psg_port_a_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(cgenie_state, cgenie_psg_port_b_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(cgenie_state, cgenie_psg_port_a_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(cgenie_state, cgenie_psg_port_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(cgenie_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
	MCFG_CASSETTE_INTERFACE("cgenie_cass")

	MCFG_DEVICE_ADD("wd179x", FD1793, 0) // TODO confirm type
	MCFG_WD17XX_DEFAULT_DRIVE4_TAGS
	MCFG_WD17XX_INTRQ_CALLBACK(WRITELINE(cgenie_state, cgenie_fdc_intrq_w))

	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(cgenie_floppy_interface)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "cgenie_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(cgenie_state, cgenie_cart)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "cgenie_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "cgenie_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cgenie, cgenie_common )
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(cgenie_state, cgenie )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cgenienz, cgenie_common )
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(cgenie_state, cgenienz )
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START (cgenie)
	ROM_REGION(0x13000,"maincpu",0)
	ROM_LOAD ("cgenie.rom",  0x00000, 0x4000, CRC(d359ead7) SHA1(d8c2fc389ad38c45fba0ed556a7d91abac5463f4))
	ROM_LOAD ("cgdos.rom",   0x10000, 0x2000, CRC(2a96cf74) SHA1(6dcac110f87897e1ee7521aefbb3d77a14815893))

	ROM_REGION(0x0c00,"gfx1",0)
	ROM_LOAD ("cgenie1.fnt", 0x0000, 0x0800, CRC(4fed774a) SHA1(d53df8212b521892cc56be690db0bb474627d2ff))

	/* Empty memory region for the character generator */
	ROM_REGION(0x0800,"gfx2",ROMREGION_ERASEFF)

ROM_END

ROM_START (cgenienz)
	ROM_REGION(0x13000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "old", "Old ROM")
	ROMX_LOAD( "cg-basic-rom-v1-pal-en.rom",   0x0000, 0x4000, CRC(844aaedd) SHA1(b7f984bc5cd979c7ad11ff909e8134f694aea7aa), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "new", "New ROM")
	ROMX_LOAD( "cgromv2.rom",   0x0000, 0x4000, CRC(cfb84e09) SHA1(e199e4429bab6f9fca2bb05e71324538928a693a), ROM_BIOS(2) )
	ROM_LOAD ("cgdos.rom",   0x10000, 0x2000, CRC(2a96cf74) SHA1(6dcac110f87897e1ee7521aefbb3d77a14815893))

	ROM_REGION(0x0c00,"gfx1",0)
	ROM_LOAD ("cgenie1.fnt", 0x0000, 0x0800, CRC(4fed774a) SHA1(d53df8212b521892cc56be690db0bb474627d2ff))

	/* Empty memory region for the character generator */
	ROM_REGION(0x0800,"gfx2",ROMREGION_ERASEFF)

ROM_END

// Code below is previous non-working implementation , just for reference
#if 0

#define CGENIE_DRIVE_INFO



//
//   abbreviations used:
//   GPL    Granules Per Lump
//   GAT    Granule Allocation Table
//   GATL GAT Length
//   GATM GAT Mask
//   DDGA Disk Directory Granule Allocation
struct PDRIVE
{
	UINT8 DDSL;      // Disk Directory Start Lump (lump number of GAT)
	UINT8 GATL;      // # of bytes used in the Granule Allocation Table sector
	UINT8 STEPRATE;  // step rate and somet SD/DD flag ...
	UINT8 TRK;       // number of tracks
	UINT8 SPT;       // sectors per track (both heads counted!)
	UINT8 GATM;      // number of used bits per byte in the GAT sector (GAT mask)
	UINT8 P7;        // ???? always zero
	UINT8 FLAGS;     // ???? some flags (SS/DS bit 6)
	UINT8 GPL;       // Sectors per granule (always 5 for the Colour Genie)
	UINT8 DDGA;      // Disk Directory Granule allocation (number of driectory granules)
};

static const PDRIVE pd_list[12] = {
	{0x14, 0x28, 0x07, 0x28, 0x0A, 0x02, 0x00, 0x00, 0x05, 0x02}, // CMD"<0=A" 40 tracks, SS, SD
	{0x14, 0x28, 0x07, 0x28, 0x14, 0x04, 0x00, 0x40, 0x05, 0x04}, // CMD"<0=B" 40 tracks, DS, SD
	{0x18, 0x30, 0x53, 0x28, 0x12, 0x03, 0x00, 0x03, 0x05, 0x03}, // CMD"<0=C" 40 tracks, SS, DD
	{0x18, 0x30, 0x53, 0x28, 0x24, 0x06, 0x00, 0x43, 0x05, 0x06}, // CMD"<0=D" 40 tracks, DS, DD
	{0x14, 0x28, 0x07, 0x28, 0x0A, 0x02, 0x00, 0x04, 0x05, 0x02}, // CMD"<0=E" 40 tracks, SS, SD
	{0x14, 0x28, 0x07, 0x28, 0x14, 0x04, 0x00, 0x44, 0x05, 0x04}, // CMD"<0=F" 40 tracks, DS, SD
	{0x18, 0x30, 0x53, 0x28, 0x12, 0x03, 0x00, 0x07, 0x05, 0x03}, // CMD"<0=G" 40 tracks, SS, DD
	{0x18, 0x30, 0x53, 0x28, 0x24, 0x06, 0x00, 0x47, 0x05, 0x06}, // CMD"<0=H" 40 tracks, DS, DD
	{0x28, 0x50, 0x07, 0x50, 0x0A, 0x02, 0x00, 0x00, 0x05, 0x02}, // CMD"<0=I" 80 tracks, SS, SD
	{0x28, 0x50, 0x07, 0x50, 0x14, 0x04, 0x00, 0x40, 0x05, 0x04}, // CMD"<0=J" 80 tracks, DS, SD
	{0x30, 0x60, 0x53, 0x50, 0x12, 0x03, 0x00, 0x03, 0x05, 0x03}, // CMD"<0=K" 80 tracks, SS, DD
	{0x30, 0x60, 0x53, 0x50, 0x24, 0x06, 0x00, 0x43, 0x05, 0x06}, // CMD"<0=L" 80 tracks, DS, DD
};

// basic-dsk is a disk image format which has the tracks and sectors
// stored in order, no information is stored which details the number
// of tracks, number of sides, number of sectors etc, so we need to
// set that up here
//
DEVICE_IMAGE_LOAD( cgenie_floppy )
{
	int i, j, dir_offset;
	UINT8 buff[16];
	UINT8 tracks = 0;
	UINT8 heads = 0;
	UINT8 spt = 0;
	short dir_sector = 0;
	short dir_length = 0;

	// A Floppy Isnt manditory, so return if none
	if (device_load_basicdsk_floppy(image) != IMAGE_INIT_PASS)
		return IMAGE_INIT_FAIL;

	// determine image geometry
	image.fseek(0, SEEK_SET);

	// determine geometry from disk contents
	for( i = 0; i < 12; i++ )
	{
		image.fseek(pd_list[i].SPT * 256, SEEK_SET);
		image.fread( buff, 16);
		// find an entry with matching DDSL
		if (buff[0] != 0x00 || buff[1] != 0xfe || buff[2] != pd_list[i].DDSL)
			continue;
		logerror("cgenie: checking format #%d\n", i);

		dir_sector = pd_list[i].DDSL * pd_list[i].GATM * pd_list[i].GPL + pd_list[i].SPT;
		dir_length = pd_list[i].DDGA * pd_list[i].GPL;

		// scan directory for DIR/SYS or NCW1983/JHL files
		// look into sector 2 and 3 first entry relative to DDSL
		for( j = 16; j < 32; j += 8 )
		{
			dir_offset = dir_sector * 256 + j * 32;
			if( image.fseek(dir_offset, SEEK_SET) < 0 )
				break;
			if( image.fread( buff, 16) != 16 )
				break;
			if( !strncmp((char*)buff + 5, "DIR     SYS", 11) ||
				!strncmp((char*)buff + 5, "NCW1983 JHL", 11) )
			{
				tracks = pd_list[i].TRK;
				heads = (pd_list[i].SPT > 18) ? 2 : 1;
				spt = pd_list[i].SPT / heads;
				dir_sector = pd_list[i].DDSL * pd_list[i].GATM * pd_list[i].GPL + pd_list[i].SPT;
				dir_length = pd_list[i].DDGA * pd_list[i].GPL;
				memcpy(memregion("maincpu")->base() + 0x5A71 + floppy_get_drive(image) * sizeof(PDRIVE), &pd_list[i], sizeof(PDRIVE));
				break;
			}
		}

		logerror("cgenie: geometry %d tracks, %d heads, %d sec/track\n", tracks, heads, spt);
		// set geometry so disk image can be read
		basicdsk_set_geometry(image, tracks, heads, spt, 256, 0, 0, FALSE);

		logerror("cgenie: directory sectors %d - %d (%d sectors)\n", dir_sector, dir_sector + dir_length - 1, dir_length);
		// mark directory sectors with deleted data address mark
		// assumption dir_sector is a sector offset
		for (j = 0; j < dir_length; j++)
		{
			UINT8 track;
			UINT8 side;
			UINT8 sector_id;
			UINT16 track_offset;
			UINT16 sector_offset;

			// calc sector offset
			sector_offset = dir_sector + j;

			// get track offset
			track_offset = sector_offset / spt;

			// calc track
			track = track_offset / heads;

			// calc side
			side = track_offset % heads;

			// calc sector id - first sector id is 0!
			sector_id = sector_offset % spt;

			// set deleted data address mark for sector specified
			basicdsk_set_ddam(image, track, side, sector_id, 1);
		}

	}
	return IMAGE_INIT_PASS;
}
#endif

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT     COMPANY    FULLNAME */
COMP( 1982, cgenie,   0,        0,      cgenie,   cgenie, driver_device,    0,       "EACA Computers Ltd",  "Colour Genie EG2000" , 0)
COMP( 1982, cgenienz, cgenie,   0,      cgenienz, cgenie, driver_device,    0,       "EACA Computers Ltd",  "Colour Genie EG2000 (New Zealand)" , 0)
