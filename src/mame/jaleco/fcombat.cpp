// license:BSD-3-Clause
// copyright-holders: David Haywood, Tomasz Slanina

/* Field Combat (c)1985 Jaleco

Hardware has similarities with Exerion

TS 2004.10.22.
- fixed sprite issues
- added backgrounds and terrain info (external ROMs)

(press buttons 1+2 at the same time, to release 'army' ;)

TODO:
- verify sprite colors, not many PCB references online, but comparing with
  the small amount of photos that are there, they match


PCB Notes:

From a working board.

Label: FC8524
CPU: Z80 (running at 3.332 MHz measured at pin 6)
Sound: Z80 (running at 3.332 MHz measured at pin 6), YM2149 (x3)
Other: Unmarked 24 pin near ROMs 2 & 3 (maybe same protection chip as dday?)

RAM: 6116 (x3)

X-TAL: 20 MHz

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class fcombat_state : public driver_device
{
public:
	fcombat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bgdata_rom(*this, "bgdata"),
		m_terrain_rom(*this, "terrain_info"),
		m_inputs(*this, "IN%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void fcombat(machine_config &config);

	void init_fcombat();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_spriteram;
	required_region_ptr<u8> m_bgdata_rom;
	required_region_ptr<u8> m_terrain_rom;
	required_ioport_array<3> m_inputs;

	// video-related
	tilemap_t *m_bgmap = nullptr;
	tilemap_t *m_fgmap = nullptr;
	u8 m_cocktail_flip = 0;
	u8 m_char_palette = 0;
	u8 m_char_bank = 0;
	u8 m_sprite_bank = 0;

	// misc
	u8 m_fcombat_sh = 0;
	u16 m_fcombat_sv = 0;
	u8 m_tx = 0;
	u8 m_ty = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	u8 protection_r();
	u8 port01_r();
	void e900_w(u8 data);
	void ea00_w(u8 data);
	void eb00_w(u8 data);
	void ec00_w(u8 data);
	void ed00_w(u8 data);
	u8 e300_r();
	void ee00_w(u8 data);
	void videoreg_w(u8 data);
	void videoram_w(offs_t offset, u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void fcombat_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


// this is copied from Exerion, but it should be correct
static constexpr XTAL MASTER_CLOCK = 20_MHz_XTAL;
static constexpr XTAL CPU_CLOCK    = MASTER_CLOCK / 6;
static constexpr XTAL AY8910_CLOCK = CPU_CLOCK / 2;
static constexpr XTAL PIXEL_CLOCK  = MASTER_CLOCK / 3;
static constexpr int HCOUNT_START  = 0x58;
static constexpr int HTOTAL        = 512 - HCOUNT_START;
static constexpr int HBEND         = 12 * 8;  // ??
static constexpr int HBSTART       = 52 * 8;  // ??
static constexpr int VTOTAL        = 256;
static constexpr int VBEND         = 16;
static constexpr int VBSTART       = 240;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void fcombat_state::fcombat_palette(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// fg chars/sprites
	for (int i = 0; i < 0x200; i++)
	{
		const u8 ctabentry = (color_prom[(i & 0x1c0) | bitswap<6>(i,1,0,5,4,3,2)] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	// bg chars
	for (int i = 0x200; i < 0x300; i++)
	{
		const u8 ctabentry = color_prom[(i & 0x3c0) | bitswap<6>(i,1,0,5,4,3,2)] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}


/*************************************
 *
 *  Video system startup
 *
 *************************************/

void fcombat_state::video_start()
{
	m_fgmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fcombat_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fgmap->set_transparent_pen(0);

	m_bgmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fcombat_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32 * 8 * 2, 32);
}

TILE_GET_INFO_MEMBER(fcombat_state::get_bg_tile_info)
{
	const int tileno = m_bgdata_rom[tile_index];
	const int palno = (tileno >> 2 & 3) | (tileno >> 4 & 0xc);

	tileinfo.set(2, tileno, palno, 0);
}

TILE_GET_INFO_MEMBER(fcombat_state::get_fg_tile_info)
{
	const int tileno = m_videoram[tile_index] | (m_char_bank << 8);
	const int palno = (tileno >> 4 & 0xf) | (m_char_palette << 4);

	tileinfo.set(0, tileno, palno, 0);
}

void fcombat_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fgmap->mark_tile_dirty(offset);
}


/*************************************
 *
 *  Video register I/O
 *
 *************************************/

void fcombat_state::videoreg_w(u8 data)
{
	// bit 0: flip screen and joystick input multiplexer
	m_cocktail_flip = data & 1;
	flip_screen_set(m_cocktail_flip);

	// bits 1-2: char lookup table bank
	// bit 3: char bank
	u8 char_pal = (data & 0x06) >> 1;
	u8 char_bank = (data & 0x08) >> 3;
	if (m_char_palette != char_pal || m_char_bank != char_bank)
	{
		m_char_palette = char_pal;
		m_char_bank = char_bank;
		m_fgmap->mark_all_dirty();
	}

	// bits 4-5: unused

	// bits 6-7: heli sprite block bank
	m_sprite_bank = data >> 6 & 3;
}


u32 fcombat_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw background
	m_bgmap->set_scrolly(0, m_fcombat_sh);
	if (m_cocktail_flip)
		m_bgmap->set_scrollx(0, m_fcombat_sv + 8 - 2);
	else
		m_bgmap->set_scrollx(0, m_fcombat_sv - 8);

	m_bgmap->draw(screen, bitmap, cliprect);

	// draw sprites
	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		const int flags = m_spriteram[i + 0];
		int y = 256 - m_spriteram[i + 1];
		int code = m_spriteram[i + 2] + ((flags & 0x20) << 3);
		int x = m_spriteram[i + 3] * 2 + (flags & 0x01) + 52;

		int xflip = flags & 0x80;
		int yflip = flags & 0x40;

		const int color = ((flags >> 1) & 0x03) | ((code >> 5) & 0x04) | (code & 0x08) | (code >> 4 & 0x10);
		gfx_element *gfx = m_gfxdecode->gfx(1);

		if ((code & 0x108) == 0x108)
		{
			static constexpr int mask[4] = { 0x308, 0x300, 0x08, 0x00 };
			code ^= mask[m_sprite_bank];
		}

		if (m_cocktail_flip)
		{
			x = 64 * 8 - gfx->width() - x + 2;
			y = 32 * 8 - gfx->height() - y + 2;
			xflip = !xflip;
			yflip = !yflip;
		}

		switch (flags >> 3 & 3)
		{
			// 16x16
			case 0:
				gfx->transpen(bitmap, cliprect, code, color, xflip, yflip, x, y, 0);
				break;

			// 16x32
			case 1:
				if (m_cocktail_flip)
					y -= 16;
				code &= ~0x10;

				gfx->transpen(bitmap, cliprect, code | (yflip ? 16 : 0), color, xflip, yflip, x, y, 0);
				gfx->transpen(bitmap, cliprect, code | (yflip ? 0 : 16), color, xflip, yflip, x, y + 16, 0);
				break;

			// 16x64
			case 2:
				if (m_cocktail_flip)
					y -= 48;
				code &= ~0x30;

				for (int j = 0; j < 4; j++)
				{
					int co = 16 * (j ^ (yflip ? 3 : 0));
					gfx->transpen(bitmap, cliprect, code | co, color, xflip, yflip, x, y + j * 16, 0);
				}
				break;

			// unused
			case 3:
				break;
		}
	}

	// draw text layer
	m_fgmap->draw(screen, bitmap, cliprect);

	return 0;
}


INPUT_CHANGED_MEMBER(fcombat_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


// is it protection?
u8 fcombat_state::protection_r()
{
	/* Must match ONE of these values after a "and  $3E" instruction :

	    76F0: 1E 04 2E 26 34 32 3A 16 3E 36

	   Check code at 0x76c8 for more infos.
	*/
	return 0xff;    // seems enough
}


// same as exerion again

u8 fcombat_state::port01_r()
{
	// the cocktail flip bit muxes between ports 0 and 1
	u8 start = m_inputs[0]->read() & 0xc0;
	return (m_inputs[(m_cocktail_flip & 1) + 1]->read() & 0x3f) | start;
}


// bg scrolls

void fcombat_state::e900_w(u8 data)
{
	m_fcombat_sh = data;
}

void fcombat_state::ea00_w(u8 data)
{
	m_fcombat_sv = (m_fcombat_sv & 0xff00) | data;
}

void fcombat_state::eb00_w(u8 data)
{
	m_fcombat_sv = (m_fcombat_sv & 0xff) | (data << 8);
}


// terrain info (ec00=x, ed00=y, return val in e300)

void fcombat_state::ec00_w(u8 data)
{
	m_tx = data;
}

void fcombat_state::ed00_w(u8 data)
{
	m_ty = data;
}

u8 fcombat_state::e300_r()
{
	const int wx = (m_tx + m_fcombat_sh) / 16;
	const int wy = (m_ty * 2 + m_fcombat_sv) / 16;

	return m_terrain_rom[wx * 32 * 16 + wy];
}

void fcombat_state::ee00_w(u8 data)
{
	// related to protection ? - doesn't seem to have any effect
}

void fcombat_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd7ff).ram().share(m_videoram).w(FUNC(fcombat_state::videoram_w));
	map(0xd800, 0xd8ff).ram().share(m_spriteram);
	map(0xe000, 0xe000).r(FUNC(fcombat_state::port01_r));
	map(0xe100, 0xe100).portr("DSW0");
	map(0xe200, 0xe200).portr("DSW1");
	map(0xe300, 0xe300).r(FUNC(fcombat_state::e300_r));
	map(0xe400, 0xe400).r(FUNC(fcombat_state::protection_r));
	map(0xe800, 0xe800).w(FUNC(fcombat_state::videoreg_w));
	map(0xe900, 0xe900).w(FUNC(fcombat_state::e900_w));
	map(0xea00, 0xea00).w(FUNC(fcombat_state::ea00_w));
	map(0xeb00, 0xeb00).w(FUNC(fcombat_state::eb00_w));
	map(0xec00, 0xec00).w(FUNC(fcombat_state::ec00_w));
	map(0xed00, 0xed00).w(FUNC(fcombat_state::ed00_w));
	map(0xee00, 0xee00).w(FUNC(fcombat_state::ee00_w));
	map(0xef00, 0xef00).w("soundlatch", FUNC(generic_latch_8_device::write));
}


void fcombat_state::audio_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8001, 0x8001).r("ay1", FUNC(ym2149_device::data_r));
	map(0x8002, 0x8003).w("ay1", FUNC(ym2149_device::data_address_w));
	map(0xa001, 0xa001).r("ay2", FUNC(ym2149_device::data_r));
	map(0xa002, 0xa003).w("ay2", FUNC(ym2149_device::data_address_w));
	map(0xc001, 0xc001).r("ay3", FUNC(ym2149_device::data_r));
	map(0xc002, 0xc003).w("ay3", FUNC(ym2149_device::data_address_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( fcombat )
	PORT_START("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")      // player 1 inputs (muxed on 0xe000)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")      // player 2 inputs (muxed on 0xe000)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW0")      // dip switches (0xe100)
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x07, "Infinite (Cheat)")
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      // dip switches/VBLANK (0xe200)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fcombat_state, coin_inserted, 0)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0,4) },
	{ STEP4(3,-1), STEP4(4*2+3,-1) },
	{ STEP8(0,4*4) },
	16*8
};

// 16 x 16 sprites -- requires reorganizing characters in init_fcombat()
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0,4) },
	{ STEP4(3,-1), STEP4(4*2+3,-1), STEP4(4*4+3,-1), STEP4(4*6+3,-1) },
	{ STEP16(0,4*8) },
	64*8
};

static GFXDECODE_START( gfx_fcombat )
	GFXDECODE_ENTRY( "fgtiles", 0, charlayout,         0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     256, 64 )
	GFXDECODE_ENTRY( "bgtiles", 0, spritelayout,     512, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void fcombat_state::machine_start()
{
	save_item(NAME(m_cocktail_flip));
	save_item(NAME(m_char_palette));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_fcombat_sh));
	save_item(NAME(m_fcombat_sv));
	save_item(NAME(m_tx));
	save_item(NAME(m_ty));
}

void fcombat_state::fcombat(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &fcombat_state::main_map);

	z80_device &audiocpu(Z80(config, "audiocpu", CPU_CLOCK));
	audiocpu.set_addrmap(AS_PROGRAM, &fcombat_state::audio_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(fcombat_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fcombat);
	PALETTE(config, m_palette, FUNC(fcombat_state::fcombat_palette), 256 * 3, 32);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2149(config, "ay1", AY8910_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.25);
	YM2149(config, "ay2", AY8910_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.25);
	YM2149(config, "ay3", AY8910_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.25);
}


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void fcombat_state::init_fcombat()
{
	// allocate some temporary space
	std::vector<u8> temp(0x10000);

	// make a temporary copy of the character data
	u8 *src = &temp[0];
	u8 *dst = memregion("fgtiles")->base();
	u32 length = memregion("fgtiles")->bytes();
	memcpy(src, dst, length);

	/* decode the characters
	   the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2
	   we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2 */

	for (u32 oldaddr = 0; oldaddr < length; oldaddr++)
	{
		u32 newaddr = ((oldaddr     ) & 0x1f00) |       // keep n8-n4
					  ((oldaddr << 3) & 0x00f0) |       // move n3-n0
					  ((oldaddr >> 4) & 0x000e) |       // move v2-v0
					  ((oldaddr     ) & 0x0001);        // keep h2
		dst[newaddr] = src[oldaddr];
	}

	// make a temporary copy of the sprite data
	src = &temp[0];
	dst = memregion("sprites")->base();
	length = memregion("sprites")->bytes();
	memcpy(src, dst, length);

	/* decode the sprites
	   the bits in the ROMs are ordered: n9 n8 n3 n7-n6 n5 n4 v3-v2 v1 v0 n2-n1 n0 h3 h2
	   we want them ordered like this:   n9 n8 n7 n6-n5 n4 n3 n2-n1 n0 v3 v2-v1 v0 h3 h2 */

	for (u32 oldaddr = 0; oldaddr < length; oldaddr++)
	{
		u32 newaddr = ((oldaddr << 1) & 0x3c00) |       // move n7-n4
					  ((oldaddr >> 4) & 0x0200) |       // move n3
					  ((oldaddr << 4) & 0x01c0) |       // move n2-n0
					  ((oldaddr >> 3) & 0x003c) |       // move v3-v0
					  ((oldaddr     ) & 0xc003);        // keep n9-n8 h3-h2

		dst[newaddr] = src[oldaddr];
	}

	// make a temporary copy of the character data
	src = &temp[0];
	dst = memregion("bgtiles")->base();
	length = memregion("bgtiles")->bytes();
	memcpy(src, dst, length);

	/* decode the characters
	   the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2
	   we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2 */

	for (u32 oldaddr = 0; oldaddr < length; oldaddr++)
	{
		u32 newaddr = ((oldaddr << 1) & 0x3c00) |       // move n7-n4
					  ((oldaddr >> 4) & 0x0200) |       // move n3
					  ((oldaddr << 4) & 0x01c0) |       // move n2-n0
					  ((oldaddr >> 3) & 0x003c) |       // move v3-v0
					  ((oldaddr     ) & 0xc003);        // keep n9-n8 h3-h2
		dst[newaddr] = src[oldaddr];
	}

	src = &temp[0];
	dst = memregion("bgdata")->base();
	length = memregion("bgdata")->bytes();
	memcpy(src, dst, length);

	for (u32 oldaddr = 0; oldaddr < 32; oldaddr++)
	{
		memcpy(&dst[oldaddr * 32 * 8 * 2], &src[oldaddr * 32 * 8], 32 * 8);
		memcpy(&dst[oldaddr * 32 * 8 * 2 + 32 * 8], &src[oldaddr * 32 * 8 + 0x2000], 32 * 8);
	}

	src = &temp[0];
	dst = memregion("terrain_info")->base();
	length = memregion("terrain_info")->bytes();
	memcpy(src, dst, length);

	for (u32 oldaddr = 0; oldaddr < 32; oldaddr++)
	{
		memcpy(&dst[oldaddr * 32 * 8 * 2], &src[oldaddr * 32 * 8], 32 * 8);
		memcpy(&dst[oldaddr * 32 * 8 * 2 + 32 * 8], &src[oldaddr * 32 * 8 + 0x2000], 32 * 8);
	}
}

ROM_START( fcombat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fcombat2.t9",  0x0000, 0x4000, CRC(30cb0c14) SHA1(8b5b6a4efaca2f138709184725e9e0e0b9cfc4c7) )
	ROM_LOAD( "fcombat3.10t", 0x4000, 0x4000, CRC(e8511da0) SHA1(bab5c9244c970b97c025381c37ad372aa3b5cddf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fcombat1.t5",  0x0000, 0x4000, CRC(a0cc1216) SHA1(3a8963ffde2ff4a3f428369133f94bb37717cae5) )

	ROM_REGION( 0x02000, "fgtiles", 0 )
	ROM_LOAD( "fcombat7.l11", 0x00000, 0x2000, CRC(401061b5) SHA1(09dd23e86a56db8021e14432aced0eaf013fefe2) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "fcombat8.d10", 0x00000, 0x4000, CRC(e810941e) SHA1(19ae85af0bf245caf3afe10d65e618cfb47d33c2) )
	ROM_LOAD( "fcombat9.d11", 0x04000, 0x4000, CRC(f95988e6) SHA1(25876652decca7ec1e9b37a16536c15ca2d1cb12) )
	ROM_LOAD( "fcomba10.d12", 0x08000, 0x4000, CRC(908f154c) SHA1(b3761ee60d4a5ea36376759875105d23c57b4bf2) )

	ROM_REGION( 0x04000, "bgtiles", 0 )
	ROM_LOAD( "fcombat6.f3",  0x00000, 0x4000, CRC(97282729) SHA1(72db0593551c2d15631341bf621b96013b46ce72) )

	ROM_REGION( 0x04000, "bgdata", 0 )
	ROM_LOAD( "fcombat5.l3",  0x00000, 0x4000, CRC(96194ca7) SHA1(087d6ac8f93f087cb5e378dbe9a8cfcffa2cdddc) )

	ROM_REGION( 0x04000, "terrain_info", 0 )
	ROM_LOAD( "fcombat4.p3",  0x00000, 0x4000, CRC(efe098ab) SHA1(fe64a5e9170835d242368109b1b221b0f8090e7e) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "fcprom_a.c2",  0x0000, 0x0020, CRC(7ac480f0) SHA1(f491fe4da19d8c037e3733a5836de35cc438907e) ) // palette
	ROM_LOAD( "fcprom_d.k12", 0x0020, 0x0100, CRC(9a348250) SHA1(faf8db4c42adee07795d06bea20704f8c51090ff) ) // fg char lookup table
	ROM_LOAD( "fcprom_c.a9",  0x0120, 0x0100, CRC(768ac120) SHA1(ceede1d6cbeae08da96ef52bdca2718a839d88ab) ) // sprite lookup table
	ROM_LOAD( "fcprom_b.c4",  0x0220, 0x0100, CRC(ac9049f6) SHA1(57aa5b5df3e181bad76149745a422c3dd1edad49) ) // bg char lookup table
ROM_END

} // anonymous namespace


GAME( 1985, fcombat, 0, fcombat, fcombat, fcombat_state, init_fcombat, ROT90, "Jaleco", "Field Combat", MACHINE_SUPPORTS_SAVE )
