// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Stefan Jokisch
/***************************************************************************

Atari Starship 1 driver

  "starshp1" -> regular version, bonus time for 3500 points
  "starshpp" -> possible prototype, bonus time for 2700 points

***************************************************************************/

#include "emu.h"
#include "includes/starshp1.h"
#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "speaker.h"




INTERRUPT_GEN_MEMBER(starshp1_state::starshp1_interrupt)
{
	if ((ioport("SYSTEM")->read() & 0x90) != 0x90)
		device.execute().pulse_input_line(0, device.execute().minimum_quantum_time());
}


WRITE_LINE_MEMBER(starshp1_state::attract_w)
{
	m_attract = state;
	m_discrete->write(STARSHP1_ATTRACT, state);

	machine().bookkeeping().coin_lockout_w(0, !m_attract);
	machine().bookkeeping().coin_lockout_w(1, !m_attract);
}


WRITE_LINE_MEMBER(starshp1_state::phasor_w)
{
	m_phasor = state;
	m_discrete->write(STARSHP1_PHASOR_ON, state);
}


WRITE8_MEMBER(starshp1_state::starshp1_collision_reset_w)
{
	m_collision_latch = 0;
}


CUSTOM_INPUT_MEMBER(starshp1_state::starshp1_analog_r)
{
	int val = 0;

	switch (m_analog_in_select)
	{
	case 0:
		val = ioport("STICKY")->read();
		break;
	case 1:
		val = ioport("STICKX")->read();
		break;
	case 2:
		val = 0x20; /* DAC feedback, not used */
		break;
	case 3:
		val = ioport("PLAYTIME")->read();
		break;
	}

	return val & 0x3f;
}


CUSTOM_INPUT_MEMBER(starshp1_state::collision_latch_r)
{
	return m_collision_latch & 0x0f;
}


WRITE8_MEMBER(starshp1_state::starshp1_analog_in_w)
{
	m_analog_in_select = offset & 3;
}


WRITE8_MEMBER(starshp1_state::starshp1_analog_out_w)
{
	switch (offset & 7)
	{
	case 1:
		m_ship_size = data;
		break;
	case 2:
		m_discrete->write(STARSHP1_NOISE_AMPLITUDE, data);
		break;
	case 3:
		m_discrete->write(STARSHP1_TONE_PITCH, data);
		break;
	case 4:
		m_discrete->write(STARSHP1_MOTOR_SPEED, data);
		break;
	case 5:
		m_circle_hpos = data;
		break;
	case 6:
		m_circle_vpos = data;
		break;
	case 7:
		m_circle_size = data;
		break;
	}
}


WRITE_LINE_MEMBER(starshp1_state::ship_explode_w)
{
	m_ship_explode = state;
}


WRITE_LINE_MEMBER(starshp1_state::circle_mod_w)
{
	m_circle_mod = state;
}


WRITE_LINE_MEMBER(starshp1_state::circle_kill_w)
{
	m_circle_kill = !state;
}


WRITE_LINE_MEMBER(starshp1_state::starfield_kill_w)
{
	m_starfield_kill = state;
}


WRITE_LINE_MEMBER(starshp1_state::inverse_w)
{
	m_inverse = state;
}


WRITE_LINE_MEMBER(starshp1_state::mux_w)
{
	m_mux = state;
}


WRITE_LINE_MEMBER(starshp1_state::led_w)
{
	m_led = state ? 0 : 1;
}


void starshp1_state::starshp1_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x100);
	map(0x2c00, 0x3fff).rom();
	map(0xa000, 0xa000).portr("SYSTEM");
	map(0xb000, 0xb000).portr("VBLANK");
	map(0xc300, 0xc3ff).w(FUNC(starshp1_state::starshp1_sspic_w)); /* spaceship picture */
	map(0xc400, 0xc400).portr("COINAGE");
	map(0xc400, 0xc4ff).w(FUNC(starshp1_state::starshp1_ssadd_w)); /* spaceship address */
	map(0xc800, 0xc9ff).ram().w(FUNC(starshp1_state::starshp1_playfield_w)).share("playfield_ram");
	map(0xcc00, 0xcc0f).writeonly().share("hpos_ram");
	map(0xd000, 0xd00f).writeonly().share("vpos_ram");
	map(0xd400, 0xd40f).writeonly().share("obj_ram");
	map(0xd800, 0xd800).r(FUNC(starshp1_state::starshp1_rng_r));
	map(0xd800, 0xd80f).w(FUNC(starshp1_state::starshp1_collision_reset_w));
	map(0xdc00, 0xdc07).mirror(0x0008).w("misclatch", FUNC(f9334_device::write_d0));
	map(0xdd00, 0xdd0f).w(FUNC(starshp1_state::starshp1_analog_in_w));
	map(0xde00, 0xde07).mirror(0x0008).w("audiolatch", FUNC(f9334_device::write_d0));
	map(0xdf00, 0xdf0f).w(FUNC(starshp1_state::starshp1_analog_out_w));
	map(0xf000, 0xffff).rom();
}


void starshp1_state::machine_start()
{
	m_led.resolve();
}


static INPUT_PORTS_START( starshp1 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* SWA1? */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x20, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	// IPT_BUTTON3 is the Speed lever (Throttle)
	// This is _not_ IPT_TOGGLE, even though it looks like one.
	// It returns to SLOW unless you hold it down (FAST)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("VBLANK")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, starshp1_state,starshp1_analog_r, nullptr)   /* analog in */
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("COINAGE")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, starshp1_state,collision_latch_r, nullptr)   /* collision latch */
	PORT_DIPNAME( 0x70, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) /* ground */

	PORT_START("PLAYTIME")
	PORT_DIPNAME( 0x3f, 0x20, "Play Time" ) /* potentiometer */
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x20, "90 Seconds" )
	PORT_DIPSETTING(    0x3f, "120 Seconds" )

	PORT_START("STICKY")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("STICKX")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16, 8,  /* 16x8 tiles      */
	64,     /* 64 tiles        */
	1,      /* 1 bit per pixel */
	{ 0 },
	{
		0x204, 0x204, 0x205, 0x205, 0x206, 0x206, 0x207, 0x207,
		0x004, 0x004, 0x005, 0x005, 0x006, 0x006, 0x007, 0x007
	},
	{
		0x0000, 0x0400, 0x0800, 0x0c00,
		0x1000, 0x1400, 0x1800, 0x1c00
	},
	8       /* step */
};


static const gfx_layout spritelayout =
{
	16, 8,  /* 16x8 sprites    */
	8,      /* 8 sprites       */
	1,      /* 1 bit per pixel */
	{ 0 },
	{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f
	},
	{
		0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0
	},
	0x100   /* step */
};

static const uint32_t shiplayout_xoffset[64] =
{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3f,
		0x44, 0x45, 0x46, 0x47, 0x4c, 0x4d, 0x4e, 0x4f,
		0x54, 0x55, 0x56, 0x57, 0x5c, 0x5d, 0x5e, 0x5f,
		0x64, 0x65, 0x66, 0x67, 0x6c, 0x6d, 0x6e, 0x6f,
		0x74, 0x75, 0x76, 0x77, 0x7c, 0x7d, 0x7e, 0x7f
};

static const gfx_layout shiplayout =
{
	64, 16, /* 64x16 sprites    */
	4,      /* 4 sprites        */
	2,      /* 2 bits per pixel */
	{ 0, 0x2000 },
	EXTENDED_XOFFS,
	{ STEP16(0x000, 0x080) },
	0x800,  /* step */
	shiplayout_xoffset,
	nullptr
};


static GFXDECODE_START( gfx_starshp1 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 2, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, shiplayout,   6, 2 )
GFXDECODE_END


void starshp1_state::starshp1(machine_config &config)
{
	/* basic machine hardware */

	M6502(config, m_maincpu, STARSHP1_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &starshp1_state::starshp1_map);
	m_maincpu->set_vblank_int("screen", FUNC(starshp1_state::starshp1_interrupt));

	f9334_device &misclatch(F9334(config, "misclatch")); // C8
	misclatch.q_out_cb<0>().set(FUNC(starshp1_state::ship_explode_w));
	misclatch.q_out_cb<1>().set(FUNC(starshp1_state::circle_mod_w));
	misclatch.q_out_cb<2>().set(FUNC(starshp1_state::circle_kill_w));
	misclatch.q_out_cb<3>().set(FUNC(starshp1_state::starfield_kill_w));
	misclatch.q_out_cb<4>().set(FUNC(starshp1_state::inverse_w));
	misclatch.q_out_cb<5>().set_nop(); // BLACK HOLE, not used
	misclatch.q_out_cb<6>().set(FUNC(starshp1_state::mux_w));
	misclatch.q_out_cb<7>().set(FUNC(starshp1_state::led_w));

	/* video hardware */


	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(STARSHP1_PIXEL_CLOCK, STARSHP1_HTOTAL, STARSHP1_HBEND, STARSHP1_HBSTART, STARSHP1_VTOTAL, STARSHP1_VBEND, STARSHP1_VBSTART);
	m_screen->set_screen_update(FUNC(starshp1_state::screen_update_starshp1));
	m_screen->screen_vblank().set(FUNC(starshp1_state::screen_vblank_starshp1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_starshp1);
	PALETTE(config, m_palette, FUNC(starshp1_state::starshp1_palette), 19, 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, starshp1_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);

	f9334_device &audiolatch(F9334(config, "audiolatch")); // D9
	audiolatch.q_out_cb<0>().set(FUNC(starshp1_state::attract_w));
	audiolatch.q_out_cb<1>().set(FUNC(starshp1_state::phasor_w));
	audiolatch.q_out_cb<2>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_KICKER>));
	audiolatch.q_out_cb<3>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_SL1>));
	audiolatch.q_out_cb<4>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_SL2>));
	audiolatch.q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_MOLVL>));
	audiolatch.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_NOISE_FREQ>));
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( starshp1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "7529-02.c2", 0x2c00, 0x0400, CRC(f191c328) SHA1(5d44be879bcf16a142a69e4f1501533e02720fe5) )
	ROM_LOAD_NIB_LOW ( "7528-02.c1", 0x2c00, 0x0400, CRC(605ed4df) SHA1(b0d892bcd08b611d2c01ab23b491c1d9db498e7b) )
	ROM_LOAD(          "7530-02.h3", 0x3000, 0x0800, CRC(4b2d466c) SHA1(2104c4d163adbf53f9853334868622752ccb01b8) )
	ROM_RELOAD(                      0xf000, 0x0800 )
	ROM_LOAD(          "7531-02.e3", 0x3800, 0x0800, CRC(b35b2c0e) SHA1(e52240cdfbba3dc380ba63f24cfc07b44feafd53) )
	ROM_RELOAD(                      0xf800, 0x0800 )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "7513-01.n7",  0x0000, 0x0400, CRC(8fb0045d) SHA1(fb311c6977dec6e2a04179406e9ffdb920989a47) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "7515-01.j5",  0x0000, 0x0100, CRC(fcbcbf2e) SHA1(adf3cc43b77ad18eddbe39ee11625e552d1abab9) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "7517-01.r1",  0x0000, 0x0400, CRC(1531f85f) SHA1(291822614fc6d3a71bf56607c796e18779f8cfc9) )
	ROM_LOAD( "7516-01.p1",  0x0400, 0x0400, CRC(64fbfe4c) SHA1(b2dfdcc1c9927c693fe43b2e1411d0f14375fdeb) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "7518-01.r10", 0x0000, 0x0100, CRC(80877f7e) SHA1(8b28f48936a4247c583ca6713bfbaf4772c7a4f5) ) /* video output */
	ROM_LOAD( "7514-01.n9",  0x0100, 0x0100, CRC(3610b453) SHA1(9e33ee04f22a9174c29fafb8e71781fa330a7a08) ) /* sync */
	ROM_LOAD( "7519-01.b5",  0x0200, 0x0020, CRC(23b9cd3c) SHA1(220f9f73d86cdcf1b390c52c591750a73402af50) ) /* address */
ROM_END

ROM_START( starshpp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "7529-02.c2", 0x2c00, 0x0400, CRC(f191c328) SHA1(5d44be879bcf16a142a69e4f1501533e02720fe5) )
	ROM_LOAD_NIB_LOW ( "7528-02.c1", 0x2c00, 0x0400, CRC(605ed4df) SHA1(b0d892bcd08b611d2c01ab23b491c1d9db498e7b) )
	ROM_LOAD_NIB_HIGH( "7521.h2", 0x3000, 0x0400, CRC(6e3525db) SHA1(b615c60e4958d6576f4c179bbead9e8d330bba99) )
	ROM_RELOAD(                   0xf000, 0x0400 )
	ROM_LOAD_NIB_LOW ( "7520.h1", 0x3000, 0x0400, CRC(2fbed61b) SHA1(5cbe1aee82a32edbf33780a46e4166ec45c88170) )
	ROM_RELOAD(                   0xf000, 0x0400 )
	ROM_LOAD_NIB_HIGH( "f2",      0x3400, 0x0400, CRC(590ea913) SHA1(4baf5a6f6c9dcc5916163f85cec01d78a339ae20) )
	ROM_RELOAD(                   0xf400, 0x0400 )
	ROM_LOAD_NIB_LOW ( "f1",      0x3400, 0x0400, CRC(84fce404) SHA1(edd78f5439c4087c4a853d66446433f9a356b17f) )
	ROM_RELOAD(                   0xf400, 0x0400 )
	ROM_LOAD_NIB_HIGH( "7525.e2", 0x3800, 0x0400, CRC(5c6d12d9) SHA1(7078b685d859fd4122b814e473c83647b81ef7cd) )
	ROM_RELOAD(                   0xf800, 0x0400 )
	ROM_LOAD_NIB_LOW ( "7524.e1", 0x3800, 0x0400, CRC(6193a7bd) SHA1(3c9eab14481cb29ba2627bc73434f579d6b96a6e) )
	ROM_RELOAD(                   0xf800, 0x0400 )
	ROM_LOAD_NIB_HIGH( "d2",      0x3c00, 0x0400, CRC(a17df2ea) SHA1(ec488f4af47594e20b3d51882ee862a92e2f38fd) )
	ROM_RELOAD(                   0xfc00, 0x0400 )
	ROM_LOAD_NIB_LOW ( "d1",      0x3c00, 0x0400, CRC(be4050b6) SHA1(03ca4833769efb10f18f52b7ba4d016568d3cab9) )
	ROM_RELOAD(                   0xfc00, 0x0400 )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "7513-01.n7", 0x0000, 0x0400, CRC(8fb0045d) SHA1(fb311c6977dec6e2a04179406e9ffdb920989a47) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "7515-01.j5", 0x0000, 0x0100, CRC(fcbcbf2e) SHA1(adf3cc43b77ad18eddbe39ee11625e552d1abab9) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "7517-01.r1", 0x0000, 0x0400, CRC(1531f85f) SHA1(291822614fc6d3a71bf56607c796e18779f8cfc9) )
	ROM_LOAD( "7516-01.p1", 0x0400, 0x0400, CRC(64fbfe4c) SHA1(b2dfdcc1c9927c693fe43b2e1411d0f14375fdeb) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "7518-01.r10", 0x0000, 0x0100, CRC(80877f7e) SHA1(8b28f48936a4247c583ca6713bfbaf4772c7a4f5) ) /* video output */
	ROM_LOAD( "7514-01.n9",  0x0100, 0x0100, CRC(3610b453) SHA1(9e33ee04f22a9174c29fafb8e71781fa330a7a08) ) /* sync */
	ROM_LOAD( "7519-01.b5",  0x0200, 0x0020, CRC(23b9cd3c) SHA1(220f9f73d86cdcf1b390c52c591750a73402af50) ) /* address */
ROM_END


GAME( 1977, starshp1, 0,        starshp1, starshp1, starshp1_state, empty_init, ORIENTATION_FLIP_X, "Atari", "Starship 1",              MACHINE_IMPERFECT_SOUND )
GAME( 1977, starshpp, starshp1, starshp1, starshp1, starshp1_state, empty_init, ORIENTATION_FLIP_X, "Atari", "Starship 1 (prototype?)", MACHINE_IMPERFECT_SOUND )
