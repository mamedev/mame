/***************************************************************************

  /drivers/odyssey2.c

  Driver file to handle emulation of the Odyssey2.

  Minor update to "the voice" rom names, and add comment about
  the older revision of "the voice" - LN, 10/03/08

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/odyssey2.h"
#include "imagedev/cartslot.h"
#include "sound/sp0256.h"

static ADDRESS_MAP_START( odyssey2_mem , AS_PROGRAM, 8, odyssey2_state )
	AM_RANGE( 0x0000, 0x03FF) AM_ROM
	AM_RANGE( 0x0400, 0x0BFF) AM_RAMBANK("bank1")
	AM_RANGE( 0x0C00, 0x0FFF) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( odyssey2_io , AS_IO, 8, odyssey2_state )
	AM_RANGE( 0x00,		 0xff)		AM_READWRITE(odyssey2_bus_r, odyssey2_bus_w)
	AM_RANGE( MCS48_PORT_P1,	MCS48_PORT_P1)	AM_READWRITE(odyssey2_getp1, odyssey2_putp1 )
	AM_RANGE( MCS48_PORT_P2,	MCS48_PORT_P2)	AM_READWRITE(odyssey2_getp2, odyssey2_putp2 )
	AM_RANGE( MCS48_PORT_BUS,	MCS48_PORT_BUS)	AM_READWRITE(odyssey2_getbus, odyssey2_putbus )
	AM_RANGE( MCS48_PORT_T0,	MCS48_PORT_T0)  AM_READ(odyssey2_t0_r )
	AM_RANGE( MCS48_PORT_T1,	MCS48_PORT_T1)	AM_READ(odyssey2_t1_r )
ADDRESS_MAP_END

static ADDRESS_MAP_START( g7400_io , AS_IO, 8, odyssey2_state )
	AM_RANGE( 0x00,      0xff)      AM_READWRITE(g7400_bus_r, g7400_bus_w)
	AM_RANGE( MCS48_PORT_P1,	MCS48_PORT_P1)  AM_READWRITE(odyssey2_getp1, odyssey2_putp1 )
	AM_RANGE( MCS48_PORT_P2,	MCS48_PORT_P2)  AM_READWRITE(odyssey2_getp2, odyssey2_putp2 )
	AM_RANGE( MCS48_PORT_BUS,	MCS48_PORT_BUS) AM_READWRITE(odyssey2_getbus, odyssey2_putbus )
	AM_RANGE( MCS48_PORT_T0,	MCS48_PORT_T0)  AM_READ(odyssey2_t0_r )
	AM_RANGE( MCS48_PORT_T1,	MCS48_PORT_T1)  AM_READ(odyssey2_t1_r )
ADDRESS_MAP_END

static INPUT_PORTS_START( odyssey2 )
	PORT_START("KEY0")		/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY1")		/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("?? :") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("?? $") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("KEY2")		/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("KEY3")		/* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("KEY4")		/* IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')

	PORT_START("KEY5")		/* IN5 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( Yes )) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( No )) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')

	PORT_START("JOY0")		/* IN6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)		PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)	PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)	PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)	PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1)			PORT_PLAYER(1)
	PORT_BIT( 0xe0, 0xe0,	 IPT_UNUSED )

	PORT_START("JOY1")		/* IN7 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)		PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)	PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)	PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)	PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1)			PORT_PLAYER(2)
	PORT_BIT( 0xe0, 0xe0,	 IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout odyssey2_graphicslayout =
{
	8,1,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	},
	/* y offsets */
	{ 0 },
	1*8
};


static const gfx_layout odyssey2_spritelayout =
{
	8,1,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{
	7,6,5,4,3,2,1,0
	},
	/* y offsets */
	{ 0 },
	1*8
};

static GFXDECODE_START( odyssey2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, odyssey2_graphicslayout, 0, 2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, odyssey2_spritelayout, 0, 2 )
GFXDECODE_END

static const sp0256_interface the_voice_sp0256 =
{
	DEVCB_LINE(odyssey2_the_voice_lrq_callback),
	DEVCB_NULL
};

static MACHINE_CONFIG_FRAGMENT( odyssey2_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( odyssey2, odyssey2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, ( ( XTAL_7_15909MHz * 3 ) / 4 ) )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(odyssey2_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))


    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( XTAL_7_15909MHz/2, I824X_LINE_CLOCKS, I824X_START_ACTIVE_SCAN, I824X_END_ACTIVE_SCAN, 262, I824X_START_Y, I824X_START_Y + I824X_SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_STATIC( odyssey2 )

	MCFG_GFXDECODE( odyssey2 )
	MCFG_PALETTE_LENGTH(24)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", ODYSSEY2, XTAL_7_15909MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("sp0256_speech", SP0256, 3120000)
	MCFG_SOUND_CONFIG(the_voice_sp0256)
	/* The Voice uses a speaker with its own volume control so the relative volumes to use are subjective, these sound good */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( videopac, odyssey2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, ( XTAL_17_73447MHz / 3 ) )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(odyssey2_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( XTAL_17_73447MHz/5, I824X_LINE_CLOCKS, I824X_START_ACTIVE_SCAN, I824X_END_ACTIVE_SCAN, 312, I824X_START_Y, I824X_START_Y + I824X_SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_STATIC( odyssey2 )

	MCFG_GFXDECODE( odyssey2 )
	MCFG_PALETTE_LENGTH(24)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", ODYSSEY2, XTAL_17_73447MHz/5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("sp0256_speech", SP0256, 3120000)
	MCFG_SOUND_CONFIG(the_voice_sp0256)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( g7400, odyssey2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, 5911000 )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(g7400_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( 3547000*2, 448, 96, 416, 312, 39, 289 )	/* EF9340 doubles the input clock into dot clocks internally */
    MCFG_SCREEN_UPDATE_STATIC( odyssey2 )

	MCFG_GFXDECODE( odyssey2 )
	MCFG_PALETTE_LENGTH(24)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", ODYSSEY2, 3547000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
MACHINE_CONFIG_END

ROM_START (odyssey2)
    ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
    ROM_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee))
    ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

    ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)

	ROM_REGION( 0x10000, "sp0256_speech", 0 )
	/* SP0256B-019 Speech chip w/2KiB mask rom */
	ROM_LOAD( "sp0256b-019.bin",   0x1000, 0x0800, CRC(4bb43724) SHA1(49f5326ad45392dc96c89d1d4e089a20bd21e609) )

	/* A note about "The Voice": Two versions of "The Voice" exist:
       * An earlier version with eight 2KiB speech roms, spr016-??? through spr016-??? on a small daughterboard
       <note to self: fill in numbers later>
       * A later version with one 16KiB speech rom, spr128-003, mounted directly on the mainboard
       The rom contents of these two versions are EXACTLY the same.
       Both versions have an sp0256b-019 speech chip, which has 2KiB of its own internal speech data
       Thanks to kevtris for this info. - LN
    */
	/* External 16KiB speech ROM (spr128-003) from "The Voice" */
	ROM_LOAD( "spr128-003.bin",   0x4000, 0x4000, CRC(509367b5) SHA1(0f31f46bc02e9272885779a6dd7102c78b18895b) )
	/* Additional External 16KiB ROM (spr128-004) from S.I.D. the Spellbinder */
	ROM_LOAD( "spr128-004.bin",   0x8000, 0x4000, CRC(e79dfb75) SHA1(37f33d79ffd1739d7c2f226b010a1eac28d74ca0) )
ROM_END

ROM_START (videopac)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_SYSTEM_BIOS( 0, "g7000", "g7000" )
	ROMX_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "c52", "c52" )
	ROMX_LOAD ("c52.bin", 0x0000, 0x0400, CRC(a318e8d6) SHA1(a6120aed50831c9c0d95dbdf707820f601d9452e), ROM_BIOS(2))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)

	ROM_REGION( 0x10000, "sp0256_speech", 0 )
	/* SP0256B-019 Speech chip w/2KiB mask rom */
	ROM_LOAD( "sp0256b-019.bin",   0x1000, 0x0800, CRC(4bb43724) SHA1(49f5326ad45392dc96c89d1d4e089a20bd21e609) )
	/* External 16KiB speech ROM (spr128-003) from "The Voice" */
	ROM_LOAD( "spr128-003.bin",   0x4000, 0x4000, CRC(509367b5) SHA1(0f31f46bc02e9272885779a6dd7102c78b18895b) )
	/* Additional External 16KiB speech ROM (spr128-004) from S.I.D. the Spellbinder */
	ROM_LOAD( "spr128-004.bin",   0x8000, 0x4000, CRC(e79dfb75) SHA1(37f33d79ffd1739d7c2f226b010a1eac28d74ca0) )
ROM_END

ROM_START (g7400)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("g7400.bin", 0x0000, 0x0400, CRC(e20a9f41) SHA1(5130243429b40b01a14e1304d0394b8459a6fbae))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)
ROM_END

ROM_START (jopac)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("jopac.bin", 0x0000, 0x0400, CRC(11647ca5) SHA1(54b8d2c1317628de51a85fc1c424423a986775e4))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)
ROM_END

/*     YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     INIT      COMPANY     FULLNAME     FLAGS */
COMP( 1978, odyssey2, 0,		0,		odyssey2, odyssey2, odyssey2_state, odyssey2, "Magnavox", "Odyssey 2", 0 )
COMP( 1979, videopac, odyssey2,	0,		videopac, odyssey2, odyssey2_state, odyssey2, "Philips", "Videopac G7000/C52", 0 )
COMP( 1983, g7400, odyssey2, 0,			g7400,    odyssey2, odyssey2_state, odyssey2, "Philips", "Videopac Plus G7400", GAME_NOT_WORKING )
COMP( 1983, jopac, odyssey2, 0,			g7400,    odyssey2, odyssey2_state, odyssey2, "Brandt", "Jopac JO7400", GAME_NOT_WORKING )


