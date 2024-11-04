// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer

/***************************************************************************

    Hole Land

    driver by Mathis Rosenhauer

    Notes:
    - In stop mode press p1 start to freeze the screen, p2 start to resume

    TODO:
    - missing high bit of sprite X coordinate? (see round 2 and 3 of attract
      mode in crzrally)
    - crzrally: emulate steering wheel;

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_latch(*this, "latch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

protected:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_latch;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_palette_offset = 0U;

	void coin_counter_w(int state);

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void pal_offs_w(uint8_t data);
	void scroll_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
};

class holeland_state : public base_state
{
public:
	holeland_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag)
	{ }

	void holeland(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};

class crzrally_state : public base_state
{
public:
	crzrally_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag)
	{ }

	void crzrally(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(holeland_state::get_tile_info)
{
	/*
	x--- ---- priority (1) behind sprites
	xxxx ---- color
	---- xx-- flip yx
	---- --xx tile upper bits
	*/

	int const attr = m_colorram[tile_index];
	int const tile_number = m_videoram[tile_index] | ((attr & 0x03) << 8);

	tileinfo.set(0,
			tile_number,
			m_palette_offset + ((attr >> 4) & 0x0f),
			TILE_FLIPYX((attr >> 2) & 0x03));
	tileinfo.group = (attr >> 7) & 1;
}

TILE_GET_INFO_MEMBER(crzrally_state::get_tile_info)
{
	int const attr = m_colorram[tile_index];
	int const tile_number = m_videoram[tile_index] | ((attr & 0x03) << 8);

	tileinfo.set(0,
			tile_number,
			m_palette_offset + ((attr >> 4) & 0x0f),
			TILE_FLIPYX((attr >> 2) & 0x03));
	tileinfo.group = (attr >> 5) & 1;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void holeland_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(holeland_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bg_tilemap->set_transmask(0, 0xff, 0x00); // split type 0 is totally transparent in front half
	m_bg_tilemap->set_transmask(1, 0x01, 0xfe); // split type 1 has pen 0? transparent in front half

	save_item(NAME(m_palette_offset));
}

void crzrally_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crzrally_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	save_item(NAME(m_palette_offset));
}

void base_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void base_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void base_state::pal_offs_w(uint8_t data)
{
	if ((m_palette_offset >> 4) != (data & 3))
	{
		m_palette_offset = (data & 3) << 4;
		machine().tilemap().mark_all_dirty();
	}
}

void base_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, data);
}


void holeland_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Weird, sprites entries don't start on DWORD boundary
	for (int offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		int sy = 236 - m_spriteram[offs];
		int sx = m_spriteram[offs + 2];

		// Bit 7 unknown
		int const code = m_spriteram[offs + 1] & 0x7f;
		int const color = m_palette_offset + (m_spriteram[offs + 3] >> 4);

		// Bit 0, 1 unknown
		int flipx = m_spriteram[offs + 3] & 0x04;
		int flipy = m_spriteram[offs + 3] & 0x08;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				2 * sx, 2 * sy, 0);
	}
}

void crzrally_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Weird, sprites entries don't start on DWORD boundary
	for (int offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		int sy = 236 - m_spriteram[offs];
		int sx = m_spriteram[offs + 2];

		int const code = m_spriteram[offs + 1] + ((m_spriteram[offs + 3] & 0x01) << 8);
		int const color = (m_spriteram[offs + 3] >> 4) + ((m_spriteram[offs + 3] & 0x01) << 4);

		// Bit 1 unknown but somehow related to X offset (clipping range?)
		int flipx = m_spriteram[offs + 3] & 0x04;
		int flipy = m_spriteram[offs + 3] & 0x08;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

uint32_t holeland_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}

uint32_t crzrally_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void base_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void holeland_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xbfff).rom();
	map(0xa000, 0xa000).w("speech", FUNC(sp0256_device::ald_w));
	map(0xc000, 0xc007).w(m_latch, FUNC(ls259_device::write_d0)).nopr();
	map(0xe000, 0xe3ff).ram().w(FUNC(holeland_state::colorram_w)).share(m_colorram);
	map(0xe400, 0xe7ff).ram().w(FUNC(holeland_state::videoram_w)).share(m_videoram);
	map(0xf000, 0xf3ff).ram().share(m_spriteram);
}

void crzrally_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().share("nvram");
	map(0xe000, 0xe3ff).ram().w(FUNC(crzrally_state::colorram_w)).share(m_colorram);
	map(0xe400, 0xe7ff).ram().w(FUNC(crzrally_state::videoram_w)).share(m_videoram);
	map(0xe800, 0xebff).ram().share(m_spriteram);
	map(0xf000, 0xf000).w(FUNC(crzrally_state::scroll_w));
	map(0xf800, 0xf807).w(m_latch, FUNC(ls259_device::write_d0));
}

void base_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("watchdog", FUNC(watchdog_timer_device::reset_r));  // ?
	map(0x04, 0x04).r("ay1", FUNC(ay8910_device::data_r));
	map(0x04, 0x05).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x06, 0x06).r("ay2", FUNC(ay8910_device::data_r));
	map(0x06, 0x07).w("ay2", FUNC(ay8910_device::address_data_w));
}

// Note - manual states cocktail mode should be default
static INPUT_PORTS_START( holeland )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// change any dipswitch in input test to show the list of settings
	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPSETTING(    0x20, "90000" )
	PORT_DIPNAME( 0x40, 0x00, "Phase 3 Difficulty") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Coin Case" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Monsters" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Min" )
	PORT_DIPSETTING(    0x00, "Max" )
	PORT_DIPNAME( 0x80, 0x80, "Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "Stop" )
	PORT_DIPSETTING(    0x80, "Play" )
INPUT_PORTS_END

static INPUT_PORTS_START( holeland2 )
	PORT_INCLUDE( holeland )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Spanish ) )
INPUT_PORTS_END

static INPUT_PORTS_START( crzrally )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PC=1f37 (from 1f6c): accelerator in steering wheel mode

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Drive" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x04, "B" )
	PORT_DIPSETTING(    0x08, "C" )
	PORT_DIPSETTING(    0x0c, "D" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Extra Time" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "5 Sec" )
	PORT_DIPSETTING(    0x40, "10 Sec" )
	PORT_DIPSETTING(    0x60, "15 Sec" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controller ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "Steering Wheel" )
	PORT_DIPNAME( 0x80, 0x80, "Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "Stop" )
	PORT_DIPSETTING(    0x80, "Play" )
INPUT_PORTS_END



static const gfx_layout holeland_charlayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0,0, 1,1, 2,2, 3,3, 8+0,8+0, 8+1,8+1, 8+2,8+2, 8+3,8+3 },
	{ 0*16,0*16, 1*16,1*16, 2*16,2*16, 3*16,3*16, 4*16,4*16, 5*16,5*16, 6*16,6*16, 7*16,7*16 },
	8*16
};

static const gfx_layout crzrally_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout holeland_spritelayout =
{
	32,32,
	RGN_FRAC(1,4),
	2,
	{ 4, 0 },
	{ 0, 2, 1, 3, 1*8+0, 1*8+2, 1*8+1, 1*8+3, 2*8+0, 2*8+2, 2*8+1, 2*8+3, 3*8+0, 3*8+2, 3*8+1, 3*8+3,
			4*8+0, 4*8+2, 4*8+1, 4*8+3, 5*8+0, 5*8+2, 5*8+1, 5*8+3, 6*8+0, 6*8+2, 6*8+1, 6*8+3, 7*8+0, 7*8+2, 7*8+1, 7*8+3 },
	{ 0, 4*64, RGN_FRAC(1,4), RGN_FRAC(1,4)+4*64, RGN_FRAC(2,4), RGN_FRAC(2,4)+4*64, RGN_FRAC(3,4), RGN_FRAC(3,4)+4*64,
		1*64, 5*64, RGN_FRAC(1,4)+1*64, RGN_FRAC(1,4)+5*64, RGN_FRAC(2,4)+1*64, RGN_FRAC(2,4)+5*64, RGN_FRAC(3,4)+1*64, RGN_FRAC(3,4)+5*64,
		2*64, 6*64, RGN_FRAC(1,4)+2*64, RGN_FRAC(1,4)+6*64, RGN_FRAC(2,4)+2*64, RGN_FRAC(2,4)+6*64, RGN_FRAC(3,4)+2*64, RGN_FRAC(3,4)+6*64,
		3*64, 7*64, RGN_FRAC(1,4)+3*64, RGN_FRAC(1,4)+7*64, RGN_FRAC(2,4)+3*64, RGN_FRAC(2,4)+7*64, RGN_FRAC(3,4)+3*64, RGN_FRAC(3,4)+7*64 },
	64*8
};

static const gfx_layout crzrally_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ 0, 1 },
	{ 3*2, 2*2, 1*2, 0*2, 7*2, 6*2, 5*2, 4*2,
			16+3*2, 16+2*2, 16+1*2, 16+0*2, 16+7*2, 16+6*2, 16+5*2, 16+4*2 },
	{       RGN_FRAC(3,4)+0*16, RGN_FRAC(2,4)+0*16, RGN_FRAC(1,4)+0*16, RGN_FRAC(0,4)+0*16,
			RGN_FRAC(3,4)+2*16, RGN_FRAC(2,4)+2*16, RGN_FRAC(1,4)+2*16, RGN_FRAC(0,4)+2*16,
			RGN_FRAC(3,4)+4*16, RGN_FRAC(2,4)+4*16, RGN_FRAC(1,4)+4*16, RGN_FRAC(0,4)+4*16,
			RGN_FRAC(3,4)+6*16, RGN_FRAC(2,4)+6*16, RGN_FRAC(1,4)+6*16, RGN_FRAC(0,4)+6*16 },
	8*16
};

static GFXDECODE_START( gfx_holeland )
	GFXDECODE_ENTRY( "chars",   0, holeland_charlayout,   0, 256 )
	GFXDECODE_ENTRY( "sprites", 0, holeland_spritelayout, 0, 256 )
GFXDECODE_END

static GFXDECODE_START( gfx_crzrally )
	GFXDECODE_ENTRY( "chars",   0, crzrally_charlayout,   0, 256 )
	GFXDECODE_ENTRY( "sprites", 0, crzrally_spritelayout, 0, 256 )
GFXDECODE_END


void holeland_state::holeland(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3'355'700); // measured 298ns on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &holeland_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &holeland_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(holeland_state::irq0_line_hold));

	LS259(config, m_latch); // 3J
	m_latch->parallel_out_cb().set(FUNC(holeland_state::pal_offs_w)).mask(0x03);
	m_latch->q_out_cb<5>().set(FUNC(holeland_state::coin_counter_w));
	m_latch->q_out_cb<6>().set(FUNC(holeland_state::flip_screen_x_set));
	m_latch->q_out_cb<7>().set(FUNC(holeland_state::flip_screen_y_set));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// TODO: 448i, compensate.
	screen.set_raw((20_MHz_XTAL / 4) * 4, 332 * 2, 0, 256 * 2, 256 * 2, 16 * 2, 240 * 2);
	screen.set_screen_update(FUNC(holeland_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_holeland);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 20_MHz_XTAL / 32)); // verified on PCB
	ay1.port_a_read_callback().set_ioport("IN0");
	ay1.port_b_read_callback().set_ioport("IN1");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 20_MHz_XTAL / 16)); // verified on PCB
	ay2.port_a_read_callback().set_ioport("DSW1");
	ay2.port_b_read_callback().set_ioport("DSW2");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);

	sp0256_device &speech(SP0256(config, "speech", 3'355'700)); // measured 298ns on PCB
	speech.data_request_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	speech.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void crzrally_state::crzrally(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 20_MHz_XTAL / 4);        // 5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &crzrally_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &crzrally_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(crzrally_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	LS259(config, m_latch);
	m_latch->parallel_out_cb().set(FUNC(crzrally_state::pal_offs_w)).mask(0x03);
	m_latch->q_out_cb<5>().set(FUNC(crzrally_state::coin_counter_w));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(20_MHz_XTAL / 4, 332, 0, 256, 256, 16, 240);
	screen.set_screen_update(FUNC(crzrally_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_crzrally);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 20_MHz_XTAL / 16));
	ay1.port_a_read_callback().set_ioport("IN0");
	ay1.port_b_read_callback().set_ioport("IN1");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 20_MHz_XTAL / 16));
	ay2.port_a_read_callback().set_ioport("DSW1");
	ay2.port_b_read_callback().set_ioport("DSW2");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( holeland )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.2a",  0x0000, 0x2000, CRC(b640e12b) SHA1(68d091a92747d2f4534386aff3ddb07c0d79384c) )
	ROM_LOAD( "1.2b",  0x2000, 0x2000, CRC(2f180851) SHA1(c21bcd3e9ff31a5cc415eb53d77a9cc9ebdd862d) )
	ROM_LOAD( "2.1d",  0x4000, 0x2000, CRC(35cfde75) SHA1(0a03c0464c771d049ae8706793ec43da5372fa58) )
	ROM_LOAD( "3.2d",  0x6000, 0x2000, CRC(5537c22e) SHA1(030f34d3cbc5eea30a3ede77008eba394ef37e8f) )
	ROM_LOAD( "4.1e",  0xa000, 0x2000, CRC(c95c355d) SHA1(44984108b6a3dab05855da4c4a3ff58d849559b8) )

	ROM_REGION( 0x4000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "5.4d",  0x0000, 0x2000, CRC(7f19e1f9) SHA1(75026da91e0cff262e5f6e32f836907a786aef42) )
	ROM_LOAD( "6.4e",  0x2000, 0x2000, CRC(844400e3) SHA1(d306b26f838b043b71c5f9d2d240228986b695fa) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "7.4m",  0x0000, 0x2000, CRC(d7feb25b) SHA1(581e20b07d33ba350601fc56074c43aaf13078b4) )
	ROM_LOAD( "8.4n",  0x2000, 0x2000, CRC(4b6eec16) SHA1(4c5da89c2babeb33951d101703e6699fbcb886b4) )
	ROM_LOAD( "9.4p",  0x4000, 0x2000, CRC(6fe7fcc0) SHA1(fa982551285f728cee0055a0c473f6c74d802d2e) )
	ROM_LOAD( "10.4r", 0x6000, 0x2000, CRC(e1e11e8f) SHA1(56082fe497d8ee8ecfe1b89c0c5ada4ddfa4740f) )

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256a-al2.1b",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.3m", 0x0000, 0x0100, CRC(9d6fef5a) SHA1(e2b62909fecadfc9e0eb1ad72c8b7712a26d184e) )  // Red component
	ROM_LOAD( "82s129.3l", 0x0100, 0x0100, CRC(f6682705) SHA1(1ab952c1e2a45e9b0dc9144f50711f99f6b1ebc4) )  // Green component
	ROM_LOAD( "82s129.3n", 0x0200, 0x0100, CRC(3d7b3af6) SHA1(0c4f95b26e9fe25a5d8c79f06e7ceab78a07d35c) )  // Blue component
ROM_END

ROM_START( holeland2 ) // PCB REF.001/B
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.2a",  0x0000, 0x2000, CRC(b26212a9) SHA1(93ac3910b22e29f66a8ecbc9f7df8aa6b405ca9a) )
	ROM_LOAD( "3.2b",  0x2000, 0x2000, CRC(623bca75) SHA1(a3406077271229a3a4f253d238aece369b0120d9) )
	ROM_LOAD( "0.1d",  0x4000, 0x2000, CRC(a3bafdae) SHA1(cda2adf2a3eeab0301505cbd3bea9e9450b42b0a) )
	ROM_LOAD( "4.2d",  0x6000, 0x2000, CRC(88a8ba11) SHA1(ce810c8ea0a78f94025f1ac40d5641a9287df4f0) )
	ROM_LOAD( "1.1e",  0xa000, 0x2000, CRC(ec338f4b) SHA1(ae78a40f85b489e57377e4c60181895f781efe16) )

	ROM_REGION( 0x4000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "5.4d",  0x0000, 0x2000, CRC(7f19e1f9) SHA1(75026da91e0cff262e5f6e32f836907a786aef42) )
	ROM_LOAD( "6.4e",  0x2000, 0x2000, CRC(844400e3) SHA1(d306b26f838b043b71c5f9d2d240228986b695fa) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "7.4m",  0x0000, 0x2000, CRC(d7feb25b) SHA1(581e20b07d33ba350601fc56074c43aaf13078b4) )
	ROM_LOAD( "8.4n",  0x2000, 0x2000, CRC(4b6eec16) SHA1(4c5da89c2babeb33951d101703e6699fbcb886b4) )
	ROM_LOAD( "9.4p",  0x4000, 0x2000, CRC(6fe7fcc0) SHA1(fa982551285f728cee0055a0c473f6c74d802d2e) )
	ROM_LOAD( "10.4r", 0x6000, 0x2000, CRC(e1e11e8f) SHA1(56082fe497d8ee8ecfe1b89c0c5ada4ddfa4740f) )

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256a_al2.1b",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.3m", 0x0000, 0x0100, CRC(9d6fef5a) SHA1(e2b62909fecadfc9e0eb1ad72c8b7712a26d184e) )  // Red component
	ROM_LOAD( "82s129.3l", 0x0100, 0x0100, CRC(f6682705) SHA1(1ab952c1e2a45e9b0dc9144f50711f99f6b1ebc4) )  // Green component
	ROM_LOAD( "82s129.3n", 0x0200, 0x0100, CRC(3d7b3af6) SHA1(0c4f95b26e9fe25a5d8c79f06e7ceab78a07d35c) )  // Blue component
ROM_END

/*

Crazy Rally
Tecfri, 1985

PCB Layout
|----------------------------------------------|
|                   20MHz                      |
|  Z80                                         |
|                                        PAL   |
|              3.7D      2149                  |
|  AY-3-8910                                   |
|  DSW2   DSW1 2.7F      2149         82S147.1F|
|1                                             |
|8 AY-3-8910   1.7G       5.5G                 |
|W           555                               |
|A                        4.5H                 |
|Y                                             |
|     VOL                 2128          9.1I   |
|         741                                  |
|    TDA1510                            8.1K   |
|              2149  2149  PAL                 |
|   82S129.9L  2149  2149  PAL          7.1L   |
|   82S129.9M  2149  2149  PAL                 |
|   82S129.9N  2149  2149               6.1N   |
|----------------------------------------------|
Notes:
      Z80 clock - 5.000MHz [20/4]
      AY3-8910 clock - 1.25MHz [20/16]
      VSync - 59Hz
      HSync - 15.08kHz

*/

ROM_START( crzrally )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.7g",        0x0000, 0x4000, CRC(8fe01f86) SHA1(3e08f2cdcd08b25f2bb32d1c4d4caf4ac60c94d6) )
	ROM_LOAD( "2.7f",        0x4000, 0x4000, CRC(67110f1d) SHA1(cc500017057e39cc8a6cb4e4ccae3c3cbab6c2ba) )
	ROM_LOAD( "3.7d",        0x8000, 0x4000, CRC(25c861c3) SHA1(cc9f5f33833279b4430a4b8497cc16a222d31805) )

	ROM_REGION( 0x4000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "4.5h",        0x0000, 0x2000, CRC(29dece8b) SHA1(d8a0cfd1259d49f59f9751a2db99b46b9da6a87d) )
	ROM_LOAD( "5.5g",        0x2000, 0x2000, CRC(b34aa904) SHA1(fb4301fd06efc33df9d9f611c3e67a9f7198531d) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "6.1f",        0x0000, 0x2000, CRC(a909ff0f) SHA1(9ce37a6bbb09c936551082dea62a791d10d7d346) )
	ROM_LOAD( "7.1l",        0x2000, 0x2000, CRC(38fb0a16) SHA1(a17ec5c9acc5c244ffc715ee2376fbf8209e72fd) )
	ROM_LOAD( "8.1k",        0x4000, 0x2000, CRC(660aa0f0) SHA1(1bb85851349f772f21db9629b0086b2460614b9d) )
	ROM_LOAD( "9.1i",        0x6000, 0x2000, CRC(37d0790e) SHA1(877335a06d1842264daff9eb46d6ea1ce8249c29) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n",   0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  // Red component
	ROM_LOAD( "82s129.9m",   0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  // Green component
	ROM_LOAD( "82s129.9l",   0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  // Blue component

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16r6a.5k",  0x0000, 0x0104, CRC(3d12afba) SHA1(60245089947e4a4f7bfa94a8cc96d4d8eebe4afc) )
	ROM_LOAD( "pal16r4a.5l",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4a.5m",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r8a.1d",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( crzrallya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crzralla_1.7g",        0x0000, 0x4000, CRC(8c6a70aa) SHA1(61b10cb16ddce813a768181483b03bead5b05702) )
	ROM_LOAD( "crzralla_2.7f",        0x4000, 0x4000, CRC(7fdd4a45) SHA1(194d504adfd83adc52df2df27a18116a3072ea9d) )
	ROM_LOAD( "crzralla_3.7d",        0x8000, 0x4000, CRC(a25edd17) SHA1(8f883bf3e42b9bf929717f6f13a281f0b83669b1) )

	ROM_REGION( 0x4000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "4.5h",                 0x0000, 0x2000, CRC(29dece8b) SHA1(d8a0cfd1259d49f59f9751a2db99b46b9da6a87d) )
	ROM_LOAD( "crzralla_5.5g",        0x2000, 0x2000, CRC(81e9b043) SHA1(effc082a025ce36ab6ba8603a82be1469eee6276) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "6.1f",        0x0000, 0x2000, CRC(a909ff0f) SHA1(9ce37a6bbb09c936551082dea62a791d10d7d346) )
	ROM_LOAD( "7.1l",        0x2000, 0x2000, CRC(38fb0a16) SHA1(a17ec5c9acc5c244ffc715ee2376fbf8209e72fd) )
	ROM_LOAD( "8.1k",        0x4000, 0x2000, CRC(660aa0f0) SHA1(1bb85851349f772f21db9629b0086b2460614b9d) )
	ROM_LOAD( "9.1i",        0x6000, 0x2000, CRC(37d0790e) SHA1(877335a06d1842264daff9eb46d6ea1ce8249c29) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n",   0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  // Red component
	ROM_LOAD( "82s129.9m",   0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  // Green component
	ROM_LOAD( "82s129.9l",   0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  // Blue component

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16r6a.5k",  0x0000, 0x0104, CRC(3d12afba) SHA1(60245089947e4a4f7bfa94a8cc96d4d8eebe4afc) )
	ROM_LOAD( "pal16r4a.5l",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4a.5m",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r8a.1d",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( crzrallyg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12.7g",       0x0000, 0x4000, CRC(0cab3ef9) SHA1(6de4d4a7159e0a6ad13dbca3344759410618ea26) )
	ROM_LOAD( "13.7f",       0x4000, 0x4000, CRC(e19a8e13) SHA1(1462b21f16990eb9ae2f2d1cd5c097edf88bf614) )
	ROM_LOAD( "14.7d",       0x8000, 0x4000, CRC(4c0351ba) SHA1(0ed04825d3affe0477bb963f1c96ff223e4bcf50) )

	ROM_REGION( 0x4000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "4.5h",        0x0000, 0x2000, CRC(29dece8b) SHA1(d8a0cfd1259d49f59f9751a2db99b46b9da6a87d) )
	ROM_LOAD( "16.5g",       0x2000, 0x2000, CRC(94289f9e) SHA1(8da00814d8f769de124bc09f4c1ee851c99cec0e) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "17.1n",       0x0000, 0x2000, CRC(985ed5c8) SHA1(ee91a6701a8b8bb24d6fa08596deff95816e759e) )
	ROM_LOAD( "18.1l",       0x2000, 0x2000, CRC(c02ddda2) SHA1(262e33cada0e7935d03014583117c2bc6278865b) )
	ROM_LOAD( "19.1k",       0x4000, 0x2000, CRC(2a0d5bca) SHA1(8d7aedd63ea374a5809c24f957b0afa3cad437d0) )
	ROM_LOAD( "20.1i",       0x6000, 0x2000, CRC(49c0c2b8) SHA1(30c4fe1dc2df499927f8fd4a041a707b81a04e1d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n",   0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  // Red component
	ROM_LOAD( "82s129.9m",   0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  // Green component
	ROM_LOAD( "82s129.9l",   0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  // Blue component

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16r6a.5k",  0x0000, 0x0104, CRC(3d12afba) SHA1(60245089947e4a4f7bfa94a8cc96d4d8eebe4afc) )
	ROM_LOAD( "pal16r4a.5l",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4a.5m",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r8a.1d",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

/* Recreativos Franco */
ROM_START( crzrallyrf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crzrallyrf_1.7g", 0x0000, 0x4000, CRC(c49ec48e) SHA1(8ec87ced3c42158be735fa7c81c271e51b114986) )
	ROM_LOAD( "crzrallyrf_2.7f", 0x4000, 0x4000, CRC(8a594a0e) SHA1(8da7099ae7a272dd10bb58b114ca98a58f1df4bb) )
	ROM_LOAD( "crzrallyrf_3.7d", 0x8000, 0x4000, CRC(01ed44dc) SHA1(6078f21f281e3de54f4a2f9869da2728f184bea7) )

	ROM_REGION( 0x4000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "crzrallyrf_4.5h", 0x0000, 0x2000, CRC(68ec2811) SHA1(6a30544d905e373440740877cdbae4a9c4e361cb) )
	ROM_LOAD( "crzrallyrf_5.5g", 0x2000, 0x2000, CRC(81e9b043) SHA1(effc082a025ce36ab6ba8603a82be1469eee6276) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "crzrallyrf_6.1n", 0x0000, 0x2000, CRC(985ed5c8) SHA1(ee91a6701a8b8bb24d6fa08596deff95816e759e) )
	ROM_LOAD( "crzrallyrf_7.1l", 0x2000, 0x2000, CRC(c02ddda2) SHA1(262e33cada0e7935d03014583117c2bc6278865b) )
	ROM_LOAD( "crzrallyrf_8.1k", 0x4000, 0x2000, CRC(2a0d5bca) SHA1(8d7aedd63ea374a5809c24f957b0afa3cad437d0) )
	ROM_LOAD( "crzrallyrf_9.1i", 0x6000, 0x2000, CRC(49c0c2b8) SHA1(30c4fe1dc2df499927f8fd4a041a707b81a04e1d) )

	// Not dumped on the Recreativos Franco PCB, taken from the parent set
	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.9n", 0x0000, 0x0100, CRC(98ff725a) SHA1(553f033212a7c4785c0beb8156400cabcd53cf25) )  // Red component
	ROM_LOAD( "82s129.9m", 0x0100, 0x0100, CRC(d41f5800) SHA1(446046f5694357da876e1307f49584d79c8d9a1a) )  // Green component
	ROM_LOAD( "82s129.9l", 0x0200, 0x0100, CRC(9ed49cb4) SHA1(f54e66e2211d5fb0da9a81e11670367ee4d9b49a) )  // Blue component

	// Not dumped on the Recreativos Franco PCB, taken from the parent set
	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.1f", 0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )

	// Not dumped on the Recreativos Franco PCB, taken from the parent set
	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16r6a.5k", 0x0000, 0x0104, CRC(3d12afba) SHA1(60245089947e4a4f7bfa94a8cc96d4d8eebe4afc) )
	ROM_LOAD( "pal16r4a.5l", 0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4a.5m", 0x0400, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r8a.1d", 0x0600, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

} // anonymous namespace


GAME( 1984, holeland,   0,        holeland, holeland,  holeland_state, empty_init, ROT0,   "Tecfri",                              "Hole Land (Japan)",                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, holeland2,  holeland, holeland, holeland2, holeland_state, empty_init, ROT0,   "Tecfri",                              "Hole Land (Spain)",                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) //attract is different
GAME( 1985, crzrally,   0,        crzrally, crzrally,  crzrally_state, empty_init, ROT270, "Tecfri",                              "Crazy Rally (set 1)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, crzrallya,  crzrally, crzrally, crzrally,  crzrally_state, empty_init, ROT270, "Tecfri",                              "Crazy Rally (set 2)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, crzrallyg,  crzrally, crzrally, crzrally,  crzrally_state, empty_init, ROT270, "Tecfri (Gecas license)",              "Crazy Rally (Gecas license)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, crzrallyrf, crzrally, crzrally, crzrally,  crzrally_state, empty_init, ROT270, "Tecfri (Recreativos Franco license)", "Crazy Rally (Recreativos Franco license)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
