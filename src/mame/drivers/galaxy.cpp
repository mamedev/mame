// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Miodrag Milanovic
/***************************************************************************
Galaksija driver by Krzysztof Strzecha and Miodrag Milanovic

22/05/2008 Tape support added (Miodrag Milanovic)
21/05/2008 Galaksija plus initial support (Miodrag Milanovic)
20/05/2008 Added real video implementation (Miodrag Milanovic)
18/04/2005 Possibilty to disable ROM 2. 2k, 22k, 38k and 54k memory
       configurations added.
13/03/2005 Memory mapping improved. Palette corrected. Supprort for newer
           version of snapshots added. Lot of cleanups. Keyboard mapping
           corrected.
19/09/2002 malloc() replaced by image_malloc().
15/09/2002 Snapshot loading fixed. Code cleanup.
31/01/2001 Snapshot loading corrected.
09/01/2001 Fast mode implemented (many thanks to Kevin Thacker).
07/01/2001 Keyboard corrected (still some keys unknown).
           Horizontal screen positioning in video subsystem added.
05/01/2001 Keyboard implemented (some keys unknown).
03/01/2001 Snapshot loading added.
01/01/2001 Preliminary driver.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/wave.h"
#include "includes/galaxy.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "formats/gtp_cas.h"
#include "machine/ram.h"
#include "softlist.h"

static ADDRESS_MAP_START (galaxyp_io, AS_IO, 8, galaxy_state )
	ADDRESS_MAP_GLOBAL_MASK(0x01)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xbe, 0xbe) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0xbf, 0xbf) AM_DEVWRITE("ay8910", ay8910_device, data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START (galaxy_mem, AS_PROGRAM, 8, galaxy_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x2037) AM_MIRROR(0x07c0) AM_READ(galaxy_keyboard_r )
	AM_RANGE(0x2038, 0x203f) AM_MIRROR(0x07c0) AM_WRITE(galaxy_latch_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START (galaxyp_mem, AS_PROGRAM, 8, galaxy_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM // ROM A
	AM_RANGE(0x1000, 0x1fff) AM_ROM // ROM B
	AM_RANGE(0x2000, 0x2037) AM_MIRROR(0x07c0) AM_READ(galaxy_keyboard_r )
	AM_RANGE(0x2038, 0x203f) AM_MIRROR(0x07c0) AM_WRITE(galaxy_latch_w )
	AM_RANGE(0xe000, 0xefff) AM_ROM // ROM C
	AM_RANGE(0xf000, 0xffff) AM_ROM // ROM D
ADDRESS_MAP_END

/* 2008-05 FP:
Small note about natural keyboard support. Currently:
- "List" is mapped to 'ESC'
- "Break" is mapped to 'F1'
- "Repeat" is mapped to 'F2'                           */

static INPUT_PORTS_START (galaxy_common)
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)       PORT_CHAR('A')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)       PORT_CHAR('B')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)       PORT_CHAR('C')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)       PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)       PORT_CHAR('E')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)       PORT_CHAR('F')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)       PORT_CHAR('G')

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)       PORT_CHAR('H')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)       PORT_CHAR('I')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)       PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)       PORT_CHAR('K')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)       PORT_CHAR('L')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)       PORT_CHAR('M')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)       PORT_CHAR('N')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)       PORT_CHAR('O')

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)       PORT_CHAR('P')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)       PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)       PORT_CHAR('R')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)       PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)       PORT_CHAR('T')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)       PORT_CHAR('U')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)       PORT_CHAR('V')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)       PORT_CHAR('W')

	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)       PORT_CHAR('X')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)       PORT_CHAR('Y')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)       PORT_CHAR('Z')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)   PORT_CHAR(' ')

	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)       PORT_CHAR('0') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)   PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)   PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('=') PORT_CHAR('-')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("List") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( galaxy )
	PORT_INCLUDE( galaxy_common )
	PORT_START("ROM2")
		PORT_CONFNAME(0x01, 0x01, "ROM 2")
			PORT_CONFSETTING(0x01, "Installed")
			PORT_CONFSETTING(0x00, "Not installed")
INPUT_PORTS_END

static INPUT_PORTS_START( galaxyp )
	PORT_INCLUDE( galaxy_common )
INPUT_PORTS_END

#define XTAL 6144000

/* F4 Character Displayer */
static const gfx_layout galaxy_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8, 10*128*8, 11*128*8, 12*128*8, 13*128*8, 14*128*8, 15*128*8 },
	8                   /* every char takes 1 x 16 bytes */
};

static GFXDECODE_START( galaxy )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxy_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( galaxy, galaxy_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL / 2)
	MCFG_CPU_PROGRAM_MAP(galaxy_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxy_state,  galaxy_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(galaxy_state,galaxy_irq_callback)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MACHINE_RESET_OVERRIDE(galaxy_state, galaxy )

	/* video hardware */
	MCFG_SCREEN_SIZE(384, 212)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 0, 208-1)
	MCFG_SCREEN_UPDATE_DRIVER(galaxy_state, screen_update_galaxy)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galaxy)
	MCFG_PALETTE_ADD_MONOCHROME("palette")


	/* snapshot */
	MCFG_SNAPSHOT_ADD("snapshot", galaxy_state, galaxy, "gal", 0)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(gtp_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("galaxy_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","galaxy")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("6K")
	MCFG_RAM_EXTRA_OPTIONS("2K,22K,38K,54K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( galaxyp, galaxy_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL / 2)
	MCFG_CPU_PROGRAM_MAP(galaxyp_mem)
	MCFG_CPU_IO_MAP(galaxyp_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxy_state,  galaxy_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(galaxy_state,galaxy_irq_callback)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MACHINE_RESET_OVERRIDE(galaxy_state, galaxyp )

	/* video hardware */
	MCFG_SCREEN_SIZE(384, 208)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 0, 208-1)
	MCFG_SCREEN_UPDATE_DRIVER(galaxy_state, screen_update_galaxy)

	MCFG_PALETTE_ADD_MONOCHROME("palette")


	/* snapshot */
	MCFG_SNAPSHOT_ADD("snapshot", galaxy_state, galaxy, "gal", 0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL/4)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(gtp_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("galaxy_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","galaxy")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("38K")
MACHINE_CONFIG_END

ROM_START (galaxy)
	ROM_REGION (0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ("galrom1.bin", 0x0000, 0x1000, CRC(dc970a32) SHA1(dfc92163654a756b70f5a446daf49d7534f4c739))
	ROM_LOAD_OPTIONAL ("galrom2.bin", 0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1))
	ROM_REGION(0x0800, "gfx1",0)
	ROM_LOAD ("galchr.bin", 0x0000, 0x0800, CRC(5c3b5bb5) SHA1(19429a61dc5e55ddec3242a8f695e06dd7961f88))
ROM_END

ROM_START (galaxyp)
	ROM_REGION (0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ("galrom1.bin", 0x0000, 0x1000, CRC(dc970a32) SHA1(dfc92163654a756b70f5a446daf49d7534f4c739))
	ROM_LOAD ("galrom2.bin", 0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1))
	ROM_LOAD ("galplus.bin", 0xe000, 0x1000, CRC(d4cfab14) SHA1(b507b9026844eeb757547679907394aa42055eee))
	ROM_REGION(0x0800, "gfx1",0)
	ROM_LOAD ("galchr.bin", 0x0000, 0x0800, CRC(5c3b5bb5) SHA1(19429a61dc5e55ddec3242a8f695e06dd7961f88))
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY FULLNAME */
COMP(1983,  galaxy,     0,      0,  galaxy, galaxy, galaxy_state,   galaxy, "Voja Antonic / Elektronika inzenjering",          "Galaksija",      0)
COMP(1985,  galaxyp,    galaxy, 0,  galaxyp,galaxyp, galaxy_state,galaxyp,"Nenad Dunjic",            "Galaksija plus", 0)
