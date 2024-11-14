// license:BSD-3-Clause
// copyright-holders: Richard Davies

/***************************************************************************

    Exed Exes
    Capcom 84110-A-1 + 84110-B-1 PCBs

    Notes:
    - Flip screen is not supported, but doesn't seem to be used (no flip screen
      dip switch and no cocktail mode)
    - Some writes to unknown memory locations (always 0?)

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_nbg_yscroll(*this, "nbg_yscroll"),
		m_nbg_xscroll(*this, "nbg_xscroll"),
		m_bg_scroll(*this, "bg_scroll"),
		m_tilerom(*this, "tilerom")
	{ }

	void exedexes(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	// memory pointers
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_shared_ptr<u8> m_nbg_yscroll;
	required_shared_ptr<u8> m_nbg_xscroll;
	required_shared_ptr<u8> m_bg_scroll;
	required_region_ptr<u8> m_tilerom;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	uint8_t m_chon = 0;
	uint8_t m_objon = 0;
	uint8_t m_sc1on = 0;
	uint8_t m_sc2on = 0;

	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void c804_w(u8 data);
	void gfxctrl_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILEMAP_MAPPER_MEMBER(bg_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	void palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Exed Exes has three 256x4 palette PROMs (one per gun), three 256x4 lookup
  table PROMs (one for characters, one for sprites, one for background tiles)
  and one 256x4 sprite palette bank selector PROM.

  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void exedexes_state::palette(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		const int r = pal4bit(color_prom[i + 0x000]);
		const int g = pal4bit(color_prom[i + 0x100]);
		const int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0xc0-0xcf
	for (int i = 0; i < 0x100; i++)
	{
		const u8 ctabentry = color_prom[i] | 0xc0;
		palette.set_pen_indirect(i, ctabentry);
	}

	// 32x32 tiles use colors 0-0x0f
	for (int i = 0x100; i < 0x200; i++)
	{
		const u8 ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}

	// 16x16 tiles use colors 0x40-0x4f
	for (int i = 0x200; i < 0x300; i++)
	{
		const u8 ctabentry = color_prom[i] | 0x40;
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites use colors 0x80-0xbf in four banks
	for (int i = 0x300; i < 0x400; i++)
	{
		const u8 ctabentry = color_prom[i] | (color_prom[i + 0x100] << 4) | 0x80;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void exedexes_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void exedexes_state::colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void exedexes_state::c804_w(u8 data)
{
	// bits 0 and 1 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	machine().bookkeeping().coin_lockout_w(0, data & 0x04);
	machine().bookkeeping().coin_lockout_w(1, data & 0x08);

	// bit 7 is text enable
	m_chon = data & 0x80;

	// other bits seem to be unused
}

void exedexes_state::gfxctrl_w(u8 data)
{
	// bit 4 is bg enable
	m_sc2on = data & 0x10;

	// bit 5 is fg enable
	m_sc1on = data & 0x20;

	// bit 6 is sprite enable
	m_objon = data & 0x40;

	// other bits seem to be unused
}


TILE_GET_INFO_MEMBER(exedexes_state::get_bg_tile_info)
{
	const u8 attr = m_tilerom[tile_index];
	const u8 code = attr & 0x3f;
	const u8 color = m_tilerom[tile_index + (8 * 8)];
	const int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(exedexes_state::get_fg_tile_info)
{
	const u8 code = m_tilerom[tile_index];

	tileinfo.set(2, code, 0, 0);
}

TILE_GET_INFO_MEMBER(exedexes_state::get_tx_tile_info)
{
	const u32 code = m_videoram[tile_index] + 2 * (m_colorram[tile_index] & 0x80);
	const u8 color = m_colorram[tile_index] & 0x3f;

	tileinfo.group = color;

	tileinfo.set(0, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(exedexes_state::bg_tilemap_scan)
{
	// logical (col,row) -> memory offset
	return ((col * 32 & 0xe0) >> 5) + ((row * 32 & 0xe0) >> 2) + ((col * 32 & 0x3f00) >> 1) + 0x4000;
}

TILEMAP_MAPPER_MEMBER(exedexes_state::fg_tilemap_scan)
{
	// logical (col,row) -> memory offset
	return ((col * 16 & 0xf0) >> 4) + (row * 16 & 0xf0) + (col * 16 & 0x700) + ((row * 16 & 0x700) << 3);
}

void exedexes_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exedexes_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(exedexes_state::bg_tilemap_scan)), 32, 32, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exedexes_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(exedexes_state::fg_tilemap_scan)), 16, 16, 128, 128);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exedexes_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0xcf);
}

void exedexes_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 *buffered_spriteram = m_spriteram->buffer();

	if (!m_objon)
		return;

	for (int offs = 0; offs < m_spriteram->bytes(); offs += 32)
	{
		u32 primask = 0;
		if (buffered_spriteram[offs + 1] & 0x40)
			primask |= GFX_PMASK_2;

		const u32 code = buffered_spriteram[offs];
		const u32 color = buffered_spriteram[offs + 1] & 0x0f;
		const bool flipx = buffered_spriteram[offs + 1] & 0x10;
		const bool flipy = buffered_spriteram[offs + 1] & 0x20;
		const int sx = buffered_spriteram[offs + 3] - ((buffered_spriteram[offs + 1] & 0x80) << 1);
		const int sy = buffered_spriteram[offs + 2];

		m_gfxdecode->gfx(3)->prio_transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, screen.priority(), primask, 0);
	}
}

u32 exedexes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	if (m_sc2on)
	{
		m_bg_tilemap->set_scrollx(0, ((m_bg_scroll[1]) << 8) + m_bg_scroll[0]);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	}
	else
		bitmap.fill(0, cliprect);

	if (m_sc1on)
	{
		m_fg_tilemap->set_scrollx(0, ((m_nbg_yscroll[1]) << 8) + m_nbg_yscroll[0]);
		m_fg_tilemap->set_scrolly(0, ((m_nbg_xscroll[1]) << 8) + m_nbg_xscroll[0]);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	}

	draw_sprites(screen, bitmap, cliprect);

	if (m_chon)
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(exedexes_state::scanline)
{
	const int scanline = param;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   // Z80 - RST 10h - vblank

	if (scanline == 0) // unknown irq event
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   // Z80 - RST 08h
}


void exedexes_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW0");
	map(0xc004, 0xc004).portr("DSW1");
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc804, 0xc804).w(FUNC(exedexes_state::c804_w));                       // coin counters + text layer enable
	map(0xc806, 0xc806).nopw();                                                // watchdog ??
	map(0xd000, 0xd3ff).ram().w(FUNC(exedexes_state::videoram_w)).share(m_videoram);
	map(0xd400, 0xd7ff).ram().w(FUNC(exedexes_state::colorram_w)).share(m_colorram);
	map(0xd800, 0xd801).writeonly().share(m_nbg_yscroll);
	map(0xd802, 0xd803).writeonly().share(m_nbg_xscroll);
	map(0xd804, 0xd805).writeonly().share(m_bg_scroll);
	map(0xd807, 0xd807).w(FUNC(exedexes_state::gfxctrl_w));                    // layer enables
	map(0xe000, 0xefff).ram();                                                 // work RAM
	map(0xf000, 0xffff).ram().share("spriteram");
}


void exedexes_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8001).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x8002, 0x8002).w("sn1", FUNC(sn76489_device::write));
	map(0x8003, 0x8003).w("sn2", FUNC(sn76489_device::write));
}


static INPUT_PORTS_START( exedexes )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "2 Players Game" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x10, "2 Credits" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ))
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ))
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,1),    // 512 characters
	2,  // 2 bits per pixel
	{ 4, 0 },
	{ STEP4(0,1), STEP4(4*2,1) },
	{ STEP8(0,4*2*2) },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,2),    // 256 sprites
	4,      // 4 bits per pixel
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1), STEP4(4*2,1), STEP4(4*2*2*16,1), STEP4(4*2*2*16+4*2,1) },
	{ STEP16(0,4*2*2) },
	64*8    // every sprite takes 64 consecutive bytes
};

static const gfx_layout tilelayout =
{
	32,32,  // 32*32 tiles
	RGN_FRAC(1,1),    // 64 tiles
	2,      // 2 bits per pixel
	{ 4, 0 },
	{ STEP4(0,1), STEP4(4*2,1), STEP4(4*2*2*32,1), STEP4(4*2*2*32+4*2,1),
		STEP4(4*2*2*64,1), STEP4(4*2*2*64+4*2,1), STEP4(4*2*2*96,1), STEP4(4*2*2*96+4*2,1) },
	{ STEP32(0,4*2*2) },
	256*8   // every tile takes 256 consecutive bytes
};


static GFXDECODE_START( gfx_exedexes )
	GFXDECODE_ENTRY( "chars",      0, charlayout,              0, 64 )
	GFXDECODE_ENTRY( "32x32tiles", 0, tilelayout,           64*4, 64 )
	GFXDECODE_ENTRY( "16x16tiles", 0, spritelayout,       2*64*4, 16 )
	GFXDECODE_ENTRY( "sprites",    0, spritelayout, 2*64*4+16*16, 16 )
GFXDECODE_END


void exedexes_state::machine_start()
{
	save_item(NAME(m_chon));
	save_item(NAME(m_objon));
	save_item(NAME(m_sc1on));
	save_item(NAME(m_sc2on));
}

void exedexes_state::machine_reset()
{
	m_chon = 0;
	m_objon = 0;
	m_sc1on = 0;
	m_sc2on = 0;
}

void exedexes_state::exedexes(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // 3 MHz, verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &exedexes_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(exedexes_state::scanline), "screen", 0, 1);

	z80_device &audiocpu(Z80(config, "audiocpu", 12_MHz_XTAL / 4)); // 3 MHz, verified on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &exedexes_state::sound_map);
	audiocpu.set_periodic_int(FUNC(exedexes_state::irq0_line_hold), attotime::from_hz(4*60));

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.60); // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(exedexes_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram8_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exedexes);

	PALETTE(config, m_palette, FUNC(exedexes_state::palette), 64*4+64*4+16*16+16*16, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "aysnd", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.10); // 1.5 MHz, verified on PCB

	SN76489(config, "sn1", 12_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.36); // 3 MHz, verified on PCB

	SN76489(config, "sn2", 12_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.36); // 3 MHz, verified on PCB
}


ROM_START( exedexes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11m_ee04.bin", 0x0000, 0x4000, CRC(44140dbd) SHA1(7b56f614f7cd7655ffa3e1f4adba5a20fa25822d) )
	ROM_LOAD( "10m_ee03.bin", 0x4000, 0x4000, CRC(bf72cfba) SHA1(9f0b9472890db95e16a71f26da954780d5ec7c16) )
	ROM_LOAD( "09m_ee02.bin", 0x8000, 0x4000, CRC(7ad95e2f) SHA1(53fd8d6985d08106bab45e83827a509486d640b7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11e_ee01.bin", 0x00000, 0x4000, CRC(73cdf3b2) SHA1(c9f2c91011bdeecec8fa76a42d95f3a5ec77cec9) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "05c_ee00.bin", 0x00000, 0x2000, CRC(cadb75bd) SHA1(2086be5e295e5d870bcb35f116cc925f811b7583) )

	ROM_REGION( 0x04000, "32x32tiles", 0 )
	ROM_LOAD( "h01_ee08.bin", 0x00000, 0x4000, CRC(96a65c1d) SHA1(3b49c64b32f01ec72cf2d943bfe3aa575d62a765) ) // planes 0-1

	ROM_REGION( 0x08000, "16x16tiles", 0 )
	ROM_LOAD( "a03_ee06.bin", 0x00000, 0x4000, CRC(6039bdd1) SHA1(01156e02ed59e6c1e55204729e515cd4419568fb) ) // planes 0-1
	ROM_LOAD( "a02_ee05.bin", 0x04000, 0x4000, CRC(b32d8252) SHA1(738225146ba38f2a9216fda278838e7ebb29a0bb) ) // planes 2-3

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "j11_ee10.bin", 0x00000, 0x4000, CRC(bc83e265) SHA1(ac9b4cce9e539c560414abf2fc239910f2bfbb2d) ) // planes 0-1
	ROM_LOAD( "j12_ee11.bin", 0x04000, 0x4000, CRC(0e0f300d) SHA1(2f973748e459b16673115abf7de8615219e39fa4) ) // planes 2-3

	ROM_REGION( 0x6000, "tilerom", 0 ) // background tilemaps
	ROM_LOAD( "c01_ee07.bin", 0x0000, 0x4000, CRC(3625a68d) SHA1(83010ca356385b713bafe03a502c566f6a9a8365) )    // Front Tile Map
	ROM_LOAD( "h04_ee09.bin", 0x4000, 0x2000, CRC(6057c907) SHA1(886790641b84b8cd659d2eb5fd1adbabdd7dad3d) )    // Back Tile map

	ROM_REGION( 0x0b20, "proms", 0 )
	ROM_LOAD( "02d_e-02.bin", 0x0000, 0x0100, CRC(8d0d5935) SHA1(a0ab827ff3b641965ef851893c399e3988fde55e) )    // red component
	ROM_LOAD( "03d_e-03.bin", 0x0100, 0x0100, CRC(d3c17efc) SHA1(af88340287bd732c91bc5c75970f9de0431b4304) )    // green component
	ROM_LOAD( "04d_e-04.bin", 0x0200, 0x0100, CRC(58ba964c) SHA1(1f98f8e484a0462f1a9fadef9e57612a32652599) )    // blue component
	ROM_LOAD( "06f_e-05.bin", 0x0300, 0x0100, CRC(35a03579) SHA1(1f1b8c777622a1f5564409c5f3ce69cc68199dae) )    // char lookup table
	ROM_LOAD( "l04_e-10.bin", 0x0400, 0x0100, CRC(1dfad87a) SHA1(684844c24e630f46525df97ed67e2e63f7e66d0f) )    // 32x32 tile lookup table
	ROM_LOAD( "c04_e-07.bin", 0x0500, 0x0100, CRC(850064e0) SHA1(3884485e91bd82539d0d33f46b7abac60f4c3b1c) )    // 16x16 tile lookup table
	ROM_LOAD( "l09_e-11.bin", 0x0600, 0x0100, CRC(2bb68710) SHA1(cfb375316245cb8751e765f163e6acf071dda9ca) )    // sprite lookup table
	ROM_LOAD( "l10_e-12.bin", 0x0700, 0x0100, CRC(173184ef) SHA1(f91ecbdc67af1eed6757f660cac8a0e6866c1822) )    // sprite palette bank
	ROM_LOAD( "06l_e-06.bin", 0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "k06_e-08.bin", 0x0900, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "l03_e-09.bin", 0x0a00, 0x0100, CRC(0d968558) SHA1(b376885ac8452b6cbf9ced81b1080bfd570d9b91) )    // unknown (all 0)
	ROM_LOAD( "03e_e-01.bin", 0x0b00, 0x0020, CRC(1acee376) SHA1(367094d924f8e0ec36d8310fada4d8143358f697) )    // unknown (priority?)
ROM_END

ROM_START( savgbees )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ee04e.11m",    0x0000, 0x4000, CRC(c0caf442) SHA1(f6e137c1707db620db4f79a1e038101bb3acf812) )
	ROM_LOAD( "ee03e.10m",    0x4000, 0x4000, CRC(9cd70ae1) SHA1(ad2c5de469cdc04a8e877e334a93d68d722cec9a) )
	ROM_LOAD( "ee02e.9m",     0x8000, 0x4000, CRC(a04e6368) SHA1(ed350fb490f8f84dcd9e4a9f5fb3b23079d6b996) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ee01e.11e",    0x00000, 0x4000, CRC(93d3f952) SHA1(5c86d1ddf03083ac2787efb7a29c09b2f46ec3fa) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "ee00e.5c",     0x00000, 0x2000, CRC(5972f95f) SHA1(7b90ceca37dba773f72a80da6272b00061526348) )

	ROM_REGION( 0x04000, "32x32tiles", 0 )
	ROM_LOAD( "h01_ee08.bin", 0x00000, 0x4000, CRC(96a65c1d) SHA1(3b49c64b32f01ec72cf2d943bfe3aa575d62a765) ) // planes 0-1

	ROM_REGION( 0x08000, "16x16tiles", 0 )
	ROM_LOAD( "a03_ee06.bin", 0x00000, 0x4000, CRC(6039bdd1) SHA1(01156e02ed59e6c1e55204729e515cd4419568fb) ) // planes 0-1
	ROM_LOAD( "a02_ee05.bin", 0x04000, 0x4000, CRC(b32d8252) SHA1(738225146ba38f2a9216fda278838e7ebb29a0bb) ) // planes 2-3

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "j11_ee10.bin", 0x00000, 0x4000, CRC(bc83e265) SHA1(ac9b4cce9e539c560414abf2fc239910f2bfbb2d) ) // planes 0-1
	ROM_LOAD( "j12_ee11.bin", 0x04000, 0x4000, CRC(0e0f300d) SHA1(2f973748e459b16673115abf7de8615219e39fa4) ) // planes 2-3

	ROM_REGION( 0x6000, "tilerom", 0 ) // background tilemaps
	ROM_LOAD( "c01_ee07.bin", 0x0000, 0x4000, CRC(3625a68d) SHA1(83010ca356385b713bafe03a502c566f6a9a8365) )    // Front Tile Map
	ROM_LOAD( "h04_ee09.bin", 0x4000, 0x2000, CRC(6057c907) SHA1(886790641b84b8cd659d2eb5fd1adbabdd7dad3d) )    // Back Tile map

	ROM_REGION( 0x0b20, "proms", 0 )
	ROM_LOAD( "02d_e-02.bin", 0x0000, 0x0100, CRC(8d0d5935) SHA1(a0ab827ff3b641965ef851893c399e3988fde55e) )    // red component
	ROM_LOAD( "03d_e-03.bin", 0x0100, 0x0100, CRC(d3c17efc) SHA1(af88340287bd732c91bc5c75970f9de0431b4304) )    // green component
	ROM_LOAD( "04d_e-04.bin", 0x0200, 0x0100, CRC(58ba964c) SHA1(1f98f8e484a0462f1a9fadef9e57612a32652599) )    // blue component
	ROM_LOAD( "06f_e-05.bin", 0x0300, 0x0100, CRC(35a03579) SHA1(1f1b8c777622a1f5564409c5f3ce69cc68199dae) )    // char lookup table
	ROM_LOAD( "l04_e-10.bin", 0x0400, 0x0100, CRC(1dfad87a) SHA1(684844c24e630f46525df97ed67e2e63f7e66d0f) )    // 32x32 tile lookup table
	ROM_LOAD( "c04_e-07.bin", 0x0500, 0x0100, CRC(850064e0) SHA1(3884485e91bd82539d0d33f46b7abac60f4c3b1c) )    // 16x16 tile lookup table
	ROM_LOAD( "l09_e-11.bin", 0x0600, 0x0100, CRC(2bb68710) SHA1(cfb375316245cb8751e765f163e6acf071dda9ca) )    // sprite lookup table
	ROM_LOAD( "l10_e-12.bin", 0x0700, 0x0100, CRC(173184ef) SHA1(f91ecbdc67af1eed6757f660cac8a0e6866c1822) )    // sprite palette bank
	ROM_LOAD( "06l_e-06.bin", 0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "k06_e-08.bin", 0x0900, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "l03_e-09.bin", 0x0a00, 0x0100, CRC(0d968558) SHA1(b376885ac8452b6cbf9ced81b1080bfd570d9b91) )    // unknown (all 0)
	ROM_LOAD( "03e_e-01.bin", 0x0b00, 0x0020, CRC(1acee376) SHA1(367094d924f8e0ec36d8310fada4d8143358f697) )    // unknown (priority?)
ROM_END

} // anonymous namespace


GAME( 1985, exedexes, 0,        exedexes, exedexes, exedexes_state, empty_init, ROT270, "Capcom",                    "Exed Exes",   MACHINE_SUPPORTS_SAVE )
GAME( 1985, savgbees, exedexes, exedexes, exedexes, exedexes_state, empty_init, ROT270, "Capcom (Memetron license)", "Savage Bees", MACHINE_SUPPORTS_SAVE )
