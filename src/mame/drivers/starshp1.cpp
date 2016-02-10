// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Stefan Jokisch
/***************************************************************************

Atari Starship 1 driver

  "starshp1" -> regular version, bonus time for 3500 points
  "starshpp" -> possible prototype, bonus time for 2700 points

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/starshp1.h"




INTERRUPT_GEN_MEMBER(starshp1_state::starshp1_interrupt)
{
	if ((ioport("SYSTEM")->read() & 0x90) != 0x90)
		generic_pulse_irq_line(device.execute(), 0, 1);
}


WRITE8_MEMBER(starshp1_state::starshp1_audio_w)
{
	data &= 1;

	switch (offset & 7)
	{
	case 0:
		m_attract = data;
		m_discrete->write(space, STARSHP1_ATTRACT, data);
		break;
	case 1:
		m_phasor = data;
		m_discrete->write(space, STARSHP1_PHASOR_ON, data);
		break;
	case 2:
		m_discrete->write(space, STARSHP1_KICKER, data);
		break;
	case 3:
		m_discrete->write(space, STARSHP1_SL1, data);
		break;
	case 4:
		m_discrete->write(space, STARSHP1_SL2, data);
		break;
	case 5:
		m_discrete->write(space, STARSHP1_MOLVL, data);
		break;
	case 6:
		m_discrete->write(space, STARSHP1_NOISE_FREQ, data);
		break;
	}

	machine().bookkeeping().coin_lockout_w(0, !m_attract);
	machine().bookkeeping().coin_lockout_w(1, !m_attract);
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
		m_discrete->write(space, STARSHP1_NOISE_AMPLITUDE, data);
		break;
	case 3:
		m_discrete->write(space, STARSHP1_TONE_PITCH, data);
		break;
	case 4:
		m_discrete->write(space, STARSHP1_MOTOR_SPEED, data);
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


WRITE8_MEMBER(starshp1_state::starshp1_misc_w)
{
	data &= 1;

	switch (offset & 7)
	{
	case 0:
		m_ship_explode = data;
		break;
	case 1:
		m_circle_mod = data;
		break;
	case 2:
		m_circle_kill = !data;
		break;
	case 3:
		m_starfield_kill = data;
		break;
	case 4:
		m_inverse = data;
		break;
	case 5:
		/* BLACK HOLE, not used */
		break;
	case 6:
		m_mux = data;
		break;
	case 7:
		output().set_led_value(0, !data);
		break;
	}
}


static ADDRESS_MAP_START( starshp1_map, AS_PROGRAM, 8, starshp1_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_MIRROR(0x100)
	AM_RANGE(0x2c00, 0x3fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("VBLANK")
	AM_RANGE(0xc300, 0xc3ff) AM_WRITE(starshp1_sspic_w) /* spaceship picture */
	AM_RANGE(0xc400, 0xc400) AM_READ_PORT("COINAGE")
	AM_RANGE(0xc400, 0xc4ff) AM_WRITE(starshp1_ssadd_w) /* spaceship address */
	AM_RANGE(0xc800, 0xc9ff) AM_RAM_WRITE(starshp1_playfield_w) AM_SHARE("playfield_ram")
	AM_RANGE(0xcc00, 0xcc0f) AM_WRITEONLY AM_SHARE("hpos_ram")
	AM_RANGE(0xd000, 0xd00f) AM_WRITEONLY AM_SHARE("vpos_ram")
	AM_RANGE(0xd400, 0xd40f) AM_WRITEONLY AM_SHARE("obj_ram")
	AM_RANGE(0xd800, 0xd800) AM_READ(starshp1_rng_r)
	AM_RANGE(0xd800, 0xd80f) AM_WRITE(starshp1_collision_reset_w)
	AM_RANGE(0xdc00, 0xdc0f) AM_WRITE(starshp1_misc_w)
	AM_RANGE(0xdd00, 0xdd0f) AM_WRITE(starshp1_analog_in_w)
	AM_RANGE(0xde00, 0xde0f) AM_WRITE(starshp1_audio_w)
	AM_RANGE(0xdf00, 0xdf0f) AM_WRITE(starshp1_analog_out_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("VBLANK")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, starshp1_state,starshp1_analog_r, NULL)   /* analog in */
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("COINAGE")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, starshp1_state,collision_latch_r, NULL)   /* collision latch */
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

static const UINT32 shiplayout_xoffset[64] =
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


static GFXDECODE_START( starshp1 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 2, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, shiplayout,   6, 2 )
GFXDECODE_END


static MACHINE_CONFIG_START( starshp1, starshp1_state )

	/* basic machine hardware */

	MCFG_CPU_ADD("maincpu", M6502, STARSHP1_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(starshp1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", starshp1_state,  starshp1_interrupt)

	/* video hardware */


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(STARSHP1_PIXEL_CLOCK, STARSHP1_HTOTAL, STARSHP1_HBEND, STARSHP1_HBSTART, STARSHP1_VTOTAL, STARSHP1_VBEND, STARSHP1_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(starshp1_state, screen_update_starshp1)
	MCFG_SCREEN_VBLANK_DRIVER(starshp1_state, screen_eof_starshp1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", starshp1)
	MCFG_PALETTE_ADD("palette", 19)
	MCFG_PALETTE_INDIRECT_ENTRIES(8)
	MCFG_PALETTE_INIT_OWNER(starshp1_state, starshp1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(starshp1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


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


GAME( 1977, starshp1, 0,        starshp1, starshp1, driver_device, 0, ORIENTATION_FLIP_X, "Atari", "Starship 1",              MACHINE_IMPERFECT_SOUND )
GAME( 1977, starshpp, starshp1, starshp1, starshp1, driver_device, 0, ORIENTATION_FLIP_X, "Atari", "Starship 1 (prototype?)", MACHINE_IMPERFECT_SOUND )
