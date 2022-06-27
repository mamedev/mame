// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Triple Hunt Driver

  Calibrate controls in service mode the first time you run this game.

***************************************************************************/

#include "emu.h"
#include "triplhnt.h"
#include "cpu/m6800/m6800.h"
#include "machine/nvram.h"
#include "speaker.h"



void triplhnt_state::init_triplhnt()
{
	subdevice<nvram_device>("nvram")->set_base(m_cmos, sizeof(m_cmos));
}


TIMER_CALLBACK_MEMBER(triplhnt_state::set_collision)
{
	m_hit_code = param;

	m_maincpu->set_input_line(0, HOLD_LINE);
}


WRITE_LINE_MEMBER(triplhnt_state::coin_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
	machine().bookkeeping().coin_lockout_w(1, !state);
}


WRITE_LINE_MEMBER(triplhnt_state::tape_control_w)
{
	bool is_witch_hunt = ioport("0C09")->read() == 0x40;
	bool bit = !state;

	/* if we're not playing the sample yet, start it */
	if (!m_samples->playing(0))
		m_samples->start(0, 0, true);
	if (!m_samples->playing(1))
		m_samples->start(1, 1, true);

	/* bit 6 turns cassette on/off */
	m_samples->pause(0,  is_witch_hunt || bit);
	m_samples->pause(1, !is_witch_hunt || bit);
}


uint8_t triplhnt_state::cmos_r(offs_t offset)
{
	m_cmos_latch = offset;

	return m_cmos[m_cmos_latch] ^ 15;
}


uint8_t triplhnt_state::input_port_4_r()
{
	m_watchdog->watchdog_reset();
	return ioport("0C0B")->read();
}


uint8_t triplhnt_state::misc_r(offs_t offset)
{
	m_latch->write_a0(offset);
	return ioport("VBLANK")->read() | m_hit_code;
}


uint8_t triplhnt_state::da_latch_r(offs_t offset)
{
	int cross_x = ioport("STICKX")->read();
	int cross_y = ioport("STICKY")->read();

	m_da_latch = offset;

	/* the following is a slight simplification */

	return (offset & 1) ? cross_x : cross_y;
}


void triplhnt_state::machine_start()
{
	m_lamp.resolve();

	m_hit_code = 0;
}


void triplhnt_state::triplhnt_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram().mirror(0x300);
	map(0x0400, 0x04ff).writeonly().share("playfield_ram");
	map(0x0800, 0x080f).writeonly().share("vpos_ram");
	map(0x0810, 0x081f).writeonly().share("hpos_ram");
	map(0x0820, 0x082f).writeonly().share("orga_ram");
	map(0x0830, 0x083f).writeonly().share("code_ram");
	map(0x0c00, 0x0c00).portr("0C00");
	map(0x0c08, 0x0c08).portr("0C08");
	map(0x0c09, 0x0c09).portr("0C09");
	map(0x0c0a, 0x0c0a).portr("0C0A");
	map(0x0c0b, 0x0c0b).r(FUNC(triplhnt_state::input_port_4_r));
	map(0x0c10, 0x0c1f).r(FUNC(triplhnt_state::da_latch_r));
	map(0x0c20, 0x0c2f).r(FUNC(triplhnt_state::cmos_r)).share("nvram");
	map(0x0c30, 0x0c3f).r(FUNC(triplhnt_state::misc_r)).w(m_latch, FUNC(f9334_device::write_a0));
	map(0x0c40, 0x0c40).portr("0C40");
	map(0x0c48, 0x0c48).portr("0C48");
	map(0x7000, 0x7fff).rom(); /* program */
}


static INPUT_PORTS_START( triplhnt )
	PORT_START("0C00")  /* 0C00 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("0C08")  /* 0C08 */
	PORT_DIPNAME( 0xc0, 0x00, "Play Time" )
	PORT_DIPSETTING( 0x00, "32 seconds / 16 raccoons" )
	PORT_DIPSETTING( 0x40, "64 seconds / 32 raccoons" )
	PORT_DIPSETTING( 0x80, "96 seconds / 48 raccoons" )
	PORT_DIPSETTING( 0xc0, "128 seconds / 64 raccoons" )

	PORT_START("0C09")  /* 0C09 */
	PORT_DIPNAME( 0xc0, 0x40, "Game Select" )
	PORT_DIPSETTING( 0x00, "Hit the Bear" )
	PORT_DIPSETTING( 0x40, "Witch Hunt" )
	PORT_DIPSETTING( 0xc0, "Raccoon Hunt" )

	PORT_START("0C0A")  /* 0C0A */
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING( 0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING( 0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING( 0x80, DEF_STR( 1C_2C ))

	PORT_START("0C0B")  /* 0C0B */
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))

	PORT_START("0C40")  /* 0C40 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("0C48")  /* 0C48 */
// default to service enabled to make users calibrate gun
//  PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Service_Mode )) PORT_TOGGLE PORT_CODE(KEYCODE_F2)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("VBLANK")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("STICKX")
	PORT_BIT( 0xfc, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xfc)  PORT_CROSSHAIR(X, 62.0/64, 1.0/64, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15)

	PORT_START("STICKY")
	PORT_BIT( 0xfc, 0x78, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xec)  PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15)

	PORT_START("BEAR")  /* 10 */
	PORT_ADJUSTER( 35, "Bear Roar Frequency" )
INPUT_PORTS_END


static const gfx_layout triplhnt_small_sprite_layout =
{
	32, 32,   /* width, height */
	16,       /* total         */
	2,        /* planes        */
				/* plane offsets */
	{ 0x0000, 0x4000 },
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0,
		0x200, 0x220, 0x240, 0x260, 0x280, 0x2A0, 0x2C0, 0x2E0,
		0x300, 0x320, 0x340, 0x360, 0x380, 0x3A0, 0x3C0, 0x3E0
	},
	0x400     /* increment */
};


static const uint32_t triplhnt_large_sprite_layout_xoffset[64] =
{
		0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
		0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
		0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
		0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
		0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13,
		0x14, 0x14, 0x15, 0x15, 0x16, 0x16, 0x17, 0x17,
		0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
		0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F
};

static const uint32_t triplhnt_large_sprite_layout_yoffset[64] =
{
		0x000, 0x000, 0x020, 0x020, 0x040, 0x040, 0x060, 0x060,
		0x080, 0x080, 0x0A0, 0x0A0, 0x0C0, 0x0C0, 0x0E0, 0x0E0,
		0x100, 0x100, 0x120, 0x120, 0x140, 0x140, 0x160, 0x160,
		0x180, 0x180, 0x1A0, 0x1A0, 0x1C0, 0x1C0, 0x1E0, 0x1E0,
		0x200, 0x200, 0x220, 0x220, 0x240, 0x240, 0x260, 0x260,
		0x280, 0x280, 0x2A0, 0x2A0, 0x2C0, 0x2C0, 0x2E0, 0x2E0,
		0x300, 0x300, 0x320, 0x320, 0x340, 0x340, 0x360, 0x360,
		0x380, 0x380, 0x3A0, 0x3A0, 0x3C0, 0x3C0, 0x3E0, 0x3E0
};

static const gfx_layout triplhnt_large_sprite_layout =
{
	64, 64,   /* width, height */
	16,       /* total         */
	2,        /* planes        */
	{ 0x0000, 0x4000 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x400,
	triplhnt_large_sprite_layout_xoffset,
	triplhnt_large_sprite_layout_yoffset
};


static const gfx_layout triplhnt_tile_layout =
{
	16, 16,   /* width, height */
	64,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7
	},
	{
		0x00, 0x00, 0x08, 0x08, 0x10, 0x10, 0x18, 0x18,
		0x20, 0x20, 0x28, 0x28, 0x30, 0x30, 0x38, 0x38
	},
	0x40      /* increment */
};


static GFXDECODE_START( gfx_triplhnt )
	GFXDECODE_ENTRY( "gfx1", 0, triplhnt_small_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, triplhnt_large_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, triplhnt_tile_layout, 4, 2 )
GFXDECODE_END


void triplhnt_state::triplhnt_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xaf, 0xaf, 0xaf));  // sprites
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(2, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(3, rgb_t(0x50, 0x50, 0x50));
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0x00));  // tiles
	palette.set_pen_color(5, rgb_t(0x3f, 0x3f, 0x3f));
	palette.set_pen_color(6, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(7, rgb_t(0x3f, 0x3f, 0x3f));
}


void triplhnt_state::triplhnt(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 800000);
	m_maincpu->set_addrmap(AS_PROGRAM, &triplhnt_state::triplhnt_map);
	m_maincpu->set_vblank_int("screen", FUNC(triplhnt_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // battery-backed 74C89 at J5

	F9334(config, m_latch); // J7
	m_latch->q_out_cb<0>().set_nop(); // unused
	m_latch->q_out_cb<1>().set([this] (int state) { m_lamp = state ? 1 : 0; });
	m_latch->q_out_cb<1>().append(m_discrete, FUNC(discrete_device::write_line<TRIPLHNT_LAMP_EN>)); // Lamp is used to reset noise
	m_latch->q_out_cb<2>().set(m_discrete, FUNC(discrete_device::write_line<TRIPLHNT_SCREECH_EN>)); // screech
	m_latch->q_out_cb<3>().set(FUNC(triplhnt_state::coin_lockout_w));
	m_latch->q_out_cb<4>().set([this] (int state) { m_sprite_zoom = state; });
	m_latch->q_out_cb<5>().set([this] (int state) { if (state) m_cmos[m_cmos_latch] = m_da_latch; }); // CMOS write
	m_latch->q_out_cb<6>().set(FUNC(triplhnt_state::tape_control_w));
	m_latch->q_out_cb<7>().set([this] (int state) { m_sprite_bank = state; });
	m_latch->q_out_cb<7>().append(m_discrete, FUNC(discrete_device::write_line<TRIPLHNT_BEAR_EN>)); // bear

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(256, 262);
	screen.set_visarea(0, 255, 0, 239);
	screen.set_screen_update(FUNC(triplhnt_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_triplhnt);
	PALETTE(config, m_palette, FUNC(triplhnt_state::triplhnt_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);  /* 2 channels */
	m_samples->set_samples_names(triplhnt_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.20);

	DISCRETE(config, m_discrete, triplhnt_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.90);
}


ROM_START( triplhnt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "8404.f1", 0x7000, 0x400, CRC(abc8acd5) SHA1(bcef2abc5829829a01aa21776c3deb2e1bf1d4ac) )
	ROM_LOAD_NIB_LOW ( "8408.f2", 0x7000, 0x400, CRC(77fcdd3f) SHA1(ce0196abb8d6510aa9a5308f8efd6442e94272c4) )
	ROM_LOAD_NIB_HIGH( "8403.e1", 0x7400, 0x400, CRC(8d756fa1) SHA1(48a74f710b130d9af0c866483d6fc4ecce4a3ac5) )
	ROM_LOAD_NIB_LOW ( "8407.e2", 0x7400, 0x400, CRC(de268f4b) SHA1(937f97421ffb4f0f704402847892382ae8032b7c) )
	ROM_LOAD_NIB_HIGH( "8402.d1", 0x7800, 0x400, CRC(eb75c936) SHA1(48f9d4113a7ab8413a5aacd44b3506afc99d26ce) )
	ROM_LOAD_NIB_LOW ( "8406.d2", 0x7800, 0x400, CRC(e7ab1186) SHA1(7185fb837966bfb4aa70be3dd948d44f356b0452) )
	ROM_LOAD_NIB_HIGH( "8401.c1", 0x7C00, 0x400, CRC(7461b05e) SHA1(16573ae655c306a38ff0f29a3c3285d636907f38) )
	ROM_LOAD_NIB_LOW ( "8405.c2", 0x7C00, 0x400, CRC(ba370b97) SHA1(5d799ce6ae56c315ff0abedea7ad9204bacc266b) )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "8423.n1", 0x0000, 0x800, CRC(9937d0da) SHA1(abb906c2d9869b09be5172cc7639bb9cda38831b) )
	ROM_LOAD( "8422.r1", 0x0800, 0x800, CRC(803621dd) SHA1(ffbd7f87a86477e5eb94f12fc20a837128a02442) )

	ROM_REGION( 0x200, "gfx2", 0 )   /* tiles */
	ROM_LOAD_NIB_HIGH( "8409.l3", 0x0000, 0x200, CRC(ec304172) SHA1(ccbf7e117fef7fa4288e3bf68f1a150b3a492ce6) )
	ROM_LOAD_NIB_LOW ( "8410.m3", 0x0000, 0x200, CRC(f75a1b08) SHA1(81b4733194462cd4cef7f4221ecb7abd1556b871) )
ROM_END


GAME( 1977, triplhnt, 0, triplhnt, triplhnt, triplhnt_state, init_triplhnt, 0, "Atari", "Triple Hunt", MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
