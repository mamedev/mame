// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

    Gun.Smoke
    Capcom

    driver by Paul Leaman

    Games supported:
        * Gun.Smoke (World)
        * Gun.Smoke (bootleg)
        * Gun.Smoke (Japan)
        * Gun.Smoke (US set 1)
        * Gun.Smoke (US set 2)

****************************************************************************

  GUNSMOKE
  ========

  Driver provided by Paul Leaman


Stephh's notes (based on the games Z80 code and some tests) :

0) all games

  - There is some code that allows you to select your starting level
    (at 0x08dc in 'gunsmoka' and at 0x08d2 in the other sets).
    To do so, once the game has booted (after the "notice" screen),
    turn the "service" mode Dip Switch ON, and change Dip Switches
    DSW 1-0 to 1-3 (which are used by coinage).
  - About the ingame bug at the end of level 2 : enemy's energy
    (stored at 0xf790) is in fact not infinite, but it turns back to
    0xff, so when it reaches 0 again, the boss is dead.


1) 'gunsmoke'

  - World version.
    You can enter 3 chars for your initials.


2) 'gunsmokej'

  - Japan version (but English text though).
    You can enter 8 chars for your initials.


3) 'gunsmokeub'

  - US version licensed to Romstar.
    You can enter 3 chars for your initials.


4) 'gunsmokeu'

  - US version licensed to Romstar.
    You can enter 3 chars for your initials.
  - This is probably a later version of the game because some code
    has been added for the "Lives" Dip Switch that replaces the
    "Demonstation" one (so demonstration is always OFF).
  - Other changes :
      * Year is 1986 instead of 1985.
      * High score is 110000 instead of 100000.
      * Levels 3 and 6 are swapped.


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/gunsmoke.h"


/* Read/Write Handlers */

READ8_MEMBER(gunsmoke_state::gunsmoke_protection_r)
{
	/*
	    The routine at 0x0e69 tries to read data starting at 0xc4c9.
	    If this value is zero, it interprets the next two bytes as a
	    jump address.

	    This was resulting in a reboot which happens at the end of level 3
	    if you go too far to the right of the screen when fighting the level boss.

	    A non-zero for the first byte seems to be harmless
	    (although it may not be the correct behaviour).

	    This could be some devious protection or it could be a bug in the
	    arcade game.  It's hard to tell without pulling the code apart.
	*/

	static const UINT8 gunsmoke_fixed_data[] = { 0xff, 0x00, 0x00 };
	return gunsmoke_fixed_data[offset];
}

/* Memory Maps */

static ADDRESS_MAP_START( gunsmoke_map, AS_PROGRAM, 8, gunsmoke_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("P2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW1")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSW2")
	AM_RANGE(0xc4c9, 0xc4cb) AM_READ(gunsmoke_protection_r)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(gunsmoke_c804_w)  // ROM bank switch, screen flip
	AM_RANGE(0xc806, 0xc806) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(gunsmoke_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(gunsmoke_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd800, 0xd801) AM_RAM AM_SHARE("scrollx")
	AM_RANGE(0xd802, 0xd802) AM_RAM AM_SHARE("scrolly")
	AM_RANGE(0xd806, 0xd806) AM_WRITE(gunsmoke_d806_w)  // sprites and bg enable
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, gunsmoke_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0xe002, 0xe003) AM_DEVWRITE("ym2", ym2203_device, write)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( gunsmoke )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )    // VBLANK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "30K 80K 80K+" )
	PORT_DIPSETTING(    0x03, "30K 100K 100K+" )
	PORT_DIPSETTING(    0x00, "30K 100K 150K+" )
	PORT_DIPSETTING(    0x02, "30K 100K")
	PORT_DIPNAME( 0x04, 0x04, "Demo" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW ) // Also "debug mode"

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gunsmokeu )
	PORT_INCLUDE(gunsmoke)

	// Same as 'gunsmoke', but "Lives" Dip Switch instead of "Demonstration" Dip Switch
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	2,  /* 2 bits per pixel */
	{ 4, 0 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	32,32,  /* 32*32 tiles */
	512,    /* 512 tiles */
	4,      /* 4 bits per pixel */
	{ 512*256*8+4, 512*256*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			64*8+0, 64*8+1, 64*8+2, 64*8+3, 65*8+0, 65*8+1, 65*8+2, 65*8+3,
			128*8+0, 128*8+1, 128*8+2, 128*8+3, 129*8+0, 129*8+1, 129*8+2, 129*8+3,
			192*8+0, 192*8+1, 192*8+2, 192*8+3, 193*8+0, 193*8+1, 193*8+2, 193*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
			24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	256*8   /* every tile takes 256 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	2048,   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ 2048*64*8+4, 2048*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

/* Graphics Decode Info */

static GFXDECODE_START( gunsmoke )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,            0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,         32*4, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 32*4+16*16, 16 )
GFXDECODE_END

/* Machine Driver */

void gunsmoke_state::machine_start()
{
	UINT8 *rombase = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &rombase[0x10000], 0x4000);

	save_item(NAME(m_chon));
	save_item(NAME(m_objon));
	save_item(NAME(m_bgon));
	save_item(NAME(m_sprite3bank));
}

void gunsmoke_state::machine_reset()
{
	m_chon = 0;
	m_objon = 0;
	m_bgon = 0;
	m_sprite3bank = 0;
}

static MACHINE_CONFIG_START( gunsmoke, gunsmoke_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   // 4 MHz
	MCFG_CPU_PROGRAM_MAP(gunsmoke_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gunsmoke_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)  // 3 MHz
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(gunsmoke_state, irq0_line_hold,  4*60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gunsmoke_state, screen_update_gunsmoke)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gunsmoke)

	MCFG_PALETTE_ADD("palette", 32*4+16*16+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(gunsmoke_state, gunsmoke)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(0, "mono", 0.22)
	MCFG_SOUND_ROUTE(1, "mono", 0.22)
	MCFG_SOUND_ROUTE(2, "mono", 0.22)
	MCFG_SOUND_ROUTE(3, "mono", 0.14)

	MCFG_SOUND_ADD("ym2", YM2203, 1500000)
	MCFG_SOUND_ROUTE(0, "mono", 0.22)
	MCFG_SOUND_ROUTE(1, "mono", 0.22)
	MCFG_SOUND_ROUTE(2, "mono", 0.22)
	MCFG_SOUND_ROUTE(3, "mono", 0.14)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( gunsmoke )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gs03.09n", 0x00000, 0x8000, CRC(40a06cef) SHA1(3e2a52d476298b7252f0adaefdb42090351e921c) ) /* Code 0000-7fff */ // gse_03 ?
	ROM_LOAD( "gs04.10n", 0x10000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) /* Paged code */
	ROM_LOAD( "gs05.12n", 0x18000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) /* Paged code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) /* 32x32 tiles planes 2-3 */
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) /* 32x32 tiles planes 0-1 */
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) /* Sprites planes 0-1 */

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    /* red component */
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    /* green component */
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    /* blue component */
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    /* char lookup table */
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    /* tile lookup table */
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    /* tile palette bank */
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    /* sprite lookup table */
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    /* sprite palette bank */
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    /* priority? (not used) */
ROM_END

ROM_START( gunsmokeb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.ic85", 0x00000, 0x8000, CRC(ae6f4b75) SHA1(f4ee4f7a7d507ceaef9ce8165704fd80c8c1e8ba) ) /* Code 0000-7fff */
	ROM_LOAD( "4.ic86", 0x10000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) /* Paged code */
	ROM_LOAD( "5.ic87", 0x18000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) /* Paged code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.ic41", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "1.ic39", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "13.ic21", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) /* 32x32 tiles planes 2-3 */
	ROM_LOAD( "12.ic20", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "11.ic19", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "10.ic18", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "9.ic04", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) /* 32x32 tiles planes 0-1 */
	ROM_LOAD( "8.ic03", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "7.ic02", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "6.ic01", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "22.ic134", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) /* Sprites planes 2-3 */
	ROM_LOAD( "21.ic133", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) /* Sprites planes 2-3 */
	ROM_LOAD( "20.ic132", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) /* Sprites planes 2-3 */
	ROM_LOAD( "19.ic131", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) /* Sprites planes 2-3 */
	ROM_LOAD( "18.ic115", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) /* Sprites planes 0-1 */
	ROM_LOAD( "17.ic114", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) /* Sprites planes 0-1 */
	ROM_LOAD( "16.ic113", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) /* Sprites planes 0-1 */
	ROM_LOAD( "15.ic112", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) /* Sprites planes 0-1 */

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "14.ic25", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	/* The names of the proms starting with "g-" do not yet reflect their position in the pcb layout of this bootleg.
	   As the ICs are not socketed, but directly soldered to the pcb, it is harder to identify which is which.
	   But it would be good to figure this out at some point, for the sake of documenting this specific board layout. */
	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "prom.ic3", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    /* red component */
	ROM_LOAD( "prom.ic4", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    /* green component */
	ROM_LOAD( "prom.ic5", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    /* blue component */
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    /* char lookup table */
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    /* tile lookup table */
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    /* tile palette bank */
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    /* sprite lookup table */
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    /* sprite palette bank */
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    /* priority? (not used) */
ROM_END

ROM_START( gunsmokej )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gsj_03.09n",  0x00000, 0x8000, CRC(b56b5df6) SHA1(0295a3ef491b6b8ee9c198fd08dddc29d88bbef6) ) /* Code 0000-7fff */
	ROM_LOAD( "gs04.10n", 0x10000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) /* Paged code */
	ROM_LOAD( "gs05.12n", 0x18000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) /* Paged code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) /* 32x32 tiles planes 2-3 */
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) /* 32x32 tiles planes 0-1 */
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) /* Sprites planes 0-1 */

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    /* red component */
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    /* green component */
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    /* blue component */
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    /* char lookup table */
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    /* tile lookup table */
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    /* tile palette bank */
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    /* sprite lookup table */
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    /* sprite palette bank */
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    /* priority? (not used) */
ROM_END

ROM_START( gunsmokeu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gsa_03.9n",     0x00000, 0x8000, CRC(51dc3f76) SHA1(2a188fee73c3662b665b56a825eb908b7b42dcd0) ) /* Code 0000-7fff */
	ROM_LOAD( "gs04.10n",     0x10000, 0x8000, CRC(5ecf31b8) SHA1(34ec9727330821a45b497c78c970a1a4f14ff4ee) ) /* Paged code */
	ROM_LOAD( "gs05.12n",     0x18000, 0x8000, CRC(1c9aca13) SHA1(eb92c373d2241aea4c59248e1b82717733105ac0) ) /* Paged code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) /* 32x32 tiles planes 2-3 */
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) /* 32x32 tiles planes 0-1 */
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) /* Sprites planes 0-1 */

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    /* red component */
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    /* green component */
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    /* blue component */
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    /* char lookup table */
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    /* tile lookup table */
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    /* tile palette bank */
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    /* sprite lookup table */
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    /* sprite palette bank */
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    /* priority? (not used) */
ROM_END




ROM_START( gunsmokeua )
	ROM_REGION( 0x20000, "maincpu", 0 ) // has a small extra piece of code at 0x2f00 and a jump to it at 0x297b, otherwise the same as gunsmokeub including the datecode, chip had an 'A' stamped on it, bugfix?
	ROM_LOAD( "gsr_03a.9n",  0x00000, 0x8000, CRC(2f6e6ad7) SHA1(e9e4a367c240a35a1ba2eeaec9458996f7926f16) ) /* Code 0000-7fff */
	ROM_LOAD( "gs04.10n", 0x10000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) /* Paged code */
	ROM_LOAD( "gs05.12n", 0x18000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) /* Paged code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) /* 32x32 tiles planes 2-3 */
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) /* 32x32 tiles planes 0-1 */
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) /* Sprites planes 0-1 */

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    /* red component */
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    /* green component */
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    /* blue component */
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    /* char lookup table */
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    /* tile lookup table */
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    /* tile palette bank */
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    /* sprite lookup table */
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    /* sprite palette bank */
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    /* priority? (not used) */
ROM_END

ROM_START( gunsmokeub )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gsr_03.9n",  0x00000, 0x8000, CRC(592f211b) SHA1(8de44b3cafa3d2ce9aba515cf3ec4bac0bcdeb5b) ) /* Code 0000-7fff */
	ROM_LOAD( "gs04.10n", 0x10000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) /* Paged code */
	ROM_LOAD( "gs05.12n", 0x18000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) /* Paged code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) /* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) /* 32x32 tiles planes 2-3 */
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) /* 32x32 tiles planes 0-1 */
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) /* Sprites planes 2-3 */
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) /* Sprites planes 0-1 */
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) /* Sprites planes 0-1 */

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    /* red component */
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    /* green component */
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    /* blue component */
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    /* char lookup table */
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    /* tile lookup table */
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    /* tile palette bank */
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    /* sprite lookup table */
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    /* sprite palette bank */
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    /* priority? (not used) */
ROM_END

/* Game Drivers */

// at 0x7E50 in the first rom is 85113 (project ident code?) and the project codename 'Gunman' both stored as ASCII.
// Following that at (stored as raw data) is the build date in yyyymmdd format.  After that a ROM identification string(?) which I've
// left in the comment after each set.

// this information is not displayed onscreen

GAME( 1985, gunsmoke,   0,        gunsmoke, gunsmoke,   driver_device, 0, ROT270, "Capcom",                   "Gun.Smoke (World, 851115)", MACHINE_SUPPORTS_SAVE ) // GSE_03
GAME( 1985, gunsmokeb,  gunsmoke, gunsmoke, gunsmoke,   driver_device, 0, ROT270, "bootleg",                  "Gun.Smoke (World, 851115) (bootleg)", MACHINE_SUPPORTS_SAVE ) // based  on above version, warning message patched out
GAME( 1985, gunsmokej,  gunsmoke, gunsmoke, gunsmoke,   driver_device, 0, ROT270, "Capcom",                   "Gun.Smoke (Japan, 851115)", MACHINE_SUPPORTS_SAVE ) // GSJ_03
GAME( 1986, gunsmokeu,  gunsmoke, gunsmoke, gunsmokeu,  driver_device, 0, ROT270, "Capcom (Romstar license)", "Gun.Smoke (US, 860408)", MACHINE_SUPPORTS_SAVE ) // GSA_03
GAME( 1985, gunsmokeua, gunsmoke, gunsmoke, gunsmoke,   driver_device, 0, ROT270, "Capcom (Romstar license)", "Gun.Smoke (US, 851115, set 1)", MACHINE_SUPPORTS_SAVE ) // GSR_03 (03A on the chip)
GAME( 1986, gunsmokeub, gunsmoke, gunsmoke, gunsmoke,   driver_device, 0, ROT270, "Capcom (Romstar license)", "Gun.Smoke (US, 851115, set 2)", MACHINE_SUPPORTS_SAVE ) // GSR_03
