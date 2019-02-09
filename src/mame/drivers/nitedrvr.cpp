// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Night Driver hardware

    driver by Mike Balfour

    Games supported:
        * Night Driver

    Known issues:
        * The road boxes in service mode are flipped horizontally and there
          is an extraneous box according to the service manual.

****************************************************************************

    Memory Map:
        0000-01FF   R/W     SCRAM (Scratchpad RAM)
        0200-03FF    W      PFW (Playfield Write)
        0400-05FF    W      HVC (Horiz/Vert/Char for Roadway)
        0600-07FF    R      IN0
        0800-09FF    R      IN1
        0A00-0BFF    W      OUT0
        0C00-0DFF    W      OUT1
        0E00-0FFF    -      OUT2 (Not used)
        8000-83FF    R      PFR (Playfield Read)
        8400-87FF           Steering Reset
        8800-8FFF    -      Spare (Not used)
        9000-97FF    R      Program ROM1
        9800-9FFF    R      Program ROM2
        (F800-FFFF)  R      Program ROM2 - only needed for the 6502 vectors

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"
#include "includes/nitedrvr.h"

#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "screen.h"
#include "speaker.h"

/* Memory Map */

void nitedrvr_state::nitedrvr_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x100); // SCRAM
	map(0x0200, 0x027f).nopr().w(FUNC(nitedrvr_state::nitedrvr_videoram_w)).mirror(0x180).share("videoram"); // PFW
	map(0x0400, 0x042f).nopr().writeonly().mirror(0x1c0).share("hvc"); // POSH, POSV, CHAR
	map(0x0430, 0x043f).w("watchdog", FUNC(watchdog_timer_device::reset_w)).mirror(0x1c0);
	map(0x0600, 0x07ff).r(FUNC(nitedrvr_state::nitedrvr_in0_r));
	map(0x0800, 0x09ff).r(FUNC(nitedrvr_state::nitedrvr_in1_r));
	map(0x0a00, 0x0bff).w(FUNC(nitedrvr_state::nitedrvr_out0_w));
	map(0x0c00, 0x0dff).w(FUNC(nitedrvr_state::nitedrvr_out1_w));
	map(0x8000, 0x807f).readonly().mirror(0x380).share("videoram"); // PFR
	map(0x8400, 0x87ff).rw(FUNC(nitedrvr_state::nitedrvr_steering_reset_r), FUNC(nitedrvr_state::nitedrvr_steering_reset_w));
	map(0x9000, 0x9fff).rom(); // ROM1-ROM2
	map(0xfff0, 0xffff).rom(); // ROM2 for 6502 vectors
}

/* Input Ports */

static INPUT_PORTS_START( nitedrvr )
	PORT_START("DSW0")  // fake
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	//PORT_DIPSETTING(  0x20, DEF_STR( 1C_1C ) ) // not a typo
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x80, "Playing Time" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x40, "75" )
	PORT_DIPSETTING(    0x80, "100" )
	PORT_DIPSETTING(    0xC0, "125" )

	PORT_START("DSW1")  // fake
	PORT_DIPNAME( 0x10, 0x00, "Track Set" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Reverse ) )
	PORT_DIPNAME( 0x20, 0x20, "Bonus Time" )
	PORT_DIPSETTING(    0x00, DEF_STR ( No ) )
	PORT_DIPSETTING(    0x20, "Score = 350" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("GEARS") // fake
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("1st Gear")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("2nd Gear")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("3rd Gear")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("4th Gear")

	PORT_START("DSW2")  // fake
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // Spare
	PORT_DIPNAME( 0x20, 0x00, "Difficult Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")   // fake
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Novice Track")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Expert Track")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pro Track")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Alternating signal?

	PORT_START("STEER") // fake
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("MOTOR")
	PORT_ADJUSTER( 60, "Motor RPM" )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

/* Graphics Decode Information */

static GFXDECODE_START( gfx_nitedrvr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
GFXDECODE_END

/* Machine Driver */

MACHINE_CONFIG_START(nitedrvr_state::nitedrvr)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6502, 12.096_MHz_XTAL / 12) // 1 MHz
	MCFG_DEVICE_PROGRAM_MAP(nitedrvr_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", nitedrvr_state, irq0_line_hold)

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 3);

	TIMER(config, "crash_timer").configure_periodic(FUNC(nitedrvr_state::nitedrvr_crash_toggle_callback), PERIOD_OF_555_ASTABLE(RES_K(180), 330, CAP_U(1)));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(12.096_MHz_XTAL / 2, 384, 0, 256, 278, 0, 256) // ~57 Hz
	// PROM derives VRESET, VBLANK, VSYNC, IRQ from vertical scan count and last VBLANK
	MCFG_SCREEN_UPDATE_DRIVER(nitedrvr_state, screen_update_nitedrvr)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nitedrvr);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("discrete", DISCRETE, nitedrvr_discrete)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* ROMs */

/*
ROM_START( nitedrvo )       // early revision has the program code stored in 8 chips
    ROM_REGION( 0x10000, "maincpu", 0 )
    ROM_LOAD( "006560-01.h1", 0x9000, 0x0200, NO_DUMP ) // PROM 1
    ROM_LOAD( "006561-01.c1", 0x9200, 0x0200, NO_DUMP ) // PROM 2
    ROM_LOAD( "006562-01.j1", 0x9400, 0x0200, NO_DUMP ) // PROM 3
    ROM_LOAD( "006563-01.d1", 0x9600, 0x0200, NO_DUMP ) // PROM 4
    ROM_LOAD( "006564-01.k1", 0x9800, 0x0200, NO_DUMP ) // PROM 5
    ROM_LOAD( "006565-01.e1", 0x9a00, 0x0200, NO_DUMP ) // PROM 6
    ROM_LOAD( "006566-01.l1", 0x9c00, 0x0200, NO_DUMP ) // PROM 7
    ROM_LOAD( "006567-01.f1", 0x9e00, 0x0200, NO_DUMP ) // PROM 8
ROM_END
*/

ROM_START( nitedrvr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "006569-01.d2", 0x9000, 0x0800, CRC(7afa7542) SHA1(81018e25ebdeae1daf1308676661063b6fd7fd22) ) // mask ROM 1
	ROM_LOAD( "006570-01.f2", 0x9800, 0x0800, CRC(bf5d77b1) SHA1(6f603f8b0973bd89e0e721b66944aac8e9f904d9) ) // mask ROM 2
	ROM_RELOAD(               0xf800, 0x0800 ) // vectors

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD( "006568-01.p2", 0x0000, 0x0200, CRC(f80d8889) SHA1(ca573543dcce1221459d5693c476cef14bfac4f4) ) // PROM, Alpha-Numeric

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "006559-01.h7", 0x0000, 0x0100, CRC(5a8d0e42) SHA1(772220c4c24f18769696ddba26db2bc2e5b0909d) ) // PROM, Sync
ROM_END

/* Game Drivers */

GAME( 1976, nitedrvr, 0, nitedrvr, nitedrvr, nitedrvr_state, empty_init, ROT0, "Atari", "Night Driver", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
