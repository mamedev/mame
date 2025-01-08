// license:BSD-3-Clause
// copyright-holders: David Haywood, Nicola Salmoria, Tomasz Slanina

/*
***********************************************************************************
Dark Mist (c)1986  Taito / Seibu

driver by

  David Haywood
  Nicola Salmoria
  Tomasz Slanina

Main CPU : z80 (with encryption, external to z80)
Sound CPU: custom T5182 CPU (like Seibu sound system but with internal code)

The SEI8608B sound board, which features the T5182 "CPU CUSTOM" and YM2151, also
has unpopulated locations for a 76489AN, 2x MSM5205, 2x 27512 EPROM (presumably
for ADPCM samples), and additional TTL chips to support all these.

$e000 - coins (two bytes)
$e2b7 - player 1 energy

TODO:
 - when player soaks in water, color pen used is wrong (entry 1 at 0xf500 should be 0x0c and instead is 0x14), might be BTANB?
 - cocktail mode
 - unknown bit in sprite attr (there's code used for OR-ing sprite attrib with some
   value (taken from RAM) when one of coords is greater than 256-16 )
***********************************************************************************
*/

#include "emu.h"

#include "t5182.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class darkmist_state : public driver_device
{
public:
	darkmist_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_t5182(*this, "t5182"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritebank(*this, "spritebank"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bg_clut(*this, "bg_clut"),
		m_fg_clut(*this, "fg_clut"),
		m_spr_clut(*this, "spr_clut"),
		m_tx_clut(*this, "tx_clut"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_bg_map(*this, "bg_map"),
		m_fg_map(*this, "fg_map"),
		m_rombank(*this, "rombank")
	{ }

	void darkmist(machine_config &config);

	void init_darkmist();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void hw_w(uint8_t data);
	void tx_vram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bgtile_info);
	TILE_GET_INFO_MEMBER(get_fgtile_info);
	TILE_GET_INFO_MEMBER(get_txttile_info);

	void palette(palette_device &palette) const;

	bitmap_ind16 m_temp_bitmap;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mix_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *clut);
	void decrypt_fgbgtiles(uint8_t *rgn, int size);
	void decrypt_gfx();
	void decrypt_snd();

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<t5182_device> m_t5182;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spritebank;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_bg_clut;
	required_region_ptr<uint8_t> m_fg_clut;
	required_region_ptr<uint8_t> m_spr_clut;
	required_region_ptr<uint8_t> m_tx_clut;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_region_ptr<uint8_t> m_bg_map;
	required_region_ptr<uint8_t> m_fg_map;
	required_memory_bank m_rombank;

	uint8_t m_hw = 0;
	tilemap_t *m_bgtilemap = nullptr;
	tilemap_t *m_fgtilemap = nullptr;
	tilemap_t *m_txtilemap = nullptr;
};


TILE_GET_INFO_MEMBER(darkmist_state::get_bgtile_info)
{
	int code = m_bg_map[tile_index * 2]; // TTTTTTTT
	int const attr = m_bg_map[(tile_index * 2) + 1]; // -PPP--TT - FIXED BITS (0xxx00xx)

	code += (attr & 3) << 8;
	int const pal = (attr >> 4) & 0xf;

	tileinfo.set(1, code, pal, 0);
}

TILE_GET_INFO_MEMBER(darkmist_state::get_fgtile_info)
{
	int code = m_fg_map[tile_index * 2]; // TTTTTTTT
	int const attr = m_fg_map[(tile_index * 2) + 1]; // -PPP--TT - FIXED BITS (0xxx00xx)

	code += (attr & 3) << 8;
	int const pal = (attr >> 4) & 0xf;

	tileinfo.set(2, code, pal, 0);
}

TILE_GET_INFO_MEMBER(darkmist_state::get_txttile_info)
{
	int code = m_videoram[tile_index];
	int const attr = m_videoram[tile_index + 0x400];
	int const pal = (attr >> 1);

	code += (attr & 1) << 8;

	tileinfo.set(0, code, pal & 0xf, 0);
}

void darkmist_state::palette(palette_device &palette) const
{
	//palette.set_indirect_color(0x100, rgb_t::black());

	std::pair<uint8_t const *, uint8_t> const planes[4]{
			{ &m_bg_clut[0], 0x80 },
			{ &m_fg_clut[0], 0x00 },
			{ &m_spr_clut[0], 0x40 },
			{ &m_tx_clut[0], 0xc0 } };

	for (unsigned plane = 0; std::size(planes) > plane; ++plane)
	{
		for (unsigned i = 0; 0x100 > i; ++i)
		{
			uint8_t const clut = planes[plane].first[i];
//          if (clut & 0x40) // 0x40 indicates transparent pen
//              ctabentry = 0x100;
//          else
			int const ctabentry = (clut & 0x3f) | planes[plane].second;
			palette.set_pen_indirect((plane << 8) | i, ctabentry);
		}
	}
}


void darkmist_state::video_start()
{
	m_bgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkmist_state::get_bgtile_info)), TILEMAP_SCAN_ROWS, 16, 16, 512, 64);
	m_fgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkmist_state::get_fgtile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 256);
	m_txtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkmist_state::get_txttile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_fgtilemap->set_transparent_pen(0);
//  m_txtilemap->set_transparent_pen(0);

	save_item(NAME(m_hw));
	m_screen->register_screen_bitmap(m_temp_bitmap);
}

// TODO: move this code into framework or substitute with a valid alternative
void darkmist_state::mix_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *clut)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		uint16_t const *const src = &m_temp_bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t const pix = (src[x] & 0xff);
			uint16_t const real = clut[pix];

			if (BIT(~real, 6))
				dest[x] = src[x];
		}
	}
}

/*
    Sprites

    76543210
0 - TTTT TTTT - tile
1 - xyBP PPP? - palette (P), flips (x,y), B - use spritebank,
                ? - unknown, according to gamecode top bit of one of coords(y/x)
2 - YYYY YYYY - y coord
3 - XXXX XXXX - x coord

*/
void darkmist_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// fetch from top to bottom
	for (int i = m_spriteram.bytes() - 32; i >= 0; i -= 32)
	{
		bool const fy = BIT(m_spriteram[i + 1], 6);
		bool const fx = BIT(m_spriteram[i + 1], 7);

		int tile = m_spriteram[i + 0];

		if (BIT(m_spriteram[i + 1], 5))
			tile += (*m_spritebank << 8);

		int palette = ((m_spriteram[i + 1]) >> 1) & 0xf;

		if (BIT(m_spriteram[i + 1], 0))
			palette = machine().rand() & 15;

		m_gfxdecode->gfx(3)->transpen(
		bitmap, cliprect,
		tile,
		palette,
		fx, fy,
		m_spriteram[i + 3], m_spriteram[i + 2], 0);
	}
}

uint32_t darkmist_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// vis. flags

	constexpr int DISPLAY_SPR = 1;
	constexpr int DISPLAY_FG = 2; // 2 or 8
	constexpr int DISPLAY_BG = 4;
	constexpr int DISPLAY_TXT = 16;

#define DM_GETSCROLL(n) (((m_scroll[(n)] << 1) & 0xff) + ((m_scroll[(n)] & 0x80) ? 1 : 0) + ( ((m_scroll[(n) - 1] << 4) | (m_scroll[(n) - 1] << 12)) & 0xff00))

	m_bgtilemap->set_scrollx(0, DM_GETSCROLL(0x2));
	m_bgtilemap->set_scrolly(0, DM_GETSCROLL(0x6));
	m_fgtilemap->set_scrollx(0, DM_GETSCROLL(0xa));
	m_fgtilemap->set_scrolly(0, DM_GETSCROLL(0xe));

	m_temp_bitmap.fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_hw & DISPLAY_BG)
	{
		m_bgtilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_layer(screen, bitmap, cliprect, m_bg_clut);
	}

	if (m_hw & DISPLAY_FG)
	{
		m_fgtilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_layer(screen, bitmap, cliprect, m_fg_clut);
	}

	if (m_hw & DISPLAY_SPR)
	{
		draw_sprites(m_temp_bitmap, cliprect);
		mix_layer(screen, bitmap, cliprect, m_spr_clut);
	}

	if (m_hw & DISPLAY_TXT)
	{
		m_txtilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
		mix_layer(screen, bitmap, cliprect, m_tx_clut);
	}

	return 0;
}

void darkmist_state::tx_vram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_txtilemap->mark_tile_dirty(offset & 0x3ff);
}


void darkmist_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void darkmist_state::hw_w(uint8_t data)
{
	m_hw = data;

	m_rombank->set_entry(BIT(data, 7));
}

void darkmist_state::memmap(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);
	map(0xc801, 0xc801).portr("P1");
	map(0xc802, 0xc802).portr("P2");
	map(0xc803, 0xc803).portr("START");
	map(0xc804, 0xc804).w(FUNC(darkmist_state::hw_w));
	map(0xc805, 0xc805).writeonly().share(m_spritebank);
	map(0xc806, 0xc806).portr("DSW1");
	map(0xc807, 0xc807).portr("DSW2");
	map(0xc808, 0xc808).portr("UNK");
	map(0xd000, 0xd0ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0xd200, 0xd2ff).ram().w(m_palette, FUNC(palette_device::write_indirect_ext)).share("palette_ext");
	map(0xd400, 0xd41f).ram().share(m_scroll);
	map(0xd600, 0xd67f).rw(m_t5182, FUNC(t5182_device::sharedram_r), FUNC(t5182_device::sharedram_w));
	map(0xd680, 0xd680).w(m_t5182, FUNC(t5182_device::sound_irq_w));
	map(0xd681, 0xd681).r(m_t5182, FUNC(t5182_device::sharedram_semaphore_snd_r));
	map(0xd682, 0xd682).w(m_t5182, FUNC(t5182_device::sharedram_semaphore_main_acquire_w));
	map(0xd683, 0xd683).w(m_t5182, FUNC(t5182_device::sharedram_semaphore_main_release_w));
	map(0xd800, 0xdfff).ram().w(FUNC(darkmist_state::tx_vram_w)).share(m_videoram);
	map(0xe000, 0xefff).ram().share("workram");
	map(0xf000, 0xffff).ram().share(m_spriteram);
}

void darkmist_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).bankr(m_rombank);
}

static INPUT_PORTS_START( darkmist )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_DIPNAME( 0x01, 0x01, "2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x20, 0x20, "2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")    // Listed as "ALWAYS ON"
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x20, "10K / 20K" )
	PORT_DIPSETTING(    0x60, "20K / 40K" )
	PORT_DIPSETTING(    0x40, "30K / 60K" )
	PORT_DIPSETTING(    0x00, "40K / 80K" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, "5-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "5-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "5-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "5-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "5-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "5-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "5-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "5-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*16
};


static GFXDECODE_START( gfx_darkmist )
	GFXDECODE_ENTRY( "tx_gfx",  0, charlayout, 0x300, 16 )
	GFXDECODE_ENTRY( "bg_gfx",  0, tilelayout, 0x000, 16 )
	GFXDECODE_ENTRY( "fg_gfx",  0, tilelayout, 0x100, 16 )
	GFXDECODE_ENTRY( "spr_gfx", 0, tilelayout, 0x200, 16 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(darkmist_state::scanline)
{
	int const scanline = param;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h

	if (scanline == 0) // vblank-in irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80 - RST 08h
}


void darkmist_state::darkmist(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4'000'000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &darkmist_state::memmap);
	m_maincpu->set_addrmap(AS_OPCODES, &darkmist_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(darkmist_state::scanline), "screen", 0, 1);

	T5182(config, m_t5182, 14'318'180 / 4);
	m_t5182->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_t5182->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 16, 256-16-1);
	m_screen->set_screen_update(FUNC(darkmist_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_darkmist);
	PALETTE(config, m_palette, FUNC(darkmist_state::palette));
	m_palette->set_format(palette_device::xRGB_444, 0x100*4);
	m_palette->set_indirect_entries(256+1);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 14'318'180 / 4));    // 3.579545 MHz
	ymsnd.irq_handler().set(m_t5182, FUNC(t5182_device::ym2151_irq_handler));
	ymsnd.add_route(0, "mono", 1.0);
	ymsnd.add_route(1, "mono", 1.0);
}

ROM_START( darkmist )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "dm_15.rom", 0x00000, 0x08000, CRC(21e6503c) SHA1(09174fb424b76f7f2a381297e3420ddd2e76b008) )

	ROM_LOAD( "dm_16.rom", 0x10000, 0x08000, CRC(094579d9) SHA1(2449bc9ba38396912ee9b72dd870ea9fcff95776) )

	ROM_REGION( 0x8000, "t5182:external", 0 ) // Toshiba T5182 external ROM
	ROM_LOAD( "dm_17.rom", 0x0000, 0x8000, CRC(7723dcae) SHA1(a0c69e7a7b6fd74f7ed6b9c6419aed94aabcd4b0) )

	ROM_REGION( 0x4000, "tx_gfx", 0 )
	ROM_LOAD( "dm_13.rom", 0x00000, 0x02000, CRC(38bb38d9) SHA1(d751990166dd3d503c5de7667679b96210061cd1) )
	ROM_LOAD( "dm_14.rom", 0x02000, 0x02000, CRC(ac5a31f3) SHA1(79083390671062be2eab93cc875a0f86d709a963) )

	ROM_REGION( 0x20000, "fg_gfx", 0 )
	ROM_LOAD( "dm_05.rom", 0x00000, 0x10000, CRC(ca79a738) SHA1(66a76ea0d8ecc44f6cc77102303df74f40bf6118) )
	ROM_LOAD( "dm_06.rom", 0x10000, 0x10000, CRC(9629ed2c) SHA1(453f6a0b12efdadd7fcbe03ad37afb0afa6be051) )

	ROM_REGION( 0x20000, "bg_gfx", 0 )
	ROM_LOAD( "dm_01.rom", 0x00000, 0x10000, CRC(652aee6b) SHA1(f4150784f7bd7be83a0041e4c52540aa564062ba) )
	ROM_LOAD( "dm_02.rom", 0x10000, 0x10000, CRC(e2dd15aa) SHA1(1f3a6a1e1afabfe9dc47549ef13ae7696302ae88) )

	ROM_REGION( 0x40000, "spr_gfx", 0)
	ROM_LOAD( "dm_09.rom", 0x00000, 0x10000, CRC(52154b50) SHA1(5ee1a4bcf0752a057b9993b0069d744c35cf55f4) )
	ROM_LOAD( "dm_11.rom", 0x10000, 0x08000, CRC(3118e2f9) SHA1(dfd946ea1310851f97d31ce58d8280f2d92b0f59) )
	ROM_LOAD( "dm_10.rom", 0x20000, 0x10000, CRC(34fd52b5) SHA1(c4ee464ed79ec91f993b0f894572c0288f0ad1d4) )
	ROM_LOAD( "dm_12.rom", 0x30000, 0x08000, CRC(cc4b9839) SHA1(b7e95513d2e06929fed5005caf3bf8c3fba0b597) )

	ROM_REGION( 0x10000, "bg_map", 0 ) // 512x64
	ROM_LOAD16_BYTE( "dm_03.rom", 0x00000, 0x08000, CRC(60b40c2a) SHA1(c046273b15dab95ea4851c26ce941e580fa1b6ec) )
	ROM_LOAD16_BYTE( "dm_04.rom", 0x00001, 0x08000, CRC(d47b8cd9) SHA1(86eb7a5d8ea63c0c91f455b1b8322cc7b9c4a968) )

	ROM_REGION( 0x08000, "fg_map", 0 ) // 64x256
	ROM_LOAD16_BYTE( "dm_07.rom", 0x00000, 0x04000, CRC(889b1277) SHA1(78405110b9cf1ab988c0cbfdb668498dadb41229) )
	ROM_LOAD16_BYTE( "dm_08.rom", 0x00001, 0x04000, CRC(f76f6f46) SHA1(ce1c67dc8976106b24fee8d3a0b9e5deb016a327) )

	ROM_REGION( 0x0100, "bg_clut", 0 )
	ROM_LOAD( "63s281n.m7",  0x0000, 0x0100, CRC(897ef49f) SHA1(e40c0fb0a68aa91ceaee86e774a428819a4794bb) )
	ROM_REGION( 0x0100, "fg_clut", 0 )
	ROM_LOAD( "63s281n.d7",  0x0000, 0x0100, CRC(a9975a96) SHA1(3a34569fc68ac15f91e1e90d4e273f844b315091) )
	ROM_REGION( 0x0100, "spr_clut", 0 )
	ROM_LOAD( "63s281n.f11", 0x0000, 0x0100, CRC(8096b206) SHA1(257004aa3501121d058afa6f64b1129303246758) )
	ROM_REGION( 0x0100, "tx_clut", 0 )
	ROM_LOAD( "63s281n.j15", 0x0000, 0x0100, CRC(2ea780a4) SHA1(0f8d6791114705e9982f9035f291d2a305b47f0a) )

	ROM_REGION( 0x0200, "proms", 0 ) // unknown
	ROM_LOAD( "63s281n.l1",  0x0000, 0x0100, CRC(208d17ca) SHA1(a77d56337bcac8d9a7bc3411239dfb3045e069ec) )
	ROM_LOAD( "82s129.d11",  0x0100, 0x0100, CRC(866eab0e) SHA1(398ffe2b82b6e2235746fd987d5f5995d7dc8687) )
ROM_END




void darkmist_state::decrypt_fgbgtiles(uint8_t *rom, int size)
{
	std::vector<uint8_t> buf(0x40000);
	// data lines
	for (int i = 0; i < size / 2; i++)
	{
		int w1 = (rom[i + 0 * size / 2] << 8) + rom[i + 1 * size / 2];

		w1 = bitswap<16>(w1, 9, 14, 7, 2, 6, 8, 3, 15, 10, 13, 5, 12, 0, 11, 4, 1);

		buf[i + 0 * size / 2] = w1 >> 8;
		buf[i + 1 * size / 2] = w1 & 0xff;
	}

	// address lines
	for (int i = 0; i < size; i++)
	{
		rom[i] = buf[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 5, 4, 3, 2, 12, 11, 10, 9, 8, 1, 0, 7, 6)];
	}
}



void darkmist_state::decrypt_gfx()
{
	std::vector<uint8_t> buf(0x40000);
	uint8_t *rom = memregion("tx_gfx")->base();
	int size = memregion("tx_gfx")->bytes();

	// data lines
	for (int i = 0; i < size / 2; i++)
	{
		int w1 = (rom[i + 0 * size / 2] << 8) + rom[i + 1 * size / 2];

		w1 = bitswap<16>(w1, 9, 14, 7, 2, 6, 8, 3, 15, 10, 13, 5, 12, 0, 11, 4, 1);

		buf[i + 0 * size / 2] = w1 >> 8;
		buf[i + 1 * size / 2] = w1 & 0xff;
	}

	// address lines
	for (int i = 0; i < size; i++)
	{
		rom[i] = buf[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 3, 2, 1, 11, 10, 9, 8, 0, 7, 6, 5, 4)];
	}

	decrypt_fgbgtiles(memregion("bg_gfx")->base(), memregion("bg_gfx")->bytes());
	decrypt_fgbgtiles(memregion("fg_gfx")->base(), memregion("fg_gfx")->bytes());


	rom = memregion("spr_gfx")->base();
	size = memregion("spr_gfx")->bytes();

	// data lines
	for (int i = 0; i < size / 2; i++)
	{
		int w1 = (rom[i + 0 * size / 2] << 8) + rom[i + 1 * size / 2];

		w1 = bitswap<16>(w1, 9, 14, 7, 2, 6, 8, 3, 15, 10, 13, 5, 12, 0, 11, 4, 1);

		buf[i + 0 * size / 2] = w1 >> 8;
		buf[i + 1 * size / 2] = w1 & 0xff;
	}

	// address lines
	for (int i = 0; i < size; i++)
	{
		rom[i] = buf[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 12, 11, 10, 9, 8, 5, 4, 3, 13, 7, 6, 1, 0, 2)];
	}
}

void darkmist_state::decrypt_snd()
{
	uint8_t *rom = memregion("t5182:external")->base();

	for (int i = 0x0000; i < 0x8000; i++)
		rom[i] = bitswap<8>(rom[i], 7, 1, 2, 3, 4, 5, 6, 0);
}

void darkmist_state::init_darkmist()
{
	uint8_t *rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x10000);

	decrypt_gfx();

	decrypt_snd();

	for (int i = 0; i < 0x8000; i++)
	{
		uint8_t p = rom[i];
		uint8_t d = p;

		if (((i & 0x20) == 0x00) && ((i & 0x8) != 0))
			p ^= 0x20;

		if (((i & 0x20) == 0x00) && ((i & 0xa) != 0))
			d ^= 0x20;

		if (((i & 0x200) == 0x200) && ((i & 0x408) != 0))
			p ^= 0x10;

		if ((i & 0x220) != 0x200)
		{
			p = bitswap<8>(p, 7, 6, 5, 2, 3, 4, 1, 0);
			d = bitswap<8>(d, 7, 6, 5, 2, 3, 4, 1, 0);
		}

		rom[i] = d;
		m_decrypted_opcodes[i] = p;
	}

	m_rombank->set_base(&rom[0x10000]);

	// adr line swaps
	rom = memregion("bg_map")->base();
	int len = memregion("bg_map")->bytes();
	memcpy(&buffer[0], rom, len);

	for (int i = 0; i < len; i++)
	{
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 7, 6, 5, 4, 3, 15, 14, 13, 12, 9, 8, 2, 1, 11, 10, 0)];
	}


	rom = memregion("fg_map")->base();
	len = memregion("fg_map")->bytes();
	memcpy(&buffer[0], rom, len);
	for (int i = 0; i < len; i++)
	{
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15 , 6, 5, 4, 3, 12, 11, 10, 9, 14, 13, 2, 1, 8, 7, 0)];
	}

}

} // anonymous namespace


GAME( 1986, darkmist, 0, darkmist, darkmist, darkmist_state, init_darkmist, ROT270, "Seibu Kaihatsu (Taito license)", "The Lost Castle In Darkmist", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
