// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood

#include "emu.h"
#include "galaxian.h"

namespace {

class galaxian_rockclim_state : public galaxian_state
{
public:
	galaxian_rockclim_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_rockclim_videoram(*this,"rockclim_vram")
		, m_gfxdecode2(*this,"gfxdecode2")
		, m_palette2(*this,"palette2")
	{ }

	void rockclim(machine_config &config);

	void init_rockclim();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void rockclim_map(address_map &map) ATTR_COLD;

	void rockclim_palette(palette_device &palette) const;
	void rockclim_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void rockclim_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void rockclim_videoram_w(offs_t offset, uint8_t data);
	void rockclim_scroll_w(offs_t offset, uint8_t data);
	uint8_t rockclim_videoram_r(offs_t offset);
	TILE_GET_INFO_MEMBER(rockclim_get_tile_info);

	uint16_t m_rockclim_v = 0;
	uint16_t m_rockclim_h = 0;

	tilemap_t *m_rockclim_tilemap = nullptr;
	required_shared_ptr<uint8_t> m_rockclim_videoram;
	required_device<gfxdecode_device> m_gfxdecode2;
	required_device<palette_device> m_palette2;
};

void galaxian_rockclim_state::rockclim_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_rockclim_tilemap->draw(*m_screen, bitmap, cliprect, 0,0);
}

void galaxian_rockclim_state::rockclim_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)
{
	if ((*code & 0x30) == 0x20)
	{
		if (m_gfxbank[2] & 1)
		{
			int bank = (((m_gfxbank[0] & 1) << 5) | ((m_gfxbank[1] & 1) << 4));
			*code = (0x40 + bank) | (*code & 0x0f);
		}
	}
}

uint8_t galaxian_rockclim_state::rockclim_videoram_r(offs_t offset)
{
	return m_rockclim_videoram[offset];
}

void galaxian_rockclim_state::rockclim_videoram_w(offs_t offset, uint8_t data)
{
	m_rockclim_videoram[offset] = data;
	m_rockclim_tilemap->mark_tile_dirty(offset);
}

void galaxian_rockclim_state::rockclim_scroll_w(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0: m_rockclim_h = (m_rockclim_h & 0xff00) | data; m_rockclim_tilemap->set_scrollx(0, m_rockclim_h * m_x_scale); break;
	case 1: m_rockclim_h = (m_rockclim_h & 0x00ff) | (data << 8); m_rockclim_tilemap->set_scrollx(0, m_rockclim_h * m_x_scale); break;
	case 2: m_rockclim_v = (m_rockclim_v & 0xff00) | data; m_rockclim_tilemap->set_scrolly(0, m_rockclim_v); break;
	case 3: m_rockclim_v = (m_rockclim_v & 0x00ff) | (data << 8); m_rockclim_tilemap->set_scrolly(0, m_rockclim_v); break;
	}
}

void galaxian_rockclim_state::video_start()
{
	galaxian_state::video_start();

	m_rockclim_tilemap = &machine().tilemap().create(*m_gfxdecode2, tilemap_get_info_delegate(*this, FUNC(galaxian_rockclim_state::rockclim_get_tile_info)), TILEMAP_SCAN_ROWS,8 * m_x_scale,8,64,32);
	m_rockclim_v = m_rockclim_h = 0;
	save_item(NAME(m_rockclim_v));
	save_item(NAME(m_rockclim_h));
}

TILE_GET_INFO_MEMBER(galaxian_rockclim_state::rockclim_get_tile_info)
{
	uint16_t code = m_rockclim_videoram[tile_index];
	tileinfo.set(0, code, 0, 0);
}

void galaxian_rockclim_state::rockclim_map(address_map &map)
{
	galaxian_state::mooncrst_map(map);

	map(0x4000, 0x47ff).rw(FUNC(galaxian_rockclim_state::rockclim_videoram_r), FUNC(galaxian_rockclim_state::rockclim_videoram_w)).share("rockclim_vram");//4800 - 4803 = bg scroll ?
	map(0x4800, 0x4803).w(FUNC(galaxian_rockclim_state::rockclim_scroll_w));
	map(0x5000, 0x53ff).ram();
	map(0x5800, 0x5800).portr("IN3");
	map(0x6000, 0x7fff).rom();
	map(0x8800, 0x8800).portr("IN4");
}


// verified from Z80 code
static INPUT_PORTS_START( rockclim )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )             // only adds 1 credit if "Coin Slots" is set to 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )    PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )  PORT_8WAY
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("SW3:1") // fake switch for edge connector cabinet setting
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )     PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )   PORT_8WAY
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW1:7") // PCB pic shows Rock Climber has 2x 8-position DIP switches.
	PORT_DIPSETTING(    0x00, "30000" )                                          // Order may not be accurate since info not available (need DIP switch sheet to improve it)
	PORT_DIPSETTING(    0x40, "50000" )                                          // but these seem plausible.
	PORT_DIPNAME( 0x80, 0x00, "Coin Slots" )           PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )     PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )   PORT_8WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_HIGH, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_HIGH, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_HIGH, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW1:6" )

	PORT_START("IN4")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )      PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )      PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_8C ) )
INPUT_PORTS_END

static const gfx_layout rockclim_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2) },
	{ 3, 2, 1, 0,11 ,10, 9, 8 },
	{ 0*8*2, 1*8*2, 2*8*2, 3*8*2, 4*8*2, 5*8*2, 6*8*2, 7*8*2 },
	8*8*2
};

static GFXDECODE_START( gfx_rockclim )
	GFXDECODE_SCALE("bg_gfx", 0x0000, rockclim_charlayout,   0, 1, GALAXIAN_XSCALE,1)
GFXDECODE_END


void galaxian_rockclim_state::rockclim_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("bg_proms")->base();

	// first, the character/sprite palette
	int const len = memregion("bg_proms")->bytes();
	for (int i = 0; i < len; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(*color_prom, 6);
		bit1 = BIT(*color_prom, 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, r, g, b);
		color_prom++;
	}
}

void galaxian_rockclim_state::rockclim(machine_config &config)
{
	galaxian_state::mooncrst(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &galaxian_rockclim_state::rockclim_map);

	// video hardware
	GFXDECODE(config, m_gfxdecode2, m_palette2, gfx_rockclim);
	PALETTE(config, m_palette2, FUNC(galaxian_rockclim_state::rockclim_palette), 32);
}


ROM_START( rockclim )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc01.a1",   0x0000, 0x1000, CRC(8601ae8d) SHA1(6e0c3b34ce5e6879ce7a116c5c2660889a68320d) )
	ROM_LOAD( "lc02.a2",   0x1000, 0x1000, CRC(2dde9d4c) SHA1(7e343113116b94894558819a7f77f77e4e952da7) )
	ROM_LOAD( "lc03.a3",   0x2000, 0x1000, CRC(82c48a67) SHA1(abf95062eb5c9bd4bb3c9b9af59396a4ca6905d8) )
	ROM_LOAD( "lc04.a4",   0x3000, 0x1000, CRC(7cd3a04a) SHA1(756c12288e120e6f761b266b91920d17cab6926c) )
	ROM_LOAD( "lc05.a5",   0x6000, 0x1000, CRC(5e542149) SHA1(425a5a8769c3fa0887db8ff04e2a4f32f18d2679) )
	ROM_LOAD( "lc06.a6",   0x7000, 0x1000, CRC(b2bdca64) SHA1(e72e63725164c922816dda90ac964a94062eab1b) )

	ROM_REGION( 0x800, "melody", 0 ) // Epson 7910C Multi-Melody IC
	ROM_LOAD( "7910c 537 104", 0x000, 0x800, NO_DUMP ) // actual size unknown, needs decapping

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "lc08.a9",   0x0000, 0x800, CRC(7f18e1ef) SHA1(2a160b994708ec0f06774dde3ec613af7e3f32c6) )
	ROM_LOAD( "lc07.a7",   0x0800, 0x800, CRC(f18b50ac) SHA1(a2328eb55882a09403cae1a497c611b494649cac) )
	ROM_LOAD( "lc10.c9",   0x1000, 0x800, CRC(dec5781b) SHA1(b6277fc890d153db24bd48293780cf239a6aa0e7) )
	ROM_LOAD( "lc09.c7",   0x1800, 0x800, CRC(06c0b5de) SHA1(561cf99a6be03205c7bc5fd15d4d51ee4d6d164b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "lc11.f4",  0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )

	ROM_REGION( 0x2000, "bg_gfx", 0 )
	ROM_LOAD( "lc13.g5",   0x0000, 0x1000, CRC(19475f2b) SHA1(5d42aa45a7b519dacdecd3d2edbfee6971693034) )
	ROM_LOAD( "lc14.g7",   0x1000, 0x1000, CRC(cc96d1db) SHA1(9713b47b723a5d8837f2a8e8c43e46dc41a62e5b) )

	ROM_REGION( 0x0020, "bg_proms", 0 )
	ROM_LOAD( "lc12.e9",  0x0000, 0x0020, CRC(f6e76547) SHA1(c9ea78d1876156561b3bbf327d7e0299e1d9fd4a) )
ROM_END

void galaxian_rockclim_state::init_rockclim()
{
	galaxian_state::init_mooncrsu();

	m_extend_sprite_info_ptr = extend_sprite_info_delegate(&galaxian_rockclim_state::rockclim_extend_sprite_info, this);
	m_draw_background_ptr = draw_background_delegate(&galaxian_rockclim_state::rockclim_draw_background, this);
}

} // Anonymous namespace

GAME( 1981, rockclim, 0, rockclim,  rockclim,  galaxian_rockclim_state, init_rockclim, ROT180, "Taito", "Rock Climber", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // misses melody
