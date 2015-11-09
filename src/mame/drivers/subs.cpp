// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Subs hardware

    driver by Mike Balfour

    Games supported:
        * Subs

    Known issues:
        * none at this time

****************************************************************************

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/subs.h"
#include "sound/discrete.h"
#include "rendlay.h"

/*************************************
 *
 *  Palette generation
 *
 *************************************/

PALETTE_INIT_MEMBER(subs_state, subs)
{
	palette.set_pen_color(0,rgb_t(0x00,0x00,0x00)); /* BLACK - modified on video invert */
	palette.set_pen_color(1,rgb_t(0xff,0xff,0xff)); /* WHITE - modified on video invert */
	palette.set_pen_color(2,rgb_t(0x00,0x00,0x00)); /* BLACK - modified on video invert */
	palette.set_pen_color(3,rgb_t(0xff,0xff,0xff)); /* WHITE - modified on video invert*/
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, subs_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x0000) AM_WRITE(noise_reset_w)
	AM_RANGE(0x0000, 0x0007) AM_READ(control_r)
	AM_RANGE(0x0020, 0x0020) AM_WRITE(steer_reset_w)
	AM_RANGE(0x0020, 0x0027) AM_READ(coin_r)
//  AM_RANGE(0x0040, 0x0040) AM_WRITE(timer_reset_w)
	AM_RANGE(0x0060, 0x0063) AM_READ(options_r)
	AM_RANGE(0x0060, 0x0061) AM_WRITE(lamp1_w)
	AM_RANGE(0x0062, 0x0063) AM_WRITE(lamp2_w)
	AM_RANGE(0x0064, 0x0065) AM_WRITE(sonar2_w)
	AM_RANGE(0x0066, 0x0067) AM_WRITE(sonar1_w)
// Schematics show crash and explode reversed.  But this is proper.
	AM_RANGE(0x0068, 0x0069) AM_WRITE(explode_w)
	AM_RANGE(0x006a, 0x006b) AM_WRITE(crash_w)
	AM_RANGE(0x006c, 0x006d) AM_WRITE(invert1_w)
	AM_RANGE(0x006e, 0x006f) AM_WRITE(invert2_w)
	AM_RANGE(0x0090, 0x009f) AM_SHARE("spriteram")
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( subs )
	PORT_START("DSW") /* OPTIONS */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Credit/Time" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Each Coin Buys Time" )
	PORT_DIPSETTING(    0x02, "Fixed Time" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Language ) )  PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( French ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Spanish) )
	PORT_DIPSETTING(    0x0c, DEF_STR( German) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x40, "Game Length" )  PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, "0:30 Minutes" )
	PORT_DIPSETTING(    0x20, "1:00 Minutes" )
	PORT_DIPSETTING(    0x40, "1:30 Minutes" )
	PORT_DIPSETTING(    0x60, "2:00 Minutes" )
	PORT_DIPSETTING(    0x80, "2:30 Minutes" )
	PORT_DIPSETTING(    0xa0, "3:00 Minutes" )
	PORT_DIPSETTING(    0xc0, "3:30 Minutes" )
	PORT_DIPSETTING(    0xe0, "4:00 Minutes" )

	PORT_START("IN0") /* IN1 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Diag Step */
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Diag Hold */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_TILT )    /* Slam */
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  /* Spare */
	PORT_BIT ( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED ) /* Filled in with steering information */

	PORT_START("IN1") /* IN2 */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("lscreen")
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("DIAL2") /* IN3 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("DIAL1") /* IN4 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout playfield_layout =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout motion_layout =
{
	16,16,
	64,
	1,
	{ 0 },
	{ 3 + 0x400*8, 2 + 0x400*8, 1 + 0x400*8, 0 + 0x400*8,
		7 + 0x400*8, 6 + 0x400*8, 5 + 0x400*8, 4 + 0x400*8,
		3, 2, 1, 0, 7, 6, 5, 4 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};


static GFXDECODE_START( subs )
	GFXDECODE_ENTRY( "gfx1", 0, playfield_layout, 0, 2 )    /* playfield graphics */
	GFXDECODE_ENTRY( "gfx2", 0, motion_layout,    0, 2 )    /* motion graphics */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( subs, subs_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502,12096000/16)      /* clock input is the "4H" signal */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(subs_state, interrupt, 4*57)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", subs)

	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(subs_state, subs)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(subs_state, screen_update_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(subs_state, screen_update_right)
	MCFG_SCREEN_PALETTE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(subs)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( subs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "34196.e2",     0x2000, 0x0100, CRC(7c7a04c3) SHA1(269d9f7573cc5da4412f53d647127c4884435353) )   /* ROM 0 D4-D7 */
	ROM_LOAD_NIB_LOW ( "34194.e1",     0x2000, 0x0100, CRC(6b1c4acc) SHA1(3a743b721d9e7e9bdc4533aeeab294eb0ea27500) )   /* ROM 0 D0-D3 */
	ROM_LOAD( "34190.p1",     0x2800, 0x0800, CRC(a88aef21) SHA1(3811c137041ca43a6e49fbaf7d9d8ef37ba190a2) )
	ROM_LOAD( "34191.p2",     0x3000, 0x0800, CRC(2c652e72) SHA1(097b665e803cbc57b5a828403a8d9a258c19e97f) )
	ROM_LOAD( "34192.n2",     0x3800, 0x0800, CRC(3ce63d33) SHA1(a413cb3e0d03dc40a50f5b03b76a4edbe7906f3e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "34211.m4",     0x0000, 0x0800, CRC(fa8d4409) SHA1(a83b7a835212d31fe421d537fa0d78f234c26f5b) )    /* Playfield */

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "34216.d7",     0x0000, 0x0200, CRC(941d28b4) SHA1(89388ec06546dc567aa5dbc6a7898974f2871ecc) )    /* Motion */
	ROM_LOAD( "34218.e7",     0x0200, 0x0200, CRC(f4f4d874) SHA1(d99ad9a74611f9851f6bfa6000ebd70e1a364f5d) )    /* Motion */
	ROM_LOAD( "34217.d8",     0x0400, 0x0200, CRC(a7a60da3) SHA1(34fc21cc1ca69d58d3907094dc0a3faaf6f461b3) )    /* Motion */
	ROM_LOAD( "34219.e8",     0x0600, 0x0200, CRC(99a5a49b) SHA1(2cb429f8de73c7d78dc83e47f1448ea4340c333d) )    /* Motion */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1977, subs, 0, subs, subs, driver_device, 0, ROT0, "Atari", "Subs", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
