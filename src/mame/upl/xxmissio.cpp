// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

    XX Mission (c) 1986 UPL

    Driver by Uki

    31/Mar/2001 -

*****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class xxmissio_state : public driver_device
{
public:
	xxmissio_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_subbank(*this, "subbank")
	{ }

	void xxmissio(machine_config &config);

	template <int Mask> int status_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void bank_sel_w(uint8_t data);
	void status_m_w(uint8_t data);
	void status_s_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void fgram_w(offs_t offset, uint8_t data);
	void bgram_w(offs_t offset, uint8_t data);
	uint8_t bgram_r(offs_t offset);
	void scroll_x_w(uint8_t data);
	void scroll_y_w(uint8_t data);

	void interrupt_m(int state);
	INTERRUPT_GEN_MEMBER(interrupt_s);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	static rgb_t BBGGRRII(uint32_t raw);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

	void base_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_subbank;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_status = 0;
	uint8_t m_xscroll = 0;
	uint8_t m_yscroll = 0;
	uint8_t m_flipscreen = 0;
};


void xxmissio_state::scroll_x_w(uint8_t data)
{
	m_xscroll = data;
}
void xxmissio_state::scroll_y_w(uint8_t data)
{
	m_yscroll = data;
}

void xxmissio_state::flipscreen_w(uint8_t data)
{
	m_flipscreen = BIT(data, 0);
}

void xxmissio_state::bgram_w(offs_t offset, uint8_t data)
{
	int const x = (offset + (m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}
uint8_t xxmissio_state::bgram_r(offs_t offset)
{
	int const x = (offset + (m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	return m_bgram[offset];
}

void xxmissio_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

/****************************************************************************/

TILE_GET_INFO_MEMBER(xxmissio_state::get_bg_tile_info)
{
	int const code = ((m_bgram[0x400 | tile_index] & 0xc0) << 2) | m_bgram[0x000 | tile_index];
	int const color =  m_bgram[0x400 | tile_index] & 0x0f;

	tileinfo.set(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(xxmissio_state::get_fg_tile_info)
{
	int const code = m_fgram[0x000 | tile_index];
	int const color = m_fgram[0x400 | tile_index] & 0x07;

	tileinfo.set(0, code, color, 0);
}

void xxmissio_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xxmissio_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xxmissio_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(1);
	m_bg_tilemap->set_scroll_rows(1);
	m_bg_tilemap->set_scrolldx(2, 12);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_flipscreen));
}

rgb_t xxmissio_state::BBGGRRII(uint32_t raw)
{
	uint8_t const i = raw & 3;
	uint8_t const r = ((raw >> 0) & 0x0c) | i;
	uint8_t const g = ((raw >> 2) & 0x0c) | i;
	uint8_t const b = ((raw >> 4) & 0x0c) | i;

	return rgb_t(r | (r << 4), g | (g << 4), b | (b << 4));
}

void xxmissio_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx)
{
	for (int offs = 0; offs < 0x800; offs += 0x20)
	{
		int chr = m_spriteram[offs];
		int col = m_spriteram[offs + 3];

		int const fx = BIT(col, 4) ^ m_flipscreen;
		int const fy = BIT(col, 5) ^ m_flipscreen;

		int const x = m_spriteram[offs + 1] * 2;
		int const y = m_spriteram[offs + 2];

		chr += (col & 0x40) << 2;
		col &= 0x07;

		int px, py;
		if (!m_flipscreen)
		{
			px = x - 8;
			py = y;
		}
		else
		{
			px = 480 - x - 6;
			py = 240 - y;
		}

		px &= 0x1ff;

		gfx->transpen(bitmap, cliprect,
			chr,
			col,
			fx, fy,
			px, py, 0);

		if (px > 0x1e0)
			gfx->transpen(bitmap, cliprect,
				chr,
				col,
				fx, fy,
				px - 0x200, py, 0);

	}
}


uint32_t xxmissio_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_bg_tilemap->set_scrollx(0, m_xscroll * 2);
	m_bg_tilemap->set_scrolly(0, m_yscroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void xxmissio_state::bank_sel_w(uint8_t data)
{
	m_subbank->set_entry(data & 7);
}

template <int Mask>
int xxmissio_state::status_r()
{
	return (m_status & Mask) ? 1 : 0;
}

void xxmissio_state::status_m_w(uint8_t data)
{
	switch (data)
	{
		case 0x00:
			m_status |= 0x20;
			break;

		case 0x40:
			m_status &= ~0x08;
			m_subcpu->set_input_line_and_vector(0, HOLD_LINE, 0x10); // Z80
			break;

		case 0x80:
			m_status |= 0x04;
			break;
	}
}

void xxmissio_state::status_s_w(uint8_t data)
{
	switch (data)
	{
		case 0x00:
			m_status |= 0x10;
			break;

		case 0x40:
			m_status |= 0x08;
			break;

		case 0x80:
			m_status &= ~0x04;
			m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x10); // Z80
			break;
	}
}

void xxmissio_state::interrupt_m(int state)
{
	if (state)
	{
		m_status &= ~0x20;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

INTERRUPT_GEN_MEMBER(xxmissio_state::interrupt_s)
{
	m_status &= ~0x10;
	m_subcpu->set_input_line(0, HOLD_LINE);
}

void xxmissio_state::machine_start()
{
	m_subbank->configure_entries(0, 8, memregion("bankedroms")->base(), 0x4000);
	m_subbank->set_entry(0);

	save_item(NAME(m_status));
}

/****************************************************************************/

void xxmissio_state::base_map(address_map &map)
{
	map(0x8000, 0x8001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x8002, 0x8003).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));

	map(0xa000, 0xa000).portr("P1");
	map(0xa001, 0xa001).portr("P2");
	map(0xa002, 0xa002).portr("STATUS");
	map(0xa003, 0xa003).w(FUNC(xxmissio_state::flipscreen_w));

	map(0xc000, 0xc7ff).ram().w(FUNC(xxmissio_state::fgram_w)).share(m_fgram);
	map(0xc800, 0xcfff).rw(FUNC(xxmissio_state::bgram_r), FUNC(xxmissio_state::bgram_w)).share(m_bgram);
	map(0xd000, 0xd7ff).ram().share(m_spriteram);

	map(0xd800, 0xdaff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}


void xxmissio_state::main_map(address_map &map)
{
	base_map(map);

	map(0x0000, 0x7fff).rom();

	map(0xa002, 0xa002).w(FUNC(xxmissio_state::status_m_w));

	map(0xe000, 0xefff).share("workram1").ram();
	map(0xf000, 0xffff).share("workram2").ram();
}


void xxmissio_state::sub_map(address_map &map)
{
	base_map(map);

	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_subbank);

	map(0x8006, 0x8006).w(FUNC(xxmissio_state::bank_sel_w));

	map(0xa002, 0xa002).w(FUNC(xxmissio_state::status_s_w));

	map(0xe000, 0xefff).share("workram2").ram();
	map(0xf000, 0xffff).share("workram1").ram();
}


/****************************************************************************/

static INPUT_PORTS_START( xxmissio )
	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Endless Game (Cheat)")   PORT_DIPLOCATION("SW1:7") // Shown as "Unused" in the manual
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW1:8" )     // Shown as "Unused" in the manual

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "First Bonus" )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Every" )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x10, "90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" ) // Shown as "Unused" in the manual
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" ) // Shown as "Unused" in the manual
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" ) // Shown as "Unused" in the manual

	PORT_START("STATUS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x01>)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x04>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x08>)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x10>)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x20>)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x40>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xxmissio_state, status_r<0x80>)
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout charlayout =
{
	16,8,   // 16*8 characters
	RGN_FRAC(1,1),   // 2048 characters
	4,      // 4 bits per pixel
	{ STEP4(0,1) },
	{ STEP16(0,4) },
	{ STEP8(0,4*16) },
	4*16*8
};

static const gfx_layout spritelayout =
{
	32,16,    // 32*16 characters
	RGN_FRAC(1,1),      // 512 sprites
	4,        // 4 bits per pixel
	{ STEP4(0,1) },
	{ STEP16(0,4), STEP16(4*16*8,4) },
	{ STEP8(0,4*16), STEP8(4*16*8*2,4*16) },
	64*8*4
};

static GFXDECODE_START( gfx_xxmissio )
	GFXDECODE_ENTRY( "chars_sprites", 0x0000, charlayout,   256,  8 ) // FG
	GFXDECODE_ENTRY( "chars_sprites", 0x0000, spritelayout,   0,  8 ) // sprite
	GFXDECODE_ENTRY( "tiles",         0x0000, charlayout,   512, 16 ) // BG
GFXDECODE_END

/****************************************************************************/

void xxmissio_state::xxmissio(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // 3.0MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &xxmissio_state::main_map);

	Z80(config, m_subcpu, 12_MHz_XTAL / 4); // 3.0MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &xxmissio_state::sub_map);
	m_subcpu->set_periodic_int(FUNC(xxmissio_state::interrupt_s), attotime::from_hz(2*60));

	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 4*8, 28*8-1);
	screen.set_screen_update(FUNC(xxmissio_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(xxmissio_state::interrupt_m));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_xxmissio);
	PALETTE(config, m_palette).set_format(1, &xxmissio_state::BBGGRRII, 768);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8));
	ym1.port_a_read_callback().set_ioport("DSW1");
	ym1.port_b_read_callback().set_ioport("DSW2");
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.40);

	ym2203_device &ym2(YM2203(config, "ym2", 12_MHz_XTAL / 8));
	ym2.port_a_write_callback().set(FUNC(xxmissio_state::scroll_x_w));
	ym2.port_b_write_callback().set(FUNC(xxmissio_state::scroll_y_w));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.40);
}

/****************************************************************************/

ROM_START( xxmissio )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xx1.4l", 0x0000,  0x8000, CRC(86e07709) SHA1(7bfb7540b6509f07a6388ca2da6b3892f5b1df74) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "xx2.4b", 0x0000,  0x4000, CRC(13fa7049) SHA1(e8974d9f271a966611b523496ba8cd910e227a23) )

	ROM_REGION( 0x18000, "bankedroms", 0 )
	ROM_LOAD( "xx3.6a", 0x00000,  0x8000, CRC(16fdacab) SHA1(2158ca9b14c52bc1cd5ef0f4c0180f0519224403) )
	ROM_LOAD( "xx4.6b", 0x08000,  0x8000, CRC(274bd4d2) SHA1(2ddf9b953584e26f221b1c86181d827bdc3dc81b) )
	ROM_LOAD( "xx5.6d", 0x10000,  0x8000, CRC(c5f35535) SHA1(6812b70beb73fc80cf20d2d51f747952ed106887) )

	ROM_REGION( 0x20000, "chars_sprites", 0 ) // FG / sprites
	ROM_LOAD16_BYTE( "xx6.8j", 0x00001, 0x8000, CRC(dc954d01) SHA1(73ecbbc859da9db9fead91cd03bb90e5779916e2) )
	ROM_LOAD16_BYTE( "xx8.8f", 0x00000, 0x8000, CRC(a9587cc6) SHA1(5fbcb88505f89c4d8a2a228489612ff66fc5d3af) )
	ROM_LOAD16_BYTE( "xx7.8h", 0x10001, 0x8000, CRC(abe9cd68) SHA1(f3ce9b40e3d9cdc9b77a43f9d5d0411338d88833) )
	ROM_LOAD16_BYTE( "xx9.8e", 0x10000, 0x8000, CRC(854e0e5f) SHA1(b01d6a735b175c2f7ac3fc4053702c9da62c6a4e) )

	ROM_REGION( 0x10000, "tiles", 0 ) // BG
	ROM_LOAD16_BYTE( "xx10.4c", 0x0000,  0x8000, CRC(d27d7834) SHA1(60c24dc2ab7e2a33da4002f1f07eaf7898cf387f) )
	ROM_LOAD16_BYTE( "xx11.4b", 0x0001,  0x8000, CRC(d9dd827c) SHA1(aea3a5abd871adf7f75ad4d6cc57eff0833135c7) )
ROM_END

} // anonymous namespace


GAME( 1986, xxmissio, 0, xxmissio, xxmissio, xxmissio_state, empty_init, ROT90, "UPL", "XX Mission", MACHINE_SUPPORTS_SAVE )
