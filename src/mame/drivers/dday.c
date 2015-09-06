// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

D-Day

driver by Zsolt Vasvari


Convention: "sl" stands for "searchlight"

Note: This game doesn't seem to support cocktail mode, which is not too
      surprising for a gun game.

0000-3fff ROM
5000-53ff Text layer videoram
5400-57ff Foreground (vehicle) layer videoram
5800-5bff Background videoram
5c00-5fff Attributes RAM for vehicle layer
          A0-A4 seem to be ignored.
          D0 - X Flip
          D2 - Used by the software to separate area that the short shots
               cannot penetrate
          Others unknown, they don't seem to be used by this game
6000-63ff RAM

read:

6c00  Input Port #1
7000  Dip Sw #1
7400  Dip Sw #2
7800  Timer
7c00  Analog Control

write:

4000 Search light image and flip
6400 AY8910 #1 Control Port
6401 AY8910 #1 Write Port
6800 AY8910 #2 Control Port
6801 AY8910 #2 Write Port
7800 Bit 0 - Coin Counter 1
     Bit 1 - Coin Counter 2
     Bit 2 - ??? Pulsated when the player is hit
     Bit 3 - ??? Seems to be unused
     Bit 4 - Tied to AY8910 RST. Used to turn off sound
     Bit 5 - ??? Seem to be always on
     Bit 6 - Search light enable
     Bit 7 - ???


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/dday.h"


static ADDRESS_MAP_START( dday_map, AS_PROGRAM, 8, dday_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(dday_sl_control_w)
	AM_RANGE(0x5000, 0x53ff) AM_RAM_WRITE(dday_textvideoram_w) AM_SHARE("textvideoram")
	AM_RANGE(0x5400, 0x57ff) AM_RAM_WRITE(dday_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x5800, 0x5bff) AM_RAM_WRITE(dday_bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x5c00, 0x5fff) AM_READWRITE(dday_colorram_r, dday_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x6000, 0x63ff) AM_RAM
	AM_RANGE(0x6400, 0x6401) AM_MIRROR(0x000e) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x6800, 0x6801) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x6c00, 0x6c00) AM_READ_PORT("BUTTONS")
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("DSW0")
	AM_RANGE(0x7400, 0x7400) AM_READ_PORT("DSW1")
	AM_RANGE(0x7800, 0x7800) AM_READWRITE(dday_countdown_timer_r, dday_control_w)
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("PADDLE")
ADDRESS_MAP_END



static INPUT_PORTS_START( dday )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* fire button */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* doesn't seem to be */
													/* accessed */
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x00, "2" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x01, "3" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x02, "4" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x03, "5" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x00, "5" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "6" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "7" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x03, "8" ) PORT_CONDITION("DSW0", 0x80, EQUALS, 0x00)

	PORT_DIPNAME( 0x0c, 0x00, "Extended Play At" )      PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x0c, "25000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5") // No Difficulty setting?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      // Clearly old code revision, ddayc works much better
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Start with 20000 Pts" )  PORT_DIPLOCATION("SW3:8") // Works the same as Centuri License, but not as well
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )           // Doesn't mention extended play, just gives lives
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )          // Also alters table for Extended Play

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_2C ) ) /* Not shown in manual */
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ) )

	PORT_START("PADDLE")
	PORT_BIT(0xff, 96, IPT_PADDLE ) PORT_MINMAX(0,191) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
INPUT_PORTS_END

static INPUT_PORTS_START( ddayc )
	PORT_INCLUDE(dday)

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) /* Distance Button */

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0c, 0x00, "Extended Play At" )      PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x00, "4000" )
	PORT_DIPSETTING(    0x04, "6000" )
	PORT_DIPSETTING(    0x08, "8000" )
	PORT_DIPSETTING(    0x0c, "10000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )     // Easy   - No Bombs, No Troop Carriers
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )       // Normal - No Bombs, Troop Carriers
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )     // Hard   - Bombs, Troop Carriers
	PORT_DIPSETTING(    0x00, "Hard (duplicate setting)" )  // Same as 0x10
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )        // Doesn't seem to be used
/*

 The manual shows these differences:

--------------------------------------------------------------------
    DipSwitch Title   |  Function  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
--------------------------------------------------------------------
                      |  Very Easy |               |off|off|       |
      Difficulty      |    Easy    |               |on |off|       |
                      |  Difficult |               |off|on |       |
                      |V. Difficult|               |on |on |       |
--------------------------------------------------------------------
       Free Play                                               |on |
--------------------------------------------------------------------
*/

INPUT_PORTS_END



static const gfx_layout layout_1bpp =
{
	8,8,            /* 8*8 characters */
	RGN_FRAC(1,2),  /* 256 characters */
	1,              /* 1 bit per pixel */
	{ RGN_FRAC(0,1) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout layout_2bpp =
{
	8,8,            /* 8*8 characters */
	RGN_FRAC(1,2),  /* 256 characters */
	2,              /* 2 bits per pixel */
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout layout_3bpp =
{
	8,8,            /* 8*8 characters */
	RGN_FRAC(1,3),  /* 256 characters */
	3,              /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( dday )
	GFXDECODE_ENTRY( "gfx1", 0, layout_3bpp, 0,       256/8 )   /* background */
	GFXDECODE_ENTRY( "gfx2", 0, layout_2bpp, 8*4,     8 )       /* foreground */
	GFXDECODE_ENTRY( "gfx3", 0, layout_2bpp, 8*4+8*4, 8 )       /* text */
	GFXDECODE_ENTRY( "gfx4", 0, layout_1bpp, 254,     1 )       /* searchlight */
GFXDECODE_END



void dday_state::machine_start()
{
	m_ay1 = machine().device("ay1");

	save_item(NAME(m_control));
	save_item(NAME(m_sl_enable));
	save_item(NAME(m_sl_image));
	save_item(NAME(m_timer_value));
}

void dday_state::machine_reset()
{
	m_control = 0;
	m_sl_enable = 0;
	m_sl_image = 0;
}


static MACHINE_CONFIG_START( dday, dday_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2000000)     /* 2 MHz ? */
	MCFG_CPU_PROGRAM_MAP(dday_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dday_state, screen_update_dday)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dday)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INDIRECT_ENTRIES(256) /* HACK!!! */
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_INIT_OWNER(dday_state, dday)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dday )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e8_63co.bin",  0x0000, 0x1000, CRC(13d53793) SHA1(045f4b02803cb24305f90593777bb4a59f1bbb34) )
	ROM_LOAD( "e7_64co.bin",  0x1000, 0x1000, CRC(e1ef2a70) SHA1(946ef20e2cd441ca858f969e7f25ab7c940671f8) )
	ROM_LOAD( "e6_65co.bin",  0x2000, 0x1000, CRC(fe414a83) SHA1(1ca1d30b71b62af5230dfe862a67c4cff5a71f41) )
	ROM_LOAD( "e5_66co.bin",  0x3000, 0x1000, CRC(fc9f7774) SHA1(1071d05e2f0ee8869eeeb46ad219303b417f4c90) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "k4_73.bin",    0x0000, 0x0800, CRC(fa6237e4) SHA1(0dfe2a0079324a78b462203fe93f7fb186a42122) )
	ROM_LOAD( "k2_71.bin",    0x0800, 0x0800, CRC(f85461de) SHA1(e2ed34e993cd657681124df5531e35afd7d8c34b) )
	ROM_LOAD( "k3_72.bin",    0x1000, 0x0800, CRC(fdfe88b6) SHA1(cdc37d90500f4ce813b6efee31139e6776aa2bff) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "j8_70co.bin",  0x0000, 0x0800, CRC(0c60e94c) SHA1(136df37b858a7fd399acc89e59917a068165d749) )
	ROM_LOAD( "j9_69co.bin",  0x0800, 0x0800, CRC(ba341c10) SHA1(c2c7350f87d5e47ac47cb19020681f0e7340e427) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "k6_74o.bin",   0x0000, 0x0800, CRC(66719aea) SHA1(dd29f8d079868af3c7fd16dc8c383f1eba4543d2) )
	ROM_LOAD( "k7_75o.bin",   0x0800, 0x0800, CRC(5f8772e2) SHA1(16194a02bc7d5248dea7a80bf6d6d263ec8a7fd6) )

	ROM_REGION( 0x0800, "gfx4", 0 )
	ROM_LOAD( "d4_68.bin",    0x0000, 0x0800, CRC(f3649264) SHA1(5486a33fa1f7803e68d141992d6105206da1beba) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "d2_67.bin",    0x0000, 0x1000, CRC(2b693e42) SHA1(e52b987cf929ddfc7916b05456b1114076956d12) )  /* search light map */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "dday.m11",     0x0000, 0x0100, CRC(aef6bbfc) SHA1(9e07729a4389221bc120af91d8275e1d05f3be7a) )  /* red component */
	ROM_LOAD( "dday.m8",      0x0100, 0x0100, CRC(ad3314b9) SHA1(d103f4f6103987ea85f0791ffc66a1cf9c711031) )  /* green component */
	ROM_LOAD( "dday.m3",      0x0200, 0x0100, CRC(e877ab82) SHA1(03e3905aee37f6743e7a4a87338f9504c832a55b) )  /* blue component */
ROM_END

ROM_START( ddayc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e8_63-c.bin",  0x0000, 0x1000, CRC(d4fa3ae3) SHA1(587cfcd0bb3103c9875b8a5fd185a321212a86ab) )
	ROM_LOAD( "e7_64-c.bin",  0x1000, 0x1000, CRC(9fb8b1a7) SHA1(abd935274745db28039f2e341e9be0490e307772) )
	ROM_LOAD( "e6_65-c.bin",  0x2000, 0x1000, CRC(4c210686) SHA1(9d3110c4d1347f8a067c49b363a32d0f6a2c34c7) )
	ROM_LOAD( "e5_66-c.bin",  0x3000, 0x1000, CRC(e7e832f9) SHA1(3cbd2f9197e934ba3eae329511886a30c09a1ac7) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "k4_73.bin",    0x0000, 0x0800, CRC(fa6237e4) SHA1(0dfe2a0079324a78b462203fe93f7fb186a42122) )
	ROM_LOAD( "k2_71.bin",    0x0800, 0x0800, CRC(f85461de) SHA1(e2ed34e993cd657681124df5531e35afd7d8c34b) )
	ROM_LOAD( "k3_72.bin",    0x1000, 0x0800, CRC(fdfe88b6) SHA1(cdc37d90500f4ce813b6efee31139e6776aa2bff) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "j8_70-c.bin",  0x0000, 0x0800, CRC(a0c6b86b) SHA1(8b30370f2d5d7e75b8ec2a68f4e408fcb8ed2c8f) )
	ROM_LOAD( "j9_69-c.bin",  0x0800, 0x0800, CRC(d352a3d6) SHA1(4f1ba0b555f6b3dd539511bab8c55db45f719afc) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "k6_74.bin",    0x0000, 0x0800, CRC(d21a3e22) SHA1(9260353245f2ba2aca09fafd377c662ff508e5c0) )
	ROM_LOAD( "k7_75.bin",    0x0800, 0x0800, CRC(a5e5058c) SHA1(312dd55902c7e16eeee0dfc42c17fe1440b26e40) )

	ROM_REGION( 0x0800, "gfx4", 0 )
	ROM_LOAD( "d4_68.bin",    0x0000, 0x0800, CRC(f3649264) SHA1(5486a33fa1f7803e68d141992d6105206da1beba) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "d2_67.bin",    0x0000, 0x1000, CRC(2b693e42) SHA1(e52b987cf929ddfc7916b05456b1114076956d12) )  /* search light map */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "dday.m11",     0x0000, 0x0100, CRC(aef6bbfc) SHA1(9e07729a4389221bc120af91d8275e1d05f3be7a) )  /* red component */
	ROM_LOAD( "dday.m8",      0x0100, 0x0100, CRC(ad3314b9) SHA1(d103f4f6103987ea85f0791ffc66a1cf9c711031) )  /* green component */
	ROM_LOAD( "dday.m3",      0x0200, 0x0100, CRC(e877ab82) SHA1(03e3905aee37f6743e7a4a87338f9504c832a55b) )  /* blue component */
ROM_END


GAME( 1982, dday,  0,    dday, dday, driver_device,  0, ROT0, "Olympia", "D-Day", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1982, ddayc, dday, dday, ddayc, driver_device, 0, ROT0, "Olympia (Centuri license)", "D-Day (Centuri)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
