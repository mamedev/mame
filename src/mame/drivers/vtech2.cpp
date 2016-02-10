// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
    vtech2.c

    system driver
    Juergen Buchmueller <pullmoll@t-online.de> MESS driver, Jan 2000
    Davide Moretti <dave@rimini.com> ROM dump and hardware description

    LASER 350 (it has only 16K of RAM)
    FFFF|-------|
        | Empty |
        |   5   |
    C000|-------|
        |  RAM  |
        |   3   |
    8000|-------|-------|-------|
        |  ROM  |Display|  I/O  |
        |   1   |   3   |   2   |
    4000|-------|-------|-------|
        |  ROM  |
        |   0   |
    0000|-------|


    Laser 500/700 with 64K of RAM and
    Laser 350 with 64K RAM expansion module
    FFFF|-------|
        |  RAM  |
        |   5   |
    C000|-------|
        |  RAM  |
        |   4   |
    8000|-------|-------|-------|
        |  ROM  |Display|  I/O  |
        |   1   |   7   |   2   |
    4000|-------|-------|-------|
        |  ROM  |
        |   0   |
    0000|-------|


    Bank "maincpu"       Contents
    0    0x00000 - 0x03fff ROM 1st half
    1    0x04000 - 0x07fff ROM 2nd half
    2           n/a        I/O 2KB area (mirrored 8 times?)
    3    0x0c000 - 0x0ffff Display RAM (16KB) present in Laser 350 only!
    4    0x10000 - 0x13fff RAM #4
    5    0x14000 - 0x17fff RAM #5
    6    0x18000 - 0x1bfff RAM #6
    7    0x1c000 - 0x1ffff RAM #7 (Display RAM with 64KB)
    8    0x20000 - 0x23fff RAM #8 (Laser 700 or 128KB extension)
    9    0x24000 - 0x27fff RAM #9
    A    0x28000 - 0x2bfff RAM #A
    B    0x2c000 - 0x2ffff RAM #B
    C    0x30000 - 0x33fff ROM expansion
    D    0x34000 - 0x34fff ROM expansion
    E    0x38000 - 0x38fff ROM expansion
    F    0x3c000 - 0x3ffff ROM expansion

    TODO:
        Add ROMs and drivers for the Laser100, 110,
        210 and 310 machines and the Texet 8000.
        They should probably go to the vtech1.c files, though.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "includes/vtech2.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "formats/vt_cas.h"

static ADDRESS_MAP_START(vtech2_mem, AS_PROGRAM, 8, vtech2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START(vtech2_io, AS_IO, 8, vtech2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x1f) AM_READWRITE(laser_fdc_r, laser_fdc_w)
	AM_RANGE(0x40, 0x43) AM_WRITE(laser_bank_select_w)
	AM_RANGE(0x44, 0x44) AM_WRITE(laser_bg_mode_w)
	AM_RANGE(0x45, 0x45) AM_WRITE(laser_two_color_w)
ADDRESS_MAP_END

/* 2008-05 FP:
Small note about natural keyboard: currently,
- "Graph" is mapped to 'F11'
- "Del Line" is mapped to 'F12'
*/
static INPUT_PORTS_START( laser500 )
	PORT_START("ROW0")  /* KEY ROW 0 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("ROW1") /* KEY ROW 1 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("ROW2") /* KEY ROW 2 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("ROW3") /* KEY ROW 3 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("ROW4") /* KEY ROW 4 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW5") /* KEY ROW 5 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')

	PORT_START("ROW6") /* KEY ROW 6 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("ROW7") /* KEY ROW 7 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROWA") /* KEY ROW A */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROWB") /* KEY ROW B */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("ROWC") /* KEY ROW C */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cap Lock") PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Line") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("ROWD") /* KEY ROW D */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mu  \xC2\xA3") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\xA3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)             PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)          PORT_CHAR(UCHAR_MAMEKEY(INSERT))
INPUT_PORTS_END

/* 2008-05 FP: I wasn't able to find a good picture of the laser 350 to verify the mapping of the emulated keyboard.
However, old-computers.com describes it as a laser 500/700 in a laser 300/310 case. The missing inputs seem to
confirm this. */
static INPUT_PORTS_START( laser350 )
	PORT_INCLUDE( laser500 )

	PORT_MODIFY("ROW2") /* KEY ROW 2 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) /* TAB not on the Laser350 */

	PORT_MODIFY("ROW3") /* KEY ROW 3 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) /* ESC not on the Laser350 */

	PORT_MODIFY("ROW5") /* KEY ROW 5 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) /* BS not on the Laser350 */

	PORT_MODIFY("ROW7") /* KEY ROW 7 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) /* GRAPH not on the Laser350 */

	PORT_MODIFY("ROWA") /* KEY ROW A */
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) /* not on the Laser350 */

	PORT_MODIFY("ROWB") /* KEY ROW B */
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) /* not on the Laser350 */

	PORT_MODIFY("ROWC") /* KEY ROW C */
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) /* not on the Laser350 */

	PORT_MODIFY("ROWD") /* KEY ROW D */
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) /* not on the Laser350 */

	/* 2008-05 FP: This input_port seems never to be read. Is it a leftover of the old cassette code? */
	PORT_START("TAPE") /* Tape control */
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tape Start") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tape Stop") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tape Rewind") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static const gfx_layout charlayout_80 =
{
	8,8,                    /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static const gfx_layout charlayout_40 =
{
	8*2,8,                  /* 8*2 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7 },
	/* y offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static const gfx_layout gfxlayout_1bpp =
{
	8,1,                    /* 8x1 pixels */
	256,                    /* 256 codes */
	1,                      /* 1 bit per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7,6,5,4,3,2,1,0 },
	/* y offsets */
	{ 0 },
	8                       /* one byte per code */
};

static const gfx_layout gfxlayout_1bpp_dw =
{
	8*2,1,                  /* 8 times 2x1 pixels */
	256,                    /* 256 codes */
	1,                      /* 1 bit per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0 },
	/* y offsets */
	{ 0 },
	8                       /* one byte per code */
};

static const gfx_layout gfxlayout_1bpp_qw =
{
	8*4,1,                  /* 8 times 4x1 pixels */
	256,                    /* 256 codes */
	1,                      /* 1 bit per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7,7,7,7,6,6,6,6,5,5,5,5,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1,0,0,0,0 },
	/* y offsets */
	{ 0 },
	8                       /* one byte per code */
};

static const gfx_layout gfxlayout_4bpp =
{
	2*4,1,                  /* 2 times 4x1 pixels */
	256,                    /* 256 codes */
	4,                      /* 4 bit per pixel */
	{ 0,1,2,3 },            /* four bitplanes */
	/* x offsets */
	{ 4,4,4,4, 0,0,0,0 },
	/* y offsets */
	{ 0 },
	2*4                     /* one byte per code */
};

static const gfx_layout gfxlayout_4bpp_dh =
{
	2*4,2,                  /* 2 times 4x2 pixels */
	256,                    /* 256 codes */
	4,                      /* 4 bit per pixel */
	{ 0,1,2,3 },            /* four bitplanes */
	/* x offsets */
	{ 4,4,4,4, 0,0,0,0 },
	/* y offsets */
	{ 0,0 },
	2*4                     /* one byte per code */
};

static GFXDECODE_START( vtech2 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_80, 0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_40, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_1bpp, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_1bpp_dw, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_1bpp_qw, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_4bpp, 2*256, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_4bpp_dh, 2*256, 1 )
GFXDECODE_END


static const rgb_t vt_colors[] =
{
	rgb_t::black,
	rgb_t(0x00, 0x00, 0x7f),  /* blue */
	rgb_t(0x00, 0x7f, 0x00),  /* green */
	rgb_t(0x00, 0x7f, 0x7f),  /* cyan */
	rgb_t(0x7f, 0x00, 0x00),  /* red */
	rgb_t(0x7f, 0x00, 0x7f),  /* magenta */
	rgb_t(0x7f, 0x7f, 0x00),  /* yellow */
	rgb_t(0xa0, 0xa0, 0xa0),  /* bright grey */
	rgb_t(0x7f, 0x7f, 0x7f),  /* dark grey */
	rgb_t(0x00, 0x00, 0xff),  /* bright blue */
	rgb_t(0x00, 0xff, 0x00),  /* bright green */
	rgb_t(0x00, 0xff, 0xff),  /* bright cyan */
	rgb_t(0xff, 0x00, 0x00),  /* bright red */
	rgb_t(0xff, 0x00, 0xff),  /* bright magenta */
	rgb_t(0xff, 0xff, 0x00),  /* bright yellow */
	rgb_t::white
};


/* Initialise the palette */
PALETTE_INIT_MEMBER(vtech2_state, vtech2)
{
	int i;

	for ( i = 0; i < 16; i++ )
		palette.set_indirect_color(i, vt_colors[i]);

	for (i = 0; i < 256; i++)
	{
		palette.set_pen_indirect(2*i, i&15);
		palette.set_pen_indirect(2*i+1, i>>4);
	}

	for (i = 0; i < 16; i++)
		palette.set_pen_indirect(512+i, i);
}

INTERRUPT_GEN_MEMBER(vtech2_state::vtech2_interrupt)
{
	m_maincpu->set_input_line(0, HOLD_LINE);
}

static const floppy_interface vtech2_floppy_interface =
{
	FLOPPY_STANDARD_5_25_SSDD_40,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	nullptr
};

static MACHINE_CONFIG_START( laser350, vtech2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3694700)        /* 3.694700 MHz */
	MCFG_CPU_PROGRAM_MAP(vtech2_mem)
	MCFG_CPU_IO_MAP(vtech2_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vtech2_state,  vtech2_interrupt)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_SIZE(88*8, 24*8+32)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 88*8-1, 0*8, 24*8+32-1)
	MCFG_SCREEN_UPDATE_DRIVER(vtech2_state, screen_update_laser)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vtech2 )
	MCFG_PALETTE_ADD("palette", 512+16)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(vtech2_state, vtech2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(vtech2_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "vtech_cart")
	MCFG_GENERIC_EXTENSIONS("rom,bin")

	/* 5.25" Floppy drive */
	MCFG_LEGACY_FLOPPY_DRIVE_ADD( FLOPPY_0, vtech2_floppy_interface )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( laser500, laser350 )
	MCFG_MACHINE_RESET_OVERRIDE(vtech2_state, laser500 )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( laser700, laser350 )
	MCFG_MACHINE_RESET_OVERRIDE(vtech2_state, laser700 )

	/* Second 5.25" floppy drive */
	MCFG_LEGACY_FLOPPY_DRIVE_ADD( FLOPPY_1, vtech2_floppy_interface )
MACHINE_CONFIG_END


ROM_START(laser350)
	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("laserv3.rom", 0x00000, 0x08000, CRC(9bed01f7) SHA1(3210fddfab2f4c7855fa902fb8e2fc18d10d48f1))
	ROM_REGION(0x00800,"gfx1",0)
	ROM_LOAD("laser.fnt",   0x00000, 0x00800, CRC(ed6bfb2a) SHA1(95e247021a10167b9de1d6ffc91ec4ba83b0ec87))
	ROM_REGION(0x00100,"gfx2",ROMREGION_ERASEFF)
	/* initialized in init_laser */
ROM_END


ROM_START(laser500) // based on the picture at http://www.8bit-museum.de/hardware/laser500pcb-h.jpg
// There should be two roms, one 0x2000 long for the font at u10, and one longer one for the os rom at u6.
	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("27-0401-00-00.u6", 0x00000, 0x08000, CRC(9bed01f7) SHA1(3210fddfab2f4c7855fa902fb8e2fc18d10d48f1)) // may be dumped at wrong size; label is: "VTL 27-0401-00-00 // 6133-7081 // 8611MAK"
	ROM_REGION(0x00800,"gfx1",0)
	ROM_LOAD("27-393-00.u10",   0x00000, 0x00800, BAD_DUMP CRC(ed6bfb2a) SHA1(95e247021a10167b9de1d6ffc91ec4ba83b0ec87)) // dumped at wrong size; correct size is 0x2000; label is "TMS 2364-25NL // D8614L // ZA234015 // 27-393-00/VT 85 // SINGAPORE"
	ROM_REGION(0x00100,"gfx2",ROMREGION_ERASEFF)
	/* initialized in init_laser */
ROM_END

ROM_START(laser700)
	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("laserv3.rom", 0x00000, 0x08000, CRC(9bed01f7) SHA1(3210fddfab2f4c7855fa902fb8e2fc18d10d48f1))
	ROM_REGION(0x00800,"gfx1",0)
	ROM_LOAD("laser.fnt",   0x00000, 0x00800, CRC(ed6bfb2a) SHA1(95e247021a10167b9de1d6ffc91ec4ba83b0ec87))
	ROM_REGION(0x00100,"gfx2",ROMREGION_ERASEFF)
	/* initialized in init_laser */
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR   NAME      PARENT    COMPAT MACHINE   INPUT     INIT      COMPANY              FULLNAME */
COMP( 1984?, laser350, 0,        0,     laser350, laser350, vtech2_state, laser,    "Video Technology",  "Laser 350" , 0)
COMP( 1984?, laser500, laser350, 0,     laser500, laser500, vtech2_state, laser,    "Video Technology",  "Laser 500" , 0)
COMP( 1984?, laser700, laser350, 0,     laser700, laser500, vtech2_state, laser,    "Video Technology",  "Laser 700" , 0)
