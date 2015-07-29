// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

    Markham (c) 1983 Sun Electronics

    Driver by Uki

    17/Jun/2001 -

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "includes/markham.h"


READ8_MEMBER(markham_state::markham_e004_r)
{
	return 0;
}

/****************************************************************************/

static ADDRESS_MAP_START( markham_master_map, AS_PROGRAM, 8, markham_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM

	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(markham_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share1")

	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW2")
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("DSW1")
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("P1")
	AM_RANGE(0xe003, 0xe003) AM_READ_PORT("P2")

	AM_RANGE(0xe004, 0xe004) AM_READ(markham_e004_r) /* from CPU2 busack */

	AM_RANGE(0xe005, 0xe005) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xe008, 0xe008) AM_WRITENOP /* coin counter? */
	AM_RANGE(0xe009, 0xe009) AM_WRITENOP /* to CPU2 busreq */

	AM_RANGE(0xe00c, 0xe00d) AM_WRITEONLY AM_SHARE("xscroll")
	AM_RANGE(0xe00e, 0xe00e) AM_WRITE(markham_flipscreen_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( markham_slave_map, AS_PROGRAM, 8, markham_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("share1")

	AM_RANGE(0xc000, 0xc000) AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0xc001, 0xc001) AM_DEVWRITE("sn2", sn76496_device, write)

	AM_RANGE(0xc002, 0xc002) AM_WRITENOP /* unknown */
	AM_RANGE(0xc003, 0xc003) AM_WRITENOP /* unknown */
ADDRESS_MAP_END


/****************************************************************************/

static INPUT_PORTS_START( markham )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Chutes" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPSETTING(    0x08, "Common" )
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, "1C 1C / 1C 1C" )
	PORT_DIPSETTING(    0x10, "2C 1C / 2C 1C" )
	PORT_DIPSETTING(    0x20, "2C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x30, "1C 1C / 1C 2C" )
	PORT_DIPSETTING(    0x40, "1C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x50, "1C 1C / 1C 4C" )
	PORT_DIPSETTING(    0x60, "1C 1C / 1C 5C" )
	PORT_DIPSETTING(    0x70, "1C 1C / 1C 6C" )
	PORT_DIPSETTING(    0x80, "1C 2C / 1C 2C" )
	PORT_DIPSETTING(    0x90, "1C 2C / 1C 4C" )
	PORT_DIPSETTING(    0xa0, "1C 2C / 1C 5C" )
	PORT_DIPSETTING(    0xb0, "1C 2C / 1C 10C" )
	PORT_DIPSETTING(    0xc0, "1C 2C / 1C 11C" )
	PORT_DIPSETTING(    0xd0, "1C 2C / 1C 12C" )
	PORT_DIPSETTING(    0xe0, "1C 2C / 1C 6C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "20000, Every 50000" )
	PORT_DIPSETTING(    0x03, "20000, Every 80000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )    /*  These next five dips are unused according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* e002 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")        /* e003 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM")    /* e005 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END


/****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{0,8192*8,8192*8*2},
	{7,6,5,4,3,2,1,0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{8192*8*2,8192*8,0},
	{7,6,5,4,3,2,1,0,
		8*16+7,8*16+6,8*16+5,8*16+4,8*16+3,8*16+2,8*16+1,8*16+0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
		8*8,8*9,8*10,8*11,8*12,8*13,8*14,8*15},
	8*8*4
};

static GFXDECODE_START( markham )
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout,   512, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0,   64 )
GFXDECODE_END


static MACHINE_CONFIG_START( markham, markham_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000/2) /* 4.000MHz */
	MCFG_CPU_PROGRAM_MAP(markham_master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", markham_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,8000000/2) /* 4.000MHz */
	MCFG_CPU_PROGRAM_MAP(markham_slave_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", markham_state,  irq0_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(markham_state, screen_update_markham)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", markham)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(markham_state, markham)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, 8000000/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_SOUND_ADD("sn2", SN76496, 8000000/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/****************************************************************************/

ROM_START( markham )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tv3.9",   0x0000,  0x2000, CRC(59391637) SHA1(e0cfe49a5591d6a6e64c3277319a19235b0ee6ea) )
	ROM_LOAD( "tvg4.10", 0x2000,  0x2000, CRC(1837bcce) SHA1(50e1ae0a4937f09a3dced48bb12f57cee846487a) )
	ROM_LOAD( "tvg5.11", 0x4000,  0x2000, CRC(651da602) SHA1(9f33d6ea0526af9be8ac9210910ea768da825ee5) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1.5",  0x0000,  0x2000, CRC(c5299766) SHA1(a6c903088ffd6c5ae0ba7ff50c8509a185f88220) )
	ROM_LOAD( "tvg2.6",  0x4000,  0x2000, CRC(b216300a) SHA1(036fafd0277b3422cf491db77748358da1ecfb43) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg6.84", 0x0000,  0x2000, CRC(ab933ae5) SHA1(d2bdbc35d751480ddf8b89b90063510684b00db2) )
	ROM_LOAD( "tvg7.85", 0x2000,  0x2000, CRC(ce8edda7) SHA1(5312754aec20791398de57f08857d4097a7cfc2c) )
	ROM_LOAD( "tvg8.86", 0x4000,  0x2000, CRC(74d1536a) SHA1(ff2efbbe1420282643558a65bfa5fd278cdaf135) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg9.87",  0x0000,  0x2000, CRC(42168675) SHA1(d2cce79a05ca7fda9347630fe0045a2d8182025d) )
	ROM_LOAD( "tvg10.88", 0x2000,  0x2000, CRC(fa9feb67) SHA1(669c6e1defc33541c36d4deb9667b67254f53a37) )
	ROM_LOAD( "tvg11.89", 0x4000,  0x2000, CRC(71f3dd49) SHA1(8fecb6b76907c592d545dafeaa47cf765513b3fe) )

	ROM_REGION( 0x0700, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "14-3.99",  0x0000,  0x0100, CRC(89d09126) SHA1(1f78f3b3ef8c6ba9c00a58ae89837d9a92e5078f) ) /* R */
	ROM_LOAD( "14-4.100", 0x0100,  0x0100, CRC(e1cafe6c) SHA1(8c37c3829bf1b96690fb853a2436f1b5e8d45e8c) ) /* G */
	ROM_LOAD( "14-5.101", 0x0200,  0x0100, CRC(2d444fa6) SHA1(66b64133ca740686bedd33bafd20a3f9f3df97d4) ) /* B */
	ROM_LOAD( "14-1.61",  0x0300,  0x0200, CRC(3ad8306d) SHA1(877f1d58cb8da9098ec71a7c7aec633dbf9e76e6) ) /* sprite */
	ROM_LOAD( "14-2.115", 0x0500,  0x0200, CRC(12a4f1ff) SHA1(375e37d7162053d45da66eee23d66bd432303c1c) ) /* bg */
ROM_END


GAME( 1983, markham, 0, markham, markham, driver_device, 0, ROT0, "Sun Electronics", "Markham", MACHINE_SUPPORTS_SAVE )
