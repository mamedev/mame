// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/**************************
 *** PSYCHIC 5 hardware ***     (by Roberto Ventura)
 **************************


Psychic 5 (c) JALECO (early 1987)

driver by Jarek Parchanski


0) GENERAL.

The game has two Z80s.
The second CPU controls the two YM2203 sound chips.
Screen resolution is 224x256 (vertical CRT).
768 colors on screen.
96 sprites (16x16 or 32x32).

Some hardware features description is arbitrary since I
guessed their hardware implementation trusting what
I can recall of the 'real' arcade game.


1) ROM CONTENTS.

P5A 256 Kbit    Sound program ROM
P5B 512 Kbit    Sprites data ROM 0
P5C 512 Kbit    Sprites data ROM 1
P5D 256 Kbit    Main CPU program ROM
P5E 512 Kbit    Banked CPU data ROM (banks 0,1,2 and 3)
P5F 256 Kbit    Foreground data ROM
P5G 512 Kbit    Background data ROM 0
P5H 512 Kbit    Background data ROM 1

ROM banks 2 and 3 contain the eight level maps only.

Graphics format is pretty simple,each nibble contains information
for one pixel (4 planes packed).

All graphics is made of 8x8 tiles (32 consecutive bytes),16x16 tiles
are composed of 4 consecutive 8x8 tiles,while 32x32 sprites can be
considered as four 16x16 tiles assembled in the same way 8x8 tiles produce
one 16x16 tile.
Tile composition follows this scheme:

These are 4 consecutive tiles as stored in ROM - increasing memory
(the symbol ">" means the tiles are rotated 270 degrees):

A>;B>;C>;D>

This is the composite tile.

A> C>
B> D>

This is the way tile looks on the effective CRT after 90 deg rotation.

C^ D^
A^ B^


2) CPU.

The board mounts two crystals, 12.000 MHz and 5.000 MHz.
The possible effective main CPU clock may be 6 MHz (12/2).

The main Z80 runs in Interrupt mode 0 (IM0),the game program expects
execution of two different restart (RST) instructions.
RST 10,the main IRQ,is to be triggered each time the screen is refreshed.
RST 08 must be triggered in order to make the game work properly
(e.g. demo game),I guess sound has something to do with this IRQ,but I
don't know whether it is synchronized with video beam neither I know whether
it can be disabled.

Sound CPU runs in IM1.

The main CPU lies idle waiting the external IRQ occurrence executing code
from 012d to 0140.

Game data is paged,level maps and other data fit in ROM space
8000-bfff.

Video RAM is also banked at locations from c000 to dfff.



3) MAIN CPU MEMORY MAP.

0000-7fff   ROM
8000-bfff   paged ROM
c000-dfff   paged RAM (RAM/VRAM/IO)
f000-f1ff   I/O
f200-f7ff   Sprites registers (misc RAM)
f800-ffff   Work RAM

-paged RAM memory map

Bank 0

c000-cfff   Background tile buffer
d000-dfff   RAM (dummy background image for software collisions)

Bank 1

c000-c3ff   I/O
c400-cfff   Palette RAM
d000-dfff   Foreground tile buffer


4) I/O

-VRAM bank 1

c000    COIN SLOTS and START BUTTONS

        76543210
        ||    ||
        ||    |^-- coin 1
        ||    ^--- coin 2
        ||
        |^-------- start 1
        ^--------- start 2


c001    PLAYER 1 CONTROLS
c002    PLAYER 2 CONTROLS

        76543210
          ||||||
          |||||^-- right
          ||||^--- left
          |||^---- down
          ||^----- up
          |^------ fire 0
          ^------- fire 1


c003    DIPSWITCH 0

        76543210
        ||
        ^^-------- number of player's espers


c004    DIPSWITCH 1

        76543210
        |||    |
        |||    ^-- player's immortality
        ^^^------- coin/credit configurations

c308    BACKGROUND SCROLL Y  least significant 8 bits

c309    BACKGROUND SCROLL Y  most significant 2 bits

        76543210
              ||
              |^-- Y scroll bit 8
              ^--- Y scroll bit 9

c30A    BACKGROUND SCROLL X  least significant 8 bits

c30B    BACKGROUND SCROLL X  MSB

        76543210
        ||||||||
        |||||||^-- X scroll bit 8
        ||||||^--- Unknown (title screen)
        ^^^^^^---- Unknown (title screen: 0xff)

c30C    SCREEN MODE

        76543210
              ||
              |^-- background enable bit (0 means black BG)
              ^--- grey background enable bit

c5fe    BACKGROUND PALETTE INTENSITY (red and green)

        76543210
        ||||||||
        ||||^^^^-- green intensity
        ^^^^------ red intensity

c5ff    BACKGROUND PALETTE INTENSITY (blue)

        76543210
        ||||||||
        ||||^^^^-- unknown (?)
        ^^^^------ blue intensity

-RAM f000-f1ff

f000    SOUND COMMAND (?)

f001    UNKNOWN
        maybe some external HW like a flashing light
        when a coin falls in slot (?)

f002    ROM PAGE SELECTOR
        select four (0-3) ROM pages at 8000-bfff.

f003    VRAM PAGE SELECTOR
        selects two (0-1) VRAM pages at c000-dfff.

f004    UNKNOWN

f005    UNKNOWN


5) COLOR RAM

The palette system is dynamic,the game can show up to 768 different
colors on screen.

Each color component (RGB) depth is 4 bits,two consecutive bytes are used
for each color code (12 bits).

format: RRRRGGGGBBBB0000

Colors are organized in palettes,since graphics is 4 bits (16 colors)
each palette takes 32 bytes;the three different layers (background,sprites
and foreground) don't share any color,each has its own 256 color space,hence
the 768 colors on screen.

c400-c5ff       Sprites palettes
c800-c9ff       Background palettes
ca00-cbff       Foreground palettes

The last palette colors for sprites and foreground are transparent
colors,these colors aren't displayed on screen so the actual maximum
color output is 736 colors.

Some additional palette effects are provided for background.
Sprite palette 15's transparent color (c5fe-c5ff) is the global
background intensity.
Background intensity is encoded in the same way of colors,and
affects the intensity of each color component (BGR).
The least significant nibble of c5ff is unknown,it assumes value
0xf occasionally when the other nibbles are changed.The only
value which is not 0 neither 0xf is 2 and it is assumed by this nibble
during the ride on the witches' broom.

When bit 1 of c30c (VRAM bank 1) is set background screen turns
grey.Notice the output of this function is passed to the palette
intensity process.
When you hit the witch the screen gets purple,this is done via
setting grey screen and scaling color components.
(BTW,I haven't seen a Psychic5 machine for about 10 years but
I think the resulting purple tonality in my emulator is way off...)

Palette intensity acts badly during title screen,when the game
scenario rapidly shows up under the golden logo;the problem is value
0xffff causes the background to be displayed as black.


6) TILE-BASED LAYERS

The tile format for background and foreground is the same,the
only difference is that background tiles are 16x16 pixels while foreground
tiles are only 8x8.

Background virtual screen is 1024 pixels tall and 512 pixel wide.

Two consecutive bytes identify one tile.

        O7 O6 O5 O4 O3 O2 O1 O0         gfx Offset
        O9 O8 FX FY C3 C2 C1 C0         Attribute

        O= GFX offset (1024 tiles)
        F= Flip X and Y
        C= Color palette selector

Tile layers are to be displayed 'bottom-up' (eg. c000-c040
is row 63 of background screen)

Both background and foreground playfields can scroll; background tiles
are arranged in a rotational (wrap-around) buffer; window can be scrolled
vertically by an eight pixel offset (i.e. enough to scroll a single tile
smoothly),the game has to copy in new tiles every time a row is scrolled
(this feature is only used in the credits section at the end of the game).
I haven't provided the address for this feature, since I don't
know where it is mapped.


7) SPRITES

Five bytes identify each sprite,but the registers actually used
are placed at intervals of 16.
The remaining bytes are used as backup sprite coordinates and
attributes,not real sprites.
Sprites are provided in 2 different sizes,16x16 and 32x32.
Larger sprites are addressed as 16x16 sprites,but they are all
aligned by 4.

The first sprite data is located at f20b,then f21b and so on.

0b      X7 X6 X5 X4 X3 X2 X1 X0         X coord (on screen)
0c      Y7 Y6 Y5 Y5 Y3 Y2 Y1 Y0         X coord (on screen)
0d      O9 O8 FX FY SZ X8 -- Y8         hi gfx - FLIP - hi X - hi Y
0e      O7 O6 O5 O4 O3 O2 O1 O0         gfx - offset
0f      -- -- -- -- C3 C2 C1 C0         color

    Y= Y coordinate (two's complemented) (Y8 is used to clip sprite on top border)
        X= X coordinate (X8 is used to clip 32x32 sprites on left border)
        O= Gfx offset (1024 sprites)
        F= Flip
    SZ=Size 0=16x16 sprite,1=32x32 sprite
        C= Color palette selector

====

Notes (23-Jan-2016 AS):
- Bombs Away tests CPU roms with a sum8, with following algorithm:
  * tests rom 0, compares result with [$4]
  * tests banks 3,2,1, reads [$5], no bank 0 (?)
  * tests banks 7,6,5,4, reads [$6]
  fwiw extra roms contains palette/sprite/level data, nothing to do with game logic.
- Bombs Away sports following issues, which indicates it's a unfinished product:
  - missing bullets (missing data from ROMs);
  - missing levels above 4 (missing data from ROMs);
  - occasionally wrong flip x when the player route is from up to down;
  - enemy counter stops entirely when the S is collected;
  - boss fights uses always the same pattern;
  - single BGM repeated over and over, for every level;
  - a very sketchy ending screen (just credits with a special BGM played). Game sits there until BGM is finished and no new plays are allowed until so;

*/

#include "emu.h"

#include "jalblend.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <cassert>


namespace {

class psychic5_state_base : public driver_device
{
public:
	psychic5_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_bankrom(*this, "bankrom"),
		m_mainbank(*this, "mainbank"),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_control(*this, "bg_control"),
		m_palette_ram_bg(*this, "palette_ram_bg"),
		m_palette_ram_sp(*this, "palette_ram_sp"),
		m_palette_ram_tx(*this, "palette_ram_tx")
	{ }

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	memory_view m_vrambank;

	required_memory_region m_bankrom;

	required_memory_bank m_mainbank;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_control;
	required_shared_ptr<uint8_t> m_palette_ram_bg;
	required_shared_ptr<uint8_t> m_palette_ram_sp;
	required_shared_ptr<uint8_t> m_palette_ram_tx;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t m_bg_clip_mode = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	int m_sx1 = 0;
	int m_sy1 = 0;
	int m_sy2 = 0;

	uint8_t bankselect_r();
	void bankselect_w(uint8_t data);
	uint8_t vram_page_select_r();
	void vram_page_select_w(uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void sprite_col_w(offs_t offset, uint8_t data);
	void bg_col_w(offs_t offset, uint8_t data);
	void tx_col_w(offs_t offset, uint8_t data);

	void flipscreen_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	template <typename T> void draw_sprites(T &&draw_sprite);

private:
	uint8_t m_bank_mask = 0;

	uint8_t m_bank_latch = 0;

	void change_palette(int offset, uint8_t* palram, int palbase);
};


class psychic5_state : public psychic5_state_base
{
public:
	psychic5_state(const machine_config &mconfig, device_type type, const char *tag) :
		psychic5_state_base(mconfig, type, tag),
		m_blend(*this, "blend")
	{
	}

	void psychic5(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<jaleco_blend_device> m_blend;

	uint8_t m_title_screen = 0;
	uint16_t m_palette_intensity = 0;

	void coin_counter_w(uint8_t data);
	void title_screen_w(uint8_t data);

	void change_bg_palette(int color, int lo_offs, int hi_offs);
	void set_background_palette_intensity();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void soundport_map(address_map &map) ATTR_COLD;
};


class bombsa_state : public psychic5_state_base
{
public:
	bombsa_state(const machine_config &mconfig, device_type type, const char *tag) :
		psychic5_state_base(mconfig, type, tag)
	{
	}

	void bombsa(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t m_unknown = 0;

	void unknown_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void soundport_map(address_map &map) ATTR_COLD;
};



/***************************************************************************
  Palette color
***************************************************************************/

void psychic5_state_base::change_palette(int offset, uint8_t* palram, int palbase)
{
	uint8_t const lo = palram[(offset) & ~1];
	uint8_t const hi = palram[(offset) | 1];

	int const color = offset >> 1;

	m_palette->set_pen_color(palbase + color, rgb_t(hi & 0x0f, pal4bit(lo >> 4), pal4bit(lo), pal4bit(hi >> 4)));
}

void psychic5_state::change_bg_palette(int color, int lo_offs, int hi_offs)
{
	// red,green,blue intensities
	uint8_t const ir = pal4bit(m_palette_intensity >> 12);
	uint8_t const ig = pal4bit(m_palette_intensity >>  8);
	uint8_t const ib = pal4bit(m_palette_intensity >>  4);
	uint8_t const ix = m_palette_intensity & 0x0f;

	rgb_t const irgb = rgb_t(ir,ig,ib);

	uint8_t const lo = m_palette_ram_bg[lo_offs];
	uint8_t const hi = m_palette_ram_bg[hi_offs];

	// red,green,blue component
	uint8_t const r = pal4bit(lo >> 4);
	uint8_t const g = pal4bit(lo);
	uint8_t const b = pal4bit(hi >> 4);

	// Grey background enable
	if (m_bg_control[4] & 2)
	{
		uint8_t const val = (r + g + b) / 3;        // Grey
		// Just leave plain grey
		m_palette->set_pen_color(color, m_blend->func(rgb_t(val, val, val), irgb, ix));
	}
	else
	{
		// Seems fishy, but the title screen would be black otherwise...
		if (!(m_title_screen & 1))
		{
			// Leave the world as-is
			m_palette->set_pen_color(color,m_blend->func(rgb_t(r,g,b), irgb, ix));
		}
	}
}

void psychic5_state::set_background_palette_intensity()
{
	constexpr unsigned BG_PAL_INTENSITY_RG = 0x1fe;
	constexpr unsigned BG_PAL_INTENSITY_BU = 0x1ff;
	m_palette_intensity = m_palette_ram_sp[BG_PAL_INTENSITY_BU] |
						(m_palette_ram_sp[BG_PAL_INTENSITY_RG] << 8);

	// for all of the background palette
	for (int i = 0; i < 0x100; i++)
		change_bg_palette(i + 0x100, i * 2, (i * 2) + 1);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

uint8_t psychic5_state_base::bankselect_r()
{
	return m_bank_latch;
}

void psychic5_state_base::bankselect_w(uint8_t data)
{
	if (m_bank_latch != data)
	{
		m_bank_latch = data;
		m_mainbank->set_entry(data & m_bank_mask);
	}
}

void psychic5_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	flipscreen_w(data);
}

void psychic5_state_base::flipscreen_w(uint8_t data)
{
	// bit 7 toggles flip screen
	if (data & 0x80)
	{
		flip_screen_set(!flip_screen());
	}
}


uint8_t psychic5_state_base::vram_page_select_r()
{
	return *m_vrambank.entry();
}

void psychic5_state_base::vram_page_select_w(uint8_t data)
{
	m_vrambank.select(data & 1);
}

void psychic5_state::title_screen_w(uint8_t data)
{
	m_title_screen = data;
}


void psychic5_state_base::sprite_col_w(offs_t offset, uint8_t data)
{
	m_palette_ram_sp[offset] = data;
	change_palette(offset, m_palette_ram_sp, 0x000);
}

void psychic5_state_base::bg_col_w(offs_t offset, uint8_t data)
{
	m_palette_ram_bg[offset] = data;
	change_palette(offset, m_palette_ram_bg, 0x100);
}

void psychic5_state_base::tx_col_w(offs_t offset, uint8_t data)
{
	m_palette_ram_tx[offset] = data;
	change_palette(offset, m_palette_ram_tx, 0x200);
}


void psychic5_state_base::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void psychic5_state_base::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}



void bombsa_state::unknown_w(uint8_t data)
{
	m_unknown = data;
}


/***************************************************************************

  Callbacks for the tilemap code

***************************************************************************/

TILE_GET_INFO_MEMBER(psychic5_state_base::get_bg_tile_info)
{
	int const offs = tile_index << 1;
	int const attr = m_bg_videoram[offs + 1];
	int const code = m_bg_videoram[offs] | ((attr & 0xc0) << 2);
	int const color = attr & 0x0f;
	int const flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(psychic5_state_base::get_fg_tile_info)
{
	int const offs = tile_index << 1;
	int const attr = m_fg_videoram[offs + 1];
	int const code = m_fg_videoram[offs] | ((attr & 0xc0) << 2);
	int const color = attr & 0x0f;
	int const flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo.set(2, code, color, flags);
}


/***************************************************************************

  Screen refresh

***************************************************************************/

template <typename T>
inline void psychic5_state_base::draw_sprites(T &&draw_sprite)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 16)
	{
		int const attr  = m_spriteram[offs + 13];
		if (attr & 2) // Bombs Away: disable sprite if enabled
			continue;
		int const code  = m_spriteram[offs + 14] | ((attr & 0xc0) << 2);
		int const color = m_spriteram[offs + 15] & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = m_spriteram[offs + 12];
		int sy = m_spriteram[offs + 11];
		int const size = (attr & 0x08) ? 32:16;

		if (attr & 0x01) sx -= 256;
		if (attr & 0x04) sy -= 256;

		if (flip_screen())
		{
			sx = 224 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (size == 32)
		{
			int x0,x1,y0,y1;

			if (flipx) { x0 = 2; x1 = 0; }
			else { x0 = 0; x1 = 2; }

			if (flipy) { y0 = 1; y1 = 0; }
			else { y0 = 0; y1 = 1; }

			draw_sprite(code + x0 + y0, color, flipx, flipy, sx, sy);
			draw_sprite(code + x0 + y1, color, flipx, flipy, sx, sy + 16);
			draw_sprite(code + x1 + y0, color, flipx, flipy, sx + 16, sy);
			draw_sprite(code + x1 + y1, color, flipx, flipy, sx + 16, sy + 16);
		}
		else
		{
			if (flip_screen())
				draw_sprite(code, color, flipx, flipy, sx + 16, sy + 16);
			else
				draw_sprite(code, color, flipx, flipy, sx, sy);
		}
	}
}

void psychic5_state::draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip = cliprect;

	set_background_palette_intensity();

	if (!(m_title_screen & 1))
	{
		m_bg_clip_mode = 0;
		m_sx1 = m_sy1 = m_sy2 = 0;
	}
	else
	{
		int const sy1_old = m_sy1;
		int const sx1_old = m_sx1;
		int const sy2_old = m_sy2;

		m_sy1 = m_spriteram[11];       // sprite 0
		m_sx1 = m_spriteram[12];
		m_sy2 = m_spriteram[11 + 128]; // sprite 8

		switch (m_bg_clip_mode)
		{
		case  0: case  4: if (sy1_old != m_sy1) m_bg_clip_mode++; break;
		case  2: case  6: if (sy2_old != m_sy2) m_bg_clip_mode++; break;
		case  8: case 10:
		case 12: case 14: if (sx1_old != m_sx1) m_bg_clip_mode++; break;
		case  1: case  5: if (m_sy1 == 0xf0) m_bg_clip_mode++; break;
		case  3: case  7: if (m_sy2 == 0xf0) m_bg_clip_mode++; break;
		case  9: case 11: if (m_sx1 == 0xf0) m_bg_clip_mode++; break;
		case 13: case 15: if (sx1_old == 0xf0) m_bg_clip_mode++;
			[[fallthrough]];
		case 16: if (m_sy1 != 0x00) m_bg_clip_mode = 0; break;
		}

		switch (m_bg_clip_mode)
		{
		case  0: case  4: case  8: case 12: case 16:
			clip.set(0, 0, 0, 0);
			break;
		case  1: clip.min_y = m_sy1; break;
		case  3: clip.max_y = m_sy2; break;
		case  5: clip.max_y = m_sy1; break;
		case  7: clip.min_y = m_sy2; break;
		case  9: case 15: clip.min_x = m_sx1; break;
		case 11: case 13: clip.max_x = m_sx1; break;
		}

		if (flip_screen())
			clip.set(255 - clip.max_x, 255 - clip.min_x, 255 - clip.max_y, 255 - clip.min_y);
	}

	m_bg_tilemap->draw(screen, bitmap, clip, 0, 0);
}

uint32_t psychic5_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const bg_scrollx = m_bg_control[0] | (m_bg_control[1] << 8);
	m_bg_tilemap->set_scrollx(0, bg_scrollx);
	uint16_t const bg_scrolly = m_bg_control[2] | (m_bg_control[3] << 8);
	m_bg_tilemap->set_scrolly(0, bg_scrolly);

	bitmap.fill(m_palette->black_pen(), cliprect);
	if (m_bg_control[4] & 1)    // Backgound enable
		draw_background(screen, bitmap, cliprect);
	if (!(m_title_screen & 1))
		draw_sprites([this, &bitmap, &cliprect] (auto... args) { m_blend->drawgfx(*m_palette, m_gfxdecode->gfx(0), bitmap, cliprect, args..., 15); });
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

uint32_t bombsa_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const bg_scrollx = m_bg_control[0] | (m_bg_control[1] << 8);
	m_bg_tilemap->set_scrollx(0, bg_scrollx);
	uint16_t const bg_scrolly = m_bg_control[2] | (m_bg_control[3] << 8);
	m_bg_tilemap->set_scrolly(0, bg_scrolly);

	bitmap.fill(m_palette->black_pen(), cliprect);
	if (m_bg_control[4] & 1)    // Backgound enable
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(m_palette->pen(0x0ff), cliprect);
	draw_sprites([this, &bitmap, &cliprect] (auto... args) { m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, args..., 15); });
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/***************************************************************************

  Initialize and destroy video hardware emulation

***************************************************************************/

void psychic5_state_base::machine_start()
{
	// be lazy and assume banked ROM is always a power-of-two and multiple of page size
	assert(!(m_bankrom->bytes() % 0x4000));
	assert(!(m_bankrom->bytes() & (m_bankrom->bytes() - 1)));

	m_mainbank->configure_entries(0, m_bankrom->bytes() / 0x4000, m_bankrom->base(), 0x4000);
	m_bank_mask = (m_bankrom->bytes() / 0x4000) - 1;

	save_item(NAME(m_bg_clip_mode));
	save_item(NAME(m_sx1));
	save_item(NAME(m_sy1));
	save_item(NAME(m_sy2));

	save_item(NAME(m_bank_latch));
}

void psychic5_state::machine_start()
{
	psychic5_state_base::machine_start();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(psychic5_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(psychic5_state::get_fg_tile_info)), TILEMAP_SCAN_COLS,  8,  8, 32, 32);
	m_fg_tilemap->set_transparent_pen(15);

	save_item(NAME(m_palette_intensity));
	save_item(NAME(m_title_screen));
}

void bombsa_state::machine_start()
{
	psychic5_state_base::machine_start();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bombsa_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bombsa_state::get_fg_tile_info)), TILEMAP_SCAN_COLS,  8,  8,  32, 32);
	m_fg_tilemap->set_transparent_pen(15);

	save_item(NAME(m_unknown));
}

void psychic5_state_base::machine_reset()
{
	m_bank_latch = 0xff;
	m_mainbank->set_entry(m_bank_latch & m_bank_mask);

	flip_screen_set(0);

	m_vrambank.select(0);

	m_bg_clip_mode = 0;
}

void psychic5_state::machine_reset()
{
	psychic5_state_base::machine_reset();

	m_title_screen = 0;
	m_palette_intensity = 0;
}


/***************************************************************************

  Interrupt(s)

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(psychic5_state_base::scanline)
{
	int const scanline = param;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   // Z80 - RST 10h - vblank

	if (scanline == 0) // sprite buffer irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   // Z80 - RST 08h
}



/***************************************************************************

  Memory Maps

***************************************************************************/

void psychic5_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xdfff).view(m_vrambank);
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xf000).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xf001, 0xf001).nopr().w(FUNC(psychic5_state::coin_counter_w));
	map(0xf002, 0xf002).rw(FUNC(psychic5_state::bankselect_r), FUNC(psychic5_state::bankselect_w));
	map(0xf003, 0xf003).rw(FUNC(psychic5_state::vram_page_select_r), FUNC(psychic5_state::vram_page_select_w));
	map(0xf004, 0xf004).noprw(); // ???
	map(0xf005, 0xf005).nopr().w(FUNC(psychic5_state::title_screen_w));
	map(0xf006, 0xf1ff).noprw();
	map(0xf200, 0xf7ff).ram().share(m_spriteram);
	map(0xf800, 0xffff).ram();

	m_vrambank[0](0xc000, 0xcfff).ram().w(FUNC(psychic5_state::bg_videoram_w)).share(m_bg_videoram);
	m_vrambank[0](0xd000, 0xdfff).ram();

	m_vrambank[1](0xc000, 0xc000).portr("SYSTEM");
	m_vrambank[1](0xc001, 0xc001).portr("P1");
	m_vrambank[1](0xc002, 0xc002).portr("P2");
	m_vrambank[1](0xc003, 0xc003).portr("DSW1");
	m_vrambank[1](0xc004, 0xc004).portr("DSW2");

	m_vrambank[1](0xc308, 0xc30c).ram().share(m_bg_control);

	m_vrambank[1](0xc400, 0xc5ff).ram().w(FUNC(psychic5_state::sprite_col_w)).share(m_palette_ram_sp);
	m_vrambank[1](0xc800, 0xc9ff).ram().w(FUNC(psychic5_state::bg_col_w)).share(m_palette_ram_bg);
	m_vrambank[1](0xca00, 0xcbff).ram().w(FUNC(psychic5_state::tx_col_w)).share(m_palette_ram_tx);

	m_vrambank[1](0xd000, 0xd7ff).ram().w(FUNC(psychic5_state::fg_videoram_w)).share(m_fg_videoram);
}


void psychic5_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void psychic5_state::soundport_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ym1", FUNC(ym2203_device::write));
	map(0x80, 0x81).w("ym2", FUNC(ym2203_device::write));
}


void bombsa_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xcfff).ram();

	/* ports look like the other games */
	map(0xd000, 0xd1ff).ram();
	map(0xd000, 0xd000).w("soundlatch", FUNC(generic_latch_8_device::write)); // confirmed
	map(0xd001, 0xd001).w(FUNC(bombsa_state::flipscreen_w));
	map(0xd002, 0xd002).rw(FUNC(bombsa_state::bankselect_r), FUNC(bombsa_state::bankselect_w));
	map(0xd003, 0xd003).rw(FUNC(bombsa_state::vram_page_select_r), FUNC(bombsa_state::vram_page_select_w));
	map(0xd005, 0xd005).w(FUNC(bombsa_state::unknown_w)); // ?

	map(0xd200, 0xd7ff).ram().share(m_spriteram);
	map(0xd800, 0xdfff).ram();

	map(0xe000, 0xffff).view(m_vrambank);

	m_vrambank[0](0xe000, 0xffff).ram().w(FUNC(bombsa_state::bg_videoram_w)).share(m_bg_videoram);

	m_vrambank[1](0xe000, 0xe000).portr("SYSTEM");
	m_vrambank[1](0xe001, 0xe001).portr("P1");
	m_vrambank[1](0xe002, 0xe002).portr("P2");
	m_vrambank[1](0xe003, 0xe003).portr("DSW1");
	m_vrambank[1](0xe004, 0xe004).portr("DSW2");

	m_vrambank[1](0xe308, 0xe30c).ram().share(m_bg_control);

	m_vrambank[1](0xe800, 0xefff).ram().w(FUNC(bombsa_state::fg_videoram_w)).share(m_fg_videoram);

	m_vrambank[1](0xf000, 0xf1ff).ram().w(FUNC(bombsa_state::sprite_col_w)).share(m_palette_ram_sp);
	m_vrambank[1](0xf200, 0xf3ff).ram().w(FUNC(bombsa_state::bg_col_w)).share(m_palette_ram_bg);
	m_vrambank[1](0xf400, 0xf5ff).ram().w(FUNC(bombsa_state::tx_col_w)).share(m_palette_ram_tx);
}

void bombsa_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xf000, 0xf000).nopw();                               // Is this a confirm of some sort?
}

void bombsa_state::soundport_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x80, 0x81).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


INPUT_PORTS_START( common )
	PORT_START("SYSTEM")    // system control
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")        // player 1 controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        // player 2 controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( psychic5 )
	PORT_INCLUDE( common )

	PORT_MODIFY("P1")       // player 1 controls
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_MODIFY("P2")       // player 2 controls
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )               PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )               PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

INPUT_PORTS_START( bombsa )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")   // system control
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_MODIFY("P1")       // player 1 control
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")       // player 2 control
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:8" )           // Coin_B
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:6" )           // Coin_B
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:5" )           // Coin_B
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:3" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, "2 Coins 1 Credit/4 Coins 3 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:1" )           // flip screen ?
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8x8 characters
	1024,   // 1024 characters
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 }, // the four bitplanes for pixel are packed into one nibble
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8    // every char takes 32 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16x16 characters
	1024,   // 1024 characters
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 }, // the four bitplanes for pixel are packed into one nibble
	{ 0, 4, 8, 12, 16, 20, 24, 28, 64*8, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8, 32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8 },
	128*8   // every char takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_psychic5 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,  0*16, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,   32*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_bombsa )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 32*16, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0*16,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,   16*16, 16 )
GFXDECODE_END


void psychic5_state::psychic5(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &psychic5_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(psychic5_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, XTAL(5'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &psychic5_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &psychic5_state::soundport_map);

	config.set_maximum_quantum(attotime::from_hz(600));      // Allow time for 2nd CPU to interleave

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000)/2, 394, 0, 256, 282, 16, 240); // was 53.8 Hz before, assume same as Bombs Away
	screen.set_screen_update(FUNC(psychic5_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_psychic5);
	PALETTE(config, m_palette).set_entries(768);

	JALECO_BLEND(config, m_blend, 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(12'000'000)/8));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.50);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(12'000'000)/8));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.50);
}

void bombsa_state::bombsa(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000)/2); // 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &bombsa_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(bombsa_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, XTAL(5'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &bombsa_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &bombsa_state::soundport_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000)/2, 394, 0, 256, 282, 16, 240); // Measured as: VSync 54Hz, HSync 15.25kHz
	screen.set_screen_update(FUNC(bombsa_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bombsa);
	PALETTE(config, m_palette).set_entries(768);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(12'000'000)/8));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.30);
	ym1.add_route(1, "mono", 0.30);
	ym1.add_route(2, "mono", 0.30);
	ym1.add_route(3, "mono", 1.0);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(12'000'000)/8));
	ym2.add_route(0, "mono", 0.30);
	ym2.add_route(1, "mono", 0.30);
	ym2.add_route(2, "mono", 0.30);
	ym2.add_route(3, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

// Main PCB PS-8634
// Tilemap PCB PS-8635
ROM_START( psychic5j )
	ROM_REGION( 0x08000, "maincpu", 0 )     // Main CPU
	ROM_LOAD( "4.7a",         0x00000, 0x08000, CRC(90259249) SHA1(ac2d8dd95f6c04b6ad726136931e37dcd537e977) )

	ROM_REGION( 0x10000, "bankrom", 0 )
	ROM_LOAD( "5.7c",         0x00000, 0x10000, CRC(72298f34) SHA1(725be2fbf5f3622f646c0fb8e6677cbddf0b1fc2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // Sound CPU
	ROM_LOAD( "1.2b",         0x00000, 0x10000, CRC(6efee094) SHA1(ae2b5bf6199121520bf8428b8b160b987f5b474f) )

	ROM_REGION( 0x20000, "gfx1", 0 )        // sprite tiles
	ROM_LOAD( "2.4p",         0x00000, 0x10000, CRC(7e3f87d4) SHA1(b8e7fa3f96d2e3937e4cb530f105bb84d5743b43) )
	ROM_LOAD( "3.4r",         0x10000, 0x10000, CRC(8710fedb) SHA1(c7e8dc6b733e4ecce37d56fc429c00ade8736ff3) )

	ROM_REGION( 0x20000, "gfx2", 0 )        // background tiles
	ROM_LOAD( "7.2k",         0x00000, 0x10000, CRC(f9262f32) SHA1(bae2dc77be7024bd85f213e4da746c5903db6ea5) )
	ROM_LOAD( "8.2m",         0x10000, 0x10000, CRC(c411171a) SHA1(d5893563715ba231e42b084b88f5176bb94a4da9) )

	ROM_REGION( 0x08000, "gfx3", 0 )        // foreground tiles
	ROM_LOAD( "6.5f",         0x00000, 0x08000, CRC(04d7e21c) SHA1(6046c506bdedc233e3730f90c7897e847bec8758) )

	ROM_REGION( 0x08000, "proms", 0 )       // PROMs
	ROM_LOAD( "my10.7l",      0x000, 0x200, CRC(6a7d13c0) SHA1(2a835a4ac1acb7663d0b915d0339af9800284da6) )
	ROM_LOAD( "my09.3t",      0x200, 0x400, CRC(59e44236) SHA1(f53d99694fa5acd7cc51dd78e09f0d2ef730e7a4) )
ROM_END

ROM_START( psychic5 )
	ROM_REGION( 0x08000, "maincpu", 0 )     // Main CPU
	ROM_LOAD( "myp5d",        0x00000, 0x08000, CRC(1d40a8c7) SHA1(79b36e690ea334c066b55b1e39ceb5fe0688cd7b) )

	ROM_REGION( 0x10000, "bankrom", 0 )
	ROM_LOAD( "myp5e",        0x00000, 0x10000, CRC(2fa7e8c0) SHA1(d5096ebec58329346a3292ad2da1be3742fad093) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // Sound CPU
	ROM_LOAD( "myp5a",        0x00000, 0x10000, CRC(6efee094) SHA1(ae2b5bf6199121520bf8428b8b160b987f5b474f) )

	ROM_REGION( 0x20000, "gfx1", 0 )        // sprite tiles
	ROM_LOAD( "p5b",          0x00000, 0x10000, CRC(7e3f87d4) SHA1(b8e7fa3f96d2e3937e4cb530f105bb84d5743b43) )
	ROM_LOAD( "p5c",          0x10000, 0x10000, CRC(8710fedb) SHA1(c7e8dc6b733e4ecce37d56fc429c00ade8736ff3) )

	ROM_REGION( 0x20000, "gfx2", 0 )        // background tiles
	ROM_LOAD( "myp5g",        0x00000, 0x10000, CRC(617b074b) SHA1(7aaac9fddf5675b6698373333db3e096471d7ad6) )
	ROM_LOAD( "myp5h",        0x10000, 0x10000, CRC(a9dfbe67) SHA1(f31f75e88f9b37d7fe5b1a1a8e0299151b729ccf) )

	ROM_REGION( 0x08000, "gfx3", 0 )        // foreground tiles
	ROM_LOAD( "p5f",          0x00000, 0x08000, CRC(04d7e21c) SHA1(6046c506bdedc233e3730f90c7897e847bec8758) )

	ROM_REGION( 0x08000, "proms", 0 )       // PROMs
	ROM_LOAD( "my10.7l",      0x000, 0x200, CRC(6a7d13c0) SHA1(2a835a4ac1acb7663d0b915d0339af9800284da6) )
	ROM_LOAD( "my09.3t",      0x200, 0x400, CRC(59e44236) SHA1(f53d99694fa5acd7cc51dd78e09f0d2ef730e7a4) )
ROM_END

/*
Bombs Away
Jaleco, 1988

PCB Layout
----------

BB-8744
|------------------------------------------|
|         1      Z80(1)          4         |
|         6116      5MHz                  |-|
|VOL      YM2203                 5        | |
|4558     YM2203      Z80(2)              | |
|4558 YM3014                     6        | |
|     YM3014                              | |
|J          6116                6264      | |
|A                                        |-|
|M                                         |
|M                                         |
|A                                        |-|
|                                         | |
|                                         | |
|                2   62256                | |
| DSW1                                    | |
|                3   62256                | |
| DSW2    82S137                          |-|
|                                          |
|------------------------------------------|
Notes:
      Z80(1) clock - 5.000MHz
      Z80(2) clock - 6.000MHz [12/2]
      YM2203 clock - 1.500MHz [12/8, both]
      VSync - 54Hz
      HSync - 15.25kHz


BB-8745
|--------------------------------|
|                                |
|                      |-----|  |-|
|                      |N8633|  | |
|                      |-S   |  | |
|                      |-----|  | |
|  82S123                       | |
|              |-----|          | |
|              |N8633|          |-|
|              |-V   |           |
|2018          |     |           |
|2018        7 |6116 |    12MHz |-|
|              |-----|          | |
|      |-----|                  | |
|      |N8633|                  | |
|      |-V64 |                  | |
|      |     |                  | |
| 9  8 |6264 |                  |-|
|      |-----|                   |
|--------------------------------|
Notes:
      3 soldered-in 'modules' are located on this PCB.
      N-8633-V 6-7 TOYOCOM
      N-8633-V64 6-7 TOYOCOM
      N-8633-S 6-7 TOYOCOM
      The 2 larger ones have 62 pins, 31 pins on each side of a small PCB. The PCB contains some
      surface mounted logic chips and some surface mounted RAM.
      The smaller module has 40 pins, 20 pins on each side of a small PCB. The PCB contains only
      logic on both sides.
*/


ROM_START( bombsa )
	ROM_REGION( 0x08000, "maincpu", 0 )     // Main CPU
	ROM_LOAD( "4.7a",         0x00000, 0x08000, CRC(0191f6a7) SHA1(10a0434abbf4be068751e65c81b1a211729e3742) )

	ROM_REGION( 0x20000, "bankrom", 0 )     // these two fail their self-test, happens on real HW as well.
	ROM_LOAD( "5.7c",         0x00000, 0x08000, CRC(095c451a) SHA1(892ca84376f89640ad4d28f1e548c26bc8f72c0e) )
	ROM_RELOAD(               0x08000, 0x08000 )
	ROM_LOAD( "6.7d",         0x10000, 0x10000, CRC(89f9dc0f) SHA1(5cf6a7aade3d56bc229d3771bc4141ad0c0e8da2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // Sound CPU
	ROM_LOAD( "1.3a",         0x00000, 0x08000, CRC(92801404) SHA1(c4ff47989d355b18a909eaa88f138e2f68178ecc) )

	ROM_REGION( 0x20000, "gfx1", 0 )        // sprite tiles
	ROM_LOAD( "2.4p",         0x00000, 0x10000, CRC(bd972ff4) SHA1(63bfb455bc0ae1d31e6f1066864ec0c8d2d0cf99) )
	ROM_LOAD( "3.4s",         0x10000, 0x10000, CRC(9a8a8a97) SHA1(13328631202c196c9d8791cc6063048eb6be0472) )

	ROM_REGION( 0x20000, "gfx2", 0 )        // background tiles
	// some corrupt 'blank' characters in these
	ROM_LOAD( "8.2l",         0x00000, 0x10000, CRC(3391c769) SHA1(7ae7575ac81d6e0d915c279c1f57a9bc6d096bd6) )
	ROM_LOAD( "9.2m",         0x10000, 0x10000, CRC(5b315976) SHA1(d17cc1926f926bdd88b66ea6af88dac30880e7d4) )

	ROM_REGION( 0x08000, "gfx3", 0 )        // foreground tiles
	ROM_LOAD( "7.4f",         0x00000, 0x08000, CRC(400114b9) SHA1(db2f3ba05a2005ae0e0e7d19c8739353032cbeab) )

	ROM_REGION( 0x08000, "proms", 0 )       // PROMs
	ROM_LOAD( "82s131.7l",    0x000, 0x200, CRC(6a7d13c0) SHA1(2a835a4ac1acb7663d0b915d0339af9800284da6) )
	ROM_LOAD( "82s137.3t",    0x200, 0x400, CRC(59e44236) SHA1(f53d99694fa5acd7cc51dd78e09f0d2ef730e7a4) )
ROM_END

} // anonymous namespace


GAME( 1987, psychic5,  0,        psychic5, psychic5, psychic5_state, empty_init, ROT270, "Jaleco / NMK", "Psychic 5 (World)",      MACHINE_SUPPORTS_SAVE ) // "Oversea's version V2.00 CHANGED BY TAMIO NAKASATO" text present in ROM, various modifications (English names, more complete attract demo etc.)
GAME( 1987, psychic5j, psychic5, psychic5, psychic5, psychic5_state, empty_init, ROT270, "Jaleco / NMK", "Psychic 5 (Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1988, bombsa,    0,        bombsa,   bombsa,   bombsa_state,   empty_init, ROT270, "Jaleco",       "Bombs Away (prototype)", MACHINE_IS_INCOMPLETE | MACHINE_SUPPORTS_SAVE )
