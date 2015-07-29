// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/*

Galaxia by Zaccaria (1979)
Also released in several regions as "Super Galaxians".
Set regions are unknown, so all are currently named Galaxia.

Taken from an untested board.

1K byte files were 2708 or equivalent.
512 byte file is a 82S130 PROM.

This is not a direct pirate of Galaxian as you might think from the name.
The game uses a Signetics 2650A CPU with three 40-pin 2636 chips, which are
responsible for basic sound and some video functions.

Other than that, the video hardware looks like it's similar to Galaxian
(2 x 2114, 2 x 2101, 2 x EPROM) but there is no attack RAM and the graphics
EPROMS are 2708. The graphics EPROMS do contain Galaxian-like graphics...

Quick PCB sketch:
  ------------------------------------------------------------------------------
  |                                                                            |
  |                   13l    13i    13h                                        |
  |                                                                            |
  |    PROM           11l    11i    11h      S2636            S2621    XTAL    |
|6-|                                                                 14.31818  |
  |                          10i    10h      S2636                             |
|5-|                                                                           |
|--|                          8i     8h      S2636                   S2650A    |
|--|                                                                           |
|--|                                                                           |
  |                                                                            |
|4-|                                                                           |
|--|                                                                           |
|--|                                                                           |
|--|                                                                           |
  |                                                                            |
|3-|                                                                           |
|--|                                                                           |
|--|                                                                           |
|--|       DSW                                                                 |
  |                                                     3d                     |
|2-|       DSW                                                                |-1|
  |                                                     1d                    |--|
  |                                                                           |--|
  ------------------------------------------------------------------------------

Astro Wars (port of Astro Fighter) is on a stripped down board of Galaxia,
using only one 2636 chip, less RAM, and no PROM.

Manual and Schematic for Galaxia can be found at:
http://www.zzzaccaria.com/manuals/SuperGalaxiansTechnicalManual.zip
http://www.zzzaccaria.com/manuals/GalaxiaSchematics.zip

HW has many similarities with quasar.c / cvs.c / zac2650.c
---

TODO:
- correct color/star generation using info from Galaxia technical manual and schematics
- add sound board emulation
- improve bullets
- accurate sprite/bg sync in astrowar

*/

#include "emu.h"
#include "machine/s2636.h"
#include "cpu/s2650/s2650.h"
#include "includes/galaxia.h"


INTERRUPT_GEN_MEMBER(galaxia_state::galaxia_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x03);
	cvs_scroll_stars();
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(galaxia_state::galaxia_video_w)
{
//  m_screen->update_partial(m_screen->vpos());
	m_bg_tilemap->mark_tile_dirty(offset);
	cvs_video_or_color_ram_w(space, offset, data);
}

WRITE8_MEMBER(galaxia_state::galaxia_scroll_w)
{
	m_screen->update_partial(m_screen->vpos());

	// fixed scrolling area
	for (int i = 1; i < 6; i++)
		m_bg_tilemap->set_scrolly(i, data);
}

WRITE8_MEMBER(galaxia_state::galaxia_ctrlport_w)
{
	// d0/d1: maybe coincounter
	// other bits: unknown
}

WRITE8_MEMBER(galaxia_state::galaxia_dataport_w)
{
	// cvs-style video fx? or lamps?
}

READ8_MEMBER(galaxia_state::galaxia_collision_r)
{
	m_screen->update_partial(m_screen->vpos());
	return m_collision_register;
}

READ8_MEMBER(galaxia_state::galaxia_collision_clear)
{
	m_screen->update_partial(m_screen->vpos());
	m_collision_register = 0;
	return 0xff;
}

static ADDRESS_MAP_START( galaxia_mem_map, AS_PROGRAM, 8, galaxia_state )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_RAM AM_SHARE("bullet_ram")
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_0", s2636_device, work_ram_r, work_ram_w)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_1", s2636_device, work_ram_r, work_ram_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_2", s2636_device, work_ram_r, work_ram_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_READ(cvs_video_or_color_ram_r) AM_WRITE(galaxia_video_w) AM_SHARE("video_ram")
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x7214, 0x7214) AM_READ_PORT("IN0")
ADDRESS_MAP_END

static ADDRESS_MAP_START( astrowar_mem_map, AS_PROGRAM, 8, galaxia_state )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_0", s2636_device, work_ram_r, work_ram_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_READ(cvs_video_or_color_ram_r) AM_WRITE(galaxia_video_w)  AM_SHARE("video_ram")
	AM_RANGE(0x1c00, 0x1cff) AM_MIRROR(0x6000) AM_RAM AM_SHARE("bullet_ram")
	AM_RANGE(0x2000, 0x33ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( galaxia_io_map, AS_IO, 8, galaxia_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_WRITE(galaxia_scroll_w) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x05, 0x05) AM_READNOP
	AM_RANGE(0x06, 0x06) AM_READ_PORT("DSW0")
	AM_RANGE(0x07, 0x07) AM_READ_PORT("DSW1")
	AM_RANGE(0xac, 0xac) AM_READNOP
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE(galaxia_collision_r, galaxia_ctrlport_w)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE(galaxia_collision_clear, galaxia_dataport_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( galaxia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc3, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1C_1C B 2C_1C" )
	PORT_DIPSETTING(    0x01, "A 1C_2C B 2C_1C" )
	PORT_DIPSETTING(    0x02, "A 1C_3C B 2C_1C" )
	PORT_DIPSETTING(    0x03, "A 1C_5C B 2C_1C" )
	PORT_DIPSETTING(    0x04, "A 1C_1C B 1C_1C" )
	PORT_DIPSETTING(    0x05, "A 1C_2C B 1C_1C" )
	PORT_DIPSETTING(    0x06, "A 1C_3C B 1C_1C" )
	PORT_DIPSETTING(    0x07, "A 1C_5C B 1C_1C" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )

	PORT_DIPNAME( 0x10, 0x00, "UNK04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "UNK05" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "UNK06" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "UNK07" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "UNK10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "UNK11" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "UNK12" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "UNK13" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x00, "UNK14" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "UNK15" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "UNK16" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "UNK17" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


/***************************************************************************

  Machine Configs

***************************************************************************/

static const gfx_layout tiles8x8x1_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles8x8x2_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( galaxia )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x2_layout, 0, 4 )
GFXDECODE_END

static GFXDECODE_START( astrowar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x1_layout, 0, 8 )
GFXDECODE_END


static MACHINE_CONFIG_START( galaxia, galaxia_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, XTAL_14_31818MHz/8)
	MCFG_CPU_PROGRAM_MAP(galaxia_mem_map)
	MCFG_CPU_IO_MAP(galaxia_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxia_state, galaxia_interrupt)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(cvs_state, write_s2650_flag))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 30*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(galaxia_state, screen_update_galaxia)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galaxia)
	MCFG_PALETTE_ADD("palette", 0x18+2)

	MCFG_PALETTE_INIT_OWNER(galaxia_state,galaxia)
	MCFG_VIDEO_START_OVERRIDE(galaxia_state,galaxia)

	MCFG_DEVICE_ADD("s2636_0", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(3, -26)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DEVICE_ADD("s2636_1", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(3, -26)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DEVICE_ADD("s2636_2", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(3, -26)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( astrowar, galaxia_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, XTAL_14_31818MHz/8)
	MCFG_CPU_PROGRAM_MAP(astrowar_mem_map)
	MCFG_CPU_IO_MAP(galaxia_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxia_state, galaxia_interrupt)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(cvs_state, write_s2650_flag))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(galaxia_state, screen_update_astrowar)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", astrowar)
	MCFG_PALETTE_ADD("palette", 0x18+2)

	MCFG_PALETTE_INIT_OWNER(galaxia_state,astrowar)
	MCFG_VIDEO_START_OVERRIDE(galaxia_state,astrowar)

	MCFG_DEVICE_ADD("s2636_0", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(3, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( galaxia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(c4482770) SHA1(aee983cc3d80989f49aea4138961bb623039484a) )
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(c0baa654) SHA1(80e0880c32ad285fbce0f7f552268b964b97cab3) )
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) )
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) )

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

ROM_START( galaxiaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(c4482770) SHA1(aee983cc3d80989f49aea4138961bb623039484a) )
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from parent
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from parent

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

ROM_START( galaxiab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(1d22219b) SHA1(6ab8ea8c78db30d80de98879018726d0420d30fe) ) // sldh - only 1 bit difference compared with set 1/2, however not considered a bad dump since it was found on two boards
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from parent
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from parent

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

ROM_START( galaxiac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(5682d56f) SHA1(15afb3296e93f8371d36b686ce372f917bd5b771) ) // sldh
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(80dafe84) SHA1(8a71a05f1b0ddba36bf748a4801f3a78f63af1db) ) // sldh
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(ffe86fdb) SHA1(67b02a5c39dbe515b6d68583c8831b0dae15374a) ) // sldh
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(8e3f5343) SHA1(6298be9bb33975854cb3d009b89913b1a8018aee) ) // sldh
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from parent
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from parent

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END


ROM_START( astrowar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astro.8h",  0x00000, 0x0400, CRC(b0ec246c) SHA1(f9123b5e317938655f5e8b3f8a5810d0b2b7c7af) )
	ROM_LOAD( "astro.10h", 0x00400, 0x0400, CRC(090d360f) SHA1(528ddcdc30a5a291bd8850ff6f134fcc19af562f) )
	ROM_LOAD( "astro.11h", 0x00800, 0x0400, CRC(72ab1378) SHA1(50743c64c4775076aa6f1d8ab2e05c14884bf0ba) )
	ROM_LOAD( "astro.13h", 0x00c00, 0x0400, CRC(2dc4c895) SHA1(831afbfd4ebfd6522ab0758222bc6f9826148a5d) )
	ROM_LOAD( "astro.8i",  0x01000, 0x0400, CRC(ab87fbfc) SHA1(34b670f96c260f186c643e588995ae5d80377784) )
	ROM_LOAD( "astro.10i", 0x02000, 0x0400, CRC(533675c1) SHA1(69cc066e1874a135a53a21b7b2461bda456504f1) )
	ROM_LOAD( "astro.11i", 0x02400, 0x0400, CRC(59cf8901) SHA1(e849d4c99350b7e3453c156d91618b71b5be1163) )
	ROM_LOAD( "astro.13i", 0x02800, 0x0400, CRC(5149c121) SHA1(232ba594e283fb25c31d8ae0b7d8315a81852a71) )
	ROM_LOAD( "astro.11l", 0x02c00, 0x0400, CRC(29f52f57) SHA1(5cb50b82e09c537eeaeae167351fca686fde8228) )
	ROM_LOAD( "astro.13l", 0x03000, 0x0400, CRC(882cdb87) SHA1(062ee8d296316cbce2eb62e72774aa4181e9847d) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "astro.1d",  0x00000, 0x0400, CRC(6053f834) SHA1(e0b76800c241b3c8010c09869cecbc109b25310a) )
	ROM_LOAD( "astro.3d",  0x00400, 0x0400, CRC(822505aa) SHA1(f9d3465e14bb850a286f8b4f42aa0a4044413b67) )
ROM_END

GAME( 1979, galaxia,  0,       galaxia,  galaxia, driver_device, 0, ROT90, "Zaccaria / Zelco", "Galaxia (set 1)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1979, galaxiaa, galaxia, galaxia,  galaxia, driver_device, 0, ROT90, "Zaccaria / Zelco", "Galaxia (set 2)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1979, galaxiab, galaxia, galaxia,  galaxia, driver_device, 0, ROT90, "Zaccaria / Zelco", "Galaxia (set 3)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1979, galaxiac, galaxia, galaxia,  galaxia, driver_device, 0, ROT90, "Zaccaria / Zelco", "Galaxia (set 4)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1980, astrowar, 0,       astrowar, galaxia, driver_device, 0, ROT90, "Zaccaria / Zelco", "Astro Wars", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
