// license:BSD-3-Clause
// copyright-holders:Hau, Nicola Salmoria

/******************************************************************************

  Ganbare Ginkun  (Japan)  (c)1995 TECMO
  Final StarForce (US)     (c)1992 TECMO
  Riot            (Japan)  (c)1992 NMK


--
driver by Hau, Nicola Salmoria

special thanks to Nekomata, NTD & code-name'Siberia'

TODO:
- measure video timing, especially vblank+interrupt length:
  fstarfrc relies on the interrupt being held for a while,
  otherwise the titlescreen background goes wrong.
  If I had to guess: 24/4(6MHz) pixel clock, 384 hclocks, 264 scanlines
  riot does not like multiple interrupts per frame and will start to
  fail towards the end of level 1 (bad palette) then randomly lock up
  on level 2, see irq_150021_w / irq_150031_w for notes
- there could be some priorities problems in riot
  (more noticeable in level 2)
- check layer offsets

Notes:
- To enter into service mode in Final Star Force press and hold start
  buttons 1 and 2 during P.O.S.T.
- The games seem to be romswaps. At least you can swap Riot ROMs on the PCB
  with Final Star Force and it'll work without problems.

******************************************************************************/

#include "emu.h"

#include "tecmo_spr.h"
#include "tecmo_mix.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class tecmo16_state : public driver_device
{
public:
	tecmo16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_charram(*this, "charram"),
		m_spriteram(*this, "spriteram")
	{ }

	void base(machine_config &config);
	void ginkun(machine_config &config);
	void riot(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<tecmo_mix_device> m_mixer;

	required_shared_ptr_array<uint16_t, 2> m_videoram;
	required_shared_ptr_array<uint16_t, 2> m_colorram;
	required_shared_ptr<uint16_t> m_charram;
	required_device<buffered_spriteram16_device> m_spriteram;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	bitmap_ind16 m_tile_bitmap_tx;
	uint8_t m_game_is_riot = 0;
	uint16_t m_scroll_x[2]{};
	uint16_t m_scroll_y[2]{};
	uint16_t m_scroll_char_x = 0;
	uint16_t m_scroll_char_y = 0;

	template <uint8_t Which> void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> void colorram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void charram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void flipscreen_w(uint16_t data);
	template <uint8_t Which> void scroll_x_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> void scroll_y_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scroll_char_x_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scroll_char_y_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void irq_150021_w(uint16_t data);
	void irq_150031_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);

	DECLARE_VIDEO_START(ginkun);
	DECLARE_VIDEO_START(riot);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void save_state();
	void common_map(address_map &map) ATTR_COLD;
	void fstarfrc_map(address_map &map) ATTR_COLD;
	void ginkun_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


// Based on sprite drivers from tehkan/wc90.cpp by Ernesto Corvi (ernesto@imagina.com)


void tecmo16_state::save_state()
{
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
	save_item(NAME(m_scroll_char_x));
	save_item(NAME(m_scroll_char_y));
}

TILE_GET_INFO_MEMBER(tecmo16_state::fg_get_tile_info)
{
	int const tile = m_videoram[0][tile_index] & 0x1fff;
	int const color = m_colorram[0][tile_index] & 0x1f;

	// bit 4 controls blending
	//tileinfo.category = (m_colorram[0][tile_index] & 0x10) >> 4;

	tileinfo.set(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(tecmo16_state::bg_get_tile_info)
{
	int const tile = m_videoram[1][tile_index] & 0x1fff;
	int const color = (m_colorram[1][tile_index] & 0x0f);

	tileinfo.set(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(tecmo16_state::tx_get_tile_info)
{
	int const tile = m_charram[tile_index];
	tileinfo.set(0,
			tile & 0x0fff,
			tile >> 12,
			0);
}

/******************************************************************************/

void tecmo16_state::video_start()
{
	// set up tile layers
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_tile_bitmap_tx);

	// set up sprites
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::fg_get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::tx_get_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolly(0,-16);
	m_game_is_riot = 0;

	save_state();
}

VIDEO_START_MEMBER(tecmo16_state, ginkun)
{
	// set up tile layers
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_tile_bitmap_tx);

	// set up sprites
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::fg_get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::tx_get_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
	m_game_is_riot = 0;

	save_state();
}

VIDEO_START_MEMBER(tecmo16_state, riot)
{
	// set up tile layers
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_tile_bitmap_tx);

	// set up sprites
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::fg_get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo16_state::tx_get_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_scrolldy(-16,-16);
	m_game_is_riot = 1;

	save_state();
}

/******************************************************************************/

template <uint8_t Which>
void tecmo16_state::videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[Which][offset]);
	Which ? m_bg_tilemap->mark_tile_dirty(offset) : m_fg_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void tecmo16_state::colorram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_colorram[Which][offset]);
	Which ? m_bg_tilemap->mark_tile_dirty(offset) : m_fg_tilemap->mark_tile_dirty(offset);
}

void tecmo16_state::charram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_charram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

void tecmo16_state::flipscreen_w(uint16_t data)
{
	flip_screen_set(data & 0x01);
}

/******************************************************************************/

template <uint8_t Which>
void tecmo16_state::scroll_x_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll_x[Which]);
	Which ? m_bg_tilemap->set_scrollx(0, m_scroll_x[1]) : m_fg_tilemap->set_scrollx(0, m_scroll_x[0]);
}

template <uint8_t Which>
void tecmo16_state::scroll_y_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll_y[Which]);
	Which ? m_bg_tilemap->set_scrolly(0, m_scroll_y[1]) : m_fg_tilemap->set_scrolly(0, m_scroll_y[0]);
}

void tecmo16_state::scroll_char_x_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll_char_x);
	m_tx_tilemap->set_scrollx(0, m_scroll_char_x);
}

void tecmo16_state::scroll_char_y_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll_char_y);
	m_tx_tilemap->set_scrolly(0, m_scroll_char_y - 16);
}

/******************************************************************************/


/******************************************************************************/

uint32_t tecmo16_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_tile_bitmap_tx.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);
	m_tx_tilemap->draw(screen, m_tile_bitmap_tx, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, *m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, &m_tile_bitmap_tx, &m_sprite_bitmap);

	return 0;
}

void tecmo16_state::screen_vblank(int state)
{
	if (state)
	{
		const rectangle visarea = m_screen->visible_area();
		// 2 frame sprite lags
		m_sprite_bitmap.fill(0, visarea);
		if (m_game_is_riot)  m_sprgen->gaiden_draw_sprites(*m_screen, m_sprite_bitmap, visarea, m_spriteram->buffer(), 0, 0, flip_screen());
		else m_sprgen->gaiden_draw_sprites(*m_screen, m_sprite_bitmap, visarea, m_spriteram->buffer(), 2, 0, flip_screen());

		m_spriteram->copy();
	}
}


/******************************************************************************/


/*
(without setting D0)
move.b D0, $150031.l  (start of interrupt)
move.b D0, $150021.l  (end of interrupt in riot, straight after above in ginkun, not used in fstarfrc?)
*/

void tecmo16_state::irq_150021_w(uint16_t data)
{
	// does this disable the vblank IRQ until next frame? fstarfrc expects multiple interrupts per
	// frame and never writes it, while the others do. riot will start to glitch towards the end
	// of stage 1, locking up on stage 2 if you allow multiple interrupts per frame.
	m_maincpu->set_input_line(5, CLEAR_LINE);
}

void tecmo16_state::irq_150031_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// all games write this at the start of interrupt, is it the basic 'irq ack' (but without
	// disabling further interrupts?) could also be some kind of DMA trigger (palette or similar)
}

void tecmo16_state::common_map(address_map& map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram(); // Main RAM

	// tilemap sizes differ between games, probably configurable
	// but for now we add them in different maps

	map(0x130000, 0x130fff).ram().share("spriteram");
	map(0x140000, 0x141fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x150000, 0x150001).w(FUNC(tecmo16_state::flipscreen_w));
	map(0x150011, 0x150011).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x150020, 0x150021).portr("EXTRA").w(FUNC(tecmo16_state::irq_150021_w));
	map(0x150030, 0x150031).portr("DSW2").w(FUNC(tecmo16_state::irq_150031_w));
	map(0x150040, 0x150041).portr("DSW1");
	map(0x150050, 0x150051).portr("P1_P2");
//  map(0x160000, 0x160001).nopr();   // ??? Read at every scene changes
	map(0x160000, 0x160001).w(FUNC(tecmo16_state::scroll_char_x_w));
	map(0x160006, 0x160007).w(FUNC(tecmo16_state::scroll_char_y_w));
	map(0x16000c, 0x16000d).w(FUNC(tecmo16_state::scroll_x_w<0>));
	map(0x160012, 0x160013).w(FUNC(tecmo16_state::scroll_y_w<0>));
	map(0x160018, 0x160019).w(FUNC(tecmo16_state::scroll_x_w<1>));
	map(0x16001e, 0x16001f).w(FUNC(tecmo16_state::scroll_y_w<1>));
}



void tecmo16_state::fstarfrc_map(address_map &map)
{
	map(0x110000, 0x110fff).ram().w(FUNC(tecmo16_state::charram_w)).share(m_charram);
	map(0x120000, 0x1207ff).ram().w(FUNC(tecmo16_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x120800, 0x120fff).ram().w(FUNC(tecmo16_state::colorram_w<0>)).share(m_colorram[0]);
	map(0x121000, 0x1217ff).ram().w(FUNC(tecmo16_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x121800, 0x121fff).ram().w(FUNC(tecmo16_state::colorram_w<1>)).share(m_colorram[1]);
	map(0x122000, 0x127fff).ram(); // work area

	common_map(map);
}

void tecmo16_state::ginkun_map(address_map &map)
{
	map(0x110000, 0x110fff).ram().w(FUNC(tecmo16_state::charram_w)).share(m_charram);
	map(0x120000, 0x120fff).ram().w(FUNC(tecmo16_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x121000, 0x121fff).ram().w(FUNC(tecmo16_state::colorram_w<0>)).share(m_colorram[0]);
	map(0x122000, 0x122fff).ram().w(FUNC(tecmo16_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x123000, 0x123fff).ram().w(FUNC(tecmo16_state::colorram_w<1>)).share(m_colorram[1]);
	map(0x124000, 0x124fff).ram(); // extra RAM for Riot

	common_map(map);
}

void tecmo16_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xfbff).ram(); // Sound RAM
	map(0xfc00, 0xfc00).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfc04, 0xfc05).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfc08, 0xfc08).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xfc0c, 0xfc0c).noprw();
	map(0xfffe, 0xffff).ram();
}

/******************************************************************************/

static INPUT_PORTS_START( fstarfrc )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )   PORT_DIPLOCATION("SW1:8")    // flagged as "unused" in the manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:3,4")  // enemy shot speed
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium )  )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest )  )
	PORT_DIPNAME( 0x30, 0x30, "Level Up Speed" )       PORT_DIPLOCATION("SW2:5,6")  // rate of power-up
	PORT_DIPSETTING(    0x30, "Fast" )
	PORT_DIPSETTING(    0x20, "Fastest" )
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ))   PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "200000,1000000" )
	PORT_DIPSETTING(    0x80, "220000,1200000" )
	PORT_DIPSETTING(    0x40, "240000,1400000" )
	PORT_DIPSETTING(    0x00, "every 500000,once at highest score" )    // beating the hi-score gives you an extra life

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("EXTRA")
	// Not used
INPUT_PORTS_END

static INPUT_PORTS_START( ginkun )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )      PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )      PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, "Continue Plus 1up" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      // Doesn't work?
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("EXTRA")
	// Not used
INPUT_PORTS_END

static INPUT_PORTS_START( riot )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )      PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x40, "Starting Coins" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("EXTRA")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xffdd, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

/******************************************************************************/

static GFXDECODE_START( gfx_tecmo16 )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_packed_msb,         1*16*16,    16 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 0x100 )
GFXDECODE_END

static GFXDECODE_START( gfx_tecmo16_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_packed_msb, 0, 0x100 )
GFXDECODE_END

/******************************************************************************/

void tecmo16_state::base(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(24'000'000);
	static constexpr XTAL OKI_CLOCK = XTAL(8'000'000);

	// basic machine hardware
	M68000(config, m_maincpu, MASTER_CLOCK / 2);          // 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &tecmo16_state::fstarfrc_map);

	Z80(config, m_audiocpu, MASTER_CLOCK / 6);         // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &tecmo16_state::sound_map);
								// NMIs are triggered by the main CPU
	config.set_maximum_quantum(attotime::from_hz(600));

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.17);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1000)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(tecmo16_state::screen_update));
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ5);
	m_screen->screen_vblank().append(FUNC(tecmo16_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tecmo16);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_444, 4096);

	TECMO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tecmo16_spr);

	TECMO_MIXER(config, m_mixer, 0);
	m_mixer->set_mixer_shifts(10, 9, 4);
	m_mixer->set_blendcols(   0x0400 + 0x300, 0x0400 + 0x200, 0x0400 + 0x100, 0x0400 + 0x000 );
	m_mixer->set_regularcols( 0x0000 + 0x300, 0x0000 + 0x200, 0x0000 + 0x100, 0x0000 + 0x000 );
	m_mixer->set_blendsource( 0x0800 + 0x000, 0x0800 + 0x100); // riot seems to set palettes in 0x800 + 0x200, could be more to this..
	m_mixer->set_revspritetile();
	m_mixer->set_bgpen(0x000 + 0x300, 0x400 + 0x300);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", MASTER_CLOCK/6)); // 4 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);

	okim6295_device &oki(OKIM6295(config, "oki", OKI_CLOCK / 8, okim6295_device::PIN7_HIGH)); // sample rate 1 MHz / 132
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.40);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.40);
}

void tecmo16_state::ginkun(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tecmo16_state::ginkun_map);

	MCFG_VIDEO_START_OVERRIDE(tecmo16_state, ginkun)
}

void tecmo16_state::riot(machine_config &config)
{
	ginkun(config);

	MCFG_VIDEO_START_OVERRIDE(tecmo16_state, riot)
}

/******************************************************************************/

ROM_START( fstarfrc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fstarf01.rom", 0x00000, 0x40000, CRC(94c71de6) SHA1(7637aee89034d60ef74d0015db6fcbcc8689b88b) )
	ROM_LOAD16_BYTE( "fstarf02.rom", 0x00001, 0x40000, CRC(b1a07761) SHA1(efd580e06a134a8b6ed6e836eec3203c41ed03c5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fstarf07.rom", 0x00000, 0x10000, CRC(e0ad5de1) SHA1(677237341e837061b6cc02200c0752964caed907) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "fstarf03.rom", 0x00000, 0x20000, CRC(54375335) SHA1(d1af56a7c7fff877066dad3144d0b5147da28c6a) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "fstarf05.rom", 0x00000, 0x80000, CRC(77a281e7) SHA1(a87a90c2c856d45785cb56185b1a7dff3404b5cb) )
	ROM_LOAD16_BYTE( "fstarf04.rom", 0x00001, 0x80000, CRC(398a920d) SHA1(eecc167803f48517348d68ce70f15e87eac204bb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "fstarf09.rom", 0x00000, 0x80000, CRC(d51341d2) SHA1(e46c319158046d407d4387cb2d8f0b6cfd7be576) )
	ROM_LOAD16_BYTE( "fstarf06.rom", 0x00001, 0x80000, CRC(07e40e87) SHA1(22867e52a8267ae8ae0ff0dba6bb846cb3e1b63d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fstarf08.rom", 0x00000, 0x20000, CRC(f0ad5693) SHA1(a0202801bb9f9c86175ca7989fbc9efa47183188) )
ROM_END

ROM_START( fstarfrcj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.bin", 0x00000, 0x40000, CRC(1905d85d) SHA1(83d244f13064b826ccf86b5a8158478452efbf7f) )
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x40000, CRC(de9cfc39) SHA1(bd7943f366a3161222848c5f9b687a6ba8c1d43a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fstarf07.rom", 0x00000, 0x10000, CRC(e0ad5de1) SHA1(677237341e837061b6cc02200c0752964caed907) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "fstarf03.rom", 0x00000, 0x20000, CRC(54375335) SHA1(d1af56a7c7fff877066dad3144d0b5147da28c6a) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "fstarf05.rom", 0x00000, 0x80000, CRC(77a281e7) SHA1(a87a90c2c856d45785cb56185b1a7dff3404b5cb) )
	ROM_LOAD16_BYTE( "fstarf04.rom", 0x00001, 0x80000, CRC(398a920d) SHA1(eecc167803f48517348d68ce70f15e87eac204bb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "fstarf09.rom", 0x00000, 0x80000, CRC(d51341d2) SHA1(e46c319158046d407d4387cb2d8f0b6cfd7be576) )
	ROM_LOAD16_BYTE( "fstarf06.rom", 0x00001, 0x80000, CRC(07e40e87) SHA1(22867e52a8267ae8ae0ff0dba6bb846cb3e1b63d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fstarf08.rom", 0x00000, 0x20000, CRC(f0ad5693) SHA1(a0202801bb9f9c86175ca7989fbc9efa47183188) )
ROM_END

ROM_START( fstarfrcja ) // very minor differences with fstarfrcj
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fstarf01.ic1", 0x00000, 0x40000, CRC(3b495b2c) SHA1(1ba5e1a7275b30534165ec7549b265cfcebfddfc) )
	ROM_LOAD16_BYTE( "fstarf02.ic2", 0x00001, 0x40000, CRC(5dacfd3d) SHA1(01023f902ffee1eeb999df5dfb12d02c93308b45) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fstarf07.rom", 0x00000, 0x10000, CRC(e0ad5de1) SHA1(677237341e837061b6cc02200c0752964caed907) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "fstarf03.rom", 0x00000, 0x20000, CRC(54375335) SHA1(d1af56a7c7fff877066dad3144d0b5147da28c6a) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "fstarf05.rom", 0x00000, 0x80000, CRC(77a281e7) SHA1(a87a90c2c856d45785cb56185b1a7dff3404b5cb) )
	ROM_LOAD16_BYTE( "fstarf04.rom", 0x00001, 0x80000, CRC(398a920d) SHA1(eecc167803f48517348d68ce70f15e87eac204bb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "fstarf09.rom", 0x00000, 0x80000, CRC(d51341d2) SHA1(e46c319158046d407d4387cb2d8f0b6cfd7be576) )
	ROM_LOAD16_BYTE( "fstarf06.rom", 0x00001, 0x80000, CRC(07e40e87) SHA1(22867e52a8267ae8ae0ff0dba6bb846cb3e1b63d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fstarf08.rom", 0x00000, 0x20000, CRC(f0ad5693) SHA1(a0202801bb9f9c86175ca7989fbc9efa47183188) )
ROM_END

ROM_START( fstarfrcw )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.bin", 0x00000, 0x40000, CRC(5bc0a9d2) SHA1(bd8ceded1b4bcaffbe220f33b22cdf434ef4cc6c) )
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x40000, CRC(8ec787cb) SHA1(dd7976a334bdbc5a9264e866d4faf49fa72db3a3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fstarf07.rom", 0x00000, 0x10000, CRC(e0ad5de1) SHA1(677237341e837061b6cc02200c0752964caed907) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "fstarf03.rom", 0x00000, 0x20000, CRC(54375335) SHA1(d1af56a7c7fff877066dad3144d0b5147da28c6a) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "fstarf05.rom", 0x00000, 0x80000, CRC(77a281e7) SHA1(a87a90c2c856d45785cb56185b1a7dff3404b5cb) )
	ROM_LOAD16_BYTE( "fstarf04.rom", 0x00001, 0x80000, CRC(398a920d) SHA1(eecc167803f48517348d68ce70f15e87eac204bb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "fstarf09.rom", 0x00000, 0x80000, CRC(d51341d2) SHA1(e46c319158046d407d4387cb2d8f0b6cfd7be576) )
	ROM_LOAD16_BYTE( "fstarf06.rom", 0x00001, 0x80000, CRC(07e40e87) SHA1(22867e52a8267ae8ae0ff0dba6bb846cb3e1b63d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fstarf08.rom", 0x00000, 0x20000, CRC(f0ad5693) SHA1(a0202801bb9f9c86175ca7989fbc9efa47183188) )
ROM_END

ROM_START( ginkun )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ginkun01.i01", 0x00000, 0x40000, CRC(98946fd5) SHA1(e0b496d1fa5201d94a2a22243fe4b37d9ff7bc90) )
	ROM_LOAD16_BYTE( "ginkun02.i02", 0x00001, 0x40000, CRC(e98757f6) SHA1(2310b5f00b9522d5a983c8686f7d5bcf2d885964) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ginkun07.i17", 0x00000, 0x10000, CRC(8836b1aa) SHA1(22bd5258e5971aa69eaa516d7358d87fbb65bee4) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "ginkun03.i03", 0x00000, 0x20000, CRC(4456e0df) SHA1(1509474cfbb208502262b7039e28d37be1131a46) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "ginkun05.i09", 0x00000, 0x80000, CRC(1263bd42) SHA1(bff93633d42bae5b8273465e16bdb4db81bbd6e0) )
	ROM_LOAD16_BYTE( "ginkun04.i05", 0x00001, 0x80000, CRC(9e4cf611) SHA1(57242f0aac49e0569a57372e59ccc643924e9b44) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ginkun09.i22", 0x00000, 0x80000, CRC(233384b9) SHA1(031735b0fb2c89b0af26ba76061776767647c59c) )
	ROM_LOAD16_BYTE( "ginkun06.i16", 0x00001, 0x80000, CRC(f8589184) SHA1(b933265960742cb3505eb73631ec419b7e1d1d63) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "ginkun08.i18", 0x00000, 0x20000, CRC(8b7583c7) SHA1(be7ce721504afb45e16eda146f12031d818fc94c) )
ROM_END


/*
Riot
NMK, 1992

This game runs on Tecmo hardware.

PCB Layouts -

MAIN BOARD
----------

8901A-4 TECMO
|--------------------------------------------------------------------|
|LA4460                                    |----------------------|  |
|           |-----|          6264          |----------------------|  |
|           |     |                   |--------|    |--------|       |
|           |     |          6264     |        |    |        | 62256 |
|   LM324   |68000|                   |TECMO-8 |    |TECMO-07|       |
|           |     |                   |        |    |        |       |
|   LM324   |     |                   |        |    |        |       |
|           |     |                   |--------|    |--------|       |
|           |     |                                                  |
|           |     |       |-----|                                |-| |
|   YM3012  |-----| PAL1  |TECMO|                   6116   6116  | | |
|J                        |-11  |                                | | |
|A                        |-----|                                | | |
|M                                  4464                         | | |
|M                 4464   4464      4464    4464    4464   4464  | | |
|A                 4464   4464      4464            4464   4464  | | |
|                                                                |-| |
|                           |--------|                   |--------|  |
|                           |        |     |-----|       |        |  |
|                           |TECMO-10|     |TECMO|       |TECMO-06|  |
|      24MHz        YM2151  |        |     |-12  |       |        |  |
|    |--------|             |        |     |-----|       |        |  |
|    |        |             |--------|                   |--------|  |
|    |TECMO-9 |       PAL2                                           |
|    |        |                                                      |
|    |        |       6264                        |-------------|    |
|    |--------|               M6295          8MHz |   TECMO-5   |    |
|                                                 |-------------|    |
|                                          |----------------------|  |
|  DSW2(8)  DSW1(8)   Z80                  |----------------------|  |
|--------------------------------------------------------------------|
Notes:
      68000 clock 12.000MHz [24/2]
      Z80 clock 4.000MHz [24/6]
      YM2151 clock 4.000MHz [24/6]
      M6295 clock 1.000MHz [8/8], sample rate 1000000/132
      VSync 60Hz
      PAL1 - AMI 18CV8 stamped 'T-11'
      PAL2 - AMI 18CV8 stamped 'T-12'

      Custom Tecmo IC's -
                         TECMO-5   MCU? clock input 6.000MHz on pin15 (SDIP64)
                         TECMO-06, also stamped 'YM6048' (QFP160)
                         TECMO-07, also stamped 'YM6621' (QFP160)
                         TECMO-8   (QFP136)
                         TECMO-9,  also stamped 'MN53030XTB' (QFP124)
                         TECMO-10  (QFP128)
                         TECMO-11, also stamped 'MN51005XTC' (QFP64)
                         TECMO-12, also stamped 'MN51005XTD' (QFP64)


ROM BOARD
---------

TECMO OBD ROM 8901B
B-82778 (sticker)
|---------------------------------|
|       |----------------------|  |
|       |----------------------|  |
|                                 |
|                                 |
|                                 |
|  7   8    *    *    *    9      |
|                                 |
|                                 |
|                                 |
|                                 |
|                                 |
|                                 |
|  *   *    *    *    *    6      |
|                             |-| |
|                             | | |
|                             | | |
|                             | | |
|                             | | |
|                             | | |
|  *   4    *    *    *    5  | | |
|                             |-| |
|                                 |
|                                 |
|                                 |
|                                 |
|           1    2         3      |
|                                 |
|                                 |
|       |----------------------|  |
|       |----------------------|  |
|---------------------------------|
Notes:
      * = empty solder location for ROM (no socket)

*/

ROM_START( riot )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.ic1", 0x00000, 0x40000, CRC(9ef4232e) SHA1(b9dd3e0dc5785311ff2433b5eb94e327b51ef144) )
	ROM_LOAD16_BYTE( "2.ic2", 0x00001, 0x40000, CRC(f2c6fbbf) SHA1(114cc9ede8b6b4e94dad59f82f0232e9b7fa5025) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "7.ic17", 0x00000, 0x10000, CRC(0a95b8f3) SHA1(cc6bdeeeb184eb4f3867eb9c961b0b82743fac9f) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "3.ic3", 0x00000, 0x20000, CRC(f60f5c96) SHA1(56ea21f22d3cf47071bfb3555b331a676463b63e) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "5.ic9", 0x00000, 0x80000, CRC(056fce78) SHA1(25234fa0282fdbefefb06e6aa5a467f9d08ed534) )
	ROM_LOAD16_BYTE( "4.ic5", 0x00001, 0x80000, CRC(0894e7b4) SHA1(37a04476770942f292d836997c649a343f71e317) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "9.ic22", 0x00000, 0x80000, CRC(0ead54f3) SHA1(4848eb158d9e2279332225e0b25f1c96a8a5a0c4) )
	ROM_LOAD16_BYTE( "6.ic16", 0x00001, 0x80000, CRC(96ef61da) SHA1(c306e4d1eee19af0229a47c2f115f98c74f33d33) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "8.ic18", 0x00000, 0x20000, CRC(4b70e266) SHA1(4ed23de9223cc7359fbaff9dd500ef6daee00fb0) )
ROM_END

ROM_START( riotw )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "riotw1.ic1", 0x00000, 0x40000, CRC(c1849b44) SHA1(a47fa87b0c73e766e3aee935fdb58341ae8856cb) )
	ROM_LOAD16_BYTE( "riotw2.ic2", 0x00001, 0x40000, CRC(d6dc0c09) SHA1(1ccce338ee7ed0b909323a1649ffdffc6784f980) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "7.ic17", 0x00000, 0x10000, CRC(0a95b8f3) SHA1(cc6bdeeeb184eb4f3867eb9c961b0b82743fac9f) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "3.ic3", 0x00000, 0x20000, CRC(f60f5c96) SHA1(56ea21f22d3cf47071bfb3555b331a676463b63e) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "5.ic9", 0x00000, 0x80000, CRC(056fce78) SHA1(25234fa0282fdbefefb06e6aa5a467f9d08ed534) )
	ROM_LOAD16_BYTE( "4.ic5", 0x00001, 0x80000, CRC(0894e7b4) SHA1(37a04476770942f292d836997c649a343f71e317) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "9.ic22", 0x00000, 0x80000, CRC(0ead54f3) SHA1(4848eb158d9e2279332225e0b25f1c96a8a5a0c4) )
	ROM_LOAD16_BYTE( "6.ic16", 0x00001, 0x80000, CRC(96ef61da) SHA1(c306e4d1eee19af0229a47c2f115f98c74f33d33) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "8.ic18", 0x00000, 0x20000, CRC(4b70e266) SHA1(4ed23de9223cc7359fbaff9dd500ef6daee00fb0) )
ROM_END

} // anonymous namespace


/******************************************************************************/

GAME( 1992, fstarfrc,   0,        base,     fstarfrc, tecmo16_state, empty_init, ROT90, "Tecmo", "Final Star Force (US)",           MACHINE_SUPPORTS_SAVE ) // Has 'Recycle it, don't trash it"  and 'Winners don't use drugs' screens after first attract cycle
GAME( 1992, fstarfrcj,  fstarfrc, base,     fstarfrc, tecmo16_state, empty_init, ROT90, "Tecmo", "Final Star Force (Japan, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, fstarfrcja, fstarfrc, base,     fstarfrc, tecmo16_state, empty_init, ROT90, "Tecmo", "Final Star Force (Japan, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, fstarfrcw,  fstarfrc, base,     fstarfrc, tecmo16_state, empty_init, ROT90, "Tecmo", "Final Star Force (World?)",       MACHINE_SUPPORTS_SAVE ) // more similar the to the Japanese version than to the US one, not the parent because not sure it's the world version

GAME( 1992, riot,       0,        riot,     riot,     tecmo16_state, empty_init, ROT0,  "Tecmo (NMK license)",      "Riot (NMK)",      MACHINE_SUPPORTS_SAVE )
GAME( 1992, riotw,      riot,     riot,     riot,     tecmo16_state, empty_init, ROT0,  "Tecmo (Woong Bi license)", "Riot (Woong Bi)", MACHINE_SUPPORTS_SAVE )

GAME( 1995, ginkun,     0,        ginkun,   ginkun,   tecmo16_state, empty_init, ROT0,  "Tecmo", "Ganbare Ginkun", MACHINE_SUPPORTS_SAVE )
