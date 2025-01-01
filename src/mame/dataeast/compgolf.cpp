// license:BSD-3-Clause
// copyright-holders: Angelo Salese, Pierpaolo Prazzoli, Bryan McPhail

/*******************************************************************************************

    Competition Golf Final Round (c) 1986 / 1985 Data East

    Driver by Angelo Salese, Bryan McPhail and Pierpaolo Prazzoli
    Thanks to David Haywood for the bg roms expansion

    Nb:  The black border around the player sprite in attract mode happens on the real pcb
    as well.

*******************************************************************************************/


#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class compgolf_state : public driver_device
{
public:
	compgolf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"),
		m_rombank(*this, "rombank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void init_compgolf() ATTR_COLD;
	void compgolf(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_rombank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_text_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_scrollx_lo = 0;
	uint16_t m_scrollx_hi = 0;
	uint8_t m_scrolly_lo = 0;
	uint16_t m_scrolly_hi = 0;

	// misc
	int8_t m_bank = 0;

	void ctrl_w(uint8_t data);
	void video_w(offs_t offset, uint8_t data);
	void back_w(offs_t offset, uint8_t data);
	void scrollx_lo_w(uint8_t data);
	void scrolly_lo_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_text_info);
	TILEMAP_MAPPER_MEMBER(back_scan);
	TILE_GET_INFO_MEMBER(get_back_info);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void expand_bg() ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};


void compgolf_state::palette_init(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void compgolf_state::video_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset / 2);
}

void compgolf_state::back_w(offs_t offset, uint8_t data)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(compgolf_state::get_text_info)
{
	tile_index <<= 1;
	tileinfo.set(2, m_videoram[tile_index + 1] | (m_videoram[tile_index] << 8), m_videoram[tile_index] >> 2, 0);
}

TILEMAP_MAPPER_MEMBER(compgolf_state::back_scan)
{
	// logical (col,row) -> memory offset
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(compgolf_state::get_back_info)
{
	int const attr = m_bg_ram[tile_index * 2];
	int const code = m_bg_ram[tile_index * 2 + 1] + ((attr & 1) << 8);
	int const color = (attr & 0x3e) >> 1;

	tileinfo.set(1, code, color, 0);
}

void compgolf_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(compgolf_state::get_back_info)), tilemap_mapper_delegate(*this, FUNC(compgolf_state::back_scan)), 16, 16, 32, 32);
	m_text_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(compgolf_state::get_text_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_text_tilemap->set_transparent_pen(0);
}

/*
preliminary sprite list:
       0        1        2        3
xx------ xxxxxxxx -------- -------- sprite code
---x---- -------- -------- -------- Double Height
----x--- -------- -------- -------- Color,all of it?
-------- -------- xxxxxxxx -------- Y pos
-------- -------- -------- xxxxxxxx X pos
-----x-- -------- -------- -------- Flip X
-------- -------- -------- -------- Flip Y(used?)
*/
void compgolf_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x60; offs += 4)
	{
		int const sprite = m_spriteram[offs + 1] + (((m_spriteram[offs] & 0xc0) >> 6) * 0x100);
		int const x = 240 - m_spriteram[offs + 3];
		int const y = m_spriteram[offs + 2];
		int const color = (m_spriteram[offs] & 8) >> 3;
		int const fx = m_spriteram[offs] & 4;
		int const fy = 0; // ?

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y, 0);

		// Double Height
		if(m_spriteram[offs] & 0x10)
		{
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				sprite + 1,
				color, fx, fy, x, y + 16, 0);
		}
	}
}

uint32_t compgolf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const scrollx = m_scrollx_hi + m_scrollx_lo;
	int const scrolly = m_scrolly_hi + m_scrolly_lo;

	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrolly(0, scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void compgolf_state::scrollx_lo_w(uint8_t data)
{
	m_scrollx_lo = data;
}

void compgolf_state::scrolly_lo_w(uint8_t data)
{
	m_scrolly_lo = data;
}

void compgolf_state::ctrl_w(uint8_t data)
{
	// bit 4 and 6 are always set

	int const new_bank = (data & 4) >> 2;

	if (m_bank != new_bank)
	{
		m_bank = new_bank;
		m_rombank->set_entry(m_bank);
	}

	m_scrollx_hi = (data & 1) << 8;
	m_scrolly_hi = (data & 2) << 7;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void compgolf_state::program_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x17ff).ram().w(FUNC(compgolf_state::video_w)).share(m_videoram);
	map(0x1800, 0x1fff).ram().w(FUNC(compgolf_state::back_w)).share(m_bg_ram);
	map(0x2000, 0x2060).ram().share(m_spriteram);
	map(0x2061, 0x2061).nopw();
	map(0x3000, 0x3000).portr("P1");
	map(0x3001, 0x3001).portr("P2").w(FUNC(compgolf_state::ctrl_w));
	map(0x3002, 0x3002).portr("DSW1");
	map(0x3003, 0x3003).portr("DSW2");
	map(0x3800, 0x3801).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x4000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0xffff).rom();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( compgolf )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03,   0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c,   0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10,   0x10, "Wind Force" )
	PORT_DIPSETTING(      0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x00, "Strong" )
	PORT_DIPNAME( 0x20,   0x20, "Grain of Turf" )
	PORT_DIPSETTING(      0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x00, "Strong" )
	PORT_DIPNAME( 0x40,   0x40, "Range of Ball" )
	PORT_DIPSETTING(      0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x00, "Less" )
	PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01,   0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02,   0x02, "Freeze" ) // this is more likely a switch...
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04,   0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unused ) ) // Manual states dips 4-8 are "Unused"
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10,   0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(8*8*2,1), STEP8(8*8*0,1) },
	{ STEP8(8*8*0,8), STEP8(8*8*1,8) },
	16*16
};

static const gfx_layout tilelayoutbg =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+0, 0, 4 },
	{ 0, 1, 2, 3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
		2*16*8+0, 2*16*8+1, 2*16*8+2, 2*16*8+3, 3*16*8+0, 3*16*8+1, 3*16*8+2, 3*16*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*16
};

static const gfx_layout tilelayout8 =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static GFXDECODE_START( gfx_compgolf )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0, 0x10 )
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayoutbg, 0, 0x20 )
	GFXDECODE_ENTRY( "chars",   0, tilelayout8,  0, 0x10 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void compgolf_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("bgdata")->base(), 0x4000);

	save_item(NAME(m_bank));
	save_item(NAME(m_scrollx_lo));
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_lo));
	save_item(NAME(m_scrolly_hi));
}

void compgolf_state::machine_reset()
{
	m_bank = -1;
	m_scrollx_lo = 0;
	m_scrollx_hi = 0;
	m_scrolly_lo = 0;
	m_scrolly_hi = 0;
}

void compgolf_state::compgolf(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 2'000'000); // HD68B09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &compgolf_state::program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(1*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(compgolf_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, m_palette, FUNC(compgolf_state::palette_init), 0x100);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_compgolf);

	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 1'500'000));
	ymsnd.irq_handler().set_inputline(m_maincpu, 0);
	ymsnd.port_a_write_callback().set(FUNC(compgolf_state::scrollx_lo_w));
	ymsnd.port_b_write_callback().set(FUNC(compgolf_state::scrolly_lo_w));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( compgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cv05-3.bin",   0x08000, 0x8000, CRC(af9805bf) SHA1(bdde482906bb267e76317067785ac0ab7816df63) )

	ROM_REGION( 0x8000, "bgdata", 0 )
	ROM_LOAD( "cv06.bin",     0x00000, 0x8000, CRC(8f76979d) SHA1(432f6a1402fd3276669f5f45f03fd12380900178) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "cv00.bin",     0x00000, 0x8000, CRC(aa3d3b99) SHA1(eb968e40bcc7e7dd1acc0bbe885fd3f7d70d4bb5) )
	ROM_LOAD( "cv01.bin",     0x08000, 0x8000, CRC(f68c2ff6) SHA1(dda9159fb59d3855025b98c272722b031617c89a) )
	ROM_LOAD( "cv02.bin",     0x10000, 0x8000, CRC(979cdb5a) SHA1(25c1f3e6ddf50168c7e1a967bfa2753bea6106ec) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "cv03.bin",     0x00000, 0x8000, CRC(cc7ed6d8) SHA1(4ffcfa3f720414e1b7e929bdf29359ebcd8717c3) )
	// we expand ROM cv04.bin to 0x8000 - 0xffff

	ROM_REGION( 0x8000,  "chars", 0 )
	ROM_LOAD( "cv07.bin",     0x00000, 0x8000, CRC(ed5441ba) SHA1(69d50695e8b92544f9857c6f3de0efb399899a2c) )

	ROM_REGION( 0x4000, "bgtiles2", 0 )
	ROM_LOAD( "cv04.bin",     0x00000, 0x4000, CRC(df693a04) SHA1(45bef98c7e66881f8c62affecc1ab90dd2707240) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cv08-1.bpr",   0x00000, 0x0100, CRC(b7c43db9) SHA1(418b11e4c8a9bce6873b0624ac53a5011c5807d0) )
ROM_END

ROM_START( compgolfo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cv05.bin",     0x08000, 0x8000, CRC(3cef62c9) SHA1(c4827b45faf7aa4c80ddd3c57f1ed6ba76b5c49b) )

	ROM_REGION( 0x8000, "bgdata", 0 )
	ROM_LOAD( "cv06.bin",     0x00000, 0x8000, CRC(8f76979d) SHA1(432f6a1402fd3276669f5f45f03fd12380900178) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "cv00.bin",     0x00000, 0x8000, CRC(aa3d3b99) SHA1(eb968e40bcc7e7dd1acc0bbe885fd3f7d70d4bb5) )
	ROM_LOAD( "cv01.bin",     0x08000, 0x8000, CRC(f68c2ff6) SHA1(dda9159fb59d3855025b98c272722b031617c89a) )
	ROM_LOAD( "cv02.bin",     0x10000, 0x8000, CRC(979cdb5a) SHA1(25c1f3e6ddf50168c7e1a967bfa2753bea6106ec) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "cv03.bin",     0x00000, 0x8000, CRC(cc7ed6d8) SHA1(4ffcfa3f720414e1b7e929bdf29359ebcd8717c3) )
	// we expand ROM cv04.bin to 0x8000 - 0xffff

	ROM_REGION( 0x8000,  "chars", 0 )
	ROM_LOAD( "cv07.bin",     0x00000, 0x8000, CRC(ed5441ba) SHA1(69d50695e8b92544f9857c6f3de0efb399899a2c) )

	ROM_REGION( 0x4000, "bgtiles2", 0 )
	ROM_LOAD( "cv04.bin",     0x00000, 0x4000, CRC(df693a04) SHA1(45bef98c7e66881f8c62affecc1ab90dd2707240) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cv08-1.bpr",   0x00000, 0x0100, CRC(b7c43db9) SHA1(418b11e4c8a9bce6873b0624ac53a5011c5807d0) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void compgolf_state::expand_bg()
{
	uint8_t *GFXDST = memregion("bgtiles")->base();
	uint8_t *GFXSRC = memregion("bgtiles2")->base();

	for (int x = 0; x < 0x4000; x++)
	{
		GFXDST[0x8000 + x]  = (GFXSRC[x] & 0x0f) << 4;
		GFXDST[0xc000 + x]  = (GFXSRC[x] & 0xf0);
	}
}

void compgolf_state::init_compgolf()
{
	expand_bg();
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, compgolf,  0,        compgolf, compgolf, compgolf_state, init_compgolf, ROT0, "Data East Corporation", "Competition Golf Final Round (World?, revision 3)",         MACHINE_SUPPORTS_SAVE )
GAME( 1985, compgolfo, compgolf, compgolf, compgolf, compgolf_state, init_compgolf, ROT0, "Data East Corporation", "Competition Golf Final Round (Japan, old version)", MACHINE_SUPPORTS_SAVE )
