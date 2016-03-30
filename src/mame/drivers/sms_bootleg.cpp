// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

this is an SMS multi-game bootleg with 32 games


the dump is incomplete; there is a chip near a position marked on the board as ROM4 with the following markings
"SG11004A
79ST0086END
9045"
it should be a 512KB MASK rom containing 11 of the games.

there are also empty k9/k10/k11/k12 positions, but they were clearly never used.


Games ordered by MENU

Super Bubble    ( port08_w 00 )
Tetris   << ??  ( port08_w ac )
Wonder Boy << ??  ( port08_w cc )
Alex Kidd ( port08_w 07 )
Super Mario  << ?? ( port08_w 3c )
Hello Kang Si ( port08_w 0d )
Solomon Key ( port08_w 02 )
Buk Doo Gun ( port08_w 0e )

Invaders  << ?? ( port08_w 1c )
Galaxian ( port08_w 14 )
Galaga  << ?? ( port08_w 6c )
Flicky  ( port08_w 04 )
Teddy Boy ( port08_w 25 )
Ghost House << ?? ( port08_w 4c )
Bomb Jack << ?? ( port08_w 5c )
Kings Vally ( port08_w 24 )

Pippols  ( port08_w 34 )
Dragon Story ( port08_w 05 )
Spy vs Spy ( port08_w 15 )
PitFall II ( port08_w 35 )
Drol ( port08_w 06 )
Pit Pot ( port08_w 16 )
Hyper Sport ( port08_w 26 )
Super Tank ( port08_w 36 )

Congo Bongo ( port08_w 23 )
Circus ( port08_w 33 )
Road Fighter << ?? ( port08_w 9c )
Astro << ?? ( port08_w 2c )
Goonies << ?? ( port08_w 7c )
Road Runner I << ?? ( port08_w 8c )
Masic Tree ( port08_w 03 )
Mouse ( port08_w 13 )


list reordered based on ROMs and banking  (the low 4 bits are clear the ROM select, the upper 4 bits are the bank select)
you can clearly see

ROM K1.bin  / ROM K2.bin
Super Bubble     ( port08_w 00 ) (8 banks - 2 roms)

ROM K3.bin
Solomon Key      ( port08_w 02 ) (all 4 banks)

ROM K4.bin
Masic Tree       ( port08_w 03 )
Mouse            ( port08_w 13 )
Congo Bongo      ( port08_w 23 )
Circus           ( port08_w 33 )

ROM K5.bin
Flicky           ( port08_w 04 )
Galaxian         ( port08_w 14 )
Kings Vally      ( port08_w 24 )
Pippols          ( port08_w 34 )

ROM K6.bin
Dragon Story     ( port08_w 05 )
Spy vs Spy       ( port08_w 15 )
Teddy Boy        ( port08_w 25 )
PitFall II       ( port08_w 35 )

ROM K7.bin
Drol             ( port08_w 06 )
Pit Pot          ( port08_w 16 )
Hyper Sport      ( port08_w 26 )
Super Tank       ( port08_w 36 )

ROM K8.bin
Alex Kidd        ( port08_w 07 ) (all 4 banks)

port08_w x8 would be K9 (unpopulated)
port08_w x9 would be K10 (unpopulated)
port08_w xa would be K11 (unpopulated)
port08_w xb would be K12 (unpopulated)


ROM rom4.bin? (MISSING FROM DUMP!)
Invaders  << ??      ( port08_w 1c )
Astro << ??          ( port08_w 2c )
Super Mario  << ??   ( port08_w 3c )
Ghost House << ??    ( port08_w 4c )
Bomb Jack << ??      ( port08_w 5c )
Galaga  << ??        ( port08_w 6c )
Goonies << ??        ( port08_w 7c )
Road Runner I << ??  ( port08_w 8c )
Road Fighter << ??   ( port08_w 9c )
Tetris   << ??       ( port08_w ac )
Wonder Boy << ??     ( port08_w cc )

ROM rom3.bin
Hello Kang Si    ( port08_w 0d )

ROM rom2.bin
Buk Doo Gun      ( port08_w 0e )



A Korean version has been seen too (unless this can be switched?)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/2413intf.h"
#include "video/315_5124.h"
#include "includes/sms_bootleg.h"



static ADDRESS_MAP_START( sms_supergame_map, AS_PROGRAM, 8, smsbootleg_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xfff7) AM_RAM
//  AM_RANGE(0xfffc, 0xffff) AM_READWRITE(sms_mapper_r, sms_mapper_w)       /* Bankswitch control */
ADDRESS_MAP_END

WRITE8_MEMBER(smsbootleg_state::port08_w)
{
	printf("port08_w %02x\n", data);
}

WRITE8_MEMBER(smsbootleg_state::port18_w)
{
	printf("port18_w %02x\n", data);
}


static ADDRESS_MAP_START( sms_supergame_io, AS_IO, 8, smsbootleg_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x04, 0x04) AM_READNOP //AM_READ_PORT("IN0") // these
	AM_RANGE(0x08, 0x08) AM_WRITE(port08_w)
	AM_RANGE(0x14, 0x14) AM_READNOP //AM_READ_PORT("IN1") // seem to be from a coinage / timer MCU, changing them directly changes the credits / time value
	AM_RANGE(0x18, 0x18) AM_WRITE(port18_w)

	AM_RANGE(0x40, 0x7f)                 AM_READWRITE(sms_count_r, sms_psg_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, register_read, register_write)

	AM_RANGE(0xdc, 0xdc) AM_READ_PORT("IN2")
ADDRESS_MAP_END



static MACHINE_CONFIG_START( sms_supergame, smsbootleg_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_53_693175MHz/15) // check - I can only see a 10.738635 on this PCB
	MCFG_CPU_PROGRAM_MAP(sms_supergame_map)
	MCFG_CPU_IO_MAP(sms_supergame_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("segapsg", SEGAPSG, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH , SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_REFRESH_RATE((double) XTAL_53_693175MHz/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_NTSC))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms)

	MCFG_DEVICE_ADD("sms_vdp", SEGA315_5246, 0)
	MCFG_SEGA315_5246_SET_SCREEN("screen")
	MCFG_SEGA315_5246_IS_PAL(false)
	MCFG_SEGA315_5246_INT_CB(WRITELINE(sms_state, sms_int_callback))
	MCFG_SEGA315_5246_PAUSE_CB(WRITELINE(sms_state, sms_pause_callback))

MACHINE_CONFIG_END





static INPUT_PORTS_START( sms_supergame )
	PORT_START("PAUSE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )// PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1)
#if 0
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
#endif
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

DRIVER_INIT_MEMBER(smsbootleg_state,sms_supergame)
{
	UINT8* rom = memregion("maincpu")->base();
	size_t size = memregion("maincpu")->bytes();

	for (int i = 0;i < size;i++)
	{
		rom[i] ^= 0x80;
	}
}


ROM_START( smssgame )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "Rom1.bin", 0x00000, 0x10000, CRC(0e1f258e) SHA1(9240dc0d01e3061c0c8807c07c0a1d033ebe9116) ) // yes, this rom is smaller (menu rom)

	ROM_LOAD( "rom2.bin",0x020000, 0x20000, CRC(c1478323) SHA1(27b524a234f072e81ef41fb89a5fff5617e9b951) ) // Buk Doo Sun
	ROM_LOAD( "rom3.bin",0x040000, 0x20000, CRC(96c8705d) SHA1(ba4f4af0cfdad1d63a08201ed186c79aea062b95) ) // ? Kung Fu game (Hello Kang Si?)
	// there is something in the position marked ROM4.

	// k12 unpopulated
	// k11 unpopulated
	// k10 unpopulated
	// k9 unpopulated
	ROM_LOAD( "K8.bin",  0x060000, 0x20000, CRC(eb1e8693) SHA1(3283cdcfc25f34a43f317093cd39e10a52bc3ae7) ) // Alex Kidd in Miracle World
	ROM_LOAD( "K7.bin",  0x080000, 0x20000, CRC(d24da417) SHA1(2ef5e55748e412157e55e7a62355fd66bf792d8e) ) // Drol / Pit Pot / Hyper Sports II / Super Tank
	ROM_LOAD( "K6.bin",  0x0a0000, 0x20000, CRC(d78ce5ba) SHA1(06065cec3865ff3a2bb2f56702b24427487964e2) ) // Dragon Story / Spy Vs Spy / Teddyboy Blues / Pitfall II
	ROM_LOAD( "K5.bin",  0x0c0000, 0x20000, CRC(a7b64d1c) SHA1(7c37ac3f37699c49492d4f4ea4e213670413041c) ) // Flicky / Galaxian / King's Valley / Pippos
	ROM_LOAD( "K4.bin",  0x0e0000, 0x20000, CRC(e5903942) SHA1(d0c02f4b37c8a02142868459af14ba8ed0340ccd) ) // Magical Tree / Chustle Chumy / Congo Bongo / Charlie Circus
	ROM_LOAD( "K3.bin",  0x100000, 0x20000, CRC(9bb92096) SHA1(3ca17b7a9aa20b97cac1f78ba13f70bed1b37463) ) // Solomon's Key
	ROM_LOAD( "K2.bin",  0x120000, 0x20000, CRC(a12439f4) SHA1(e957d4fe275e982bedef28af8cc2957da27dc512) ) // Final Bubble Bobble (1/2)
	ROM_LOAD( "K1.bin",  0x140000, 0x20000, CRC(dadffecd) SHA1(68ebb968539049a9e193da5200856b9f956f7e02) ) // Final Bubble Bobble (2/2)

	ROM_LOAD( "rom4.bin",0x180000, 0x80000, NO_DUMP ) // missing

	// there seems to be some kind of MCU for the timer?
ROM_END



GAME( 199?, smssgame,  0,    sms_supergame, sms_supergame, smsbootleg_state,  sms_supergame, ROT0, "Sono Corp Japan", "Super Game (Sega Master System Multi-game bootleg)", MACHINE_NOT_WORKING )
