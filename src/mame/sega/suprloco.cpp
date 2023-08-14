// license:BSD-3-Clause
// copyright-holders: Zsolt Vasvari

/******************************************************************************

Super Locomotive

driver by Zsolt Vasvari

Sega PCB 834-5137
 Sega 315-5015 (Sega custom encrypted Z80)
 Sega 315-5011
 Sega 315-5012
 Z80
 M5L8255AP
 8 switch Dipswitch x 2

Special Thanks to Raki(Sehyeon Kim) for tracing out the PCB schematics


TODO:
 * Sprite colors are still not 100% correct, because 3 of the color bits in
    the PROM come from the tilemap attribute bits for the tile behind (or,
    in the case of tilemaps with the priority bit set, potentially in front
    of) the sprite! This is hacked around by assuming one particular bit is
    always set, to make the colors correct during super locomotive mode.
   There is currently a very nasty hack in the sprite code to make this look
    correct; the correct solution will most likely require the sprite drawing
    code to be rewritten, or perhaps merged with the more-capable code
    in system1_v.cpp, as noted below.
 * Consider merging parts of or the complete driver with system1.cpp
    and system1_v.cpp, or deriving common code to its own file.
   The sprite systems use the same customs, although hooked up somewhat
    differently (system1 uses some of the sprite X bits in register 3 for
    color banking, for instance, while suprloco uses them for some other
    unknown purpose as they are seemingly ignored by the sprite chipset).
   The sound map is the same, the sound cpu and sound chip clocks are the
    same (although derived differently).
   The PPI hookup is different, and the maincpu memory map is different.
   The tilemap systems are also very similar, particularly to the early
   system1 boards like up'n down.
 * The MUTE output isn't hooked up properly yet.
 * The bg rowscroll system should have 'overflow' pixels from the right screen
    edge show on the very left screen edge for scroll values where the low 3
    bits are not zero. This is an original hardware bug and can be seen in pcb
    videos, particularly at the end of the bonus stage when the man runs on
    screen followed by the text "GO ON TO NEXT ROUND!"

BTANBs:
 * When passing through the trestle bridge, there is one light green dot per
    trestle triangle that appears in front of the player train instead of
    behind it.
   This happens on the real PCB too.

******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/segacrpt_device.h"
#include "sound/flt_rc.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class suprloco_state : public driver_device
{
public:
	suprloco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_filter_1(*this, "filter_1"),
		m_filter_1a(*this, "filter_1a"),
		m_filter_2(*this, "filter_2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_sprites_rom(*this, "sprites")
	{ }

	void suprloco(machine_config &config);

	void init_suprloco();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<segacrpt_z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<filter_rc_device> m_filter_1;
	required_device<filter_rc_device> m_filter_1a;
	required_device<filter_rc_device> m_filter_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_region_ptr<uint8_t> m_sprites_rom;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_control = 0;
	uint8_t m_m1_num = 0;

	enum
	{
		SPR_Y_TOP = 0, // y offset of top of sprite
		SPR_Y_BOTTOM,  // y offset of bottom of sprite
		SPR_X,         // x offset
		SPR_COL,       // ??? (system 1 uses this as x offset MSB and color bank, but here it is unknown but used)
		SPR_SKIP_LO,   // stride
		SPR_SKIP_HI,   // "
		SPR_GFXOFS_LO, // source
		SPR_GFXOFS_HI  // "
		// technically each sprite word is 16 bytes long, but the latter 8 bytes have unclear purpose, yet are used.
	};

	void videoram_w(offs_t offset, uint8_t data);
	void scrollram_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void draw_pixel(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color, int flip);
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int spr_number);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decrypted_opcodes_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

// This timing code is the same system used in system1.cpp, using an identical circuit.
#define MASTER_CLOCK    XTAL(20'000'000)

void suprloco_state::machine_start()
{
	m_m1_num = 0;
	save_item(NAME(m_control));
	save_item(NAME(m_m1_num));
}

// video

/***************************************************************************

  Convert the color PROMs into RGB values.
  The 3 channels each use a resistor array (8x1k), with the bits connected as such:
  bit0 -> 1
  bit1 -> 2
  bit1 -> 3
  bit2 -> 4
  bit2 -> 5
  bit2 -> 6
  bit2 -> 7
  GND -> 8

  The other ends of all the resistors are tied together.
  These 3 arrays then each connect to a Murata DST310-55Y5S222M100 EMI/RFI
   Suppression Filter Disc (2200pF w/Ferrite Beads, 100V tolerance) and then
   connect to the harness R, G and B contacts.
  These same EMI/RFI suppression filters are used on all the input contacts
   as well.

***************************************************************************/
void suprloco_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("colorprom")->base();

	for (int i = 0; i < 1024; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x24 * bit0 + 0x49 * bit1 + 0x92 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x24 * bit0 + 0x49 * bit1 + 0x92 * bit2;
		// blue component
		bit0 = 0; // the low resistor in the blue array is simply left floating
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x24 * bit0 + 0x49 * bit1 + 0x92 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* tile attribute bits:

	////////----------- low 8 bits of tile number
	||||||||  /-------- unused
	||||||||  |/------- connected to palette latch, but unused.
	||||||||  ||/------ /FORCE_TILE_PRIORITY (if 0, this tile always appears above sprites)
	||||||||  |||///--- palette selected for tile (note that bit 10(0xa) is used for BOTH the tile number AND the palette!)
	||||||||  |||||///- high 3 bits of tile number
	76543210  fedcba98
*/

TILE_GET_INFO_MEMBER(suprloco_state::get_tile_info)
{
	uint16_t const tile_attr = (m_videoram[2*tile_index] | (m_videoram[2*tile_index+1]<<8));
	tileinfo.set(0,
			BIT(tile_attr,0,10),
			((BIT(tile_attr,10,3) << 4) | 0x80 | (BIT(m_control,5,2) << 8)) >> 4, // this overlaps bit 10 intentionally
			0);
	tileinfo.category = BIT(tile_attr,13);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void suprloco_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(suprloco_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
	// pen color 0 on every palette "bank" is opaque only on layer 0, everything else is opaque on all layers.
	for (int i = 0; i < 1024; i++)
		m_bg_tilemap->map_pen_to_layer(0, i, (i & 0xf) ? TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 : TILEMAP_PIXEL_LAYER0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void suprloco_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void suprloco_state::scrollram_w(offs_t offset, uint8_t data)
{
	int const adj = flip_screen() ? -8 : 8;

	m_scrollram[offset] = data;
	m_bg_tilemap->set_scrollx(offset, data - adj);
}

void suprloco_state::control_w(uint8_t data)
{
	/* Very similar to Sega System 1, but force blank is inverted there. Sega System 2 however matches the polarity here!
	   Bit 0   - coin counter A
	   Bit 1   - coin counter B (only used if coinage differs from A)
	   Bit 2   - connected to harness connector but unused
	   Bit 3   - N/C
	   Bit 4   - force screen blank if 0
	   Bit 5   - palette prom address bit 8
	   Bit 6   - palette prom address bit 9
	   Bit 7   - flip screen
	*/
	m_screen->update_partial(m_screen->vpos()); // partial update because of the palette prom address bit changes

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	flip_screen_set(BIT(data,7));

	m_control = data;
}



inline void suprloco_state::draw_pixel(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color, int flip)
{
	if (flip)
	{
		x = bitmap.width() - x - 1;
		y = bitmap.height() - y - 1;
	}

	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = color;
}


void suprloco_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int spr_number)
{
	int const flip = flip_screen();
	int adjy, dy;

	const uint8_t *spr_reg = m_spriteram + 0x10 * spr_number;

	int src = spr_reg[SPR_GFXOFS_LO] + (spr_reg[SPR_GFXOFS_HI] << 8);
	short const skip = spr_reg[SPR_SKIP_LO] + (spr_reg[SPR_SKIP_HI] << 8); // bytes to skip before drawing each row (can be negative)

	int const height = spr_reg[SPR_Y_BOTTOM] - spr_reg[SPR_Y_TOP];

	int const sx = spr_reg[SPR_X];
	int const sy = spr_reg[SPR_Y_TOP] + 1;

	if (!flip)
	{
		adjy = sy;
		dy = 1;
	}
	else
	{
		adjy = sy + 31 + height; // sprites are offset by 256-(224+1)=31 pixels
								 //  in flipscreen mode due to the way the
								 //  video timing pals and sprite line compare
								 //  counter reload value works
		dy = -1;
	}

	/*
	Palette PROM address bits:
	//--------- pallete bank bits from the PPI
	||/-------- 0 for sprites, 1 for tilemap
	|||///----- palette bank bits from the tilemap for the underlying tile
	||||||////- currently selected color based on the 4 tilemap color bits or 4 sprite bits
	9876543210
	*/
	pen_t const pen_base = ((BIT(m_control,5,2)<<8)
							| 0 /* bit 7 will always be zero for sprites */
							| ((adjy<128)?0x10:0x00) ///TODO: horrible hack, see below.
							);
	/* Horrible hack explanation: The 0x10 bit should be pulled, along with
	    0x20 and 0x40, from the tilemap 'behind' the sprite.
	   To actually implement this, we could be doing something like rendering
	    the sprite's color prom address, instead of its actual color data, into
	    a sprite bitmap. Then, when baking the bitmap and the tilemap together,
	    the sprite colors, if they are to be displayed (and not masked by bg),
	    need to use those bits from whatever tilemap pixel they are on top of!
	   This hack makes it so this bit should be set on the top half of the
	    screen and clear on the bottom half, regardless of flipscreen state
	    (i.e. it will be on the bottom half when flipped).
	   It's technically wrong, but looks correct.
	*/

	for (int row = 0; row < height; row++, adjy += dy)
	{
		int color1, color2;
		uint8_t data;

		src += skip;

		int col = 0;

		// get pointer to packed sprite data
		const uint8_t *gfx = &(m_sprites_rom[src & 0x7fff]);
		int const flipx = src & 0x8000;   // flip x

		while (1)
		{
			if (flipx)  // flip x
			{
				data = *gfx--;
				color1 = data & 0x0f;
				color2 = data >> 4;
			}
			else
			{
				data = *gfx++;
				color1 = data >> 4;
				color2 = data & 0x0f;
			}

			if (color1 == 15) break; // stop rendering this sprite immediately if its color is 0xf
			if (color1)
				draw_pixel(bitmap, cliprect,sx + col, adjy, pen_base + color1, flip);

			if (color2 == 15) break;
			if (color2)
				draw_pixel(bitmap, cliprect,sx + col + 1, adjy, pen_base + color2, flip);

			col += 2;
		}
	}
}

void suprloco_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int spr_number = 0; spr_number < (m_spriteram.bytes() >> 4); spr_number++)
	{
		const uint8_t *spr_reg = m_spriteram + 0x10 * spr_number;
		if (spr_reg[SPR_X] != 0xff)
			draw_sprite(bitmap, cliprect, spr_number);
	}
}

uint32_t suprloco_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_control,4)) // force blank
	{
		bitmap.fill(0, cliprect);
		return 0;
	}
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_OPAQUE, 0); // draw tiles with priority not set
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_OPAQUE, 0); // draw tiles with priority set
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER1, 0); // only draw layer 1 pixels on top of sprites
	return 0;
}


// machine
/*
15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 RW
0  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - 0x0000 - EPROM@IC37
0  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - 0x4000 - EPROM@IC15
1  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - 0x8000 - EPROM@IC28
1  1  *  *  *                                         MMIO DECODER 74LS138@IC54
1  1  0  0  0  *  *  *  *  *  *  *  *  *  *  *  RW  - 0xC000 - OBJRAM (MBM2148L-55 1kx4 srams arranged
                                                       as a 16-bit-wide array @IC19, IC18, IC17, IC16,
                                                       accessed via sprite custom 315-5012@IC4)
1  1  0  0  1  x  x  x  x  x  x  x  x  x  x  x  R   - 0xC800 - SYSTEM optoisolated inputs from harness connector
1  1  0  1  0  x  x  x  x  x  x  x  x  x  x  x  R   - 0xD000 - P1 optoisolated inputs from harness connector
1  1  0  1  1  x  x  x  x  x  x  x  x  x  x  x  R   - 0xD800 - P2 optoisolated inputs from harness connector
1  1  1  0  0  x  x  x  x  x  x  x  x  x  x  *  R   - 0xE000 - DSW1/2 inputs; A0 ? DSW1:DSW0
1  1  1  0  1  x  x  x  x  x  x  x  x  x  *  *  RW  - 0xE800 - PPI D8255AC-5@IC41
1  1  1  1  0  *  *  *  *  *  *  *  *  *  *  *  RW  - 0xF000 - Tilemap RAM@IC34 (this is used for several things)
1  1  1  1  1  *  *  *  *  *  *  *  *  *  *  *  RW  - 0xF800 - RAM@IC3
*/

void suprloco_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc1ff).ram().share(m_spriteram);
	map(0xc200, 0xc7ff).ram(); // additional sprite ram is present here
	map(0xc800, 0xc800).mirror(0x07ff).portr("SYSTEM");
	map(0xd000, 0xd000).mirror(0x07ff).portr("P1");
	map(0xd800, 0xd800).mirror(0x07ff).portr("P2");
	map(0xe000, 0xe000).mirror(0x07fe).portr("DSW1");
	map(0xe001, 0xe001).mirror(0x07fe).portr("DSW2");
	map(0xe800, 0xe803).mirror(0x07fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf000, 0xf6ff).ram().w(FUNC(suprloco_state::videoram_w)).share(m_videoram);
	map(0xf700, 0xf7df).ram(); // unused
	map(0xf7e0, 0xf7ff).ram().w(FUNC(suprloco_state::scrollram_w)).share(m_scrollram);
	map(0xf800, 0xffff).ram();
}

void suprloco_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
}

/*
15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 RW
*  *  *                                               MMIO DECODER 74LS138@IC66
0  0  0  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - 0x0000 - EPROM@IC64; a13 of the eprom is grounded.
0  0  1                                             - 0x2000 - OPEN BUS
0  1  0                                             - 0x4000 - OPEN BUS
0  1  1                                             - 0x6000 - OPEN BUS
1  0  0  x  x  *  *  *  *  *  *  *  *  *  *  *  RW  - 0x8000 - RAM@IC65
1  0  1  x  x  x  x  x  x  x  x  x  x  x  x  x   W  - 0xA000 - SN76489A@IC78
1  1  0  x  x  x  x  x  x  x  x  x  x  x  x  x   W  - 0xC000 - SN76489A@IC79
1  1  1  x  x  x  x  x  x  x  x  x  x  x  x  x  R   - 0xE000 - read byte from maincpu PPI@IC41 (in MODE 1?)'s port A by strobing PC6 low
*/

void suprloco_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	// 2000-7fff is open bus
	map(0x8000, 0x87ff).mirror(0x1800).ram();
	map(0xa000, 0xa000).mirror(0x1fff).w("sn1", FUNC(sn76496_device::write));
	map(0xc000, 0xc000).mirror(0x1fff).w("sn2", FUNC(sn76496_device::write));
	map(0xe000, 0xe000).mirror(0x1fff).r("ppi", FUNC(i8255_device::acka_r));
}



static INPUT_PORTS_START( suprloco )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Initial Entry" )     PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8 by 8
	1024,   // 1024 characters
	4,      // 4 bits per pixel
	{ 0, 1024*8*8, 2*1024*8*8, 3*1024*8*8 },            // plane
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_suprloco )
	GFXDECODE_ENTRY( "chars", 0x6000, charlayout, 0, 16 )
GFXDECODE_END


void suprloco_state::suprloco(machine_config &config)
{
	// basic machine hardware
	SEGA_315_5015(config, m_maincpu, MASTER_CLOCK / 5); // see below
	// this divider is actually /5 vs /6 depending on the /M1 line state from
	//  the z80 during the 74LS161 counter reload; see system1.cpp which uses
	//  an identical circuit.
	m_maincpu->set_addrmap(AS_PROGRAM, &suprloco_state::main_map);
	m_maincpu->set_addrmap(AS_OPCODES, &suprloco_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(suprloco_state::irq0_line_hold));
	m_maincpu->set_decrypted_tag(":decrypted_opcodes");
	m_maincpu->refresh_cb().set([this](offs_t offset, u8 data) {
		m_m1_num = (m_m1_num + 1) % 5;
		// if we've passed 5 M1 cycles, steal an additional
		//  5x-20mhz-cycle-long cpu cycle. We do this at the third cycle to
		//  average the correct speed.
		if (m_m1_num == 2)
			m_maincpu->adjust_icount(-1);
	});

	Z80(config, m_audiocpu, MASTER_CLOCK/5); // 4mhz
	// This divide-by-five is done by a clever circuit with a 74LS390 in
	//  bi-quinary mode.
	m_audiocpu->set_addrmap(AS_PROGRAM, &suprloco_state::sound_map);
	// Audio CPU INTs are caused by the falling edge of the V5 line counter
	//  (4x per frame) and are auto-acked.
	///TODO: once pals are dumped, make sure this uses the correct timing!
	m_audiocpu->set_periodic_int(FUNC(suprloco_state::irq0_line_hold), attotime::from_hz(4 * 60));
	// Audio CPU NMIs are caused by the main CPU via PPI portC bit7, see below

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pb_callback().set(FUNC(suprloco_state::control_w));
	ppi.tri_pb_callback().set_constant(0);
	ppi.out_pc_callback().set_output("lamp0").bit(0).invert(); ///TODO: 8255 portc bit 0 is MUTE, same as in system1, not a lamp.
	ppi.out_pc_callback().append_inputline(m_audiocpu, INPUT_LINE_NMI).bit(7).invert();

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	///TODO: use correct timing once PALs are dumped
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(5000));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(1*8, 31*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(suprloco_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suprloco);
	PALETTE(config, m_palette, FUNC(suprloco_state::palette), 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	///TODO: sn1 goes into the pin 9 input of sn2, which is poorly documented for mixing; instead, we just duplicate the second filter here for now
	FILTER_RC(config, m_filter_1).set_rc(filter_rc_device::HIGHPASS, RES_K(27), 0, 0, CAP_U(10)); //CR high-pass R5+C78+VR1(2kohm)
	m_filter_1->add_route(ALL_OUTPUTS, "mono", 0.707);
	FILTER_RC(config, m_filter_1a).set_rc(filter_rc_device::HIGHPASS, RES_K(22), 0, 0, CAP_U(10)); //CR high-pass R2+C72
	m_filter_1a->add_route(ALL_OUTPUTS, m_filter_1, 1.0);
	SN76489A(config, "sn1", MASTER_CLOCK/5).add_route(ALL_OUTPUTS, m_filter_1a, 1.0); // see above re: /5 divider
	FILTER_RC(config, m_filter_2).set_rc(filter_rc_device::HIGHPASS, RES_K(27), 0, 0, CAP_U(10)); //CR high-pass R5+C78+VR1(2kohm)
	m_filter_2->add_route(ALL_OUTPUTS, "mono", 0.707);
	SN76489A(config, "sn2", MASTER_CLOCK/10).add_route(ALL_OUTPUTS, m_filter_2, 1.0); // see above re: /5 divider (itself divided by 2)
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( suprloco )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226a.37",    0x0000, 0x4000, CRC(33b02368) SHA1(c6e3116ad4b52bcc3174de5770f7a7ce024790d5) ) // encrypted
	ROM_LOAD( "epr-5227a.15",    0x4000, 0x4000, CRC(a5e67f50) SHA1(1dd52e4cf00ce414fe1db8259c9976cdc23513b4) ) // encrypted
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "chars", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							// 0x6000 - 0xe000 will be created by init_suprloco

	ROM_REGION( 0x8000, "sprites", 0 ) // used at runtime
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							// 0x6000 empty

	ROM_REGION( 0x0400, "colorprom", 0 )
	ROM_LOAD( "pr-5220.100",     0x0000, 0x0200, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) // color PROM, 82S141 512x8
	ROM_RELOAD(0x200, 0x200) // The socket can hold an 82S181 1kx8 prom but is not used here, so repeat the prom since pin 22 is N/C on an 82S141
	ROM_REGION( 0x0400, "clutprom", 0 )
	ROM_LOAD( "pr-5219.89",      0x0000, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) // 3bpp to 4bpp Color LUT
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-5221.7",       0x0000, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) // tilemap palette latch timing

	// Two undumped PALS:
	// PAL16R4@IC20: horizontal counter carry, CSYNC generation, and sprite DMA timing
	// PAL16R4@IC30: vertical counter handling, maincpu INT generation, and VSYNC timing
	// These are NOT the same as the two pals in System1, and some of the functions are switched around there!
ROM_END

ROM_START( suprlocoo )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226.37",     0x0000, 0x4000, CRC(57f514dd) SHA1(707800b90a22547a56b01d1e11775e9ee5555d23) ) // encrypted
	ROM_LOAD( "epr-5227.15",     0x4000, 0x4000, CRC(5a1d2fb0) SHA1(fdb9416e5530718245fd597073a63feddb233c3c) ) // encrypted
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "chars", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							// 0x6000 - 0xe000 will be created by init_suprloco

	ROM_REGION( 0x8000, "sprites", 0 ) // used at runtime
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							// 0x6000 empty

	ROM_REGION( 0x0400, "colorprom", 0 )
	ROM_LOAD( "pr-5220.100",     0x0000, 0x0200, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) // color PROM, 82S141 512x8
	ROM_RELOAD(0x200, 0x200) // The socket can hold an 82S181 1kx8 prom but is not used here, so repeat the prom since pin 22 is N/C on an 82S141
	ROM_REGION( 0x0400, "clutprom", 0 )
	ROM_LOAD( "pr-5219.89",      0x0000, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) // 3bpp to 4bpp Color LUT
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-5221.7",       0x0000, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) // tilemap palette latch timing

	// Two undumped PALS:
	// PAL16R4@IC20: horizontal counter carry, CSYNC generation, and sprite DMA timing
	// PAL16R4@IC30: vertical counter handling, maincpu INT generation, and VSYNC timing
	// These are NOT the same as the two pals in System1, and some of the functions are switched around there!
ROM_END

void suprloco_state::init_suprloco()
{
	// convert graphics to 4bpp palette indices from the 3bpp data and address using the CLUT PROM
	uint8_t *source = memregion("chars")->base();
	uint8_t *dest   = source + 0x6000;
	uint8_t *lookup = memregion("clutprom")->base();

	for (int i = 0; i < 0x80; i++, lookup += 8)
	{
		for (int j = 0; j < 0x40; j++, source++, dest++)
		{
			dest[0] = dest[0x2000] = dest[0x4000] = dest[0x6000] = 0;

			for (int k = 0; k < 8; k++)
			{
				const int color_source = (((source[0x0000] >> k) & 0x01) << 2) |
										 (((source[0x2000] >> k) & 0x01) << 1) |
										 (((source[0x4000] >> k) & 0x01) << 0);

				const int color_dest = lookup[color_source];

				dest[0x0000] |= (((color_dest >> 3) & 0x01) << k);
				dest[0x2000] |= (((color_dest >> 2) & 0x01) << k);
				dest[0x4000] |= (((color_dest >> 1) & 0x01) << k);
				dest[0x6000] |= (((color_dest >> 0) & 0x01) << k);
			}
		}
	}
}

} // anonymous namespace


GAME( 1982, suprloco,         0, suprloco, suprloco, suprloco_state, init_suprloco, ROT0, "Sega", "Super Locomotive (Rev.A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1982, suprlocoo, suprloco, suprloco, suprloco, suprloco_state, init_suprloco, ROT0, "Sega", "Super Locomotive",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
