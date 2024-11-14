// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, hap, Philip Bennett, David Haywood
/*

Sea Battle by Zaccaria

driver by Mariusz Wojcieszek, hap, Phil Bennett and David Haywood

TODO:
- correct colors (note: the flyer screenshots look faked; not a good reference)
- should it have a horizon/sky?
- video timing
- video offsets
- discrete sound


2650 + 2636

sea b b_1 *.prg are 2650 progamm

sea b blu.prg is obj blue data
sea b red.prg is obj red data
sea b green.prg is obj green data

sea b wawe.prg is sea wave data

sea b screen.prg is tile data


the sound board should be fully discrete.

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "video/dm9368.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "seabattl.lh"


namespace {

class seabattl_state : public driver_device
{
public:
	seabattl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_objram(*this, "objram"),
		m_digits(*this, { "sc_thousand", "sc_hundred", "sc_half", "sc_unity", "tm_half", "tm_unity" }),
		m_s2636(*this, "s2636"),
		m_7segs(*this, "digit%u", 0U),
		m_lamp(*this, "lamp0"),
		m_waveenable(false),
		m_collision(0),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void seabattl(machine_config &config);
	void armada(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void seabattl_videoram_w(offs_t offset, uint8_t data);
	void seabattl_colorram_w(offs_t offset, uint8_t data);
	void seabattl_control_w(uint8_t data);
	uint8_t seabattl_collision_r();
	void seabattl_collision_clear_w(uint8_t data);
	uint8_t seabattl_collision_clear_r();
	void sound_w(uint8_t data);
	void sound2_w(uint8_t data);
	void time_display_w(uint8_t data);
	void score_display_w(uint8_t data);
	void score2_display_w(uint8_t data);
	template <unsigned N> void digit_w(uint8_t data) { m_7segs[N] = data; }

	void seabattl_palette(palette_device &palette) const;
	uint32_t screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void seabattl_data_map(address_map &map) ATTR_COLD;
	void seabattl_map(address_map &map) ATTR_COLD;
	void armada_map(address_map &map) ATTR_COLD;

	required_device<s2650_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_objram;
	required_device_array<dm9368_device, 6> m_digits;
	required_device<s2636_device> m_s2636;
	output_finder<6> m_7segs;
	output_finder<> m_lamp;

	tilemap_t *m_bg_tilemap = nullptr;
	bitmap_ind16 m_collision_bg;

	bool m_waveenable;
	uint8_t m_collision;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


/***************************************************************************

  Video

***************************************************************************/

void seabattl_state::seabattl_palette(palette_device &palette) const
{
	// sprites (m.obj) + s2636
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, rgb_t((i & 1) ? 0xff : 0x00, (i & 2) ? 0xff : 0x00, (i & 4) ? 0xff : 0x00));
	}

	// scr
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(8 + 2 * i + 0, rgb_t::black());
		palette.set_pen_color(8 + 2 * i + 1, rgb_t((i & 1) ? 0xff : 0x00, (i & 2) ? 0xff : 0x00, (i & 4) ? 0xff : 0x00));
	}

	// wave
	palette.set_pen_color(24, rgb_t::black());
	palette.set_pen_color(25, rgb_t(0x00, 0xff, 0xff)); // cyan
}

TILE_GET_INFO_MEMBER(seabattl_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	tileinfo.set(1, code, (color & 0x7), 0);
}

void seabattl_state::seabattl_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void seabattl_state::seabattl_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint32_t seabattl_state::screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// wave
	if ( m_waveenable )
	{
		for ( int y = 0; y < 32; y++ )
		{
			for ( int x = 0; x < 32; x++ )
			{
				m_gfxdecode->gfx(2)->opaque(bitmap,cliprect, (y & 0x0f) + (((x & 0x0f) + ((screen.frame_number() & 0xe0) >> 4)) << 4), 0, 0, 0, x*8, y*8 );
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	// background (scr.sm.obj)
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, m_collision_bg, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	// sprites (m.obj)
	for ( int offset = 0; offset < 256; offset++ )
	{
		// bits 0-3: sprite num
		// bits 4-7: x coordinate
		if ( m_objram[offset] & 0xf )
		{
			int code = (m_objram[offset] & 0x0f) | 0x10;
			int x = ((offset & 0x0f) << 4) - ((m_objram[offset] & 0xf0) >> 4);
			int y = (offset & 0xf0);

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, code, 0, 0, 0, x, y, 0);
		}
	}

	bitmap_ind16 const &s2636_0_bitmap = m_s2636->update(cliprect);

	// collisions
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// bit 0: m.obj - pvi-bkg
			// bit 1: pvi-bkg - scr.sm.obj
			// bit 2: m.obj - scr.sm.obj
			bool obj = (bitmap.pix(y,x) > 0) && (bitmap.pix(y,x) < 8);
			bool pvi = S2636_IS_PIXEL_DRAWN(s2636_0_bitmap.pix(y, x));
			bool scr = (m_collision_bg.pix(y,x) & 1) != 0;

			if (obj && pvi)
				m_collision |= 0x01;

			if (pvi && scr)
				m_collision |= 0x02;

			if (obj && scr)
				m_collision |= 0x04;
		}
	}

	// s2636 layer
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pixel = s2636_0_bitmap.pix(y, x);
			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				bitmap.pix(y, x) = S2636_PIXEL_COLOR(pixel);
			}
		}
	}

	return 0;
}

void seabattl_state::video_start()
{
	m_7segs.resolve();
	m_screen->register_screen_bitmap(m_collision_bg);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seabattl_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(-12, 0);
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void seabattl_state::seabattl_map(address_map &map)
{
	map(0x0000, 0x13ff).rom();
	map(0x2000, 0x33ff).rom();
	map(0x1400, 0x17ff).mirror(0x2000).ram().w(FUNC(seabattl_state::seabattl_colorram_w)).share("colorram");
	map(0x1800, 0x1bff).mirror(0x2000).ram().w(FUNC(seabattl_state::seabattl_videoram_w)).share("videoram");
	map(0x1c00, 0x1cff).mirror(0x2000).ram();
	map(0x1d00, 0x1dff).mirror(0x2000).ram().share("objram");
	map(0x1e00, 0x1e00).mirror(0x20f0).w(FUNC(seabattl_state::time_display_w));
	map(0x1e01, 0x1e01).mirror(0x20f0).w(FUNC(seabattl_state::score_display_w));
	map(0x1e02, 0x1e02).mirror(0x20f0).portr("IN0").w(FUNC(seabattl_state::score2_display_w));
	map(0x1e05, 0x1e05).mirror(0x20f0).portr("DIPS2");
	map(0x1e06, 0x1e06).mirror(0x20f0).portr("DIPS1").w(FUNC(seabattl_state::sound_w));
	map(0x1e07, 0x1e07).mirror(0x20f0).portr("DIPS0").w(FUNC(seabattl_state::sound2_w));
	map(0x1f00, 0x1fff).mirror(0x2000).rw(m_s2636, FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1fcc, 0x1fcc).mirror(0x2000).portr("IN1");
}

void seabattl_state::armada_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x2000, 0x2bff).rom();
	map(0x4000, 0x4bff).rom();
	map(0x6000, 0x6bff).rom();
	map(0x1400, 0x17ff).mirror(0x6000).ram().w(FUNC(seabattl_state::seabattl_colorram_w)).share("colorram");
	map(0x1800, 0x1bff).mirror(0x6000).ram().w(FUNC(seabattl_state::seabattl_videoram_w)).share("videoram");
	map(0x1c00, 0x1cff).mirror(0x6000).ram();
	map(0x1d00, 0x1dff).mirror(0x6000).ram().share("objram");
	map(0x1e00, 0x1e00).mirror(0x60f0).w(FUNC(seabattl_state::time_display_w));
	map(0x1e01, 0x1e01).mirror(0x60f0).w(FUNC(seabattl_state::score_display_w));
	map(0x1e02, 0x1e02).mirror(0x60f0).portr("IN0").w(FUNC(seabattl_state::score2_display_w));
	map(0x1e05, 0x1e05).mirror(0x60f0).portr("DIPS2");
	map(0x1e06, 0x1e06).mirror(0x60f0).portr("DIPS1").w(FUNC(seabattl_state::sound_w));
	map(0x1e07, 0x1e07).mirror(0x60f0).portr("DIPS0").w(FUNC(seabattl_state::sound2_w));
	map(0x1f00, 0x1fff).mirror(0x6000).rw(m_s2636, FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1fcc, 0x1fcc).mirror(0x6000).portr("IN1");
}

void seabattl_state::seabattl_data_map(address_map &map)
{
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(seabattl_state::seabattl_collision_r), FUNC(seabattl_state::seabattl_control_w));
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(seabattl_state::seabattl_collision_clear_r), FUNC(seabattl_state::seabattl_collision_clear_w));
}

uint8_t seabattl_state::seabattl_collision_r()
{
	m_screen->update_partial(m_screen->vpos());
	return m_collision;
}

void seabattl_state::seabattl_control_w(uint8_t data)
{
	// bit 0: play counter
	// bit 1: super bonus counter
	// bit 2: coin counter
	// bit 3: inverse image
	// bit 4: lamp
	// bit 5: enable wave
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	m_lamp = BIT(data,4);
	m_waveenable = BIT(data, 5);
}

uint8_t seabattl_state::seabattl_collision_clear_r()
{
	m_screen->update_partial(m_screen->vpos());
	m_collision = 0;
	return 0;
}

void seabattl_state::seabattl_collision_clear_w(uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	m_collision = 0;
}

void seabattl_state::sound_w(uint8_t data)
{
	// sound effects
	// bits:
	// 0 - missile
	// 1 - ship
	// 2 - aircraft
	// 3 - silence
	// 4 - torpedo
	// 5 - bomb
	// 6 - unused
	// 7 - unused
}

void seabattl_state::sound2_w(uint8_t data)
{
	// sound effects
	// bits:
	// 0 - melody
	// 1 - expl. a
	// 2 - expl. b
	// 3 - expl. c
	// 4 - expl. d
	// 5 - fall aircraft
	// 6 - unused
	// 7 - unused
}

void seabattl_state::time_display_w(uint8_t data)
{
	m_digits[5]->a_w(data & 0x0f);
	m_digits[4]->a_w((data >> 4) & 0x0f);
}

void seabattl_state::score_display_w(uint8_t data)
{
	m_digits[3]->a_w(data & 0x0f);
	m_digits[2]->a_w((data >> 4) & 0x0f);
}

void seabattl_state::score2_display_w(uint8_t data)
{
	m_digits[1]->a_w(data & 0x0f);
	m_digits[0]->a_w((data >> 4) & 0x0f);
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( seabattl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(20) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("DIPS0")
	PORT_DIPNAME( 0x01, 0x00, "Game Type" ) PORT_DIPLOCATION("DS0:3")
	PORT_DIPSETTING(    0x00, "Time Based" )
	PORT_DIPSETTING(    0x01, "Lives Based" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Game_Time ) ) PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00) PORT_DIPLOCATION("DS0:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "75 seconds" )         PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x06, "90 seconds" )         PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "105 seconds" )        PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )     PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01) PORT_DIPLOCATION("DS0:2,1")
	PORT_DIPSETTING(    0x02, "3" )                  PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x06, "4" )                  PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "5" )                  PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DS0:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS1")
	PORT_DIPNAME( 0x01, 0x00, "Enemies Speed" ) PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x06, 0x00, "Extended Play" ) PORT_DIPLOCATION("DS1:3,4")
	PORT_DIPSETTING(    0x06, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, "2000 points" )
	PORT_DIPSETTING(    0x02, "3000 points" )
	PORT_DIPSETTING(    0x00, "4000 points" )
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DS0:8,DS1:1,DS0:7")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DS1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "DS1:6" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "DS1:5")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DS1:8" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config/Interface

***************************************************************************/

void seabattl_state::machine_start()
{
	m_lamp.resolve();
}

void seabattl_state::machine_reset()
{
}

static const gfx_layout tiles32x16x3_layout =
{
	32,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 384,385,386,387,388,389,390,391, 0, 1, 2, 3, 4, 5, 6, 7, 128,129,130,131,132,133,134,135, 256,257,258,259,260,261,262,263 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*8*4
};


static GFXDECODE_START( gfx_seabattl )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x16x3_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x1,           8, 8 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x1,          24, 1 )
GFXDECODE_END

void seabattl_state::seabattl(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 14318180/4/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &seabattl_state::seabattl_map);
	m_maincpu->set_addrmap(AS_DATA, &seabattl_state::seabattl_data_map);
	m_maincpu->sense_handler().set("screen", FUNC(screen_device::vblank));
	m_maincpu->intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x03; });
	S2636(config, m_s2636, 0);
	m_s2636->set_offsets(-13, -29);
	m_s2636->add_route(ALL_OUTPUTS, "mono", 0.10);

	DM9368(config, m_digits[0], 0).update_cb().set(FUNC(seabattl_state::digit_w<0>));
	DM9368(config, m_digits[1], 0).update_cb().set(FUNC(seabattl_state::digit_w<1>));
	DM9368(config, m_digits[2], 0).update_cb().set(FUNC(seabattl_state::digit_w<2>));
	DM9368(config, m_digits[3], 0).update_cb().set(FUNC(seabattl_state::digit_w<3>));
	DM9368(config, m_digits[4], 0).update_cb().set(FUNC(seabattl_state::digit_w<4>));
	DM9368(config, m_digits[5], 0).update_cb().set(FUNC(seabattl_state::digit_w<5>));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(256, 256);
	m_screen->set_visarea(1*8, 29*8-1, 2*8, 32*8-1);
	m_screen->set_screen_update(FUNC(seabattl_state::screen_update_seabattl));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_seabattl);
	PALETTE(config, m_palette, FUNC(seabattl_state::seabattl_palette), 26);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* discrete sound */
}

// very different looking PCB (bootleg maybe?)
void seabattl_state::armada(machine_config &config)
{
	seabattl(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seabattl_state::armada_map);

	// TODO: Z80-based sound board
}



/******************************************************************************/

ROM_START( seabattl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sea_b_b_1_1.2b",  0x0000, 0x0400, CRC(16a475c0) SHA1(5380d3be39c421227e52012d1bcf0516e99f6a3f) )
	ROM_CONTINUE(                0x2000, 0x0400 )
	ROM_LOAD( "sea_b_b_1_2.2c",  0x0400, 0x0400, CRC(4bd73a82) SHA1(9ab4edf24fcd437ecd8e9e551ce0ed33be3bbad7) )
	ROM_CONTINUE(                0x2400, 0x0400 )
	ROM_LOAD( "sea_b_b_1_3.2d",  0x0800, 0x0400, CRC(e251492b) SHA1(a152f9b6f189909ff478b4d95ee764f1898405b5) )
	ROM_CONTINUE(                0x2800, 0x0400 )
	ROM_LOAD( "sea_b_b_1_4.4b",  0x0c00, 0x0400, CRC(6012b83f) SHA1(57de9e45253609b71f14fb3541760fd33647a651) )
	ROM_CONTINUE(                0x2c00, 0x0400 )
	ROM_LOAD( "sea_b_b_1_5.4c",  0x1000, 0x0400, CRC(55c263f6) SHA1(33eba61cb8c9318cf19b771c93a14397b4ee0ace) )
	ROM_CONTINUE(                0x3000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "sea_b_red.8d",    0x0000, 0x0800, CRC(fe7192df) SHA1(0b262bc1ac959d8dd79d71780e16237075f4a099) )
	ROM_LOAD( "sea_b_green.8c",  0x0800, 0x0800, CRC(cea4c0c9) SHA1(697c136ef363676b346692740d3c3a482dde6207) )
	ROM_LOAD( "sea_b_blu.8a",    0x1000, 0x0800, CRC(cd972c4a) SHA1(fcb8149bc462912c8393431ccb792ea4b1b1109d) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "sea_b_screen.7f", 0x0000, 0x0800, CRC(8e4391dd) SHA1(f5698d66e5a3c46082b515ce86f9d3e96fd9ff77) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "sea_b_wawe.10k",  0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) ) // sic
ROM_END

/*
Zaccaria Armada
---------------

Serial No.: V143107

Main PCB looks like it's a revised Sea Battle main PCB (I.G.R. LC26 C)
 - 2650 CPU
 - 2636 PVI
 - DIP switch block (8) (at IC11)

Sound PCB looks similar to Scorpion (I.G.R. LC241)
 - Z80 CPU
 - 3 x AY-3-8910
 - 2 x 8255
 - Stereo amplifiers
 - Epoxy block like Scorpion
 - Shaved off 40-pin IC likely the same speech IC as Scorpion
 - DIP switch block (8) (at IC55)
*/
ROM_START( armada )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "armada-1.ic83", 0x0000, 0x0400, CRC(8528510c) SHA1(fbb653a0a4c77dd31fa106f1aeab985d02ebad50) )
	ROM_CONTINUE(              0x2000, 0x0400 )
	ROM_CONTINUE(              0x4000, 0x0400 )
	ROM_CONTINUE(              0x6000, 0x0400 )
	ROM_LOAD( "armada-2.ic84", 0x0400, 0x0400, CRC(7f5a2089) SHA1(be15d9c31c8bd33c029dfa7fcfc4e36d6dc8e04f) )
	ROM_CONTINUE(              0x2400, 0x0400 )
	ROM_CONTINUE(              0x4400, 0x0400 )
	ROM_CONTINUE(              0x6400, 0x0400 )
	ROM_LOAD( "armada-3.ic85", 0x0800, 0x0400, CRC(43e4c8c9) SHA1(b2f3a492cef183875370bd0c4e75e9bdb70c9cf2) )
	ROM_CONTINUE(              0x2800, 0x0400 )
	ROM_CONTINUE(              0x4800, 0x0400 )
	ROM_CONTINUE(              0x6800, 0x0400 )
	// 2 adjacent DIP28 positions (IC72, IC73) are unpopulated

	ROM_REGION( 0x1800, "gfx1", 0 ) // probably the same as above without the blank data at the start
	ROM_LOAD( "armada-red.ic26",   0x0400, 0x0400, CRC(b588f509) SHA1(073f9dc584aba1351969ef597cd80a0037938dfb) )
	ROM_LOAD( "armada-green.ic25", 0x0c00, 0x0400, CRC(3cc861c9) SHA1(d9159ee045cc0994f468035ae28cd8b79b5985ee) )
	ROM_LOAD( "armada-blu.ic24",   0x1400, 0x0400, CRC(3689e530) SHA1(b30ab0d5ddc9b296437aa1bc2887f1416eb69f9c) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "greenobj.ic38",     0x0000, 0x0800, CRC(81a9a741) SHA1(b2725c320a232d4abf6e6fc58ccf6a5edb8dd9a0) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "seawave.ic9",       0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) ) // identical to above set

	ROM_REGION( 0x3000, "soundcpu", 0 )
	ROM_LOAD( "sound-1.ic12", 0x0000, 0x1000, CRC(3ea7fa75) SHA1(57bc4bc3b21a300c10313a79e6d535573b2e9a54) )
	ROM_LOAD( "sound-2.ic13", 0x1000, 0x1000, CRC(3bcef3ca) SHA1(a9ebeb54a41d08a6ebd3fc2821e3fcda998ec484) )
	ROM_LOAD( "sound-3.ic14", 0x2000, 0x1000, CRC(6409f99b) SHA1(e8d7f457e9a315d7f89b817c2b7cda1f2f3e5877) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "speach-1.ic25", 0x0000, 0x1000, CRC(b72a8559) SHA1(08c60b950c2ed345840a65b427b19788c66d1e27) ) // sic
	ROM_LOAD( "speach-2.ic24", 0x1000, 0x1000, CRC(93133519) SHA1(12848b80eb313df9cd552122fdee9d335b32972f) ) // "
ROM_END

} // anonymous namespace


GAMEL(1980, seabattl,  0,        seabattl, seabattl, seabattl_state, empty_init, ROT0, "Zaccaria",          "Sea Battle", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND, layout_seabattl )
GAMEL(1980, armada,    seabattl, armada,   seabattl, seabattl_state, empty_init, ROT0, "Zaccaria / I.G.R.", "Armada",     MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING, layout_seabattl ) // different hardware
