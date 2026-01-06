// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Tomasz Slanina, Angelo Salese
/*

D-DAY   (c)Jaleco 1984

TODO:
- unused upper sprite color bank;
- improve sound comms, sometimes BGM becomes silent, very hard to repro;
- insert coin sound volume cuts;
- emulate protection properly (with a m54824p_device)

--------------------------------------------------------------------------------
Is it 1984 or 1987 game ?
There's text inside rom "1987.07    BY  ELS"

Trivia: E.L.S. = the subcontractor that also developed Commando for Sega (the coin
sound effect is the same as well if you overclock the AY a bit). Jaleco Top Roller
is also by them.

$842f = lives

--------------------------------------------------------------------------------

    CPU  : Z80
    Sound: Z80 AY-3-8910(x2)
    OSC  : 12.000MHz
    Other: Intel 8257 DMA controller

    -------
    DD-8416
    -------
    ROMs:
    1  - (2764)
    2  |
    3  |
    4  |
    5  |
    6  |
    7  |
    8  |
    9  |
    10 |
    11 /

    -------
    DD-8417
    -------
    ROMs:
    12 - (2732)
    13 /

    14 - (2732)
    15 /

    16 - (2764)
    17 |
    18 |
    19 /

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8257.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dday_state : public driver_device
{
public:
	dday_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_dma(*this, "dma"),
		m_bank(*this, "bank"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_bgvram(*this, "bgram"),
		m_proms(*this, "proms")
	{ }

	void dday(machine_config &config);

	void init_dday();
	ioport_value prot_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void prot_w(offs_t offset, u8 data);
	void char_bank_w(u8 data);
	void bgvram_w(offs_t offset, u8 data);
	void vram_w(offs_t offset, u8 data);
	void sound_nmi_enable_w(u8 data);
	void main_nmi_enable_w(u8 data);
	void bg0_w(u8 data);
	void bg1_w(u8 data);
	void bg2_w(u8 data);
	void sound_irq_w(u8 data);
	void flip_screen_w(u8 data);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	void dday_palette(palette_device &palette) const;
	u32 screen_update_dday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<i8257_device> m_dma;
	required_memory_bank m_bank;

	// memory pointers
	required_shared_ptr<u8> m_mainram;
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_bgvram;
	required_region_ptr<u8> m_proms;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	u8 m_char_bank = 0;
	u8 m_bgadr = 0;

	// misc
	bool m_main_nmi_enable = false;
	bool m_sound_nmi_enable = false;
	bool m_sound_irq_clock = false;
	u8 m_prot_addr = 0;

	u8 dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, u8 data);
	u8 dma_r();
	void dma_w(u8 data);
	u8 m_dma_latch = 0;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_foreground(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/********************************
 *
 * Video section
 *
 *******************************/

TILE_GET_INFO_MEMBER(dday_state::get_tile_info_bg)
{
	u8 attr = m_bgvram[tile_index + 0x400];
	u16 code = m_bgvram[tile_index] + ((attr & 0x08) << 5);
	u8 color = (attr & 0x7);
	color |= (attr & 0x40) >> 3;

	tileinfo.category = BIT(attr, 7);
	tileinfo.set(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(dday_state::get_tile_info_fg)
{
	u16 code = m_videoram[tile_index] + (m_char_bank << 8);
	u8 color = m_proms[0x400 | (tile_index >> 2 & 0xe0) | (tile_index & 0x1f)];

	tileinfo.set(1, code, color, 0);
}

void dday_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dday_state::get_tile_info_bg)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dday_state::get_tile_info_fg)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

/****************************
 [0] xxxx xxxx Y offset
 [1] x--- ---- Y flip
     -xxx xxxx code lower offset
 [2] x--- ---- X flip
     --xx ---- code upper offset
     ---- xxxx color offset
 [3] xxxx xxxx X offset
 ***************************/
void dday_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u16 i = 0; i < 0x400; i += 4)
	{
		u8 flags = m_spriteram[i + 2];
		u8 y = 256 - m_spriteram[i + 0] - 8;
		u16 code = m_spriteram[i + 1];
		u8 x = m_spriteram[i + 3] - 8;
		u8 xflip = (flags & 0x80) >> 7;
		u8 yflip = (code & 0x80) >> 7;
		u8 color = flags & 0xf;

		code = (code & 0x7f) | ((flags & 0x30) << 3);

		if (flip_screen())
		{
			x = 256 - 16 - x;
			xflip ^= 1;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, code, color, xflip, yflip, x, y, 0);
	}
}

void dday_state::draw_foreground(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	const rectangle &visarea = screen.visible_area();
	rectangle opaque_rect = cliprect;

	// top opaque part
	opaque_rect.min_x = visarea.min_x;
	opaque_rect.max_x = visarea.min_x + 16;
	opaque_rect &= cliprect;
	m_fg_tilemap->draw(screen, bitmap, opaque_rect, TILEMAP_DRAW_OPAQUE, 0);

	// bottom opaque part
	opaque_rect.min_x = visarea.max_x - 16;
	opaque_rect.max_x = visarea.max_x;
	opaque_rect &= cliprect;
	m_fg_tilemap->draw(screen, bitmap, opaque_rect, TILEMAP_DRAW_OPAQUE, 0);
}

u32 dday_state::screen_update_dday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x100, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);
	draw_foreground(screen, bitmap, cliprect);

	return 0;
}


/*
    Protection device

    24 pin IC with scratched surface (like Exerion's "ICX"), not an MCU.
    Die has label "4828A".

    It's a Mitsubishi M54828P (M54824P family).
    A frequency counter with 5-digit FLT display driver.

    It writes to S1-S4, and reads digit segments (two of b/c/e/g?)

    Pinout:

     1 - Vcc
     2 - fc - count input
     3 - S4 \
     4 - S3  \ preset selection
     5 - S2  /
     6 - S1 /
     7 - brightness control
     8 - X-IN  \ osc circuit
     9 - X-OUT /
    10 - GND
    11 - TEST
    12 - seg A

    13 - seg B
    14 - digit 1
    15 - seg C
    16 - digit 2
    17 - seg D
    18 - seg E
    19 - digit 3
    20 - seg F
    21 - digit 4
    22 - seg G
    23 - digit 5
    24 - seg DP
*/

static const u8 prot_data[0x10] =
{
	0x02, 0x02, 0x02, 0x02,
	0x02, 0x00, 0x02, 0x00,
	0x02, 0x02, 0x02, 0x00,
	0x03, 0x01, 0x00, 0x03
};

ioport_value dday_state::prot_r()
{
	return prot_data[m_prot_addr];
}

void dday_state::prot_w(offs_t offset, u8 data)
{
	m_prot_addr = (m_prot_addr & (~(1 << offset))) | ((data & 1) << offset);
}

void dday_state::char_bank_w(u8 data)
{
	m_char_bank = BIT(data, 0);
	m_fg_tilemap->mark_all_dirty();
	if(data & 0xfe)
		logerror("Warning: char_bank_w with %02x\n",data);
}

void dday_state::bgvram_w(offs_t offset, u8 data)
{
	if (!offset)
		m_bg_tilemap->set_scrollx(0, data);

	m_bgvram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void dday_state::vram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


void dday_state::sound_nmi_enable_w(u8 data)
{
	m_sound_nmi_enable = BIT(data, 0);
	if (!m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void dday_state::main_nmi_enable_w(u8 data)
{
	m_main_nmi_enable = BIT(data, 0);
	if (!m_main_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void dday_state::bg0_w(u8 data)
{
	m_bgadr = (m_bgadr & 0xfe) | (data & 1);
}

void dday_state::bg1_w(u8 data)
{
	m_bgadr = (m_bgadr & 0xfd) | ((data & 1) << 1);
}

void dday_state::bg2_w(u8 data)
{
	m_bgadr = (m_bgadr & 0xfb) | ((data & 1) << 2);
	if (m_bgadr > 2)
		m_bgadr = 0;

	m_bank->set_entry(m_bgadr);
}

void dday_state::sound_irq_w(u8 data)
{
	// 7474 to audiocpu irq? (pulse is too short for direct assert/clear)
	if (!BIT(data, 0) && m_sound_irq_clock)
		m_audiocpu->set_input_line(0, HOLD_LINE);
	m_sound_irq_clock = BIT(data, 0);
}

void dday_state::flip_screen_w(u8 data)
{
	flip_screen_set(data & 1);
}

void dday_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram().share("mainram");
	map(0x9000, 0x93ff).ram().share("spriteram");
	map(0x9400, 0x97ff).ram().w(FUNC(dday_state::vram_w)).share("videoram");
	map(0x9800, 0x9fff).ram().w(FUNC(dday_state::bgvram_w)).share("bgram"); // 9800-981f - videoregs
	map(0xa000, 0xdfff).bankr("bank").nopw();
	map(0xe000, 0xe008).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0xf000, 0xf000).portr("P1").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf080, 0xf080).portr("P2").w(FUNC(dday_state::char_bank_w));
	map(0xf081, 0xf081).w(FUNC(dday_state::flip_screen_w));
	// fn originally marked "LMSR"
	map(0xf083, 0xf083).lw8(NAME([this] (u8 data) {
		m_dma->dreq1_w(BIT(data, 0));
		m_dma->dreq0_w(BIT(data, 0));
	}));
	map(0xf084, 0xf084).w(FUNC(dday_state::bg0_w));
	map(0xf085, 0xf085).w(FUNC(dday_state::bg1_w));
	map(0xf086, 0xf086).w(FUNC(dday_state::bg2_w));
	map(0xf100, 0xf100).portr("SYSTEM").w(FUNC(dday_state::sound_irq_w));
	map(0xf101, 0xf101).w(FUNC(dday_state::main_nmi_enable_w));
	map(0xf102, 0xf105).w(FUNC(dday_state::prot_w));
	map(0xf180, 0xf180).portr("DSW1");
	map(0xf200, 0xf200).portr("DSW2");
}


void dday_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x3000, 0x3000).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x4000, 0x4000).w("ay1", FUNC(ay8910_device::address_w));
	map(0x5000, 0x5000).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x6000, 0x6000).w("ay2", FUNC(ay8910_device::address_w));
	map(0x7000, 0x7000).w(FUNC(dday_state::sound_nmi_enable_w));
}

static INPUT_PORTS_START( dday )
	// TODO: uses single input side for upright, dual for cocktail
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dday_state::prot_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, "Extend" )
	PORT_DIPSETTING(    0x00, "30K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf8, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xf8, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2)},
	{ 0+8*8, 1+8*8, 2+8*8, 3+8*8, 4+8*8, 5+8*8, 6+8*8, 7+8*8,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,0*8+2*8*8, 1*8+2*8*8, 2*8+2*8*8, 3*8+2*8*8, 4*8+2*8*8, 5*8+2*8*8, 6*8+2*8*8, 7*8+2*8*8},
	16*16,
};

static GFXDECODE_START( gfx_dday )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0x000, 16 ) // upper 16 colors are unused
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x2_planar, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x2_planar, 0x100, 16 )
GFXDECODE_END

void dday_state::vblank_irq(int state)
{
	if (state && m_main_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	if (state && m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void dday_state::machine_start()
{
	save_item(NAME(m_char_bank));
	save_item(NAME(m_bgadr));
	save_item(NAME(m_main_nmi_enable));
	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_sound_irq_clock));
	save_item(NAME(m_prot_addr));
	save_item(NAME(m_dma_latch));
}

void dday_state::machine_reset()
{
	m_char_bank = 0;
	m_bgadr = 0;
	m_sound_nmi_enable = false;
	m_main_nmi_enable = false;
	m_sound_irq_clock = false;
	m_prot_addr = 0;
	m_dma_latch = 0;
}

void dday_state::dday_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x200; i++)
	{
		int bit0, bit1, bit2;

		int const val = (m_proms[i + 0x000]) | (m_proms[i + 0x200] << 4);

		bit0 = 0;
		bit1 = BIT(val, 6);
		bit2 = BIT(val, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(val, 3);
		bit1 = BIT(val, 4);
		bit2 = BIT(val, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(val, 0);
		bit1 = BIT(val, 1);
		bit2 = BIT(val, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

u8 dday_state::dma_mem_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	return program.read_byte(offset);
}

void dday_state::dma_mem_w(offs_t offset, u8 data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.write_byte(offset, data);
}

u8 dday_state::dma_r()
{
	return m_dma_latch;
}

void dday_state::dma_w(u8 data)
{
	m_dma_latch = data;
}

void dday_state::dday(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dday_state::main_map);
	m_maincpu->busack_cb().set(m_dma, FUNC(i8257_device::hlda_w));

	Z80(config, m_audiocpu, 12_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dday_state::sound_map);

	I8257(config, m_dma, 12_MHz_XTAL / 4);
	m_dma->out_hrq_cb().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->in_memr_cb().set(FUNC(dday_state::dma_mem_r));
	m_dma->out_memw_cb().set(FUNC(dday_state::dma_mem_w));
	m_dma->in_ior_cb<1>().set(FUNC(dday_state::dma_r));
	m_dma->out_iow_cb<0>().set(FUNC(dday_state::dma_w));
	m_dma->set_reverse_rw_mode(true);

	config.set_maximum_quantum(attotime::from_hz(60000)); // for I8257

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(dday_state::screen_update_dday));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(dday_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dday);
	PALETTE(config, m_palette, FUNC(dday_state::dday_palette), 0x200);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &ay1(AY8910(config, "ay1", 12_MHz_XTAL / 8));
	ay1.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.5);

	AY8910(config, "ay2", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( ddayjlc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",  0x0000, 0x2000, CRC(dbfb8772) SHA1(1fbc9726d0cd1f8781ced2f8233107b65b9bdb1a) )
	ROM_LOAD( "2",  0x2000, 0x2000, CRC(f40ea53e) SHA1(234ef686d3e9fe12aceada7098c4cc53e56eb1a3) )
	ROM_LOAD( "3",  0x4000, 0x2000, CRC(0780ef60) SHA1(9247af38acbaea0f78892fc50081b2400abbdc1f) )
	ROM_LOAD( "4",  0x6000, 0x2000, CRC(75991a24) SHA1(175f505da6eb80479a70181d6aed01130f6a64cc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x2000, CRC(fe4de019) SHA1(16c5402e1a79756f8227d7e99dd94c5896c57444) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "16", 0x0000, 0x2000, CRC(a167fe9a) SHA1(f2770d93ee5ae4eb9b3bcb052e14e36f53eec707) )
	ROM_LOAD( "17", 0x2000, 0x2000, CRC(13ffe662) SHA1(2ea7855a14a4b8429751bae2e670e77608f93406) )
	ROM_LOAD( "18", 0x4000, 0x2000, CRC(debe6531) SHA1(34b3b70a1872527266c664b2a82014d028a4ff1e) )
	ROM_LOAD( "19", 0x6000, 0x2000, CRC(5816f947) SHA1(2236bed3e82980d3e7de3749aef0fbab042086e6) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "14", 0x0000, 0x1000, CRC(2c0e9bbe) SHA1(e34ab774d2eb17ddf51af513dbcaa0c51f8dcbf7) )
	ROM_LOAD( "15", 0x1000, 0x1000, CRC(a6eeaa50) SHA1(052cd3e906ca028e6f55d0caa1e1386482684cbf) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "12", 0x0000, 0x1000, CRC(7f7afe80) SHA1(e8a549b8a8985c61d3ba452e348414146f2bc77e) )
	ROM_LOAD( "13", 0x1000, 0x1000, CRC(f169b93f) SHA1(fb0617162542d688503fc6618dd430308e259455) )

	ROM_REGION( 0xc0000, "terrain", 0 )
	ROM_LOAD( "5",  0x00000, 0x2000, CRC(299b05f2) SHA1(3c1804bccb514bada4bed68a6af08db63a8f1b19) )
	ROM_LOAD( "6",  0x02000, 0x2000, CRC(38ae2616) SHA1(62c96f32532f0d7e2cf1606a303d81ebb4aada7d) )
	ROM_LOAD( "7",  0x04000, 0x2000, CRC(4210f6ef) SHA1(525d8413afabf97cf1d04ee9a3c3d980b91bde65) )
	ROM_LOAD( "8",  0x06000, 0x2000, CRC(91a32130) SHA1(cbcd673b47b672b9ce78c7354dacb5964a81db6f) )
	ROM_LOAD( "9",  0x08000, 0x2000, CRC(ccb82f09) SHA1(37c23f13aa0728bae82dba9e2858a8d81fa8afa5) )
	ROM_LOAD( "10", 0x0a000, 0x2000, CRC(5452aba1) SHA1(03ef47161d0ab047c8675d6ffd3b7acf81f74721) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "4l.bin",  0x00000, 0x0100, CRC(2c3fa534) SHA1(e4c0d06cf62459c1835cb27a4e659b01ad4be20c) ) // sprite color lower data
	ROM_LOAD( "5p.bin",  0x00100, 0x0100, CRC(4fd96b26) SHA1(0fb9928ab6c4ee937cefcf82145a4c9d43ca8517) ) // background color lower data
	ROM_LOAD( "4m.bin",  0x00200, 0x0100, CRC(e0ab9a8f) SHA1(77010c4039f9d408f40cea079c1ef56132ddbd2b) ) // sprite color upper data
	ROM_LOAD( "5n.bin",  0x00300, 0x0100, CRC(61d85970) SHA1(189e9da3dade54936872b80893b1318e5fbfbe5e) ) // background color upper data
	ROM_LOAD( "3l.bin",  0x00400, 0x0100, CRC(da6fe846) SHA1(e8386cf7f552facf2d1a5b7b63ca3d2f1801d215) ) // foreground tile color indices
ROM_END

ROM_START( ddayjlca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1a", 0x0000, 0x2000, CRC(d8e4f3d4) SHA1(78b30b4896a7f718975b1502c6253819bceee922) )
	ROM_LOAD( "2",  0x2000, 0x2000, CRC(f40ea53e) SHA1(234ef686d3e9fe12aceada7098c4cc53e56eb1a3) )
	ROM_LOAD( "3",  0x4000, 0x2000, CRC(0780ef60) SHA1(9247af38acbaea0f78892fc50081b2400abbdc1f) )
	ROM_LOAD( "4",  0x6000, 0x2000, CRC(75991a24) SHA1(175f505da6eb80479a70181d6aed01130f6a64cc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x2000, CRC(fe4de019) SHA1(16c5402e1a79756f8227d7e99dd94c5896c57444) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "16", 0x0000, 0x2000, CRC(a167fe9a) SHA1(f2770d93ee5ae4eb9b3bcb052e14e36f53eec707) )
	ROM_LOAD( "17", 0x2000, 0x2000, CRC(13ffe662) SHA1(2ea7855a14a4b8429751bae2e670e77608f93406) )
	ROM_LOAD( "18", 0x4000, 0x2000, CRC(debe6531) SHA1(34b3b70a1872527266c664b2a82014d028a4ff1e) )
	ROM_LOAD( "19", 0x6000, 0x2000, CRC(5816f947) SHA1(2236bed3e82980d3e7de3749aef0fbab042086e6) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "14", 0x0000, 0x1000, CRC(2c0e9bbe) SHA1(e34ab774d2eb17ddf51af513dbcaa0c51f8dcbf7) )
	ROM_LOAD( "15", 0x1000, 0x1000, CRC(a6eeaa50) SHA1(052cd3e906ca028e6f55d0caa1e1386482684cbf) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "12", 0x0000, 0x1000, CRC(7f7afe80) SHA1(e8a549b8a8985c61d3ba452e348414146f2bc77e) )
	ROM_LOAD( "13", 0x1000, 0x1000, CRC(f169b93f) SHA1(fb0617162542d688503fc6618dd430308e259455) )

	ROM_REGION( 0xc0000, "terrain", 0 )
	ROM_LOAD( "5",  0x00000, 0x2000, CRC(299b05f2) SHA1(3c1804bccb514bada4bed68a6af08db63a8f1b19) )
	ROM_LOAD( "6",  0x02000, 0x2000, CRC(38ae2616) SHA1(62c96f32532f0d7e2cf1606a303d81ebb4aada7d) )
	ROM_LOAD( "7",  0x04000, 0x2000, CRC(4210f6ef) SHA1(525d8413afabf97cf1d04ee9a3c3d980b91bde65) )
	ROM_LOAD( "8",  0x06000, 0x2000, CRC(91a32130) SHA1(cbcd673b47b672b9ce78c7354dacb5964a81db6f) )
	ROM_LOAD( "9",  0x08000, 0x2000, CRC(ccb82f09) SHA1(37c23f13aa0728bae82dba9e2858a8d81fa8afa5) )
	ROM_LOAD( "10", 0x0a000, 0x2000, CRC(5452aba1) SHA1(03ef47161d0ab047c8675d6ffd3b7acf81f74721) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "4l.bin",  0x00000, 0x0100, CRC(2c3fa534) SHA1(e4c0d06cf62459c1835cb27a4e659b01ad4be20c) ) // sprite color lower data
	ROM_LOAD( "5p.bin",  0x00100, 0x0100, CRC(4fd96b26) SHA1(0fb9928ab6c4ee937cefcf82145a4c9d43ca8517) ) // background color lower data
	ROM_LOAD( "4m.bin",  0x00200, 0x0100, CRC(e0ab9a8f) SHA1(77010c4039f9d408f40cea079c1ef56132ddbd2b) ) // sprite color upper data
	ROM_LOAD( "5n.bin",  0x00300, 0x0100, CRC(61d85970) SHA1(189e9da3dade54936872b80893b1318e5fbfbe5e) ) // background color upper data
	ROM_LOAD( "3l.bin",  0x00400, 0x0100, CRC(da6fe846) SHA1(e8386cf7f552facf2d1a5b7b63ca3d2f1801d215) ) // foreground tile color indices
ROM_END


void dday_state::init_dday()
{
	std::vector<u8> temp(0x10000);
	u8 *src = &temp[0];
	u8 *dst = memregion("gfx1")->base();
	u32 length = memregion("gfx1")->bytes();
	memcpy(src, dst, length);

	for (u32 oldaddr = 0; oldaddr < length; oldaddr++)
	{
		u32 newadr = (oldaddr & 0x4007) | (oldaddr >> 10 & 0x8) | (oldaddr << 1 & 0x3ff0);
		dst[newadr] = src[oldaddr];
	}

	m_bank->configure_entries(0, 3, memregion("terrain")->base(), 0x4000);
	m_bank->set_entry(0);
}

} // anonymous namespace


GAME( 1984, ddayjlc,  0,       dday, dday, dday_state, init_dday, ROT90, "Jaleco", "D-Day (Jaleco, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, ddayjlca, ddayjlc, dday, dday, dday_state, init_dday, ROT90, "Jaleco", "D-Day (Jaleco, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
