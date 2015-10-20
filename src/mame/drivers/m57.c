// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/****************************************************************************

    Irem M57 hardware

*****************************************************************************

    Tropical Angel

    driver by Phil Stroffolino

    IREM M57 board stack with a M52-SOUND-E sound PCB.

    M57-A-A:
     TA-A-xx roms and proms
     NEC D780C (Z80) CPU
     NANAO KNA6032601 custom chip
     NANAO KNA6032701 custom chip
     8-way dipswitch (x2)
     M58725P RAM (x3)
     CN1 - Ribbon cable connector
     CN2 - Ribbon cable connector
     Ribbon cable connector to sound PCB

    M57-B-A:
     TA-B-xx roms and proms
     18.432 MHz OSC
     CN1 - Ribbon cable connector
     CN2 - Ribbon cable connector

    M52:
     HD6803 CPU
     AY-3-9810 (x2) sound chips
     MSM5205 OKI sound chip (and an unpopulated socket for a second MSM5202)
     3.579545 MHz OSC
     2764 Program rom labeled "TA S-1A-"
     Ribbon cable connector to M57-A-A PCB

    New Tropical Angel:
     Roms were found on an official IREM board with genuine IREM Tropical Angel
     license seal and genuine IREM serial number sticker.
     The "new" roms have hand written labels, while those that match the current
     Tropical Angel set look to be factory labeled chips.

*****************************************************************************

    Locations based on m58.c driver

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/irem.h"
#include "includes/iremipt.h"
#include "includes/m57.h"


#define MASTER_CLOCK        XTAL_18_432MHz



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, m57_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(m57_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x91ff) AM_RAM AM_SHARE("scrollram")
	AM_RANGE(0xc820, 0xc8ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE("irem_audio", irem_audio_device, cmd_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(m57_flipscreen_w) /* + coin counters */
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("IN0")
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("IN1")
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("IN2")
	AM_RANGE(0xd003, 0xd003) AM_READ_PORT("DSW1")
	AM_RANGE(0xd004, 0xd004) AM_READ_PORT("DSW2")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

/* Same as m52, m58 and m62 (IREM Z80 hardware) */
static INPUT_PORTS_START( m57 )
	PORT_START("IN0")
	/* Start 1 & 2 also restarts and freezes the game with stop mode on
	   and are used in test mode to enter and esc the various tests */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* coin input must be active for 19 frames to be consistently recognized */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	/* DSW1 is so different from game to game that it isn't included here */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/*************************************
 *
 *  Games port definitions
 *
 *************************************/

static INPUT_PORTS_START( troangel )
	PORT_INCLUDE(m57)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN PORT_COCKTAIL */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP   PORT_COCKTAIL */

	PORT_MODIFY("DSW2")
	/* TODO: the following enables an analog accelerator input read from 0xd003 */
	/* however that is the DSW1 input so it must be multiplexed some way */
	PORT_DIPNAME( 0x08, 0x08, "Analog Accelarator" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Time" ) PORT_DIPLOCATION("SW1:1,2") /* table at 0x6110 - 4 * 8 bytes (B1 B2 bonus A1 A2 bonus M1 M2) */
	PORT_DIPSETTING(    0x03, "B:180/A:160/M:140/BG:120" )
	PORT_DIPSETTING(    0x02, "B:160/A:140/M:120/BG:100" )
	PORT_DIPSETTING(    0x01, "B:140/A:120/M:100/BG:80" )
	PORT_DIPSETTING(    0x00, "B:120/A:100/M:100/BG:80" )
	PORT_DIPNAME( 0x04, 0x04, "Crash Loss Time" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x08, 0x08, "Background Sound" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Boat Motor" )
	PORT_DIPSETTING(    0x00, "Music" )
	IREM_Z80_COINAGE_TYPE_2_LOC(SW1)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,32,
	64,
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8), STEP16(256*64,8) },
	32*8
};

static GFXDECODE_START( m57 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x3_planar,    0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout, 32*8, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x1000, spritelayout, 32*8, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x2000, spritelayout, 32*8, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x3000, spritelayout, 32*8, 32 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( m57, m57_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", m57_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1790)   /* accurate frequency, measured on a Moon Patrol board, is 56.75Hz. */)
				/* the Lode Runner manual (similar but different hardware) */
				/* talks about 55Hz and 1790ms vblank duration. */
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(m57_state, screen_update_m57)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", m57)
	MCFG_PALETTE_ADD("palette", 32*8+32*8)
	MCFG_PALETTE_INDIRECT_ENTRIES(256+16)
	MCFG_PALETTE_INIT_OWNER(m57_state, m57)

	/* sound hardware */
	//MCFG_FRAGMENT_ADD(m52_sound_c_audio)
	MCFG_DEVICE_ADD("irem_audio", IREM_M52_SOUNDC_AUDIO, 0)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( troangel )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "ta-a-3k", 0x0000, 0x2000, CRC(f21f8196) SHA1(7cbf74b77a559ee70312b799e707394d9b849f5b) )
	ROM_LOAD( "ta-a-3m", 0x2000, 0x2000, CRC(58801e55) SHA1(91bdda778f2c4486001bc4ad26d6f21ba275ae08) )
	ROM_LOAD( "ta-a-3n", 0x4000, 0x2000, CRC(de3dea44) SHA1(1290755ffc04dc3b3667e063118669a0eab6fb79) )
	ROM_LOAD( "ta-a-3q", 0x6000, 0x2000, CRC(fff0fc2a) SHA1(82f3f5a8817e956192323eb555daa85b7766676d) )

	ROM_REGION(  0x8000 , "irem_audio:iremsound", 0 )  /* sound CPU */
	ROM_LOAD( "ta-s-1a", 0x6000, 0x2000, CRC(15a83210) SHA1(8ada510db689ffa372b2f4dc4bd1b1c69a0c5307) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "ta-a-3e", 0x00000, 0x2000, CRC(e49f7ad8) SHA1(915de1084fd3c5fc81dd8c80107c28cc57b33226) )
	ROM_LOAD( "ta-a-3d", 0x02000, 0x2000, CRC(06eef241) SHA1(4f327a54169046d8d84b5f5cf5d9f45e1df4dae6) )
	ROM_LOAD( "ta-a-3c", 0x04000, 0x2000, CRC(7ff5482f) SHA1(fe8c181fed113007d69d11e8aa467e86a6357ffb) )    /* characters */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "ta-b-5j", 0x00000, 0x2000, CRC(86895c0c) SHA1(b42b041e3e20dadd8411805d492133d371426ebf) )    /* sprites */
	ROM_LOAD( "ta-b-5h", 0x02000, 0x2000, CRC(f8cff29d) SHA1(dabf3bbf50f73a381056131c2239c84dd966b63e) )
	ROM_LOAD( "ta-b-5e", 0x04000, 0x2000, CRC(8b21ee9a) SHA1(1272722211d22d5b153e9415cc189a5aa9028543) )
	ROM_LOAD( "ta-b-5d", 0x06000, 0x2000, CRC(cd473d47) SHA1(854cb532bd62851a206da2affd66a1257b7085b6) )
	ROM_LOAD( "ta-b-5c", 0x08000, 0x2000, CRC(c19134c9) SHA1(028660e66fd033473c468b694e870c633ca05ec6) )
	ROM_LOAD( "ta-b-5a", 0x0a000, 0x2000, CRC(0012792a) SHA1(b4380f5fbe5e9ce9b44f87ce48a8b402bab58b52) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "ta-a-5a", 0x0000, 0x0100, CRC(01de1167) SHA1(b9070f8c70eb362fc4d6a0a92235ce0a5b2ab858) ) /* chars palette low 4 bits */
	ROM_LOAD( "ta-a-5b", 0x0100, 0x0100, CRC(efd11d4b) SHA1(7c7c356063ab35e4ffb8d65cd20c27c2a4b36537) ) /* chars palette high 4 bits */
	ROM_LOAD( "ta-b-1b", 0x0200, 0x0020, CRC(f94911ea) SHA1(ad61a323476a97156a255a72048a28477b421284) ) /* sprites palette */
	ROM_LOAD( "ta-b-3d", 0x0220, 0x0100, CRC(ed3e2aa4) SHA1(cfdfc151803080d1ecdd04af1bfea3dbdce8dca0) ) /* sprites lookup table */
ROM_END

ROM_START( newtangl ) /* Offical "upgrade" or hack? */
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "3k", 0x0000, 0x2000, CRC(3c6299a8) SHA1(a21a8452b75ce6174076878128d4f20b39b6d69d) )
	ROM_LOAD( "3m", 0x2000, 0x2000, CRC(8d09056c) SHA1(4d2585103cc6e6c04015501d3c9e1578a8f9c0f5) )
	ROM_LOAD( "3n", 0x4000, 0x2000, CRC(17b5a775) SHA1(d85c3371080bea82f19ac96fa0f1b332e1c86e27) )
	ROM_LOAD( "3q", 0x6000, 0x2000, CRC(2e5fa773) SHA1(9a34fa43bde021fc7b00d8c3762c248e7b96dbf1) )

	ROM_REGION(  0x8000 , "irem_audio:iremsound", 0 )  /* sound CPU */
	ROM_LOAD( "ta-s-1a-", 0x6000, 0x2000, CRC(ea8a05cb) SHA1(5683e4dca93066ee788287ab73a766fa303ebe84) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "ta-a-3e", 0x00000, 0x2000, CRC(e49f7ad8) SHA1(915de1084fd3c5fc81dd8c80107c28cc57b33226) )
	ROM_LOAD( "ta-a-3d", 0x02000, 0x2000, CRC(06eef241) SHA1(4f327a54169046d8d84b5f5cf5d9f45e1df4dae6) )
	ROM_LOAD( "ta-a-3c", 0x04000, 0x2000, CRC(7ff5482f) SHA1(fe8c181fed113007d69d11e8aa467e86a6357ffb) )    /* characters */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "5j",      0x00000, 0x2000, CRC(89409130) SHA1(3f37f820b1b86166cde7c039d657ebd036d490dd) )    /* sprites */
	ROM_LOAD( "ta-b-5h", 0x02000, 0x2000, CRC(f8cff29d) SHA1(dabf3bbf50f73a381056131c2239c84dd966b63e) )
	ROM_LOAD( "5e",      0x04000, 0x2000, CRC(5460a467) SHA1(505c1d9e69c39a74369da17f354b90486ee6afcd) )
	ROM_LOAD( "ta-b-5d", 0x06000, 0x2000, CRC(cd473d47) SHA1(854cb532bd62851a206da2affd66a1257b7085b6) )
	ROM_LOAD( "5c",      0x08000, 0x2000, CRC(4a20637a) SHA1(74099cb7f1727c2de2f066497097f1a9eeec0cea) )
	ROM_LOAD( "ta-b-5a", 0x0a000, 0x2000, CRC(0012792a) SHA1(b4380f5fbe5e9ce9b44f87ce48a8b402bab58b52) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "ta-a-5a", 0x0000, 0x0100, CRC(01de1167) SHA1(b9070f8c70eb362fc4d6a0a92235ce0a5b2ab858) ) /* chars palette low 4 bits */
	ROM_LOAD( "ta-a-5b", 0x0100, 0x0100, CRC(efd11d4b) SHA1(7c7c356063ab35e4ffb8d65cd20c27c2a4b36537) ) /* chars palette high 4 bits */
	ROM_LOAD( "ta-b-1b", 0x0200, 0x0020, CRC(f94911ea) SHA1(ad61a323476a97156a255a72048a28477b421284) ) /* sprites palette */
	ROM_LOAD( "ta-b-3d", 0x0220, 0x0100, CRC(ed3e2aa4) SHA1(cfdfc151803080d1ecdd04af1bfea3dbdce8dca0) ) /* sprites lookup table */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, troangel, 0,        m57,   troangel, driver_device, 0, ROT0, "Irem", "Tropical Angel", MACHINE_SUPPORTS_SAVE )
GAME( 1983, newtangl, troangel, m57,   troangel, driver_device, 0, ROT0, "Irem", "New Tropical Angel", MACHINE_SUPPORTS_SAVE )
