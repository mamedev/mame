// license:BSD-3-Clause
// copyright-holders:

/*
Banpresto medal games with Seibu customs

Confirmed games (but there are probably several more):
Mario Undoukai
Terebi Denwa Doraemon

The following is from Mario Undoukai PCB pics:

BS9001-2 main PCB + BS9001-A ROM PCB
Main components:
- MC68000P10 main CPU
- 16.000 MHz XTAL (near M68000)
- 8-dip bank
- SEI0160 Seibu custom
- SEI0181 Seibu custom
- SEI0200 Seibu custom
- SEI0211 Seibu custom
- SEI0220BP Seibu custom
- 14.31818 MHz XTAL (near Seibu video customs)
- OKI M6295
The audio section also has unpopulated spaces marked for a Z80, a YM2203 and a SEI01008


TODO:
- printer or hopper emulation
- the GFX emulation was adapted from other drivers using the Seibu customs, it might need more adjustments
- verify Oki banking (needs someone who understands Japanese to check if speech makes sense when it gets called)
- lamps
- controls / dips need to be completed and better arranged
- currently stuck at the call assistant screen due to vendor test failing (printer / hopper?), but can enter test mode.
  To test the games, it's possible to get the attract mode running by enabling the hack in the memory map at 0xe0004-0xe0005.
  tvdenwad will go back to the call assistant screen shortly after starting (believed to be because of missing printer or hopper emulation)
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

#include "video/seibu_crtc.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class banprestoms_state : public driver_device
{
public:
	banprestoms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram%u", 0U),
		m_spriteram(*this, "sprite_ram"),
		m_okibank(*this, "okibank")
	{ }

	void banprestoms(machine_config &config);

	void init_oki();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint16_t, 4> m_vram;
	required_shared_ptr<uint16_t> m_spriteram;

	required_memory_bank m_okibank;

	tilemap_t *m_tilemap[4];

	uint16_t m_layer_en;
	uint16_t m_scrollram[6];

	void okibank_w(uint16_t data);

	template <uint8_t Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void layer_en_w(uint16_t data);
	void layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(tile_info);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map);
	void oki_map(address_map &map);
};


void banprestoms_state::machine_start()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x40000);
	m_okibank->set_entry(0);
}


void banprestoms_state::okibank_w(uint16_t data)
{
	m_okibank->set_entry(data & 0x03);
	// TODO: what do the other bits do?
}


void banprestoms_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(15);
	m_tilemap[2]->set_transparent_pen(15);
	m_tilemap[3]->set_transparent_pen(15);

	save_item(NAME(m_layer_en));
	save_item(NAME(m_scrollram));
}


template <uint8_t Which>
void banprestoms_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(banprestoms_state::tile_info)
{
	int tile = m_vram[Which][tile_index] & 0xfff;
	int color = (m_vram[Which][tile_index] >> 12) & 0x0f;
	tileinfo.set(Which + 1, tile, color, 0);
}

void banprestoms_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	for (int offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		if ((m_spriteram[offs] & 0x8000) != 0x8000) continue;
		int sprite = m_spriteram[offs + 1];
		if ((sprite >> 14) != pri) continue;
		sprite &= 0x1fff;

		int y = m_spriteram[offs + 3];
		int x = m_spriteram[offs + 2];

		int color = m_spriteram[offs] & 0x3f;
		int fx = m_spriteram[offs] & 0x4000;
		int fy = m_spriteram[offs] & 0x2000;
		int dy = ((m_spriteram[offs] & 0x0380) >> 7) + 1;
		int dx = ((m_spriteram[offs] & 0x1c00) >> 10) + 1;

		// co-ordinates don't have a sign bit, there must be wrapping logic
		// co-ordinate masking might need to be applied during drawing instead
		x &= 0x1ff;
		if (x >= 0x180)
			x -= 0x200;

		y &= 0x1ff;
		if (y >= 0x180)
			y -= 0x200;

		for (int ax = 0; ax < dx; ax++)
		{
			for (int ay = 0; ay < dy; ay++)
			{
				if (!fy)
				{
					if (!fx)
						m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite++, color, fx, fy, x + ax * 16, y + ay * 16, 15);
					else
						m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite++, color, fx, fy, x + (dx - 1 - ax) * 16, y + ay * 16, 15);
				}
				else
				{
					if (!fx)
						m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite++, color, fx, fy, x + ax * 16, y + (dy - 1 - ay) * 16, 15);
					else
						m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite++, color, fx, fy, x + (dx - 1 - ax) * 16, y + (dy - 1 - ay) * 16, 15);
				}
			}
		}
	}
}

uint32_t banprestoms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0x7ff), cliprect); //black pen

	m_tilemap[0]->set_scrollx(0, m_scrollram[0]);
	m_tilemap[0]->set_scrolly(0, m_scrollram[1] - 16);
	m_tilemap[1]->set_scrollx(0, m_scrollram[2]);
	m_tilemap[1]->set_scrolly(0, m_scrollram[3] - 16);
	m_tilemap[2]->set_scrollx(0, m_scrollram[4]);
	m_tilemap[2]->set_scrolly(0, m_scrollram[5] - 16);
	m_tilemap[3]->set_scrollx(0, 128);
	m_tilemap[3]->set_scrolly(0, -16);

	if (!(m_layer_en & 0x0001)) { m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0); }
	if (!(m_layer_en & 0x0010)) { draw_sprites(bitmap, cliprect, 2); }
	if (!(m_layer_en & 0x0002)) { m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0); }
	if (!(m_layer_en & 0x0010)) { draw_sprites(bitmap, cliprect, 1); }
	if (!(m_layer_en & 0x0004)) { m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0); }
	if (!(m_layer_en & 0x0010)) { draw_sprites(bitmap, cliprect, 0); }
	if (!(m_layer_en & 0x0008)) { m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0); }
	if (!(m_layer_en & 0x0010)) { draw_sprites(bitmap, cliprect, 3); }

	return 0;
}

void banprestoms_state::layer_en_w(uint16_t data)
{
	m_layer_en = data;
}

void banprestoms_state::layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrollram[offset]);
}


void banprestoms_state::prg_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("maincpu", 0);
	map(0x80000, 0x807ff).ram().share("nvram");
	map(0x80800, 0x80fff).ram().w(FUNC(banprestoms_state::vram_w<0>)).share(m_vram[0]);
	map(0x81000, 0x817ff).ram().w(FUNC(banprestoms_state::vram_w<1>)).share(m_vram[1]);
	map(0x81800, 0x81fff).ram().w(FUNC(banprestoms_state::vram_w<2>)).share(m_vram[2]);
	map(0x82000, 0x82fff).ram().w(FUNC(banprestoms_state::vram_w<3>)).share(m_vram[3]);
	map(0x83000, 0x837ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x83800, 0x83fff).ram().share(m_spriteram);
	map(0xa0001, 0xa0001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc0000, 0xc004f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0xc0080, 0xc0081).nopw(); // CRTC related ?
	map(0xc00c0, 0xc00c1).nopw(); // CRTC related ?
	map(0xc0100, 0xc0101).w(FUNC(banprestoms_state::okibank_w));
	//map(0xc0140, 0xc0141).nopw(); // in marioun bit 3 is lamp according to test mode
	map(0xe0000, 0xe0001).portr("DSW1");
	map(0xe0002, 0xe0003).portr("IN1");
	map(0xe0004, 0xe0005).portr("IN2"); //.lr16(NAME([this] () -> uint16_t { return (machine().rand() & 0x0004) | (ioport("IN2")->read() & 0xfffb); }));
}

void banprestoms_state::oki_map(address_map &map)
{
	map(0x00000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( tvdenwad )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) // 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) // 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) // 4
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // 5
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) // 6
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) // 7
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) // 8
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON9 ) // 9
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON10 ) // 0, why active high?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON11 ) // #
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON12 ) // *
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED ) // couldn't find anything from here on
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON13 ) // Card Emp in switch test
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON14 ) // Card Pos in switch test
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON15 ) // Card Pay in switch test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED ) // ?
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) // ?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON16 ) // Hook in switch test
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED ) // couldn't find anything from here on
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START("DSW1") // marked SW0913 on PCB
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( marioun ) // inputs defined as IPT_UNKNOWN don't show any effect in switch test in test mode
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // upper space on the feet platform
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) // right space on the feet platform
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) // lower space on the feet platform
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) // left space on the feet platform
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // left central space on the feet platform
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) // right central space on the feet platform
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW ) // also used to navigate in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) // Card Emp
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) // Card Pos
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON9 ) // Card Pay
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("DSW1") // marked SW0913 on PCB
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
				3+32*16, 2+32*16, 1+32*16, 0+32*16, 16+3+32*16, 16+2+32*16, 16+1+32*16, 16+0+32*16 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16,
			16*16, 18*16, 20*16, 22*16, 24*16, 26*16, 28*16, 30*16 },
	128*8
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16 },
	128*8
};

static GFXDECODE_START( gfx_banprestoms )
	GFXDECODE_ENTRY( "spr_gfx",0, tilelayout, 0x000, 0x40 )
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x100, 0x10 ) // TODO, doesn't seem to be used by the dumped games
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x100, 0x10 ) // TODO, doesn't seem to be used by the dumped games
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x100, 0x10 )
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x200, 0x10 )
GFXDECODE_END


void banprestoms_state::banprestoms(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &banprestoms_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(banprestoms_state::irq4_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: copied from other drivers using the same CRTC
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 320-1, 16, 240-1);
	screen.set_screen_update(FUNC(banprestoms_state::screen_update));
	screen.set_palette(m_palette);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(banprestoms_state::layer_en_w));
	crtc.layer_scroll_callback().set(FUNC(banprestoms_state::layer_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_banprestoms);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400); // TODO: copied from other drivers using the same CRTC

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH)); // TODO: check frequency and pin 7.
	oki.set_addrmap(0, &banprestoms_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);
}


ROM_START( tvdenwad )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s82_a02.u15",  0x00000, 0x20000, CRC(479f790f) SHA1(a544c6493feb74ec8727f6b7f491ce6e5d24e316) )
	ROM_LOAD16_BYTE( "s82_a01.u14",  0x00001, 0x20000, CRC(ac7cccdb) SHA1(5c0877e1831663113aa3284e323bf7665b5baa71) )

	ROM_REGION( 0x80000, "spr_gfx", 0 )
	ROM_LOAD( "s82_a05.u119", 0x00000, 0x80000, CRC(980be413) SHA1(d35cb6bb2299fc34226c59c3c97f8789dd1f71ce) )

	ROM_REGION( 0x80000, "gfx_tiles", 0 )
	ROM_LOAD( "s82_a04.u18", 0x00000, 0x80000, CRC(55f31697) SHA1(58011800b2e2c6cac55a3881a9970bbd325d74ad) ) // TODO: are all of the following used? in attract it seems only fg_gfx and tx_gfx are used

	ROM_REGION( 0x80000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s82_a03.u17", 0x000000, 0x100000, CRC(80014273) SHA1(9155248e9b7f8b7a9962b29b9063f9fe5ba471de) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "sc001.u110", 0x000, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc002.u235", 0x200, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc003.u36",  0x400, 0x155, NO_DUMP ) // 18cv8pc-25
	ROM_LOAD( "sc004c.u68", 0x600, 0x117, NO_DUMP ) // gal16v8a
	ROM_LOAD( "sc006.u248", 0x800, 0x117, NO_DUMP ) // gal16v8a
ROM_END

ROM_START( marioun )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s98_b02.u15",  0x00000, 0x20000, CRC(d88eecfc) SHA1(5a15a1f925ae10e439e5aee8f3ef5a2fa956b80f) )
	ROM_LOAD16_BYTE( "s98_b01.u14",  0x00001, 0x20000, CRC(60f5e7b0) SHA1(ccbdc42d0d59ec4f96ced682afd879c9fa9f24cc) )

	ROM_REGION( 0x80000, "spr_gfx", 0 )
	ROM_LOAD( "s98_a05.u119", 0x00000, 0x80000, CRC(b8317dd8) SHA1(37f0be38607e40d7925faf9731b95577cbd56bb0) )

	ROM_REGION( 0x80000, "gfx_tiles", 0 )
	ROM_LOAD( "s98_a04.u18", 0x00000, 0x80000, CRC(b107c5a0) SHA1(d4e7ef71bfb9a10e72b6292405d0378c95ebba25) ) // TODO: are all of the following used? in attract it seems only fg_gfx and tx_gfx are used

	ROM_REGION( 0x80000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s98_a03.u17", 0x000000, 0x100000, CRC(d6b89223) SHA1(7d7ab34decb994caac82178455f014628cf070b8) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "sc001.u110", 0x000, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc002.u235", 0x200, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc003.u36",  0x400, 0x155, NO_DUMP ) // 18cv8pc-25
	ROM_LOAD( "sc004c.u68", 0x600, 0x117, NO_DUMP ) // gal16v8a
	ROM_LOAD( "sc006.u248", 0x800, 0x117, NO_DUMP ) // gal16v8a
ROM_END

void banprestoms_state::init_oki() // The Oki mask ROM is in an unusual format, load it so that MAME can make use of it
{
	uint8_t *okirom = memregion("oki")->base();
	std::vector<uint8_t> buffer(0x100000);
	memcpy(&buffer[0], okirom, 0x100000);

	for (int i = 0; i < 0x80000; i += 4)
		okirom[i / 2] = buffer[i];

	for (int i = 1; i < 0x80000; i += 4)
		okirom[(i - 1) / 2 + 0x40000] = buffer[i];

	for (int i = 2; i < 0x80000; i += 4)
		okirom[i / 2] = buffer[i];

	for (int i = 3; i < 0x80000; i += 4)
		okirom[(i - 1) / 2 + 0x40000] = buffer[i];

	for (int i = 0x80000; i < 0x100000; i += 4)
		okirom[i / 2 + 0x40000] = buffer[i];

	for (int i = 0x80001; i < 0x100000; i += 4)
		okirom[(i - 1) / 2 + 0x80000] = buffer[i];

	for (int i = 0x80002; i < 0x100000; i += 4)
		okirom[i / 2 + 0x40000] = buffer[i];

	for (int i = 0x80003; i < 0x100000; i += 4)
		okirom[(i - 1) / 2 + 0x80000] = buffer[i];
}

} // Anonymous namespace


GAME( 1991, tvdenwad, 0, banprestoms, tvdenwad, banprestoms_state, init_oki, ROT0, "Banpresto", "Terebi Denwa Doraemon", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, marioun,  0, banprestoms, marioun,  banprestoms_state, init_oki, ROT0, "Banpresto", "Super Mario World - Mario Undoukai",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
