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
    - Ram pak
    - undumped DOS ROM
    - need software

    Cartslot works, even though it seems there were no game carts made
    for these systems. The bios checks the first few bytes for a particular
    sequence; if found, the cart is executed at the next byte.
    We allow a cart of any size up to 64k, and it gets loaded into bank 12,
    continuing on to banks 13, 14 and 15 if needed. The bios checks for the
    sequence at each bank boundary.

***************************************************************************/

#include "emu.h"
#include "vtech2.h"
#include "cpu/z80/z80.h"
#include "formats/vt_cas.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"



void vtech2_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).m(m_banka, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0x7fff).m(m_bankb, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xbfff).m(m_bankc, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xffff).m(m_bankd, FUNC(address_map_bank_device::amap8));
}

void vtech2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0xff).noprw();
	map(0x10, 0x1f).rw(FUNC(vtech2_state::laser_fdc_r), FUNC(vtech2_state::laser_fdc_w));
	map(0x40, 0x40).lw8(NAME([this] (u8 data) { m_banka->set_bank(data & 15); }));
	map(0x41, 0x41).lw8(NAME([this] (u8 data) { m_bankb->set_bank(data & 15); }));
	map(0x42, 0x42).lw8(NAME([this] (u8 data) { m_bankc->set_bank(data & 15); }));
	map(0x43, 0x43).lw8(NAME([this] (u8 data) { m_bankd->set_bank(data & 15); }));
	map(0x44, 0x44).w(FUNC(vtech2_state::laser_bg_mode_w));
	map(0x45, 0x45).w(FUNC(vtech2_state::laser_two_color_w));
}

// Laser 350, 16k ram
void vtech2_state::m_map350(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("maincpu", 0);
	map(0x04000, 0x07fff).rom().region("maincpu", 0x4000);
	map(0x08000, 0x0bfff).rw(FUNC(vtech2_state::mmio_r), FUNC(vtech2_state::mmio_w));
	map(0x0c000, 0x0ffff).ram().share(m_vram);
	map(0x10000, 0x13fff).noprw();
	map(0x14000, 0x17fff).noprw();
	map(0x18000, 0x1bfff).noprw();
	map(0x1c000, 0x1ffff).noprw();
	map(0x20000, 0x23fff).noprw(); // TODO: 64k ram expansion pak
	map(0x24000, 0x27fff).noprw(); // TODO: 64k ram expansion pak
	map(0x28000, 0x2bfff).noprw(); // TODO: 64k ram expansion pak
	map(0x2c000, 0x2ffff).noprw(); // TODO: 64k ram expansion pak
	map(0x30000, 0x3ffff).r(FUNC(vtech2_state::cart_r));
}

// Laser 500, 64k ram
void vtech2_state::m_map500(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("maincpu", 0);
	map(0x04000, 0x07fff).rom().region("maincpu", 0x4000);
	map(0x08000, 0x0bfff).rw(FUNC(vtech2_state::mmio_r), FUNC(vtech2_state::mmio_w));
	map(0x0c000, 0x0ffff).noprw();
	map(0x10000, 0x13fff).ram();
	map(0x14000, 0x17fff).ram();
	map(0x18000, 0x1bfff).ram();
	map(0x1c000, 0x1ffff).ram().share(m_vram);
	map(0x20000, 0x23fff).noprw(); // TODO: 64k ram expansion pak
	map(0x24000, 0x27fff).noprw(); // TODO: 64k ram expansion pak
	map(0x28000, 0x2bfff).noprw(); // TODO: 64k ram expansion pak
	map(0x2c000, 0x2ffff).noprw(); // TODO: 64k ram expansion pak
	map(0x30000, 0x3ffff).r(FUNC(vtech2_state::cart_r));
}

// Laser 700, 128k ram
void vtech2_state::m_map700(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("maincpu", 0);
	map(0x04000, 0x07fff).rom().region("maincpu", 0x4000);
	map(0x08000, 0x0bfff).rw(FUNC(vtech2_state::mmio_r), FUNC(vtech2_state::mmio_w));
	map(0x0c000, 0x0ffff).noprw();
	map(0x10000, 0x13fff).ram();
	map(0x14000, 0x17fff).ram();
	map(0x18000, 0x1bfff).ram();
	map(0x1c000, 0x1ffff).ram().share(m_vram);
	map(0x20000, 0x23fff).ram();
	map(0x24000, 0x27fff).ram();
	map(0x28000, 0x2bfff).ram();
	map(0x2c000, 0x2ffff).ram();
	map(0x30000, 0x3ffff).r(FUNC(vtech2_state::cart_r));
}


static INPUT_PORTS_START( laser500 )
	PORT_START("ROW0")  /* KEY ROW 0 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('N') PORT_CHAR('n')

	PORT_START("ROW1") /* KEY ROW 1 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('H') PORT_CHAR('h')

	PORT_START("ROW2") /* KEY ROW 2 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR(9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y') PORT_CHAR('y')

	PORT_START("ROW3") /* KEY ROW 3 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("ROW4") /* KEY ROW 4 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW5") /* KEY ROW 5 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_CONFNAME( 0x30, 0x30, "Language")
	PORT_CONFSETTING(    0x10, "French")
	PORT_CONFSETTING(    0x20, "German")
	PORT_CONFSETTING(    0x30, "English")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('U') PORT_CHAR('u')

	PORT_START("ROW6") /* KEY ROW 6 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('J') PORT_CHAR('j')

	PORT_START("ROW7") /* KEY ROW 7 */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("ROWA") /* KEY ROW A */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROWB") /* KEY ROW B */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("ROWC") /* KEY ROW C */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cap Lock") PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Line") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("ROWD") /* KEY ROW D */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xCE\xBC  \xC2\xA3") PORT_CODE(KEYCODE_END) PORT_CHAR(0xa3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)             PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)          PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CHANGED_MEMBER(DEVICE_SELF, vtech2_state, reset_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(vtech2_state::reset_button)
{
	// RESET button is directly wired to Z80 RESET pin, BIOS will detect it (doesn't reset the computer)
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}


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

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) /* not on the Laser350 */
INPUT_PORTS_END


static const gfx_layout charlayout_80 =
{
	8,8,                    /* 8 x 8 characters */
	1024,                    /* characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static const gfx_layout charlayout_40 =
{
	8*2,8,                  /* 8*2 x 8 characters */
	1024,                    /* characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 7,7, 6,6, 5,5, 4,4, 3,3, 2,2, 1,1, 0,0 },
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

static GFXDECODE_START( gfx_vtech2 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_80, 0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_40, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_1bpp, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_1bpp_dw, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_1bpp_qw, 0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_4bpp, 2*256, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_4bpp_dh, 2*256, 1 )
GFXDECODE_END


static constexpr rgb_t vt_colors[] =
{
	rgb_t::black(),
	{ 0x00, 0x00, 0x7f },  // blue
	{ 0x00, 0x7f, 0x00 },  // green
	{ 0x00, 0x7f, 0x7f },  // cyan
	{ 0x7f, 0x00, 0x00 },  // red
	{ 0x7f, 0x00, 0x7f },  // magenta
	{ 0x7f, 0x7f, 0x00 },  // yellow
	{ 0xa0, 0xa0, 0xa0 },  // bright grey
	{ 0x7f, 0x7f, 0x7f },  // dark grey
	{ 0x00, 0x00, 0xff },  // bright blue
	{ 0x00, 0xff, 0x00 },  // bright green
	{ 0x00, 0xff, 0xff },  // bright cyan
	{ 0xff, 0x00, 0x00 },  // bright red
	{ 0xff, 0x00, 0xff },  // bright magenta
	{ 0xff, 0xff, 0x00 },  // bright yellow
	rgb_t::white()
};


// Initialise the palette
void vtech2_state::vtech2_palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
		palette.set_indirect_color(i, vt_colors[i]);

	for (int i = 0; i < 256; i++)
	{
		palette.set_pen_indirect(2*i, i & 15);
		palette.set_pen_indirect(2*i + 1, i >> 4);
	}

	for (int i = 0; i < 16; i++)
		palette.set_pen_indirect(512 + i, i);
}

static const floppy_interface vtech2_floppy_interface =
{
	FLOPPY_STANDARD_5_25_SSDD_40,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	nullptr
};

void vtech2_state::laser350(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3694700);        /* 3.694700 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &vtech2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vtech2_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(vtech2_state::irq0_line_hold));
	config.set_maximum_quantum(attotime::from_hz(60));

	ADDRESS_MAP_BANK(config, "banka").set_map(&vtech2_state::m_map350).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config, "bankb").set_map(&vtech2_state::m_map350).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config, "bankc").set_map(&vtech2_state::m_map350).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config, "bankd").set_map(&vtech2_state::m_map350).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(0);
	screen.set_size(88*8, 24*8+32);
	screen.set_visarea(0*8, 88*8-1, 0*8, 24*8+32-1);
	screen.set_screen_update(FUNC(vtech2_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vtech2);
	PALETTE(config, m_palette, FUNC(vtech2_state::vtech2_palette), 512 + 16, 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.75);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(vtech2_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("vtech2_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("vtech2_cass");

	VTECH_IOEXP_SLOT(config, m_ioexp);
	m_ioexp->set_iospace(m_maincpu, AS_IO);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "vtech_cart", "rom,bin").set_device_load(FUNC(vtech2_state::cart_load));

	/* 5.25" Floppy drive */
	LEGACY_FLOPPY(config, m_laser_file[0], 0, &vtech2_floppy_interface);
}


void vtech2_state::laser500(machine_config &config)
{
	laser350(config);

	ADDRESS_MAP_BANK(config.replace(), "banka").set_map(&vtech2_state::m_map500).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config.replace(), "bankb").set_map(&vtech2_state::m_map500).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config.replace(), "bankc").set_map(&vtech2_state::m_map500).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config.replace(), "bankd").set_map(&vtech2_state::m_map500).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
}


void vtech2_state::laser700(machine_config &config)
{
	laser350(config);

	ADDRESS_MAP_BANK(config.replace(), "banka").set_map(&vtech2_state::m_map700).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config.replace(), "bankb").set_map(&vtech2_state::m_map700).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config.replace(), "bankc").set_map(&vtech2_state::m_map700).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config.replace(), "bankd").set_map(&vtech2_state::m_map700).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);

	/* Second 5.25" floppy drive */
	LEGACY_FLOPPY(config, m_laser_file[1], 0, &vtech2_floppy_interface);
}


ROM_START(laser350)
	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("27-0401-00-00.u6", 0x0000, 0x8000, CRC(9bed01f7) SHA1(3210fddfab2f4c7855fa902fb8e2fc18d10d48f1))

	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "27-393-00.u10", 0x0000, 0x2000, CRC(d47313a2) SHA1(4650e8e339aad628c0e5d8a1944b21abff793446) )

	ROM_REGION(0x0100,"gfx2",ROMREGION_ERASEFF)
	/* initialized in init_laser */
ROM_END


ROM_START(laser500) // based on the picture at http://www.8bit-museum.de/hardware/laser500pcb-h.jpg
// There should be two roms, one 0x2000 long for the font at u10, and one longer one for the os rom at u6.
	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("27-0401-00-00.u6", 0x0000, 0x8000, CRC(9bed01f7) SHA1(3210fddfab2f4c7855fa902fb8e2fc18d10d48f1)) // may be dumped at wrong size; label is: "VTL 27-0401-00-00 // 6133-7081 // 8611MAK"

	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "27-393-00.u10", 0x0000, 0x2000, CRC(d47313a2) SHA1(4650e8e339aad628c0e5d8a1944b21abff793446) ) // label is "TMS 2364-25NL // D8614L // ZA234015 // 27-393-00/VT 85 // SINGAPORE"

	ROM_REGION(0x0100,"gfx2",ROMREGION_ERASEFF)
	/* initialized in init_laser */
ROM_END

ROM_START(laser700)
	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("27-0401-00-00.u6", 0x0000, 0x8000, CRC(9bed01f7) SHA1(3210fddfab2f4c7855fa902fb8e2fc18d10d48f1))

	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "27-393-00.u10", 0x0000, 0x2000, CRC(d47313a2) SHA1(4650e8e339aad628c0e5d8a1944b21abff793446) )

	ROM_REGION(0x0100,"gfx2",ROMREGION_ERASEFF)
	/* initialized in init_laser */
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR   NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS         INIT        COMPANY             FULLNAME      FLAGS
COMP( 1985, laser350, 0,        0,      laser350, laser350, vtech2_state, init_laser, "Video Technology", "Laser 350" , MACHINE_SUPPORTS_SAVE )
COMP( 1985, laser500, laser350, 0,      laser500, laser500, vtech2_state, init_laser, "Video Technology", "Laser 500" , MACHINE_SUPPORTS_SAVE )
COMP( 1985, laser700, laser350, 0,      laser700, laser500, vtech2_state, init_laser, "Video Technology", "Laser 700" , MACHINE_SUPPORTS_SAVE )
