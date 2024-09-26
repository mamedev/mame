// license:BSD-3-Clause
// copyright-holders: Uki

/*****************************************************************************

Quiz DNA no Hanran (c) 1992 Face
Quiz Gakuen Paradise (c) 1991 NMK
Quiz Gekiretsu Scramble (Gakuen Paradise 2) (c) 1993 Face

    Driver by Uki

*****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class quizdna_state : public driver_device
{
public:
	quizdna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_banked_ram(*this, "banked_ram", 0x4000, ENDIANNESS_LITTLE),
		m_fgctrl_rom(*this, "fgctrl"),
		m_mainbank(*this, "mainbank")
	{ }

	void gakupara(machine_config &config);
	void quizdna(machine_config &config);
	void gekiretu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;
	memory_share_creator<uint8_t> m_banked_ram;
	required_region_ptr<uint8_t> m_fgctrl_rom;
	required_memory_bank m_mainbank;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_bg_xscroll[2];
	int8_t m_flipscreen = 0;
	uint8_t m_video_enable = 0;

	// common
	void bg_ram_w(offs_t offset, uint8_t data);
	void fg_ram_w(offs_t offset, uint8_t data);
	void bg_yscroll_w(uint8_t data);
	void bg_xscroll_w(offs_t offset, uint8_t data);
	void screen_ctrl_w(uint8_t data);
	void paletteram_xBGR_RRRR_GGGG_BBBB_w(offs_t offset, uint8_t data);
	void rombank_w(uint8_t data);

	// game specific
	void gekiretu_rombank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gakupara_io_map(address_map &map) ATTR_COLD;
	void gekiretu_io_map(address_map &map) ATTR_COLD;
	void gekiretu_map(address_map &map) ATTR_COLD;
	void quizdna_io_map(address_map &map) ATTR_COLD;
	void quizdna_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(quizdna_state::get_bg_tile_info)
{
	int code = m_banked_ram[0x2000 + tile_index * 2] + m_banked_ram[0x2000 + tile_index * 2 + 1] * 0x100;
	int const col = m_banked_ram[0x2000 + tile_index * 2 + 0x1000] & 0x7f;

	if (code > 0x7fff)
		code &= 0x83ff;

	tileinfo.set(1, code, col, 0);
}

TILE_GET_INFO_MEMBER(quizdna_state::get_fg_tile_info)
{
	int const x = tile_index & 0x1f;
	int y = m_fgctrl_rom[(tile_index >> 5) & 0x1f] & 0x3f;
	int code = y & 1;

	y >>= 1;

	int col = m_banked_ram[x * 2 + y * 0x40 + 1];
	code += (m_banked_ram[x * 2 + y * 0x40] + (col & 0x1f) * 0x100) * 2;
	col >>= 5;
	col = (col & 3) | ((col & 4) << 1);

	tileinfo.set(0, code, col, 0);
}


void quizdna_state::video_start()
{
	m_flipscreen = -1;
	m_video_enable = 0;
	m_bg_xscroll[0] = 0;
	m_bg_xscroll[1] = 0;

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizdna_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizdna_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_bg_xscroll));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_video_enable));
}

void quizdna_state::bg_ram_w(offs_t offset, uint8_t data)
{
	m_banked_ram[offset + 0x2000] = data;

	m_bg_tilemap->mark_tile_dirty((offset & 0xfff) / 2);
}

void quizdna_state::fg_ram_w(offs_t offset, uint8_t data)
{
	int const offs = offset & 0xfff;

	m_banked_ram[offs] = data;
	m_banked_ram[offs + 0x1000] = data; // mirror

	for (int i = 0; i < 32; i++)
		m_fg_tilemap->mark_tile_dirty(((offs/2) & 0x1f) + i * 0x20);
}

void quizdna_state::bg_yscroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrolldy(255 - data, 255 - data + 1);
}

void quizdna_state::bg_xscroll_w(offs_t offset, uint8_t data)
{
	m_bg_xscroll[offset] = data;
	int const x = ~(m_bg_xscroll[0] + m_bg_xscroll[1] * 0x100) & 0x1ff;

	m_bg_tilemap->set_scrolldx(x + 64, x - 64 + 10);
}

void quizdna_state::screen_ctrl_w(uint8_t data)
{
	int const tmp = (data & 0x10) >> 4;
	m_video_enable = data & 0x20;

	machine().bookkeeping().coin_counter_w(0, data & 1);

	if (m_flipscreen == tmp)
		return;

	m_flipscreen = tmp;

	flip_screen_set(tmp);
	m_fg_tilemap->set_scrolldx(64, -64 +16);
}

void quizdna_state::paletteram_xBGR_RRRR_GGGG_BBBB_w(offs_t offset, uint8_t data)
{
	int const offs = offset & ~1;

	m_generic_paletteram_8[offset] = data;

	int const d0 = m_generic_paletteram_8[offs];
	int const d1 = m_generic_paletteram_8[offs+1];

	int const r = ((d1 << 1) & 0x1e) | ((d1 >> 4) & 1);
	int const g = ((d0 >> 3) & 0x1e) | ((d1 >> 5) & 1);
	int const b = ((d0 << 1) & 0x1e) | ((d1 >> 6) & 1);

	m_palette->set_pen_color(offs / 2, pal5bit(r), pal5bit(g), pal5bit(b));
}

void quizdna_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 8)
	{
		int x = m_spriteram[offs + 3] * 0x100 + m_spriteram[offs + 2] + 64 - 8;
		int y = (m_spriteram[offs + 1] & 1) * 0x100 + m_spriteram[offs + 0];
		int code = (m_spriteram[offs + 5] * 0x100 + m_spriteram[offs + 4]) & 0x3fff;
		int col = m_spriteram[offs + 6];
		int const fx = col & 0x80;
		int const fy = col & 0x40;
		int const ysize = (m_spriteram[offs + 1] & 0xc0) >> 6;
		int dy = 0x10;
		col &= 0x1f;

		if (m_flipscreen)
		{
			x -= 7;
			y += 1;
		}

		x &= 0x1ff;
		if (x > 0x1f0)
			x -= 0x200;

		if (fy)
		{
			dy = -0x10;
			y += 0x10 * ysize;
		}

		if (code >= 0x2100)
			code &= 0x20ff;

		for (int i = 0; i < ysize + 1; i++)
		{
			y &= 0x1ff;

			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code ^ i,
					col,
					fx, fy,
					x, y, 0);

			y += dy;
		}
	}
}

uint32_t quizdna_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_enable)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}


void quizdna_state::rombank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x3f);
}

void quizdna_state::gekiretu_rombank_w(uint8_t data)
{
	m_mainbank->set_entry((data & 0x3f) ^ 0x0a);
}

/****************************************************************************/

void quizdna_state::quizdna_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0x8000, 0x9fff).w(FUNC(quizdna_state::fg_ram_w));
	map(0xa000, 0xbfff).w(FUNC(quizdna_state::bg_ram_w));
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe1ff).ram().share(m_spriteram);
	map(0xe200, 0xefff).ram();
	map(0xf000, 0xffff).ram().w(FUNC(quizdna_state::paletteram_xBGR_RRRR_GGGG_BBBB_w)).share("paletteram");
}

void quizdna_state::gekiretu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0x8000, 0x9fff).w(FUNC(quizdna_state::fg_ram_w));
	map(0xa000, 0xbfff).w(FUNC(quizdna_state::bg_ram_w));
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().w(FUNC(quizdna_state::paletteram_xBGR_RRRR_GGGG_BBBB_w)).share("paletteram");
	map(0xf000, 0xf1ff).ram().share(m_spriteram);
	map(0xf200, 0xffff).ram();
}

void quizdna_state::quizdna_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).w(FUNC(quizdna_state::bg_xscroll_w));
	map(0x04, 0x04).w(FUNC(quizdna_state::bg_yscroll_w));
	map(0x05, 0x06).nopw(); // unknown
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x90, 0x90).portr("SYSTEM");
	map(0x91, 0x91).portr("SERVICE");
	map(0xc0, 0xc0).w(FUNC(quizdna_state::rombank_w));
	map(0xd0, 0xd0).w(FUNC(quizdna_state::screen_ctrl_w));
	map(0xe0, 0xe1).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf0, 0xf0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void quizdna_state::gakupara_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(FUNC(quizdna_state::bg_xscroll_w));
	map(0x02, 0x02).w(FUNC(quizdna_state::bg_yscroll_w));
	map(0x03, 0x04).nopw(); // unknown
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x90, 0x90).portr("SYSTEM");
	map(0x91, 0x91).portr("SERVICE");
	map(0xc0, 0xc0).w(FUNC(quizdna_state::rombank_w));
	map(0xd0, 0xd0).w(FUNC(quizdna_state::screen_ctrl_w));
	map(0xe0, 0xe1).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf0, 0xf0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void quizdna_state::gekiretu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).w(FUNC(quizdna_state::bg_xscroll_w));
	map(0x04, 0x04).w(FUNC(quizdna_state::bg_yscroll_w));
	map(0x05, 0x06).nopw(); // unknown
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x90, 0x90).portr("SYSTEM");
	map(0x91, 0x91).portr("SERVICE");
	map(0xc0, 0xc0).w(FUNC(quizdna_state::gekiretu_rombank_w));
	map(0xd0, 0xd0).w(FUNC(quizdna_state::screen_ctrl_w));
	map(0xe0, 0xe1).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf0, 0xf0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


/****************************************************************************/

static INPUT_PORTS_START( quizdna ) // DIP definitions and defaults verified with manual
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW2:1,2,3,4")
	PORT_DIPSETTING(    0x01, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:6") // confirmed 'Unused' by the manual
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSW2:8")

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Timer" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, "Every 500k" )
	PORT_DIPSETTING(    0x20, "Every 1000k" )
	PORT_DIPSETTING(    0x10, "Every 2000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, "Carat" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gakupara ) // DIP definitions and defaults verified with manual
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x00, "Demo Music" ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, "Timer" ) PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "Slow" )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )

	PORT_START("DSW3")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:2") // 0xfe are "prohibited from use" according to manual
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gekiretu )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Timer" )
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout fglayout =
{
	16,8,     // 16*8 characters
	8192*2,   // 16384 characters
	1,        // 1 bit per pixel
	{0},
	{ STEP16(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout bglayout =
{
	8,8,        // 8*8 characters
	32768+1024, // 32768+1024 characters
	4,          // 4 bits per pixel
	{0,1,2,3},
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	8*8*4
};

static const gfx_layout objlayout =
{
	16,16,    // 16*16 characters
	8192+256, // 8192+256 characters
	4,        // 4 bits per pixel
	{0,1,2,3},
	{ STEP16(0,4) },
	{ STEP16(0,64) },
	16*16*4
};

static GFXDECODE_START( gfx_quizdna )
	GFXDECODE_ENTRY( "fgtiles", 0x0000, fglayout,  0x7e0,  16 )
	GFXDECODE_ENTRY( "bgtiles", 0x0000, bglayout,  0x000, 128 )
	GFXDECODE_ENTRY( "sprites", 0x0000, objlayout, 0x600,  32 )
GFXDECODE_END

void quizdna_state::machine_start()
{
	m_mainbank->configure_entry(0, m_banked_ram);
	m_mainbank->configure_entries(1, 63, memregion("maincpu")->base() + 0x14000, 0x4000);
}


void quizdna_state::quizdna(machine_config &config)
{
	static constexpr XTAL MCLK = 16_MHz_XTAL;

	// basic machine hardware
	Z80(config, m_maincpu, MCLK / 2); // 8.000 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &quizdna_state::quizdna_map);
	m_maincpu->set_addrmap(AS_IO, &quizdna_state::quizdna_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(quizdna_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 56*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(quizdna_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_quizdna);
	PALETTE(config, m_palette).set_entries(2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", MCLK / 4));
	ymsnd.port_a_read_callback().set_ioport("DSW3");
	ymsnd.port_b_read_callback().set_ioport("DSW2");
	ymsnd.add_route(0, "mono", 0.10);
	ymsnd.add_route(1, "mono", 0.10);
	ymsnd.add_route(2, "mono", 0.10);
	ymsnd.add_route(3, "mono", 0.40);

	OKIM6295(config, "oki", (MCLK / 1024) * 132, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30); // clock frequency & pin 7 not verified
}

void quizdna_state::gakupara(machine_config &config)
{
	quizdna(config);

	// basic machine hardware

	m_maincpu->set_addrmap(AS_IO, &quizdna_state::gakupara_io_map);
}

void quizdna_state::gekiretu(machine_config &config)
{
	quizdna(config);

	// basic machine hardware

	m_maincpu->set_addrmap(AS_PROGRAM, &quizdna_state::gekiretu_map);
	m_maincpu->set_addrmap(AS_IO, &quizdna_state::gekiretu_io_map);
}


/****************************************************************************/

ROM_START( quizdna )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "quiz2-pr.28",  0x00000,  0x08000, CRC(a428ede4) SHA1(cdca3bd84b2ea421fb05502ea29e9eb605e574eb) )
	ROM_CONTINUE(             0x18000,  0x78000 ) // banked
	// empty

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "quiz2.102",    0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	// empty

	ROM_REGION( 0x108000, "bgtiles", 0 )
	ROM_LOAD( "quiz2-bg.100", 0x000000,  0x100000, CRC(f1d0cac2) SHA1(26d25c1157d1916dfe4496c6cf119c4a9339e31c) )
	// empty

	ROM_REGION( 0x108000, "sprites", 0 )
	ROM_LOAD( "quiz2-ob.98",  0x000000,  0x100000, CRC(682f19a6) SHA1(6b8e6e583f423cf8ef9095f2c300201db7d7b8b3) )
	ROM_LOAD( "quiz2ob2.97",  0x100000,  0x008000, CRC(03736b1a) SHA1(bc42ac293260f58a8a138702d890f69aec99c05e) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "quiz2-sn.32",  0x000000,  0x040000, CRC(1c044637) SHA1(dc749295e149f968495272f1a3ec27c6b719be8e) )

	ROM_REGION( 0x00020, "fgctrl", 0 )
	ROM_LOAD( "quiz2.148",    0x000000,  0x000020, CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
ROM_END

ROM_START( gakupara )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "u28.bin",  0x00000,  0x08000, CRC(72124bb8) SHA1(e734acff7e9d6b8c6a95c76860732320a2e3a828) )
	ROM_CONTINUE(         0x18000,  0x78000 )             // banked
	ROM_LOAD( "u29.bin",  0x90000,  0x40000, CRC(09f4948e) SHA1(21ccf5af6935cf40c0cf73fbee14bff3c4e1d23d) ) // banked

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "u102.bin", 0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	ROM_LOAD( "u103.bin", 0x20000,  0x20000, CRC(38644251) SHA1(ebfdc43c38e1380709ed08575c346b2467ad1592) )

	ROM_REGION( 0x108000, "bgtiles", 0 )
	ROM_LOAD( "u100.bin", 0x000000,  0x100000, CRC(f9d886ea) SHA1(d7763f54a165af720216b96e601a66fbc59e3568) )
	ROM_LOAD( "u99.bin",  0x100000,  0x008000, CRC(ac224d0a) SHA1(f187c3b74bc18606d0fe638f6a829f71c109998d) )

	ROM_REGION( 0x108000, "sprites", 0 )
	ROM_LOAD( "u98.bin",  0x000000,  0x100000, CRC(a6e8cb56) SHA1(2fc85c1769513cc7aa5e23afaf0c99c38de9b855) )
	ROM_LOAD( "u97.bin",  0x100000,  0x008000, CRC(9dacd5c9) SHA1(e40211059e71408be3d67807463304f4d4ecae37) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "u32.bin",  0x000000,  0x040000, CRC(eb03c535) SHA1(4d6c749ccab4681eee0a1fb243e9f3dbe61b9f94) )

	ROM_REGION( 0x00020, "fgctrl", 0 )
	ROM_LOAD( "u148.bin", 0x000000,  0x000020, CRC(971df9d2) SHA1(280f5b386922b9902ca9211c719642c2bd0ba899) )
ROM_END

ROM_START( gakupara102 ) // ARC-0004-1 PCB
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "u28.bin",  0x00000,  0x08000, CRC(9256c18a) SHA1(6704fa48c468621af76ce91b38addeee0d654b56) ) // SLDH
	ROM_CONTINUE(         0x18000,  0x78000 )             // banked
	ROM_LOAD( "u29.bin",  0x90000,  0x40000, CRC(09f4948e) SHA1(21ccf5af6935cf40c0cf73fbee14bff3c4e1d23d) ) // banked

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "u102.bin", 0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	ROM_LOAD( "u103.bin", 0x20000,  0x20000, CRC(38644251) SHA1(ebfdc43c38e1380709ed08575c346b2467ad1592) )

	ROM_REGION( 0x108000, "bgtiles", 0 )
	ROM_LOAD( "u100.bin", 0x000000,  0x100000, CRC(f9d886ea) SHA1(d7763f54a165af720216b96e601a66fbc59e3568) )
	ROM_LOAD( "u99.bin",  0x100000,  0x008000, CRC(ac224d0a) SHA1(f187c3b74bc18606d0fe638f6a829f71c109998d) )

	ROM_REGION( 0x108000, "sprites", 0 )
	ROM_LOAD( "u98.bin",  0x000000,  0x100000, CRC(a6e8cb56) SHA1(2fc85c1769513cc7aa5e23afaf0c99c38de9b855) )
	ROM_LOAD( "u97.bin",  0x100000,  0x008000, CRC(9dacd5c9) SHA1(e40211059e71408be3d67807463304f4d4ecae37) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "u32.bin",  0x000000,  0x040000, CRC(eb03c535) SHA1(4d6c749ccab4681eee0a1fb243e9f3dbe61b9f94) )

	ROM_REGION( 0x00020, "fgctrl", 0 )
	ROM_LOAD( "u148.bin", 0x000000,  0x000020, CRC(971df9d2) SHA1(280f5b386922b9902ca9211c719642c2bd0ba899) )
ROM_END

ROM_START( gekiretu )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "quiz3-pr.28",  0x00000,  0x08000, CRC(a761e86f) SHA1(85331ef53598491e78c2d123b1ebd358aff46436) )
	ROM_CONTINUE(             0x18000,  0x78000 ) // banked
	// empty

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "quiz3.102",    0x00000,  0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	// empty

	ROM_REGION( 0x108000, "bgtiles", 0 )
	ROM_LOAD( "quiz3-bg.100", 0x000000,  0x100000, CRC(cb9272fd) SHA1(cfc1ff93d1fdc7d144e161a77e534cea75d7f181) )
	// empty

	ROM_REGION( 0x108000, "sprites", 0 )
	ROM_LOAD( "quiz3-ob.98",  0x000000,  0x100000, CRC(01bed020) SHA1(5cc56c8823ee5e538371debe1cbeb57c4976677b) )
	// empty

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "quiz3-sn.32",  0x000000,  0x040000, CRC(36dca582) SHA1(2607602e942244cfaae931da2ad36da9a8f855f7) )

	ROM_REGION( 0x00020, "fgctrl", 0 )
	ROM_LOAD( "quiz3.148",    0x000000,  0x000020, CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
ROM_END

ROM_START( gekiretup ) // ARC-0005-1 PCB, hand-written labels
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "quiz3-pr.28",  0x00000,  0x08000, CRC(a761e86f) SHA1(85331ef53598491e78c2d123b1ebd358aff46436) ) // same as final
	ROM_CONTINUE(             0x18000,  0x78000 ) // banked
	// empty

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "quiz2_kanji.u102",  0x00000,  0x20000, BAD_DUMP CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) ) // using the one from final for now, redump pending
	// empty

	ROM_REGION( 0x108000, "bgtiles", 0 ) // almost identical to final (some changes in the second ROM - i.e. missing copyright and Gakuen Paradise 2 subtitle in title screen)
	ROM_LOAD( "bg0mask.u100", 0x000000,  0x80000, CRC(2344be20) SHA1(f6656ec19c0cb8fecf21873ecfb0e1ea1c9ea570) )
	ROM_LOAD( "bghi.u99",     0x080000,  0x80000, CRC(0353ce23) SHA1(71ea78458b7c70805f32dd7a6329b1b302cb5a9e) )
	// empty

	ROM_REGION( 0x108000, "sprites", 0 ) // same as final, just split
	ROM_LOAD( "obj0_mask.u98",  0x000000,  0x080000, CRC(a367c004) SHA1(a092dbc3186a5ca2f88377f4851702f2af7e71ab) )
	ROM_LOAD( "obj1_mask.u97",  0x080000,  0x080000, CRC(c4444ae0) SHA1(da113019920dbfc23c55ececc4ca3667f978224e) )
	// empty

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "quiz3-sn.32",  0x000000,  0x040000, CRC(36dca582) SHA1(2607602e942244cfaae931da2ad36da9a8f855f7) ) // same as final

	ROM_REGION( 0x00020, "fgctrl", 0 )
	ROM_LOAD( "quiz3.148",    0x000000,  0x000020, CRC(91267e8a) SHA1(ae5bd8efea5322c4d9986d06680a781392f9a642) )
ROM_END

} // anonymous namespace


GAME( 1991, gakupara,    0,        gakupara, gakupara, quizdna_state, empty_init, ROT0, "NMK",  "Quiz Gakuen Paradise (Japan, ver. 1.04)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, gakupara102, gakupara, gakupara, gakupara, quizdna_state, empty_init, ROT0, "NMK",  "Quiz Gakuen Paradise (Japan, ver. 1.02)",    MACHINE_SUPPORTS_SAVE )
GAME( 1992, quizdna,     0,        quizdna,  quizdna,  quizdna_state, empty_init, ROT0, "Face", "Quiz DNA no Hanran (Japan)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1992, gekiretu,    0,        gekiretu, gekiretu, quizdna_state, empty_init, ROT0, "Face", "Quiz Gekiretsu Scramble (Japan)",            MACHINE_SUPPORTS_SAVE )
GAME( 1992, gekiretup,   gekiretu, gekiretu, gekiretu, quizdna_state, empty_init, ROT0, "Face", "Quiz Gekiretsu Scramble (Japan, prototype)", MACHINE_SUPPORTS_SAVE )
