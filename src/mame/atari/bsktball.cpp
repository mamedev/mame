// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Basketball hardware

    driver by Mike Balfour

    Games supported:
        * Basketball

    Known issues:
        * Arcade screen hblank (left/right) border is grey, MAME screen emulation
          is limited to active display area.

****************************************************************************

    Note:  The original hardware uses the Player 1 and Player 2 Start buttons
    as the Jump/Shoot buttons.

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    2008-07
    Dip locations verified with manual

***************************************************************************/

#include "emu.h"
#include "bsktball.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "sound/discrete.h"
#include "speaker.h"


/*************************************
 *
 *  Palette generation
 *
 *************************************/

void bsktball_state::bsktball_palette(palette_device &palette) const
{
	palette.set_indirect_color(0,rgb_t(0x00,0x00,0x00)); // BLACK
	palette.set_indirect_color(1,rgb_t(0x80,0x80,0x80)); // LIGHT GREY
	palette.set_indirect_color(2,rgb_t(0x50,0x50,0x50)); // DARK GREY
	palette.set_indirect_color(3,rgb_t(0xff,0xff,0xff)); // WHITE

	/* playfield */
	for (int i = 0; i < 2; i++)
	{
		palette.set_pen_indirect(i*4 + 0, 1);
		palette.set_pen_indirect(i*4 + 1, 3 * i);
		palette.set_pen_indirect(i*4 + 2, 3 * i);
		palette.set_pen_indirect(i*4 + 3, 3 * i);
	}

	/* motion */
	for (int i = 0; i < 4*4*4; i++)
	{
		palette.set_pen_indirect(2*4 + i*4 + 0, 1);
		palette.set_pen_indirect(2*4 + i*4 + 1, (i >> 2) & 3);
		palette.set_pen_indirect(2*4 + i*4 + 2, (i >> 0) & 3);
		palette.set_pen_indirect(2*4 + i*4 + 3, (i >> 4) & 3);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void bsktball_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x01ff).ram(); /* Zero Page RAM */
	map(0x0800, 0x0800).r(FUNC(bsktball_state::bsktball_in0_r));
	map(0x0802, 0x0802).portr("IN1");
	map(0x0803, 0x0803).portr("DSW");
	map(0x1000, 0x1000).nopw(); /* Timer Reset */
	map(0x1010, 0x1010).w(FUNC(bsktball_state::bsktball_bounce_w)); /* Crowd Amp / Bounce */
	map(0x1020, 0x102f).w("outlatch", FUNC(f9334_device::write_a0));
	map(0x1030, 0x1030).w(FUNC(bsktball_state::bsktball_note_w)); /* Music Ckt Note Dvsr */
	map(0x1800, 0x1bbf).ram().w(FUNC(bsktball_state::bsktball_videoram_w)).share("videoram"); /* DISPLAY */
	map(0x1bc0, 0x1bff).ram().share("motion");
	map(0x1c00, 0x1cff).ram();
	map(0x2000, 0x3fff).rom(); /* PROGRAM */
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( bsktball )
	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) /* Sensitivity, clip, min, max */

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* Sensitivity, clip, min, max */

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	/* 0x04 - SPARE */
	/* 0x08 - SPARE */
	/* 0x10 - DR0 = PL2 H DIR */
	/* 0x20 - DR1 = PL2 V DIR */
	/* 0x40 - DR2 = PL1 H DIR */
	/* 0x80 - DR3 = PL1 V DIR */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* SPARE */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* TEST STEP */
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* COIN 0 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) /* COIN 1 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) /* COIN 2 */

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, "Play Time per Credit" ) PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x06, "2:30" )
	PORT_DIPSETTING(    0x05, "2:00" )
	PORT_DIPSETTING(    0x04, "1:30" )
	PORT_DIPSETTING(    0x03, "1:15" )
	PORT_DIPSETTING(    0x02, "0:45" )
	PORT_DIPSETTING(    0x01, "0:30" )
	PORT_DIPSETTING(    0x00, "1:00" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x00, "Cost" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, "Two Coin Minimum" )
	PORT_DIPSETTING(    0x00, "One Coin Minimum" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	2,
	{ 0, 8*0x800 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout motionlayout =
{
	8,32,
	64,
	2,
	{ 0, 8*0x800 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{   0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	32*8
};


static GFXDECODE_START( gfx_bsktball )
	GFXDECODE_ENTRY( "gfx1", 0x0600, charlayout,   0x00, 0x02 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, motionlayout, 0x08, 0x40 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void bsktball_state::machine_start()
{
	save_item(NAME(m_nmi_on));
	save_item(NAME(m_ld1));
	save_item(NAME(m_ld2));
	save_item(NAME(m_dir0));
	save_item(NAME(m_dir1));
	save_item(NAME(m_dir2));
	save_item(NAME(m_dir3));
	save_item(NAME(m_last_p1_horiz));
	save_item(NAME(m_last_p1_vert));
	save_item(NAME(m_last_p2_horiz));
	save_item(NAME(m_last_p2_vert));
}

void bsktball_state::machine_reset()
{
	m_ld1 = 0;
	m_ld2 = 0;
	m_dir0 = 0;
	m_dir1 = 0;
	m_dir2 = 0;
	m_dir3 = 0;
	m_last_p1_horiz = 0;
	m_last_p1_vert = 0;
	m_last_p2_horiz = 0;
	m_last_p2_vert = 0;
}


void bsktball_state::bsktball(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12.096_MHz_XTAL/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &bsktball_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(bsktball_state::irq0_line_hold));
	TIMER(config, "nmi_timer").configure_scanline(FUNC(bsktball_state::bsktball_scanline), "screen", 32, 64);

	f9334_device &outlatch(F9334(config, "outlatch")); // M6
	outlatch.q_out_cb<1>().set([this](int state) { machine().bookkeeping().coin_counter_w(0, state); }); // Coin Counter
	outlatch.q_out_cb<2>().set_output("led0"); // LED 1
	outlatch.q_out_cb<3>().set_output("led1"); // LED 2
	outlatch.q_out_cb<4>().set(FUNC(bsktball_state::ld1_w)); // LD 1
	outlatch.q_out_cb<5>().set(FUNC(bsktball_state::ld2_w)); // LD 2
	outlatch.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<BSKTBALL_NOISE_EN>)); // Noise Reset
	outlatch.q_out_cb<7>().set(FUNC(bsktball_state::nmion_w)); // NMI On

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12.096_MHz_XTAL/2, 384, 0, 256, 262, 0, 224);
	m_screen->set_screen_update(FUNC(bsktball_state::screen_update_bsktball));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bsktball);
	PALETTE(config, m_palette, FUNC(bsktball_state::bsktball_palette), 2*4 + 4*4*4*4, 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, bsktball_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( bsktball )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "034765.d1",    0x2000, 0x0800, CRC(798cea39) SHA1(b1b709a74258b01b21d7c2038a3b6abe879944c5) )
	ROM_LOAD( "034764.c1",    0x2800, 0x0800, CRC(a087109e) SHA1(f5d6dcccc4a54db35be3d8997bc51e73892747fb) )
	ROM_LOAD( "034766.f1",    0x3000, 0x0800, CRC(a82e9a9f) SHA1(9aca236c5145c04a8aaebb316179482bbdc9ddfc) )
	ROM_LOAD( "034763.b1",    0x3800, 0x0800, CRC(1fc69359) SHA1(a215ba3bb18ea2c57c443dfc4c4a0a3846bbedfe) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "034757.a6",    0x0000, 0x0800, CRC(010e8ad3) SHA1(43ce2c2089ec3011e2d28e8257a35efeed0e71c5) )
	ROM_LOAD( "034758.b6",    0x0800, 0x0800, CRC(f7bea344) SHA1(df544bff67bb0334f77cef11792199d9c3f5fdf4) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, bsktball, 0, bsktball, bsktball, bsktball_state, empty_init, ROT0, "Atari", "Atari Basketball", MACHINE_SUPPORTS_SAVE )
