// license:BSD-3-Clause
// copyright-holders: Allard van der Bas

/***************************************************************************

Shaolin's Road

driver by Allard van der Bas

****************************************************************************

Shaolin's Road & Kicker hardware info by Guru

This is a single screen platform beat-em up game using an 8-way joystick and 2 buttons.
Based on the soldered-in PROM labels, Shaolin's Road came first (likely the Japan release)
and Kicker was probably the later US/World release.
Up to two speakers are connected in the cabinet but the output is mono/dual mono.

KONAMI GX477 PWB 200214B
|----------------------------------------------------------|
|  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15  16  A|
|                         083   477K07  477J11             |
|    LA4460           477J09 477K06  477J10 477J12         |
|                                                    502  B|
|                                                          |
|       VOLUME                                            C|
|                                                          |
|1                       477L03                            |
|8                          477L04              2148 2148 D|
|w                            477L05                       |
|A                 6809                                   E|
|Y           76489                                         |
|                                                          |
|            76489           2009 2009    085      477J08 F|
|                                                          |
|                                                          |
|TA7900S        18.432MHz                                 G|
|          DIPSW2                                          |
|      DIPSW1  DIPSW3                       477K01     083 |
|    CN1         504   082           2016 503   477K02    H|
|----------------------------------------------------------|
Notes:
      TA7900S  - Toshiba TA7900S 5V Reference & Watchdog Timer
      LA4460   - Sanyo LA4460 12W Power Amplifier IC
      477J*    - 82S129 256 x4-bit Bi-Polar PROM, compatible with MMI6301, TBP24S10, N74S287, MB7052 etc
      76489    - Texas Instruments SN76489 Complex Digital Sound Generator (x2). Clock inputs are 1.536MHz and 3.072MHz
      CN1      - 4-pin connector for monitors that require separate syncs. Pin 1 is negative vertical sync output.
                 VSync measures 60.6060Hz *exactly*. Horizontal sync is taken from the regular composite sync on the PCB on pin B14.
      DIPSW1/2 - 8-position DIP switch
      DIPSW3   - 4-position DIP switch
      2016     - TMM2016 2k x8-bit SRAM
      2009     - TMM2009. This RAM is completely undocumented (even in Toshiba Databooks of the era) but the
                 schematic shows it has a pinout that matches TMM2016/6116 2k x8-bit SRAM
      2148     - Fujitsu MBM2148 1k x4-bit SRAM
      6809     - Clock 1.536MHz [18.432/12]
      ROMs     - 477L03(2764), 477L04(27128), 477L05(27128) - Main Program
                 477J01(27128), 477J02(27128) - Sprites
                 477J06(2764), 477J07(2764) - Characters/Backgrounds etc
                 477J08(82S129) - Sprite Lookup Table
                 477J09(82S129) - Character Lookup Table
                 477J10(82S129) - Red PROM
                 477J11(82S129) - Green PROM
                 477J12(82S129) - Blue PROM
  Custom Chips - 085, 083, 083, 082, 502, 503, 504
                 Sometimes the custom chips are replaced by a daughterboard containing common logic chips.
                 These have been seen:
                                      PWB4000231 replaces 502
                                      KC001 replaces 503
                                      PWB4000206A replaces 504
                                      300381 replaces 082

Shaolin's Road / Kicker 18-WAY PCB Edge Connector Pinout
--------------------------------------------------------
Note this is the standard Konami pinout used in many
Konami games from this era with some pins not used.

---------------+-----+-----+---------------
   Solder Side |     |     | Parts Side
---------------+-----+-----+---------------
-5V (not used) | A1  | B1  | +12V
       Speaker | A2  | B2  | Speaker
   2P Button 2 | A3  | B3  | 2P Btn 1
       2P Left | A4  | B4  | 2P Right
      1P Start | A5  | B5  | 2P Start
      1P Btn 1 | A6  | B6  | 2P Up
      1P Btn 2 | A7  | B7  | Service
      1P Right | A8  | B8  | 1P Left
         1P Up | A9  | B9  | 2P Down
        Coin 1 | A10 | B10 | Coin 2
       1P Down | A11 | B11 | Coin Counter 1
    (not used) | A12 | B12 | Coin Counter 2
   Video Green | A13 | B13 | Video Blue
     Video Red | A14 | B14 | Video Sync
    (not used) | A15 | B15 | (not used)
           GND | A16 | B16 | GND
           GND | A17 | B17 | GND
           +5V | A18 | B18 | +5V
---------------+-----+-----+---------------

****************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class shaolins_state : public driver_device
{
public:
	shaolins_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_screen(*this,"screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram")
	{ }

	void shaolins(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	uint8_t m_palettebank = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_nmi_enable = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void palettebank_w(uint8_t data);
	void scroll_w(uint8_t data);
	void nmi_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void prg_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Shao-lin's Road has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
void shaolins_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 2200, 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i | 0x000], 0);
		bit1 = BIT(color_prom[i | 0x000], 1);
		bit2 = BIT(color_prom[i | 0x000], 2);
		bit3 = BIT(color_prom[i | 0x000], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i | 0x100], 0);
		bit1 = BIT(color_prom[i | 0x100], 1);
		bit2 = BIT(color_prom[i | 0x100], 2);
		bit3 = BIT(color_prom[i | 0x100], 3);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i | 0x200], 0);
		bit1 = BIT(color_prom[i | 0x200], 1);
		bit2 = BIT(color_prom[i | 0x200], 2);
		bit3 = BIT(color_prom[i | 0x200], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table,
	color_prom += 0x300;

	// characters use colors 0x10-0x1f of each 0x20 color bank, while sprites use colors 0-0x0f
	for (int i = 0; i < 0x200; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t const ctabentry = (j << 5) | ((~i & 0x100) >> 4) | (color_prom[i] & 0x0f);
			palette.set_pen_indirect(((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

void shaolins_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void shaolins_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void shaolins_state::palettebank_w(uint8_t data)
{
	if (m_palettebank != (data & 0x07))
	{
		m_palettebank = data & 0x07;
		machine().tilemap().mark_all_dirty();
	}
}

void shaolins_state::scroll_w(uint8_t data)
{
	for (int col = 4; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, data + 1);
}

void shaolins_state::nmi_w(uint8_t data)
{
	m_nmi_enable = data;

	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);
}

TILE_GET_INFO_MEMBER(shaolins_state::get_bg_tile_info)
{
	int const attr = m_colorram[tile_index];
	int const code = m_videoram[tile_index] + ((attr & 0x40) << 2);
	int const color = (attr & 0x0f) + 16 * m_palettebank;
	int const flags = (attr & 0x20) ? TILE_FLIPY : 0;

	tileinfo.set(0, code, color, flags);
}

void shaolins_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shaolins_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_palettebank));
	save_item(NAME(m_nmi_enable));
}

void shaolins_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 23 * 2; offs >= 0; offs -= 2) // max 24 sprites
	{
		int const code = m_spriteram[1][offs + 1];
		int const color = (m_spriteram[0][offs] & 0x0f) | (m_palettebank << 4);
		int const flipx = !(m_spriteram[0][offs] & 0x40);
		int const flipy = m_spriteram[0][offs] & 0x80;
		int const sx = m_spriteram[1][offs];
		int sy = 241 - m_spriteram[0][offs + 1];

		if (flip_screen())
			sy--;

		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, m_palettebank << 5));
	}
}

uint32_t shaolins_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(shaolins_state::interrupt)
{
	int const scanline = param;

	if (scanline == 240)
			m_maincpu->set_input_line(0, HOLD_LINE);
	else if ((scanline % 32) == 0)
		if (m_nmi_enable & 0x02)
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



void shaolins_state::prg_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(shaolins_state::nmi_w));   // bit 0 = flip screen, bit 1 = nmi enable, bit 2 = ?
														  // bit 3, bit 4 = coin counters */
	map(0x0100, 0x0100).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x0200, 0x02ff).lr8(NAME([this]() -> u8 { return m_screen->vpos(); }));
	map(0x0300, 0x0300).w("sn1", FUNC(sn76489a_device::write)); // trigger chip to read from latch. The program always
	map(0x0400, 0x0400).w("sn2", FUNC(sn76489a_device::write)); // writes the same number as the latch, so we don't
																// bother emulating them.
	map(0x0500, 0x0500).portr("DSW2");
	map(0x0600, 0x0600).portr("DSW3");
	map(0x0700, 0x0700).portr("SYSTEM");
	map(0x0701, 0x0701).portr("P1");
	map(0x0702, 0x0702).portr("P2");
	map(0x0703, 0x0703).portr("DSW1");
	map(0x0800, 0x0800).nopw();                    // latch for 76496 #0
	map(0x1000, 0x1000).nopw();                    // latch for 76496 #1
	map(0x1800, 0x1800).w(FUNC(shaolins_state::palettebank_w));
	map(0x2000, 0x2000).w(FUNC(shaolins_state::scroll_w));
	map(0x2800, 0x2bff).ram().share(m_spriteram[0]);  // RAM BANK 2
	map(0x3000, 0x33ff).ram().share(m_spriteram[1]); // RAM BANK 1
	map(0x3800, 0x3bff).ram().w(FUNC(shaolins_state::colorram_w)).share(m_colorram);
	map(0x3c00, 0x3fff).ram().w(FUNC(shaolins_state::videoram_w)).share(m_videoram);
	map(0x4000, 0x5fff).rom();                         // Machine checks for extra ROM
	map(0x6000, 0xffff).rom();
}


static INPUT_PORTS_START( shaolins )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DIPSW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DIPSW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )  // coin sound made but no credit given so effectively Coin B is disabled

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("DIPSW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DIPSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DIPSW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 and every 70000" )
	PORT_DIPSETTING(    0x10, "40000 and every 80000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DIPSW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DIPSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// This bank only has four switches
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DIPSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )          PORT_DIPLOCATION("DIPSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )                     PORT_DIPLOCATION("DIPSW3:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DIPSW3:4" )

INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8*8 chars
	512,    // 512 characters
	4,  // 4 bits per pixel
	{ 512*16*8+4, 512*16*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	256,    // 256 sprites
	4,  // 4 bits per pixel
	{ 256*64*8+4, 256*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    // every sprite takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_shaolins )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,         0, 16*8 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 16*8*16, 16*8 )
GFXDECODE_END


void shaolins_state::shaolins(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);

	// basic machine hardware
	MC6809E(config, m_maincpu, MASTER_CLOCK / 12);        // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &shaolins_state::prg_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(shaolins_state::interrupt), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
//  m_screen->set_size(32*8, 32*8);
//  m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	// Pixel clock is / 3 the master clock (6'144'000)
	// Refresh rate is 60.606060 Hz, with 40 vblank lines
	m_screen->set_raw(MASTER_CLOCK / 3, 384, 0, 256, 264, 16, 240);
	m_screen->set_screen_update(FUNC(shaolins_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shaolins);
	PALETTE(config, m_palette, FUNC(shaolins_state::palette), 16*8*16+16*8*16, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", MASTER_CLOCK / 12).add_route(ALL_OUTPUTS, "mono", 1.0);        // verified on PCB

	SN76489A(config, "sn2", MASTER_CLOCK / 6).add_route(ALL_OUTPUTS, "mono", 1.0);        // verified on PCB
}

#if 0 // a bootleg board was found with downgraded sound hardware, but is otherwise the same
void shaolins_state::shaolinb(machine_config &config)
{
	shaolins(config);

	SN76489(config.replace(), "sn1", MASTER_CLOCK / 12).add_route(ALL_OUTPUTS, "mono", 1.0); // only type verified on PCB

	SN76489(config.replace(), "sn2", MASTER_CLOCK / 6).add_route(ALL_OUTPUTS, "mono", 1.0);
}
#endif

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kicker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "477l03.d9",   0x6000, 0x2000, CRC(2598dfdd) SHA1(70a9d81b73bbd4ff6b627a3e4102d5328a946d20) )
	ROM_LOAD( "477l04.d10",  0x8000, 0x4000, CRC(0cf0351a) SHA1(a9da783b29a63a46912a29715e8d11dc4cd22265) )
	ROM_LOAD( "477l05.d11",  0xC000, 0x4000, CRC(654037f8) SHA1(52d098386fe87ae97d4dfefab0bd3a902f66d70b) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "477k06.a10",  0x0000, 0x2000, CRC(4d156afc) SHA1(29eb66e2ebcf2f1c1d5ece5413d1ebf54663f9cf) )
	ROM_LOAD( "477k07.a11",  0x2000, 0x2000, CRC(ff6ca5df) SHA1(dfcd445c8b233a0a4168eb249472e53784eda25d) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "477k02.h15",  0x0000, 0x4000, CRC(b94e645b) SHA1(65ae48134a0fe1e910a787714f7ae721734ded5b) )
	ROM_LOAD( "477k01.h14",  0x4000, 0x4000, CRC(61bbf797) SHA1(97d276099172975499f646f381a6fc587c022435) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "477j10.a12",   0x0000, 0x0100, CRC(b09db4b4) SHA1(d21176cdc7def760da109083eb52e5b6a515021f) ) // palette red component
	ROM_LOAD( "477j11.a13",   0x0100, 0x0100, CRC(270a2bf3) SHA1(c0aec04bd3bceccddf5f5a814a560a893b29ef6b) ) // palette green component
	ROM_LOAD( "477j12.a14",   0x0200, 0x0100, CRC(83e95ea8) SHA1(e0bfa20600488f5c66233e13ea6ad857f62acb7c) ) // palette blue component
	ROM_LOAD( "477j09.b8",    0x0300, 0x0100, CRC(aa900724) SHA1(c5343273d0a7101b8ba6876c4f22e43d77610c75) ) // character lookup table
	ROM_LOAD( "477j08.f16",   0x0400, 0x0100, CRC(80009cf5) SHA1(a367f3f55d75a9d5bf4d43f9d77272eb910a1344) ) // sprite lookup table
ROM_END

ROM_START( shaolins )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "477l03.d9",   0x6000, 0x2000, CRC(2598dfdd) SHA1(70a9d81b73bbd4ff6b627a3e4102d5328a946d20) )
	ROM_LOAD( "477l04.d10",  0x8000, 0x4000, CRC(0cf0351a) SHA1(a9da783b29a63a46912a29715e8d11dc4cd22265) )
	ROM_LOAD( "477l05.d11",  0xC000, 0x4000, CRC(654037f8) SHA1(52d098386fe87ae97d4dfefab0bd3a902f66d70b) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "477j06.a10", 0x0000, 0x2000, CRC(ff18a7ed) SHA1(f28bfeff84bb6a08a8bee999a0b7a19e09a8dfc3) )
	ROM_LOAD( "477j07.a11", 0x2000, 0x2000, CRC(5f53ae61) SHA1(ad29e2255855c503295c6b63eb4cd6700a1e3f0e) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "477j02.h15",  0x0000, 0x4000, CRC(b94e645b) SHA1(65ae48134a0fe1e910a787714f7ae721734ded5b) ) // actual labels are 477J02 but data matches Kicker 477K02
	ROM_LOAD( "477j01.h14",  0x4000, 0x4000, CRC(61bbf797) SHA1(97d276099172975499f646f381a6fc587c022435) ) // actual labels are 477J01 but data matches Kicker 477K01

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "477j10.a12",   0x0000, 0x0100, CRC(b09db4b4) SHA1(d21176cdc7def760da109083eb52e5b6a515021f) ) // palette red component
	ROM_LOAD( "477j11.a13",   0x0100, 0x0100, CRC(270a2bf3) SHA1(c0aec04bd3bceccddf5f5a814a560a893b29ef6b) ) // palette green component
	ROM_LOAD( "477j12.a14",   0x0200, 0x0100, CRC(83e95ea8) SHA1(e0bfa20600488f5c66233e13ea6ad857f62acb7c) ) // palette blue component
	ROM_LOAD( "477j09.b8",    0x0300, 0x0100, CRC(aa900724) SHA1(c5343273d0a7101b8ba6876c4f22e43d77610c75) ) // character lookup table
	ROM_LOAD( "477j08.f16",   0x0400, 0x0100, CRC(80009cf5) SHA1(a367f3f55d75a9d5bf4d43f9d77272eb910a1344) ) // sprite lookup table
ROM_END

/*
    Shao-lin's Road (Bootleg) - has also been found on an original board

    Main Board:    VWXYZ
    Daughterboard: QSTU (Replaces 3 custom Konami chips)
    Daughterboard: RSTU (Replaces 4 custom Konami chips)

    All the ROMs/PROMs are located on the main board.

    Board Layout with edge connector on the left.  ROMs/PROMs are marked too.
    PROM's have an asterisk suffix to distinguish them from the ROMs.

         A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
    |-------------------------------------------------|
    |                                                 |
    |                            6  7  3* 4* 5*       | 1
    |                                                 |
    |                      2*                         | 2
    |--|                                              |
       |                                              | 3
    |--|                                              |
    |                         3  4  5                 | 4
    |                                                 |
    |                                                 | 5
    |--|                                              |
       |                                           1* | 6
    |--|                                              |
    |                                                 | 7
    |                                                 |
    |                                        1  2     | 8
    |                                                 |
    |-------------------------------------------------|
*/

ROM_START( shaolinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.h4", 0x6000, 0x2000, CRC(2598dfdd) SHA1(70a9d81b73bbd4ff6b627a3e4102d5328a946d20) ) // 2764
	ROM_LOAD( "4.i4", 0x8000, 0x4000, CRC(0cf0351a) SHA1(a9da783b29a63a46912a29715e8d11dc4cd22265) ) // 27128
	ROM_LOAD( "5.j4", 0xC000, 0x4000, CRC(654037f8) SHA1(52d098386fe87ae97d4dfefab0bd3a902f66d70b) ) // 27128

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "6.i1", 0x0000, 0x2000, CRC(ff18a7ed) SHA1(f28bfeff84bb6a08a8bee999a0b7a19e09a8dfc3) ) // 2764
	ROM_LOAD( "7.j1", 0x2000, 0x4000, CRC(d9a7cff6) SHA1(47244426b9a674326c5303347112aa9d33bcf1df) ) // 27128

	ROM_REGION( 0x8000, "sprites", 0 ) // All ROMs are 27128
	ROM_LOAD( "2.m8", 0x0000, 0x4000, CRC(560521c7) SHA1(f8a50c66364995041e29ed7be2e4ea1ad16aa735) )
	ROM_LOAD( "1.l8", 0x4000, 0x4000, CRC(a79959b2) SHA1(9c58975c55f7be32add0dccb259d9680410fa9bc) )

	ROM_REGION( 0x0500, "proms", 0 ) // All PROMs are N82S129N
	ROM_LOAD( "3.k1", 0x0000, 0x0100, CRC(b09db4b4) SHA1(d21176cdc7def760da109083eb52e5b6a515021f) ) // palette red component
	ROM_LOAD( "4.l1", 0x0100, 0x0100, CRC(270a2bf3) SHA1(c0aec04bd3bceccddf5f5a814a560a893b29ef6b) ) // palette green component
	ROM_LOAD( "5.m1", 0x0200, 0x0100, CRC(83e95ea8) SHA1(e0bfa20600488f5c66233e13ea6ad857f62acb7c) ) // palette blue component
	ROM_LOAD( "2.g2", 0x0300, 0x0100, CRC(aa900724) SHA1(c5343273d0a7101b8ba6876c4f22e43d77610c75) ) // character lookup table
	ROM_LOAD( "1.o6", 0x0400, 0x0100, CRC(80009cf5) SHA1(a367f3f55d75a9d5bf4d43f9d77272eb910a1344) ) // sprite lookup table
ROM_END

} // anonymous namespace


//    YEAR, NAME,     PARENT, MACHINE,  INPUT,    STATE,          INIT,       MONITOR, COMPANY,  FULLNAME,                  FLAGS
GAME( 1985, kicker,   0,      shaolins, shaolins, shaolins_state, empty_init, ROT90,   "Konami", "Kicker",                  MACHINE_SUPPORTS_SAVE )
GAME( 1985, shaolins, kicker, shaolins, shaolins, shaolins_state, empty_init, ROT90,   "Konami", "Shao-lin's Road (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, shaolinb, kicker, shaolins, shaolins, shaolins_state, empty_init, ROT90,   "Konami", "Shao-lin's Road (set 2)", MACHINE_SUPPORTS_SAVE )
