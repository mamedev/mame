// license:BSD-3-Clause
// copyright-holders: Uki
/*****************************************************************************

Ikki (c) 1985 Sun Electronics

    Driver by Uki

    20/Jun/2001 -

TODO:
- understand proper CPU communications and irq firing;
- merge with markham.cpp
- timings

*****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class ikki_state : public driver_device
{
public:
	ikki_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_videoattr_prom(*this, "videoattr_proms")
	{ }

	void ikki(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_videoattr_prom;

	// video-related
	bitmap_ind16 m_sprite_bitmap{};
	uint8_t m_flipscreen = 0U;
	uint16_t m_punch_through_pen = 0U;
	uint8_t m_irq_source = 0U;

	uint8_t e000_r();
	void coin_counters(uint8_t data);
	void scrn_ctrl_w(uint8_t data);
	void palette(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


void ikki_state::palette(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i | 0x000]);
		int const g = pal4bit(color_prom[i | 0x100]);
		int const b = pal4bit(color_prom[i | 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// sprites lookup table
	for (int i = 0; i < 0x200; i++)
	{
		uint16_t ctabentry = color_prom[i] ^ 0xff;

		if (((i & 0x07) == 0x07) && (ctabentry == 0))
		{
			// punch through
			m_punch_through_pen = i;
			ctabentry = 0x100;
		}

		palette.set_pen_indirect(i, ctabentry);
	}

	// bg lookup table
	for (int i = 0x200; i < 0x400; i++)
	{
		uint8_t const ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}
}

void ikki_state::scrn_ctrl_w(uint8_t data)
{
	m_flipscreen = BIT(data, 2);
}


void ikki_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprite_bitmap.fill(m_punch_through_pen, cliprect);

	for (offs_t offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int const code = (m_spriteram[offs + 2] & 0x80) | (m_spriteram[offs + 1] >> 1);
		int const color = m_spriteram[offs + 2] & 0x3f;

		int x = m_spriteram[offs + 3];
		int y = m_spriteram[offs + 0];

		if (m_flipscreen)
			x = 240 - x;
		else
			y = 224 - y;

		x = x & 0xff;
		y = y & 0xff;

		if (x > 248)
			x = x - 256;

		if (y > 240)
			y = y - 256;

		m_gfxdecode->gfx(1)->transmask(m_sprite_bitmap, cliprect,
				code, color,
				m_flipscreen, m_flipscreen,
				x, y,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}

	// copy the sprite bitmap into the main bitmap, skipping the transparent pixels
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t const pen = m_sprite_bitmap.pix(y, x);

			if (m_palette->pen_indirect(pen) != 0x100)
				bitmap.pix(y, x) = pen;
		}
	}
}


void ikki_state::video_start()
{
	m_screen->register_screen_bitmap(m_sprite_bitmap);
	save_item(NAME(m_sprite_bitmap));
}


uint32_t ikki_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw bg layer
	for (offs_t offs = 0; offs < (m_videoram.bytes() / 2); offs++)
	{
		int const sx = offs / 32;
		int const sy = offs % 32;
		int y = sy * 8;
		int x = sx * 8;

		int const d = m_videoattr_prom[sx];
		switch (d)
		{
			case 0x02: // scroll area
				x = sx * 8 - m_scroll[1];
				if (x < 0)
					x += 8 * 22;
				y = (sy * 8 + ~m_scroll[0]) & 0xff;
				break;

			case 0x03: // non-scroll area
				break;

			case 0x00: // sprite disable?
				break;

			case 0x0d: // sprite disable?
				break;

			case 0x0b: // non-scroll area (?)
				break;

			case 0x0e: // unknown
				break;
		}

		if (m_flipscreen)
		{
			x = 248 - x;
			y = 248 - y;
		}

		int color = m_videoram[offs * 2];
		int const bank = (color & 0xe0) << 3;
		color = ((color & 0x1f) << 0) | ((color & 0x80) >> 2);

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
				m_videoram[offs * 2 + 1] + bank,
				color,
				m_flipscreen, m_flipscreen,
				x, y);
	}

	draw_sprites(bitmap, cliprect);

	// mask sprites
	for (offs_t offs = 0; offs < (m_videoram.bytes() / 2); offs++)
	{
		int const sx = offs / 32;
		int const sy = offs % 32;

		int const d = m_videoattr_prom[sx];
		if ((d == 0) || (d == 0x0d))
		{
			int y = sy * 8;
			int x = sx * 8;

			if (m_flipscreen)
			{
				x = 248 - x;
				y = 248 - y;
			}

			int color = m_videoram[offs * 2];
			int const bank = (color & 0xe0) << 3;
			color = ((color & 0x1f)<<0) | ((color & 0x80) >> 2);

			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
					m_videoram[offs * 2 + 1] + bank,
					color,
					m_flipscreen, m_flipscreen,
					x, y);
		}
	}

	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t ikki_state::e000_r()
{
// bit1: interrupt type?, bit0: CPU2 busack?

	return (m_irq_source << 1);
}

void ikki_state::coin_counters(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void ikki_state::main_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xcfff).ram().share("shared_ram");
	map(0xd000, 0xd7ff).ram().share(m_videoram);
	map(0xe000, 0xe000).r(FUNC(ikki_state::e000_r));
	map(0xe001, 0xe001).portr("DSW1");
	map(0xe002, 0xe002).portr("DSW2");
	map(0xe003, 0xe003).portr("SYSTEM");
	map(0xe004, 0xe004).portr("P1");
	map(0xe005, 0xe005).portr("P2");
	map(0xe008, 0xe008).w(FUNC(ikki_state::scrn_ctrl_w));
	map(0xe009, 0xe009).w(FUNC(ikki_state::coin_counters));
	map(0xe00a, 0xe00b).writeonly().share(m_scroll);
}

void ikki_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0xc000, 0xc7ff).ram().share(m_spriteram);
	map(0xc800, 0xcfff).ram().share("shared_ram");
	map(0xd801, 0xd801).w("sn1", FUNC(sn76496_device::write));
	map(0xd802, 0xd802).w("sn2", FUNC(sn76496_device::write));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( ikki )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, "2 Players Game" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPSETTING(    0x02, "1 Credit" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unknown 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" )
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
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "1 (Normal)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x06, "4 (Difficult)" )
	PORT_DIPNAME( 0x08, 0x00, "Unknown 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unknown 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Unknown 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Unknown 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        // e004
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")        // e005
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM")    // e003
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,3),   // 2048 characters
	3,      // 3 bits per pixel
	{RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3)},
	{7,6,5,4,3,2,1,0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};

static const gfx_layout spritelayout =
{
	16,32,  // 16*32 characters
	RGN_FRAC(1,3),    // 256 characters
	3,      // 3 bits per pixel
	{RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3)},
	{7,6,5,4,3,2,1,0,
		8*16+7,8*16+6,8*16+5,8*16+4,8*16+3,8*16+2,8*16+1,8*16+0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
	8*8,8*9,8*10,8*11,8*12,8*13,8*14,8*15,
	8*32,8*33,8*34,8*35,8*36,8*37,8*38,8*39,
	8*40,8*41,8*42,8*43,8*44,8*45,8*46,8*47},
	8*8*8
};

static GFXDECODE_START( gfx_ikki )
	GFXDECODE_ENTRY( "tiles",   0x0000, charlayout,   512, 64 )
	GFXDECODE_ENTRY( "sprites", 0x0000, spritelayout, 0,   64 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void ikki_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_punch_through_pen));
	save_item(NAME(m_irq_source));
}

void ikki_state::machine_reset()
{
	m_flipscreen = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(ikki_state::irq)
{
	int const scanline = param;

	// TODO: where non-vblank IRQ happens?
	if (scanline == 240 || scanline == 120)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);

		m_irq_source = (scanline != 240);
	}
}




void ikki_state::ikki(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 20_MHz_XTAL;
	constexpr XTAL PIXEL_CLOCK = MASTER_CLOCK / 4; // guess
	constexpr XTAL CPU_CLOCK = 8_MHz_XTAL;

	// also a guess
	constexpr int HTOTAL  = 320;
	constexpr int HBEND   = 8;
	constexpr int HBSTART = 248;
	constexpr int VTOTAL  = 262;
	constexpr int VBEND   = 16;
	constexpr int VBSTART = 240;

	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK / 2); // 4.000MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &ikki_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(ikki_state::irq), "screen", 0, 1);

	Z80(config, m_subcpu, CPU_CLOCK / 2); // 4.000MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &ikki_state::sub_map);
	m_subcpu->set_periodic_int(FUNC(ikki_state::irq0_line_hold), attotime::from_hz(2 * (PIXEL_CLOCK / HTOTAL / VTOTAL)));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(ikki_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ikki);
	PALETTE(config, m_palette, FUNC(ikki_state::palette), 1024, 256 + 1);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76496(config, "sn1", CPU_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 0.75);

	SN76496(config, "sn2", CPU_CLOCK / 2).add_route(ALL_OUTPUTS, "mono", 0.75);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( ikki )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg17_1",  0x0000,  0x2000, CRC(cb28167c) SHA1(6843553faee0d3bbe432689fdf5f5454470e2b09) )
	ROM_CONTINUE(         0x8000,  0x2000 )
	ROM_LOAD( "tvg17_2",  0x2000,  0x2000, CRC(756c7450) SHA1(043e4f3085d1800b569ee397a968229d547ffbe1) )
	ROM_LOAD( "tvg17_3",  0x4000,  0x2000, CRC(91f0a8b6) SHA1(529fee561c26aa9da75ee58488070329c459540c) )
	ROM_LOAD( "tvg17_4",  0x6000,  0x2000, CRC(696fcf7d) SHA1(6affec60490012fdc762e7a104c0031d44f95bbd) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "tvg17_5",  0x0000,  0x2000, CRC(22bdb40e) SHA1(265801ad660a5a3fc5bb187fa92dbe6098b390f5) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "tvg17_8",  0x0000,  0x4000, CRC(45c9087a) SHA1(9db82fc194096588fde5048e922a654e2ad12c23) )
	ROM_LOAD( "tvg17_7",  0x4000,  0x4000, CRC(0e9efeba) SHA1(d922c4276a988b78b9a2a3ca632136e64a80d995) )
	ROM_LOAD( "tvg17_6",  0x8000,  0x4000, CRC(dc8aa269) SHA1(fd8b5c2bead52e1e136d4df4c26f136d8992d9be) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "tvg17_11", 0x0000,  0x4000, CRC(35012775) SHA1(c90386660755c85fb9f020f8161805dd02a16271) )
	ROM_LOAD( "tvg17_10", 0x4000,  0x4000, CRC(2e510b4e) SHA1(c0ff4515e66ab4959b597a4d930cbbcc31c53cda) )
	ROM_LOAD( "tvg17_9",  0x8000,  0x4000, CRC(c594f3c5) SHA1(6fe19d9ccbe6934a210eb2cab441cd0ba83cbcf4) )

	ROM_REGION( 0x0700, "proms", 0 )// colors
	ROM_LOAD( "prom17_3", 0x0000,  0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) // R
	ROM_LOAD( "prom17_4", 0x0100,  0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) // G
	ROM_LOAD( "prom17_5", 0x0200,  0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) // B
	ROM_LOAD( "prom17_6", 0x0300,  0x0200, CRC(962e619d) SHA1(d2cbcd7b2f1438d1d3759cc1ef6b76b42d9952fe) )
	ROM_LOAD( "prom17_7", 0x0500,  0x0200, CRC(b1f5148c) SHA1(251ddaabf65a87306970b79918849da95beb8ee7) )

	ROM_REGION( 0x0200, "videoattr_proms", 0 )
	ROM_LOAD( "prom17_1", 0x0000,  0x0100, CRC(ca0af30c) SHA1(6d7cfeb16daf61c6e7f93172809b0983bf13cd6c) ) // video attribute
	ROM_LOAD( "prom17_2", 0x0100,  0x0100, CRC(f3c55174) SHA1(936c5432c4fccfcb2601c1e08b98d5509202fe5b) ) // unknown
ROM_END

ROM_START( farmer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg-1.10", 0x0000, 0x2000, CRC(2c0bd392) SHA1(138efa9bc2e40c847f5ac3d31bd62021fd894f49) )
	ROM_CONTINUE(         0x8000, 0x2000 )
	ROM_LOAD( "tvg-2.9",  0x2000, 0x2000, CRC(b86efe02) SHA1(a11cabd001b1577b5708c3f8b1f2717761096c75) )
	ROM_LOAD( "tvg-3.8",  0x4000, 0x2000, CRC(fd686ff4) SHA1(0857b3061126c8dc18c0cd4bd43431f5f5551aef) )
	ROM_LOAD( "tvg-4.7",  0x6000, 0x2000, CRC(1415355d) SHA1(5a3adcb6d03270b4139fbbd0097b6a089cf8c2e1) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "tvg-5.30",  0x0000, 0x2000, CRC(22bdb40e) SHA1(265801ad660a5a3fc5bb187fa92dbe6098b390f5) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "tvg-8.102", 0x0000, 0x4000, CRC(45c9087a) SHA1(9db82fc194096588fde5048e922a654e2ad12c23) )
	ROM_LOAD( "tvg-7.103", 0x4000, 0x4000, CRC(0e9efeba) SHA1(d922c4276a988b78b9a2a3ca632136e64a80d995) )
	ROM_LOAD( "tvg-6.104", 0x8000, 0x4000, CRC(dc8aa269) SHA1(fd8b5c2bead52e1e136d4df4c26f136d8992d9be) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "tvg17_11", 0x0000,  0x4000, CRC(35012775) SHA1(c90386660755c85fb9f020f8161805dd02a16271) )
	ROM_LOAD( "tvg17_10", 0x4000,  0x4000, CRC(2e510b4e) SHA1(c0ff4515e66ab4959b597a4d930cbbcc31c53cda) )
	ROM_LOAD( "tvg17_9",  0x8000,  0x4000, CRC(c594f3c5) SHA1(6fe19d9ccbe6934a210eb2cab441cd0ba83cbcf4) )

	ROM_REGION( 0x0700, "proms", 0 )// colors
	ROM_LOAD( "prom17_3", 0x0000,  0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) // R
	ROM_LOAD( "prom17_4", 0x0100,  0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) // G
	ROM_LOAD( "prom17_5", 0x0200,  0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) // B
	ROM_LOAD( "prom17_6", 0x0300,  0x0200, CRC(962e619d) SHA1(d2cbcd7b2f1438d1d3759cc1ef6b76b42d9952fe) )
	ROM_LOAD( "prom17_7", 0x0500,  0x0200, CRC(b1f5148c) SHA1(251ddaabf65a87306970b79918849da95beb8ee7) )

	ROM_REGION( 0x0200, "videoattr_proms", 0 )
	ROM_LOAD( "prom17_1", 0x0000,  0x0100, CRC(ca0af30c) SHA1(6d7cfeb16daf61c6e7f93172809b0983bf13cd6c) ) // video attribute
	ROM_LOAD( "prom17_2", 0x0100,  0x0100, CRC(f3c55174) SHA1(936c5432c4fccfcb2601c1e08b98d5509202fe5b) ) // unknown
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1985, ikki,   0,    ikki, ikki, ikki_state, empty_init, ROT0, "Sun Electronics", "Ikki (Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1985, farmer, ikki, ikki, ikki, ikki_state, empty_init, ROT0, "Sun Electronics", "Farmers Rebellion", MACHINE_SUPPORTS_SAVE )
