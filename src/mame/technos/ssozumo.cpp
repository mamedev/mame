// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

Syusse Oozumou
(c) 1984 Technos Japan (Licensed by Data East)

Driver by Takahiro Nogi 1999/10/04

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ssozumo_state : public driver_device
{
public:
	ssozumo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U)
	{ }

	void ssozumo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void sound_nmi_mask_w(uint8_t data);
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	void scroll_w(uint8_t data);
	void flipscreen_w(uint8_t data);

	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_sound_nmi_mask = 0;
	uint8_t m_color_bank = 0;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/**************************************************************************/

void ssozumo_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0 ; i < 64 ; i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[64], 0);
		bit1 = BIT(color_prom[64], 1);
		bit2 = BIT(color_prom[64], 2);
		bit3 = BIT(color_prom[64], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


template <uint8_t Which>
void ssozumo_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void ssozumo_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

void ssozumo_state::paletteram_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2, bit3, val;

	m_paletteram[offset] = data;
	int const offs2 = offset & 0x0f;

	val = m_paletteram[offs2];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offs2 | 0x10];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offs2 | 0x20];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	m_palette->set_pen_color(offs2 + 64, rgb_t(r, g, b));
}

void ssozumo_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, data);
}

void ssozumo_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x80);
	m_color_bank = data & 3;
	m_fg_tilemap->mark_all_dirty();
}

TILE_GET_INFO_MEMBER(ssozumo_state::get_bg_tile_info)
{
	int const code = m_videoram[0][tile_index] + ((m_colorram[0][tile_index] & 0x08) << 5);
	int const color = (m_colorram[0][tile_index] & 0x30) >> 4;
	int const flags = ((tile_index % 32) >= 16) ? TILE_FLIPY : 0;

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(ssozumo_state::get_fg_tile_info)
{
	int const code = m_videoram[1][tile_index] + 256 * (m_colorram[1][tile_index] & 0x07);

	tileinfo.set(0, code, m_color_bank, 0);
}

void ssozumo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssozumo_state::get_bg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X,
			16, 16, 16, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssozumo_state::get_fg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_color_bank));
}

void ssozumo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] & 0x01)
		{
			int const code = m_spriteram[offs + 1] + ((m_spriteram[offs] & 0xf0) << 4);
			int const color = (m_spriteram[offs] & 0x08) >> 3;
			int flipx = m_spriteram[offs] & 0x04;
			int flipy = m_spriteram[offs] & 0x02;
			int sx = 239 - m_spriteram[offs + 3];
			int sy = (240 - m_spriteram[offs + 2]) & 0xff;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code, color,
					flipx, flipy,
					sx, sy, 0);
		}
	}
}

uint32_t ssozumo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void ssozumo_state::machine_start()
{
	save_item(NAME(m_sound_nmi_mask));
}


void ssozumo_state::main_map(address_map &map)
{
	map(0x0000, 0x077f).ram();
	map(0x0780, 0x07ff).ram().share(m_spriteram);
	map(0x2000, 0x23ff).ram().w(FUNC(ssozumo_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x2400, 0x27ff).ram().w(FUNC(ssozumo_state::colorram_w<1>)).share(m_colorram[1]);
	map(0x3000, 0x31ff).ram().w(FUNC(ssozumo_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x3200, 0x33ff).ram().w(FUNC(ssozumo_state::colorram_w<0>)).share(m_colorram[0]);
	map(0x3400, 0x35ff).ram();
	map(0x3600, 0x37ff).ram();
	map(0x4000, 0x4000).portr("P1").w(FUNC(ssozumo_state::flipscreen_w));
	map(0x4010, 0x4010).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x4020, 0x4020).portr("DSW2").w(FUNC(ssozumo_state::scroll_w));
	map(0x4030, 0x4030).portr("DSW1");
//  map(0x4030, 0x4030).writeonly();
	map(0x4050, 0x407f).ram().w(FUNC(ssozumo_state::paletteram_w)).share("paletteram");
	map(0x6000, 0xffff).rom();
}


void ssozumo_state::sound_nmi_mask_w(uint8_t data)
{
	m_sound_nmi_mask = data & 1;
}

// Same as Tag Team
void ssozumo_state::sound_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x2000, 0x2001).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x2002, 0x2003).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x2004, 0x2004).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2005, 0x2005).w(FUNC(ssozumo_state::sound_nmi_mask_w));
	map(0x2007, 0x2007).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4000, 0xffff).rom();
}

INPUT_CHANGED_MEMBER(ssozumo_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( ssozumo )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, ssozumo_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, ssozumo_state, coin_inserted, 0)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Dual ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	16,16,  // 16*16 tiles
	256,    // 256 tiles
	3,      // 3 bits per pixel
	{ 2*256*16*16, 256*16*16, 0 },  // the bitplanes are separated
	{ 16*8 + 0, 16*8 + 1, 16*8 + 2, 16*8 + 3, 16*8 + 4, 16*8 + 5, 16*8 + 6, 16*8 + 7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8        // every tile takes 16 consecutive bytes
};


static const gfx_layout spritelayout =
{
	16,16,      // 16*16 sprites
	1280,       // 1280 sprites
	3,      // 3 bits per pixel
	{ 2*1280*16*16, 1280*16*16, 0 },    // the bitplanes are separated
	{ 16*8 + 0, 16*8 + 1, 16*8 + 2, 16*8 + 3, 16*8 + 4, 16*8 + 5, 16*8 + 6, 16*8 + 7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8        // every sprite takes 16 consecutive bytes
};


static GFXDECODE_START( gfx_ssozumo )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x3_planar, 0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayout,     4*8, 4 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   8*8, 2 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(ssozumo_state::sound_timer_irq)
{
	if (m_sound_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void ssozumo_state::ssozumo(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 1'200'000); // 1.2 MHz ????
	m_maincpu->set_addrmap(AS_PROGRAM, &ssozumo_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(ssozumo_state::irq0_line_hold));

	M6502(config, m_audiocpu, 975'000);         // 975 kHz ??
	m_audiocpu->set_addrmap(AS_PROGRAM, &ssozumo_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(ssozumo_state::sound_timer_irq), attotime::from_hz(272 / 16 * 57)); // guess, assume to be the same as tagteam

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8 - 1, 1*8, 31*8 - 1);
	// DECO video CRTC, unverified
	screen.set_raw(XTAL(12'000'000) / 2, 384, 0, 256, 272, 8, 248);
	screen.set_screen_update(FUNC(ssozumo_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ssozumo);
	PALETTE(config, m_palette, FUNC(ssozumo_state::palette), 64 + 16);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	YM2149(config, "ay1", 1'500'000).add_route(ALL_OUTPUTS, "speaker", 0.3);
	YM2149(config, "ay2", 1'500'000).add_route(ALL_OUTPUTS, "speaker", 0.3);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.3); // unknown DAC
}



ROM_START( ssozumo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic61.g01",   0x06000, 0x2000, CRC(86968f46) SHA1(6acd111b71fbb4ef00ae03be4fb93d305a6564e7) ) // m1
	ROM_LOAD( "ic60.g11",   0x08000, 0x2000, CRC(1a5143dd) SHA1(19e36afcd0827f14f4360b55d952cc1af38327fd) ) // m2
	ROM_LOAD( "ic59.g21",   0x0a000, 0x2000, CRC(d3df04d7) SHA1(a95cff7f67ad2a3dbf7147018889a0de3f9fcbac) ) // m3
	ROM_LOAD( "ic58.g31",   0x0c000, 0x2000, CRC(0ee43a78) SHA1(383a29a2dfdbd600dacf3885039759efab718a45) ) // m4
	ROM_LOAD( "ic57.g41",   0x0e000, 0x2000, CRC(ac77aa4c) SHA1(36ee826327e4433bcdcb8d770fc6176f53d3eed0) ) // m5

	ROM_REGION( 0x10000, "audiocpu", 0 ) // program & voice samples
	ROM_LOAD( "ic47.g50",   0x04000, 0x2000, CRC(b64ec829) SHA1(684f1c37c05fc3812f11e040fb96789c8abb987f) ) // a1
	ROM_LOAD( "ic46.g60",   0x06000, 0x2000, CRC(630d7380) SHA1(aab3f034417a9712c8fa922946eda02751c9e319) ) // a2
	ROM_LOAD( "ic45.g70",   0x08000, 0x2000, CRC(1854b657) SHA1(c4f3c24a2b03bdf4d9fd80d6df944a157f98e617) ) // a3
	ROM_LOAD( "ic44.g80",   0x0a000, 0x2000, CRC(40b9a0da) SHA1(ef51977d23e14fb638b26afcb2617933446d8143) ) // a4
	ROM_LOAD( "ic43.g90",   0x0c000, 0x2000, CRC(20262064) SHA1(2845efa458f4fd873b8559489bcee4b9d8e437c1) ) // a5
	ROM_LOAD( "ic42.ga0",   0x0e000, 0x2000, CRC(98d7e998) SHA1(16bb3315db7d52531a3297e1255478aa1ebc32c2) ) // a6

	ROM_REGION( 0x06000, "fgtiles", 0 )
	ROM_LOAD( "ic22.gq0",   0x00000, 0x2000, CRC(b4c7e612) SHA1(2d4f6f79b65aa27e00f173777959ec07e81ff15e) ) // c1
	ROM_LOAD( "ic23.gr0",   0x02000, 0x2000, CRC(90bb9fda) SHA1(9c065a54330133e5afadcb2ae29add5e1005d977) ) // c2
	ROM_LOAD( "ic21.gs0",   0x04000, 0x2000, CRC(d8cd5c78) SHA1(f1567850db649d2b7a029a5f71bbade25bb0393f) ) // c3

	ROM_REGION( 0x06000, "bgtiles", 0 )
	ROM_LOAD( "ic69.gt0",   0x00000, 0x2000, CRC(771116ca) SHA1(2d1c656315f57e1a142725e2d2034543cb3917ea) ) // t1
	ROM_LOAD( "ic59.gu0",   0x02000, 0x2000, CRC(68035bfd) SHA1(da535ff6860f71c1780d4d9dfd1944e355234c5b) ) // t2
	ROM_LOAD( "ic81.gv0",   0x04000, 0x2000, CRC(cdda1f9f) SHA1(d1f1b3e0578fd991c74d4a85313c5d37f08f1eee) ) // t3

	ROM_REGION( 0x1e000, "sprites", 0 )
	ROM_LOAD( "ic06.gg0",   0x00000, 0x2000, CRC(d2342c50) SHA1(f502b716d659d9fd3119dbb454296fe9e280fa5d) ) // s1a
	ROM_LOAD( "ic05.gh0",   0x02000, 0x2000, CRC(14a3cb10) SHA1(7b6d63f43ebbe3c3aea7f2e04789cdb78cdd8495) ) // s1b
	ROM_LOAD( "ic04.gi0",   0x04000, 0x2000, CRC(169276c1) SHA1(7f0b54425e0f82f7fcc892d7b8e7719087060d2a) ) // s1c
	ROM_LOAD( "ic03.gj0",   0x06000, 0x2000, CRC(e71b9f28) SHA1(1f4f1a4d44fecb212778bb191e14bbfdc41556a5) ) // s1d
	ROM_LOAD( "ic02.gk0",   0x08000, 0x2000, CRC(6e94773c) SHA1(c3a1b950c1abce7103e6a0c19b5bc47a46612b05) ) // s1e
	ROM_LOAD( "ic29.gl0",   0x0a000, 0x2000, CRC(40f67cc4) SHA1(fb6cfa9c9665c719926fc6ef050682f040852840) ) // s2a
	ROM_LOAD( "ic28.gm0",   0x0c000, 0x2000, CRC(8c97b1a2) SHA1(72ca28959b532f98e0836a9650bb3dd3fdfa755a) ) // s2b
	ROM_LOAD( "ic27.gn0",   0x0e000, 0x2000, CRC(be8bb3dd) SHA1(d032591e73b09e2f076a18298d606edf16998a64) ) // s2c
	ROM_LOAD( "ic26.go0",   0x10000, 0x2000, CRC(9c098a2c) SHA1(d2093f1a4f4b3bf3bbff0adea5bd910993ed4704) ) // s2d
	ROM_LOAD( "ic25.gp0",   0x12000, 0x2000, CRC(f73f8a76) SHA1(13652779d3d30de0b4136eb3f43ee5429861bf35) ) // s2e
	ROM_LOAD( "ic44.gb0",   0x14000, 0x2000, CRC(cdd7f2eb) SHA1(57cf788804f9d2a1283032c25b608ac45064eddb) ) // s3a
	ROM_LOAD( "ic43.gc0",   0x16000, 0x2000, CRC(7b4c632e) SHA1(2acb0f2213928b97fdf239fbabc6d24329cbdd7a) ) // s3b
	ROM_LOAD( "ic42.gd0",   0x18000, 0x2000, CRC(cd1c8fe6) SHA1(ac085a0e8e228ea6bfbe86f209be08221bb066ee) ) // s3c
	ROM_LOAD( "ic41.ge0",   0x1a000, 0x2000, CRC(935578d0) SHA1(e9a9f439e0781627df076c454b16f5796ac991bc) ) // s3d
	ROM_LOAD( "ic40.gf0",   0x1c000, 0x2000, CRC(5a3bf1ba) SHA1(6beebb7ac9c8baa3bbb5b0ebf6a6da768e52d1d3) ) // s3e

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "ic33.gz0",   0x00000, 0x0020, CRC(523d29ad) SHA1(48d0ae83a07e4409a1def56772c5156e8d505749) ) // char palette red and green components
	ROM_LOAD( "ic30.gz2",   0x00020, 0x0020, CRC(0de202e1) SHA1(ca1aa66c1d3d4724d322ec0346860c37729ddaed) ) // tile palette red and green components
	ROM_LOAD( "ic32.gz1",   0x00040, 0x0020, CRC(6fbff4d2) SHA1(b2cd38fa8e9a74539b96d6e8e0375fff2dd77a20) ) // char palette blue component
	ROM_LOAD( "ic31.gz3",   0x00060, 0x0020, CRC(18e7fe63) SHA1(b0834b94b22ead765ddac5591ab1dc66ec20f17f) ) // tile palette blue component
ROM_END

} // anonymous namespace


GAME( 1984, ssozumo, 0, ssozumo, ssozumo, ssozumo_state, empty_init, ROT270, "Technos Japan", "Syusse Oozumou (Japan)", MACHINE_SUPPORTS_SAVE )
