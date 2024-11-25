// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Bogey Manor               (c) 1985 Technos Japan

    This game runs on Data East designed hardware.
    It uses the following Data East customs:
    - HMC20
    - TC15G032AY
    - VSC30

    Emulation by Bryan McPhail, mish@tendril.co.uk and T.Nogi

    2008-07
    Dip Locations added based on crazykong BogeyManor.txt

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class bogeyman_state : public driver_device
{
public:
	bogeyman_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ay(*this, "ay%u", 1U),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_spriteram(*this, "spriteram")
	{ }

	void bogeyman(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<ay8910_device, 2> m_ay;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	// misc
	uint8_t m_psg_latch = 0U;
	uint8_t m_last_write = 0U;
	uint8_t m_colbank = 0U;

	void ay8910_latch_w(uint8_t data);
	void ay8910_control_w(uint8_t data);
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	void colbank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};


void bogeyman_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	// first 16 colors are RAM
	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[0], 3);
		bit1 = BIT(color_prom[256], 0);
		bit2 = BIT(color_prom[256], 1);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[256], 2);
		bit2 = BIT(color_prom[256], 3);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i + 16, rgb_t(r, g, b));
		color_prom++;
	}
}

template <uint8_t Which>
void bogeyman_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void bogeyman_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(bogeyman_state::get_bg_tile_info)
{
	int attr = m_colorram[0][tile_index];
	int gfxbank = ((((attr & 0x01) << 8) + m_videoram[0][tile_index]) / 0x80) + 3;
	int code = m_videoram[0][tile_index] & 0x7f;
	int color = (attr >> 1) & 0x07;

	tileinfo.set(gfxbank, code, color, 0);
}

TILE_GET_INFO_MEMBER(bogeyman_state::get_fg_tile_info)
{
	int attr = m_colorram[1][tile_index];
	int tile = m_videoram[1][tile_index] | ((attr & 0x03) << 8);
	int gfxbank = tile / 0x200;
	int code = tile & 0x1ff;

	tileinfo.set(gfxbank, code, m_colbank, 0);
}

void bogeyman_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bogeyman_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bogeyman_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void bogeyman_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int const attr = m_spriteram[offs];

		if (attr & 0x01)
		{
			int const code = m_spriteram[offs + 1] + ((attr & 0x40) << 2);
			int const color = (attr & 0x08) >> 3;
			int flipx = !(attr & 0x04);
			int flipy = attr & 0x02;
			int sx = m_spriteram[offs + 3];
			int sy = (240 - m_spriteram[offs + 2]) & 0xff;
			int const multi = attr & 0x10;

			if (multi) sy -= 16;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}


				m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);

			if (multi)
			{
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code + 1, color,
					flipx, flipy,
					sx, sy + (flip_screen() ? -16 : 16), 0);
			}
		}
	}
}

uint32_t bogeyman_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// Read/Write Handlers

// Sound section is copied from Mysterious Stones driver by Nicola, Mike, Brad

void bogeyman_state::ay8910_latch_w(uint8_t data)
{
	m_psg_latch = data;
}

void bogeyman_state::ay8910_control_w(uint8_t data)
{
	// bit 0 is flipscreen
	flip_screen_set(~data & 0x01);

	// bit 5 goes to 8910 #0 BDIR pin
	if ((m_last_write & 0x20) == 0x20 && (data & 0x20) == 0x00)
		m_ay[0]->data_address_w(m_last_write >> 4, m_psg_latch);

	// bit 7 goes to 8910 #1 BDIR pin
	if ((m_last_write & 0x80) == 0x80 && (data & 0x80) == 0x00)
		m_ay[1]->data_address_w(m_last_write >> 6, m_psg_latch);

	m_last_write = data;
}

// Memory Map

void bogeyman_state::prg_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1bff).ram().w(FUNC(bogeyman_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x1c00, 0x1fff).ram().w(FUNC(bogeyman_state::colorram_w<1>)).share(m_colorram[1]);
	map(0x2000, 0x20ff).ram().w(FUNC(bogeyman_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x2100, 0x21ff).ram().w(FUNC(bogeyman_state::colorram_w<0>)).share(m_colorram[0]);
	map(0x2800, 0x2bff).ram().share(m_spriteram);
	map(0x3000, 0x300f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x3800, 0x3800).portr("P1").w(FUNC(bogeyman_state::ay8910_control_w));
	map(0x3801, 0x3801).portr("P2").w(FUNC(bogeyman_state::ay8910_latch_w));
	map(0x3802, 0x3802).portr("DSW1");
	map(0x3803, 0x3803).portr("DSW2").nopw(); // ??? sound
	map(0x4000, 0xffff).rom();
}

// Input Ports

static INPUT_PORTS_START( bogeyman )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "50K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

// Graphics Layouts

static const gfx_layout charlayout1 =
{
	8, 8,
	512,
	3,
	{ 0x8000*8+4, 0, 4 },
	{ 0x2000*8+3, 0x2000*8+2, 0x2000*8+1, 0x2000*8+0, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout2 =
{
	8, 8,
	512,
	3,
	{ 0x8000*8, 0+0x1000*8, 4+0x1000*8 },
	{ 0x2000*8+3, 0x2000*8+2, 0x2000*8+1, 0x2000*8+0, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles1a =
{
	16, 16,
	128,
	3,
	{ 0x8000*8+4, 0, 4 },
	{ 1024*8*8+3, 1024*8*8+2, 1024*8*8+1, 1024*8*8+0, 3, 2, 1, 0,
		1024*8*8+3+64, 1024*8*8+2+64, 1024*8*8+1+64, 1024*8*8+0+64, 3+64,2+64,1+64,0+64 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		0*8+16*8, 1*8+16*8, 2*8+16*8, 3*8+16*8, 4*8+16*8, 5*8+16*8, 6*8+16*8, 7*8+16*8 },
	32*8
};

static const gfx_layout tiles1b =
{
	16, 16,
	128,
	3,
	{ 0x8000*8+0, 0+0x1000*8+0, 4+0x1000*8 },
	{ 1024*8*8+3, 1024*8*8+2, 1024*8*8+1, 1024*8*8+0, 3, 2, 1, 0,
		1024*8*8+3+64, 1024*8*8+2+64, 1024*8*8+1+64, 1024*8*8+0+64, 3+64,2+64, 1+64,0+64 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		0*8+16*8, 1*8+16*8, 2*8+16*8, 3*8+16*8, 4*8+16*8, 5*8+16*8, 6*8+16*8, 7*8+16*8 },
	32*8
};

static const gfx_layout sprites =
{
	16, 16,
	512,
	3,
	{ 0x8000*8, 0x4000*8, 0 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

// Graphics Decode Information

static GFXDECODE_START( gfx_bogeyman )
	GFXDECODE_ENTRY( "chars",   0x00000, charlayout1,     16, 32 )
	GFXDECODE_ENTRY( "chars",   0x00000, charlayout2,     16, 32 )
	GFXDECODE_ENTRY( "sprites", 0x00000, sprites,          0,  2 )
	GFXDECODE_ENTRY( "tiles",   0x00000, tiles1a,     16+128,  8 )
	GFXDECODE_ENTRY( "tiles",   0x00000, tiles1b,     16+128,  8 )
	GFXDECODE_ENTRY( "tiles",   0x04000, tiles1a,     16+128,  8 )
	GFXDECODE_ENTRY( "tiles",   0x04000, tiles1b,     16+128,  8 )
	// colors 16+192 to 16+255 are currently unassigned
GFXDECODE_END


// Machine Driver

void bogeyman_state::machine_start()
{
	save_item(NAME(m_psg_latch));
	save_item(NAME(m_last_write));
	save_item(NAME(m_colbank));
}

void bogeyman_state::machine_reset()
{
	m_psg_latch = 0;
	m_last_write = 0;
	m_colbank = 0;
}

void bogeyman_state::colbank_w(uint8_t data)
{
	if ((data & 1) != (m_colbank & 1))
	{
		m_colbank = data & 1;
		m_fg_tilemap->mark_all_dirty();
	}
}

void bogeyman_state::bogeyman(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, XTAL(12'000'000) / 8); // Verified
	m_maincpu->set_addrmap(AS_PROGRAM, &bogeyman_state::prg_map);
	m_maincpu->set_periodic_int(FUNC(bogeyman_state::irq0_line_hold), attotime::from_hz(16 * 60)); // Controls sound

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	// DECO video CRTC, unverified
	screen.set_raw(XTAL(12'000'000) / 2, 384, 0, 256, 272, 8, 248);
	screen.set_screen_update(FUNC(bogeyman_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bogeyman);
	PALETTE(config, m_palette, FUNC(bogeyman_state::palette)).set_format(palette_device::BGR_233_inverted, 16 + 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// verified to be YM2149s from PCB pic. Another PCB had 2 AY-8910s
	YM2149(config, m_ay[0], XTAL(12'000'000) / 8);  // Verified
	m_ay[0]->port_a_write_callback().set(FUNC(bogeyman_state::colbank_w));
	m_ay[0]->add_route(ALL_OUTPUTS, "mono", 0.30);

	YM2149(config, m_ay[1], XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.30);  // Verified
}

// ROMs

ROM_START( bogeyman ) // PCB 0204-0
	ROM_REGION( 0x58000, "maincpu", 0 )
	ROM_LOAD( "j20.c14",  0x04000, 0x04000, CRC(ea90d637) SHA1(aa89bee806badb05119516d84e7674cd302aaf4e) )
	ROM_LOAD( "j10.c15",  0x08000, 0x04000, CRC(0a8f218d) SHA1(5e5958cccfe634e3d274d187a0a7fe4789f3a9c3) )
	ROM_LOAD( "j00.c17",  0x0c000, 0x04000, CRC(5d486de9) SHA1(40ea14a4a25f8f38d33a8844f627ba42503e1280) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "j70.h15",  0x00000, 0x04000, CRC(fdc787bf) SHA1(1f185a1927fff6ce793d673ebd882a852ac547e4) )
	ROM_LOAD( "j60.c17",  0x08000, 0x01000, CRC(cc03ceb2) SHA1(0149eacac2c1469be6e19f7a43c13d1fe8790f2c) )
	ROM_CONTINUE(         0x0a000, 0x01000 )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "j30.c9",   0x00000, 0x04000, CRC(41af81c0) SHA1(d8465622cdf16bc906818641d7988fc412454a45) )
	ROM_LOAD( "j40.c7",   0x04000, 0x04000, CRC(8b438421) SHA1(295806c119f4ddc01afc15550e1ff397fbf5d862) )
	ROM_LOAD( "j50.c5",   0x08000, 0x04000, CRC(b507157f) SHA1(471f67eb5e7aedef52353581405d9613d2a86898) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "j90.h12",  0x00000, 0x04000, CRC(46b2d4d0) SHA1(35cd320d4db7aa6a89f83ba4d9ff88925357d640) )
	ROM_LOAD( "j80.h13",  0x04000, 0x04000, CRC(77ebd0a4) SHA1(c6921ee59633eeeda97c73cb7833578fa8a84fa3) )
	ROM_LOAD( "ja0.h10",  0x08000, 0x01000, CRC(f2aa05ed) SHA1(e6df96e4128eff6de7e6483254608dd8a7b258b9) )
	ROM_CONTINUE(         0x0a000, 0x01000 )
	ROM_CONTINUE(         0x0c000, 0x01000 )
	ROM_CONTINUE(         0x0e000, 0x01000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s129.5k",  0x0000, 0x0100, CRC(4a7c5367) SHA1(a67f5b90c18238cbfb1507230b4614191d37eef4) )  // Colour prom 1
	ROM_LOAD( "82s129.6k",  0x0100, 0x0100, CRC(b6127713) SHA1(5bd8627453916ac6605af7d1193f79c748eab981) )  // Colour prom 2
ROM_END

} // anonymous namespace


// Game Driver

// ROT180 confirmed by Kold
GAME( 1985, bogeyman, 0, bogeyman, bogeyman, bogeyman_state, empty_init, ROT180, "Technos Japan", "Bogey Manor", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
