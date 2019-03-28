// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Poolshark Driver

***************************************************************************/

#include "emu.h"
#include "includes/poolshrk.h"

#include "cpu/m6800/m6800.h"
#include "sound/discrete.h"

#include "screen.h"
#include "speaker.h"



void poolshrk_state::init_poolshrk()
{
	uint8_t* pSprite = memregion("gfx1")->base();
	uint8_t* pOffset = memregion("proms")->base();

	/* re-arrange sprite data using the PROM */

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			uint16_t v =
				(pSprite[0] << 0xC) |
				(pSprite[1] << 0x8) |
				(pSprite[2] << 0x4) |
				(pSprite[3] << 0x0);

			v >>= pOffset[j];

			pSprite[0] = (v >> 0xC) & 15;
			pSprite[1] = (v >> 0x8) & 15;
			pSprite[2] = (v >> 0x4) & 15;
			pSprite[3] = (v >> 0x0) & 15;

			pSprite += 4;
		}
	}

	save_item(NAME(m_da_latch));
}


WRITE8_MEMBER(poolshrk_state::da_latch_w)
{
	m_da_latch = data & 15;
}


WRITE8_MEMBER(poolshrk_state::led_w)
{
	if (offset & 2)
		m_leds[0] = BIT(offset, 0);
	if (offset & 4)
		m_leds[1] = BIT(offset, 0);
}


WRITE8_MEMBER(poolshrk_state::watchdog_w)
{
	if ((offset & 3) == 3)
	{
		m_watchdog->reset_w(space, 0, 0);
	}
}


READ8_MEMBER(poolshrk_state::input_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };
	uint8_t val = ioport(portnames[offset & 3])->read();

	int x = ioport((offset & 1) ? "AN1" : "AN0")->read();
	int y = ioport((offset & 1) ? "AN3" : "AN2")->read();

	if (x >= m_da_latch) val |= 8;
	if (y >= m_da_latch) val |= 4;

	if ((offset & 3) == 3)
	{
		m_watchdog->reset_r(space, 0);
	}

	return val;
}


READ8_MEMBER(poolshrk_state::irq_reset_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}


void poolshrk_state::poolshrk_cpu_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).mirror(0x2300).ram();
	map(0x0400, 0x07ff).mirror(0x2000).writeonly().share("playfield_ram");
	map(0x0800, 0x080f).mirror(0x23f0).writeonly().share("hpos_ram");
	map(0x0c00, 0x0c0f).mirror(0x23f0).writeonly().share("vpos_ram");
	map(0x1000, 0x13ff).mirror(0x2000).rw(FUNC(poolshrk_state::input_r), FUNC(poolshrk_state::watchdog_w));
	map(0x1400, 0x17ff).mirror(0x2000).w(FUNC(poolshrk_state::scratch_sound_w));
	map(0x1800, 0x1bff).mirror(0x2000).w(FUNC(poolshrk_state::score_sound_w));
	map(0x1c00, 0x1fff).mirror(0x2000).w(FUNC(poolshrk_state::click_sound_w));
	map(0x4000, 0x4000).noprw(); /* diagnostic ROM location */
	map(0x6000, 0x63ff).w(FUNC(poolshrk_state::da_latch_w));
	map(0x6400, 0x67ff).w(FUNC(poolshrk_state::bump_sound_w));
	map(0x6800, 0x6bff).r(FUNC(poolshrk_state::irq_reset_r));
	map(0x6c00, 0x6fff).w(FUNC(poolshrk_state::led_w));
	map(0x7000, 0x7fff).rom();
}


static INPUT_PORTS_START( poolshrk )
	PORT_START("IN0")
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, "Racks Per Game" )
	PORT_DIPSETTING( 0x03, "2" )
	PORT_DIPSETTING( 0x02, "3" )
	PORT_DIPSETTING( 0x01, "4" )
	PORT_DIPSETTING( 0x00, "5" )
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ))
	PORT_DIPSETTING( 0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_4C ))
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("AN0")
	PORT_BIT( 15, 8, IPT_AD_STICK_X ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 15, 8, IPT_AD_STICK_X ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_START("AN2")
	PORT_BIT( 15, 8, IPT_AD_STICK_Y ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 15, 8, IPT_AD_STICK_Y ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(2)

INPUT_PORTS_END


static const gfx_layout poolshrk_sprite_layout =
{
	16, 16,   /* width, height */
	16,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
		0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0
	},
	0x200     /* increment */
};


static const gfx_layout poolshrk_tile_layout =
{
	8, 8,     /* width, height */
	64,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		7, 6, 5, 4, 3, 2, 1, 0
	},
	{
		0x000, 0x200, 0x400, 0x600, 0x800, 0xA00, 0xC00, 0xE00
	},
	0x8       /* increment */
};


static GFXDECODE_START( gfx_poolshrk )
	GFXDECODE_ENTRY( "gfx1", 0, poolshrk_sprite_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, poolshrk_tile_layout, 0, 1 )
GFXDECODE_END


void poolshrk_state::poolshrk_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x7f, 0x7f, 0x7f));
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(2, rgb_t(0x7f, 0x7f, 0x7f));
	palette.set_pen_color(3, rgb_t(0x00, 0x00, 0x00));
}


void poolshrk_state::poolshrk(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 11055000 / 8); /* ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &poolshrk_state::poolshrk_cpu_map);
	m_maincpu->set_vblank_int("screen", FUNC(poolshrk_state::irq0_line_assert));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(256, 256);
	screen.set_visarea(1, 255, 24, 255);
	screen.set_screen_update(FUNC(poolshrk_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_poolshrk);
	PALETTE(config, m_palette, FUNC(poolshrk_state::poolshrk_palette), 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, poolshrk_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( poolshrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7329.k1", 0x7000, 0x800, CRC(88152245) SHA1(c7c5e43ea488a197e92a1dc2231578f8ed86c98d) )
	ROM_LOAD( "7330.l1", 0x7800, 0x800, CRC(fb41d3e9) SHA1(c17994179362da13acfcd36a28f45e328428c031) )

	ROM_REGION( 0x400, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "7325.j5", 0x0000, 0x200, CRC(fae87eed) SHA1(8891d0ea60f72f826d71dc6b064a2ba81b298914) )
	ROM_LOAD( "7326.h5", 0x0200, 0x200, CRC(05ec9762) SHA1(6119c4529334c98a0a42ca13a98a8661fc594d80) )

	ROM_REGION( 0x200, "gfx2", 0 )   /* tiles */
	ROM_LOAD( "7328.n6", 0x0000, 0x200, CRC(64bcbf3a) SHA1(a4e3ce6b4734234359e3ef784a771e40580c2a2a) )

	ROM_REGION( 0x20, "proms", 0 )                   /* line offsets */
	ROM_LOAD( "7327.k6", 0x0000, 0x020, CRC(f74cef5b) SHA1(f470bf5b193dae4b44e89bc4c4476cf8d98e7cfd) )
ROM_END


GAME( 1977, poolshrk, 0, poolshrk, poolshrk, poolshrk_state, init_poolshrk, 0, "Atari", "Poolshark", MACHINE_SUPPORTS_SAVE )
