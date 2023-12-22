// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/***************************************************************************

   Taito F3 Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

Brief overview:

    4 scrolling layers (512x512 or 1024x512) of 4, 5, or 6 bpp tiles.
    1 scrolling text layer (512x512, characters generated in vram), 4bpp chars.
    1 scrolling pixel layer (512x256 pixels generated in pivot ram), 4bpp pixels.
    2 sprite banks (for double buffering of sprites)
    Sprites can be 4, 5 or 6 bpp
    Sprite scaling.
    Rowscroll on all playfields
    Line by line zoom on all playfields
    Column scroll on all playfields
    Line by line sprite and playfield priority mixing

Notes:
    All special effects are controlled by an area in 'line ram'.  Typically
    there are 256 values, one for each line of the screen (including clipped
    lines at the top of the screen).  For example, at 0x8000 in lineram,
    there are 4 sets of 256 values (one for each playfield) and each value
    is the scale control for that line in the destination bitmap (screen).
    Therefore each line can have a different zoom value for special effects.

    This also applies to playfield priority, rowscroll, column scroll, sprite
    priority and VRAM/pivot layers.

    However - at the start of line ram there are also sets of 256 values
    with bits controlling each effect subsection, which cause the last allowed
    value to be latched (typically used so a game can use one zoom or row value
    over the whole playfield).

    The programmers of some of these games made strange use of flipscreen -
    some games have all their graphics flipped in ROM, and use the flipscreen
    bit to display them correctly!.

    Most games display 232 scanlines, but some use lineram effects to clip
    themselves to 224 or less.

****************************************************************************

Line ram memory map:

    Here 'playfield 1' refers to the first playfield in memory, etc

  Line set ram: (one word per line, 256 lines per section)
    Each enable bit corresponds to a subsection, e.g. bit 0 sets/latches
    a line in 0x...000-1fe, bit 1 latches a line in 0x...200-3fe, etc.
    Latch enable bits 4,5,6,7 for potential subsections or alternate
    subsections 800,a00,c00,e00 are known to be enabled in rare cases

    0x0000: Line set ram for 4000 (column scroll, alt tilemap, clip) section
        100x
    0x0200: Line set ram for 5000 (clip planes) section
        140x
    0x0400: Line set ram for 6000 (blending) section
        180x
    0x0600: Line set ram for 7000 (pivot and sprite layer mixing) section
        1c0x
    0x0800: Line set ram for 8000 (zoom) section
        200x
    0x0a00: Line set ram for 9000 (palette add) section
        240x
    0x0c00: Line set ram for a000 (row scroll) section
        280x
    0x0e00: Line set ram for b000 (playfield mixing info) section
        2c0x

  "Pivot port" (0x1000-2fff) has only one known used address.
    0x1000: Unknown control word?
        (usually 0x00f0; gseeker, spcinvdj, twinqix, puchicar set 0x0000)


  Line data ram (8 sections, 4 normal subsections, 256 lines each):

    0x4000: Column scroll (playfield 3/4) & clipping
      4400 Bits: RLrl --Ts ssss ssss
          s = playfield 3 column scroll (0-511)
          T = use alternate tilemap (+0x2000) - kirameki, kaiserkn, hthero
          - = unused?
          l = clip 0 left high bit
          r = clip 0 right high bit
          L = clip 1 left high bit
          R = clip 1 right high bit
      4600: as 4400, for playfield 4 / clip plane 2 and 3

    0x5000: Clip plane 0 (low bits (high in 4400))
      5200: Clip plane 1 (low bits (high in 4400))
      5400: Clip plane 2 (low bits (high in 4600))
      5600: Clip plane 3 (low bits (high in 4600))
        Bits:  rrrr rrrr llll llll

    0x6000: Sync register
    0x6004: Sprite alpha control  (+ pivot-related bits?)
        0xff00: VRAM/Pixel layer control
          Bits: B?p? o???
            0xa000 enables pixel layer for this line (Disables VRAM layer)
            0x2000 enables garbage pixels for this line (maybe another pixel bank?) [unemulated]
            0x0800 seems to set the vram layer to be opaque [unemulated]
            Effect of other bits is unknown.
        0x00ff: Bits: DdCc BbAa
        0x00ff: Bits: DdCc BbAa
            Dd = Alpha mode for sprites with pri value 0xc0
            Cc = Alpha mode for sprites with pri value 0x80
            Bb = Alpha mode for sprites with pri value 0x40
            Aa = Alpha mode for sprites with pri value 0x00

    0x6200: Alpha blend values
        Bits: AAAA BBBB CCCC DDDD

    0x6400: forward repeat [unemulated]
        0x00ff: x repeat / mosaic - each tile collapses to 16 single colour lines [unemulated]
          Bits: mmmm 4321
            4321 = enable effect for respective playfield
            mmmm = x repeat - 0 = repeat 1st pixel 16 times (sample every 16)
                              1 = repeat 1st pixel 15 times (sample every 15)
                              f = repeat 1st pixel  1 times (sample every pixel)
            (spcinvdj title screen, riding fight)

       0xff00: previous pixel response? and palette ram format?? [unemulated]
        x1xx = ? (bubsymph, commandw, ridingf)
        3xxx = ? (arabianm, ridingf)
        4xxx = blend forward with next pixel on line (after layer mixing) (gseeker intro)
        7xxx = ? (bubsymph, commandw)
        (old comment:)
        1xxx = interpret palette ram for this playfield line as 15 bit and bilinear filter framebuffer(??)
        3xxx = interpret palette ram for this playfield line as 15 bit
        7xxx = interpret palette ram for this playfield line as 24 bit palette
        8xxx = interpret palette ram for this playfield line as 21 bit palette

    0x6600: Background - background palette entry (under all tilemaps) (takes part in alpha blending)

        (effect not emulated)

    0x7000: Pivot/vram layer enable ? [unemulated?]
        0x0001 - ? (pbobble4)
        0x00ff - ? (quizhuhu)
        0xc000 - ? (commandw, ridingf)
    0x7200: VRAM layer mixing info (incl. priority)
        Bits: BAEI cccc iiii pppp  (see 0xb000)
    0x7400: Sprite mixing info (without priority, like playfield priority clip bits but shifted)
        Bits: ???? ??EI cccc iiii  (see 0xb000)
              ^^^^ ^ set by arabianm, bubsymph, bubblem, cleopatr, commandw, cupfinal, gseeker, spcinv95, twinqix
              ||||
              |||| 0x1000 enable brightness for sprite prio group 0x00 ?
              ||| 0x2000 enable brightness for sprite prio group 0x40 ? (see puchicar vs win/loss)
              || 0x4000 enable brightness for sprite prio group 0x80 ? (see puchicar vs win/loss)
              | 0x8000 enable brightness for sprite prio group 0xc0 ? (see puchicar vs win/loss)
    0x7600: Sprite priority values
        0xf000: Relative priority for sprites with pri value 0xc0
        0x0f00: Relative priority for sprites with pri value 0x80
        0x00f0: Relative priority for sprites with pri value 0x40
        0x000f: Relative priority for sprites with pri value 0x00

    0x8000: Playfield 1 scale (1 word per line, 256 lines, 0x80 = default no scale)
    0x8200: Playfield 2/4 scale
    0x8400: Playfield 3 scale
    0x8600: Playfield 2/4 scale
        0x00ff = Y scale (0x80 = no scale, > 0x80 is zoom in, < 0x80 is zoom out [0xc0 is half height of 0x80])
        0xff00 = X scale (0 = no scale, > 0 = zoom in, [0x8000 would be double length])

        Playfield 2 & 4 registers seem to be interleaved, playfield 2 Y zoom is stored where you would
        expect playfield 4 y zoom to be and vice versa.

    0x9000: Palette add (can affect opacity)
      9200: Playfield 2 palette add
      9400: Playfield 2 palette add
      9600: Playfield 2 palette add

    0xa000: Playfield 1 rowscroll (1 word per line, 256 lines)
      a200: Playfield 2 rowscroll
      a400: Playfield 3 rowscroll
      a600: Playfield 4 rowscroll

    0xb000: Playfield 1 mixing info (layer compositing information)
      b200: Playfield 2 mixing info
      b400: Playfield 3 mixing info
      b600: Playfield 4 mixing info
        Bits: BAEI cccc iiii pppp
          p = Layer priority
          i = Clip inverse mode for corresponding plane
          c = If set, enable corresponding clip plane
          I = Affects interpretation of inverse mode bits. if on, 1 = invert. if off, 0 = invert.
          E = Enable line
          A = Blend alpha mode A (?)
          B = Blend alpha mode B (?)

    0xc000 - 0xffff: Unused.

    When sprite priority==playfield priority sprite takes precedence (Bubble Symph title)

****************************************************************************

    F3 sprite format:

    Word 0: 0xffff      Tile number (LSB)
    Word 1: 0xff00      X zoom
            0x00ff      Y zoom
    Word 2: 0x0fff      X position (12 bits signed)
    Word 3: 0x0fff      Y position (12 bits signed)
    Word 4: 0xf000      Sprite block position controls 
            0x0800      Is child
            0x0400      Reuse prev color
            0x0200      Y flip
            0x0100      X flip
            0x00ff      Colour
    Word 5: 0xc000      ? Unknown/unused?
            0x2000      Flipscreen...?
            0x1000      Sprite is disabled? (check Riding Fight)
            0x0c00      ? Unknown/unused?
            0x0300      Extra planes enable (00 = 4bpp, 01 = 5bpp, 11 = 6bpp)
            0x00fc      ? Unknown/unused?
            0x0002      Set during darius gaiden sprite trails (disable unblitting?)
            0x0001      Tile number (MSB/bankswitch)
    Word 6: 0x8000      If set, jump to sprite location in low bits
            0x03ff      Location to jump to.
    Word 7: 0xffff      Unused?  Always zero?

****************************************************************************

    Playfield control information (0x660000-1f):

    Word 0- 3: X scroll values for the 4 playfields.
    Word 4- 7: Y scroll values for the 4 playfields.
    Word 8-11: Unused.  Always zero.
    Word   12: Pixel + VRAM playfields X scroll
    Word   13: Pixel + VRAM playfields Y scroll
    Word   14: Unused. Always zero.
    Word   15: If set to 0x80, then 1024x512 playfields are used, else 512x512

    top bits unused - writing to low 4 bits is desync or blank tilemap

Playfield tile info:
    0xc000 0000 - X/Y Flip
    0x3000 ffff - Tile index
    0x0c00 0000 - Extra planes enable (00 = 4bpp, 01 = 5bpp, 10 = unused?, 11 = 6bpp)
    0x0200 0000 - Alpha blend mode
    0x01ff 0000 - Colour

***************************************************************************/

#include "emu.h"
#include "taito_f3.h"
#include "render.h"

#include <algorithm>
#include <variant>

#define VERBOSE 0
#define DARIUSG_KLUDGE
#define TAITOF3_VIDEO_DEBUG 0

typedef int fixed4;
typedef int fixed8;

// Game specific data - some of this can be removed when the software values are figured out
struct taito_f3_state::F3config
{
	int name;
	int extend;     // playfield control 0x1F bit 7
	int sprite_lag;
};

const taito_f3_state::F3config taito_f3_state::f3_config_table[] =
{
	/* Name    Extend  Lag */
	{ RINGRAGE,  0,     2 },
	{ ARABIANM,  0,     2 },
	{ RIDINGF,   1,     1 },
	{ GSEEKER,   0,     1 },
	{ COMMANDW,  1,     1 },
	{ SCFINALS,  0,     1 },
	{ TRSTAR,    1,     0 },
	{ GUNLOCK,   1,     2 },
	{ LIGHTBR,   1,     2 },
	{ KAISERKN,  0,     2 },
	{ DARIUSG,   0,     2 },
	{ BUBSYMPH,  1,     1 },
	{ SPCINVDX,  1,     1 },
	{ HTHERO95,  0,     1 },
	{ QTHEATER,  1,     1 },
	{ EACTION2,  1,     2 },
	{ RECALH,    1,     1 },
	{ SPCINV95,  0,     1 },
	{ TWINQIX,   1,     1 },
	{ QUIZHUHU,  1,     1 },
	{ PBOBBLE2,  0,     1 },
	{ GEKIRIDO,  0,     1 },
	{ KTIGER2,   0,     0 },
	{ BUBBLEM,   1,     1 },
	{ CLEOPATR,  0,     1 },
	{ PBOBBLE3,  0,     1 },
	{ ARKRETRN,  1,     1 },
	{ KIRAMEKI,  0,     1 },
	{ PUCHICAR,  1,     1 },
	{ PBOBBLE4,  0,     1 },
	{ POPNPOP,   1,     1 },
	{ LANDMAKR,  1,     1 },
	{ TMDRILL,   1,     0 },
	{0}
};


/*
alpha_mode
---- --xx    0:disable 1:nomal 2:alpha 7000 3:alpha b000
---1 ----    alpha level a
--1- ----    alpha level b
1--------    opaque line
*/


/*
pri_alp_bitmap
---- ---1    sprite priority 0
---- --1-    sprite priority 1
---- -1--    sprite priority 2
---- 1---    sprite priority 3
---1 ----    alpha level a 7000
--1- ----    alpha level b 7000
-1-- ----    alpha level a b000
1--- ----    alpha level b b000
1111 1111    opaque pixel
*/


void taito_f3_state::device_post_load()
{
	/* force a reread of the dynamic tiles in the pixel layer */
	m_gfxdecode->gfx(0)->mark_all_dirty();
	m_gfxdecode->gfx(1)->mark_all_dirty();
}

/******************************************************************************/

void taito_f3_state::print_debug_info(bitmap_rgb32 &bitmap)
{
#if (TAITOF3_VIDEO_DEBUG)
	const u16 *line_ram = m_line_ram;
	popmessage("%04X %04X %04X %04X %04X %04X %04X %04X\n"
			   "%04X %04X %04X %04X %04X %04X %04X %04X\n"
			   "%04X %04X %04X %04X %04X %04X %04X %04X\n"
			   "Ctr1: %04x %04x %04x %04x\n"
			   "Ctr2: %04x %04x %04x %04x\n"
			   "Colm: %04x %04x %04x %04x\n"
			   "Clip: %04x %04x %04x %04x\n"
			   "Sprt: %04x %04x %04x %04x\n"
			   "Pivt: %04x %04x %04x %04x\n"
			   "Zoom: %04x %04x %04x %04x\n"
			   "Line: %04x %04x %04x %04x\n"
			   "Pri : %04x %04x %04x %04x\n",
			   m_spriteram16_buffered[0], m_spriteram16_buffered[1], m_spriteram16_buffered[2], m_spriteram16_buffered[3], m_spriteram16_buffered[4], m_spriteram16_buffered[5], m_spriteram16_buffered[6], m_spriteram16_buffered[7],
			   m_spriteram16_buffered[8], m_spriteram16_buffered[9], m_spriteram16_buffered[10], m_spriteram16_buffered[11], m_spriteram16_buffered[12], m_spriteram16_buffered[13], m_spriteram16_buffered[14], m_spriteram16_buffered[15],
			   m_spriteram16_buffered[16], m_spriteram16_buffered[17], m_spriteram16_buffered[18], m_spriteram16_buffered[19], m_spriteram16_buffered[20], m_spriteram16_buffered[21], m_spriteram16_buffered[22], m_spriteram16_buffered[23],
			   line_ram[0x0100/2] & 0xffff, line_ram[0x0300/2] & 0xffff, line_ram[0x0500/2] & 0xffff, line_ram[0x0700/2] & 0xffff,
			   line_ram[0x0900/2] & 0xffff, line_ram[0x0b00/2] & 0xffff, line_ram[0x0d00/2] & 0xffff, line_ram[0x0f00/2] & 0xffff,
			   line_ram[0x4180/2] & 0xffff, line_ram[0x4380/2] & 0xffff, line_ram[0x4580/2] & 0xffff, line_ram[0x4780/2] & 0xffff,
			   line_ram[0x5180/2] & 0xffff, line_ram[0x5380/2] & 0xffff, line_ram[0x5580/2] & 0xffff, line_ram[0x5780/2] & 0xffff,
			   line_ram[0x6180/2] & 0xffff, line_ram[0x6380/2] & 0xffff, line_ram[0x6580/2] & 0xffff, line_ram[0x6780/2] & 0xffff,
			   line_ram[0x7180/2] & 0xffff, line_ram[0x7380/2] & 0xffff, line_ram[0x7580/2] & 0xffff, line_ram[0x7780/2] & 0xffff,
			   line_ram[0x8180/2] & 0xffff, line_ram[0x8380/2] & 0xffff, line_ram[0x8580/2] & 0xffff, line_ram[0x8780/2] & 0xffff,
			   line_ram[0xa180/2] & 0xffff, line_ram[0xa380/2] & 0xffff, line_ram[0xa580/2] & 0xffff, line_ram[0xa780/2] & 0xffff,
			   line_ram[0xb180/2] & 0xffff, line_ram[0xb380/2] & 0xffff, line_ram[0xb580/2] & 0xffff, line_ram[0xb780/2] & 0xffff)
#endif
}

/******************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info)
{
	const u32 tile = (m_pf_data[Layer][tile_index * 2 + 0] << 16) | (m_pf_data[Layer][tile_index * 2 + 1] & 0xffff);
	const u16 palette_code = BIT(tile, 16, 9);
	const u8 abtype        = BIT(tile, 25, 1);
	const u8 extra_planes  = BIT(tile, 26, 2); // 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp

	tileinfo.set(3,
			tile & 0xffff,
			palette_code,
			TILE_FLIPYX(BIT(tile, 30, 2)));

	tileinfo.category = abtype & 1; // alpha blending type
	// gfx extra planes and palette code set the same bits of color address
	// we need to account for tilemap.h combining using "+" instead of "|"
	tileinfo.pen_mask = ((extra_planes & ~palette_code) << 4) | 0x0f;
}


TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info_text)
{
	u8 flags = 0;

	const u16 vram_tile = (m_textram[tile_index] & 0xffff);

	if (vram_tile & 0x0100) flags |= TILE_FLIPX;
	if (vram_tile & 0x8000) flags |= TILE_FLIPY;

	tileinfo.set(0,
			vram_tile & 0xff,
			(vram_tile >> 9) & 0x3f,
			flags);
}

TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info_pixel)
{
	int col_off;
	u8 flags = 0;
	int y_offs = (m_control_1[5] & 0x1ff);
	if (m_flipscreen) y_offs += 0x100;

	/* Colour is shared with VRAM layer */
	if ((((tile_index & 0x1f) * 8 + y_offs) & 0x1ff) > 0xff)
		col_off = 0x800 + ((tile_index & 0x1f) << 6) + ((tile_index & 0xfe0) >> 5);
	else
		col_off = ((tile_index & 0x1f) << 6) + ((tile_index & 0xfe0) >> 5);

	const u16 vram_tile = (m_textram[col_off] & 0xffff);

	if (vram_tile & 0x0100) flags |= TILE_FLIPX;
	if (vram_tile & 0x8000) flags |= TILE_FLIPY;

	tileinfo.set(1,
			tile_index,
			(vram_tile >> 9) & 0x3f,
			flags);
}

/******************************************************************************/

void taito_f3_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		if (m_sprite_lag==2)
		{
			if (machine().video().skip_this_frame() == 0)
			{
				get_sprite_info(m_spriteram16_buffered.get());
			}
			memcpy(m_spriteram16_buffered.get(), m_spriteram.target(), 0x10000);
		}
		else if (m_sprite_lag == 1)
		{
			if (machine().video().skip_this_frame() == 0)
			{
				get_sprite_info(m_spriteram.target());
			}
		}
	}
}

void taito_f3_state::video_start()
{
	const F3config *pCFG = &f3_config_table[0];

	m_alpha_level_2as = 127;
	m_alpha_level_2ad = 127;
	m_alpha_level_3as = 127;
	m_alpha_level_3ad = 127;
	m_alpha_level_2bs = 127;
	m_alpha_level_2bd = 127;
	m_alpha_level_3bs = 127;
	m_alpha_level_3bd = 127;
	m_alpha_level_last = -1;

	m_pdest_2a = 0x10;
	m_pdest_2b = 0x20;
	m_tr_2a = 0;
	m_tr_2b = 1;
	m_pdest_3a = 0x40;
	m_pdest_3b = 0x80;
	m_tr_3a = 0;
	m_tr_3b = 1;

	m_spritelist = nullptr;
	m_spriteram16_buffered = nullptr;
	m_line_inf = nullptr;
	m_pf_line_inf = nullptr;
	m_tile_opaque_sp = nullptr;

	/* Setup individual game */
	do {
		if (pCFG->name == m_game)
		{
			break;
		}
		pCFG++;
	} while (pCFG->name);

	m_game_config=pCFG;

	if (m_game_config->extend)
	{
		m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[4] = m_tilemap[5] = m_tilemap[6] = m_tilemap[7] = nullptr;

		m_pf_data[0] = m_pf_ram + (0x0000 / 2);
		m_pf_data[1] = m_pf_ram + (0x2000 / 2);
		m_pf_data[2] = m_pf_ram + (0x4000 / 2);
		m_pf_data[3] = m_pf_ram + (0x6000 / 2);

		m_width_mask = 0x3ff;
		m_twidth_mask = 0x7f;
		m_twidth_mask_bit = 7;

		m_tilemap[0]->set_transparent_pen(0);
		m_tilemap[1]->set_transparent_pen(0);
		m_tilemap[2]->set_transparent_pen(0);
		m_tilemap[3]->set_transparent_pen(0);
	}
	else
	{
		m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<4>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<5>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[6] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<6>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[7] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<7>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

		m_pf_data[0] = m_pf_ram + (0x0000 / 2);
		m_pf_data[1] = m_pf_ram + (0x1000 / 2);
		m_pf_data[2] = m_pf_ram + (0x2000 / 2);
		m_pf_data[3] = m_pf_ram + (0x3000 / 2);
		m_pf_data[4] = m_pf_ram + (0x4000 / 2);
		m_pf_data[5] = m_pf_ram + (0x5000 / 2);
		m_pf_data[6] = m_pf_ram + (0x6000 / 2);
		m_pf_data[7] = m_pf_ram + (0x7000 / 2);

		m_width_mask = 0x1ff;
		m_twidth_mask = 0x3f;
		m_twidth_mask_bit = 6;

		m_tilemap[0]->set_transparent_pen(0);
		m_tilemap[1]->set_transparent_pen(0);
		m_tilemap[2]->set_transparent_pen(0);
		m_tilemap[3]->set_transparent_pen(0);
		m_tilemap[4]->set_transparent_pen(0);
		m_tilemap[5]->set_transparent_pen(0);
		m_tilemap[6]->set_transparent_pen(0);
		m_tilemap[7]->set_transparent_pen(0);
	}

	m_spriteram16_buffered = std::make_unique<u16[]>(0x10000 / 2);
	m_spritelist = std::make_unique<tempsprite[]>(0x400);
	m_sprite_end = &m_spritelist[0];
	m_vram_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info_text)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_pixel_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info_pixel)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
	m_line_inf = std::make_unique<f3_line_inf[]>(256);
	m_pf_line_inf = std::make_unique<f3_playfield_line_inf[]>(5);
	m_sa_line_inf = std::make_unique<f3_spritealpha_line_inf[]>(1);
	m_screen->register_screen_bitmap(m_pri_alp_bitmap);
	m_tile_opaque_sp = std::make_unique<u8[]>(m_gfxdecode->gfx(2)->elements());
	for (int i = 0; i < 8; i++)
		m_tile_opaque_pf[i] = std::make_unique<u8[]>(m_gfxdecode->gfx(3)->elements());

	m_vram_layer->set_transparent_pen(0);
	m_pixel_layer->set_transparent_pen(0);

	// Palettes have 4 bpp indexes despite up to 6 bpp data. The unused top bits in the gfx data are cleared later.
	m_gfxdecode->gfx(2)->set_granularity(16);
	m_gfxdecode->gfx(3)->set_granularity(16);

	m_flipscreen = false;
	memset(m_spriteram16_buffered.get(), 0, 0x10000);
	memset(&m_spriteram[0], 0, 0x10000);

	save_item(NAME(m_control_0));
	save_item(NAME(m_control_1));

	m_gfxdecode->gfx(0)->set_source((u8 *)m_charram.target());
	m_gfxdecode->gfx(1)->set_source((u8 *)m_pivot_ram.target());

	m_sprite_lag = m_game_config->sprite_lag;

	init_alpha_blend_func();

	{
		gfx_element *sprite_gfx = m_gfxdecode->gfx(2);

		for (int c = 0; c < sprite_gfx->elements(); c++)
		{
			int chk_trans_or_opa = 0;
			const u8 *dp = sprite_gfx->get_data(c);
			for (int y = 0; y < sprite_gfx->height(); y++)
			{
				for (int x = 0; x < sprite_gfx->width(); x++)
				{
					if (!dp[x]) chk_trans_or_opa |= 2;
					else        chk_trans_or_opa |= 1;
				}
				dp += sprite_gfx->rowbytes();
			}
			if (chk_trans_or_opa == 1) m_tile_opaque_sp[c] = 1;
			else                       m_tile_opaque_sp[c] = 0;
		}
	}

	{
		gfx_element *pf_gfx = m_gfxdecode->gfx(3);

		for (int c = 0; c < pf_gfx->elements(); c++)
		{
			for (int extra_planes = 0; extra_planes < 4; extra_planes++)
			{
				int chk_trans_or_opa = 0;
				/* 0 = 4bpp, 1=5bpp, 2=?, 3=6bpp */
				const u8 extra_mask = ((extra_planes << 4) | 0x0f);
				const u8 *dp = pf_gfx->get_data(c);

				for (int y = 0; y < pf_gfx->height(); y++)
				{
					for (int x = 0; x < pf_gfx->width(); x++)
					{
						if (!(dp[x] & extra_mask))
							chk_trans_or_opa |= 2;
						else
							chk_trans_or_opa |= 1;
					}
					dp += pf_gfx->rowbytes();
				}
				m_tile_opaque_pf[extra_planes][c] = chk_trans_or_opa;
			}
		}
	}
}

/******************************************************************************/

u16 taito_f3_state::pf_ram_r(offs_t offset)
{
	return m_pf_ram[offset];
}

void taito_f3_state::pf_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pf_ram[offset]);

	if (m_game_config->extend)
	{
		if (offset < 0x4000)
			m_tilemap[offset >> 12]->mark_tile_dirty((offset & 0xfff) >> 1);
	}
	else
	{
		if (offset < 0x4000)
			m_tilemap[offset >> 11]->mark_tile_dirty((offset & 0x7ff) >> 1);
	}
}

void taito_f3_state::control_0_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_control_0[offset]);
}

void taito_f3_state::control_1_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_control_1[offset]);
}

u16 taito_f3_state::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void taito_f3_state::spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

u16 taito_f3_state::textram_r(offs_t offset)
{
	return m_textram[offset];
}

void taito_f3_state::textram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);

	m_vram_layer->mark_tile_dirty(offset);
	//m_vram_layer->mark_tile_dirty(offset + 1);

	if (offset > 0x7ff) offset -= 0x800;

	const int tile = offset;
	const int col_off = ((tile & 0x3f) << 5) + ((tile & 0xfc0) >> 6);

	m_pixel_layer->mark_tile_dirty(col_off);
	//m_pixel_layer->mark_tile_dirty(col_off+32);
}


u16 taito_f3_state::charram_r(offs_t offset)
{
	return m_charram[offset];
}

void taito_f3_state::charram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_charram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
}

u16 taito_f3_state::pivot_r(offs_t offset)
{
	return m_pivot_ram[offset];
}

void taito_f3_state::pivot_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pivot_ram[offset]);
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 4);
}

u16 taito_f3_state::lineram_r(offs_t offset)
{
	return m_line_ram[offset];
}

void taito_f3_state::lineram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_line_ram[offset]);
}

void taito_f3_state::palette_24bit_w(offs_t offset, u32 data, u32 mem_mask)
{
	int r, g, b;

	COMBINE_DATA(&m_paletteram32[offset]);

	/* 12 bit palette games - there has to be a palette select bit somewhere */
	if (m_game == SPCINVDX || m_game == RIDINGF || m_game == ARABIANM || m_game == RINGRAGE)
	{
		b = 15 * ((m_paletteram32[offset] >> 4) & 0xf);
		g = 15 * ((m_paletteram32[offset] >> 8) & 0xf);
		r = 15 * ((m_paletteram32[offset] >> 12) & 0xf);
	}

	/* This is weird - why are only the sprites and VRAM palettes 21 bit? */
	else if (m_game == CLEOPATR)
	{
		if (offset < 0x100 || offset > 0x1000)
		{
			r = ((m_paletteram32[offset] >> 16) & 0x7f) << 1;
			g = ((m_paletteram32[offset] >> 8) & 0x7f) << 1;
			b = ((m_paletteram32[offset] >> 0) & 0x7f) << 1;
		}
		else
		{
			r = (m_paletteram32[offset] >> 16) & 0xff;
			g = (m_paletteram32[offset] >> 8) & 0xff;
			b = (m_paletteram32[offset] >> 0) & 0xff;
		}
	}

	/* Another weird couple - perhaps this is alpha blending related? */
	else if (m_game == TWINQIX || m_game == RECALH)
	{
		if (offset > 0x1c00)
		{
			r = ((m_paletteram32[offset] >> 16) & 0x7f) << 1;
			g = ((m_paletteram32[offset] >> 8) & 0x7f) << 1;
			b = ((m_paletteram32[offset] >> 0) & 0x7f) << 1;
		}
		else
		{
			r = (m_paletteram32[offset] >> 16) & 0xff;
			g = (m_paletteram32[offset] >> 8) & 0xff;
			b = (m_paletteram32[offset] >> 0) & 0xff;
		}
	}

	/* All other games - standard 24 bit palette */
	else
	{
		r = (m_paletteram32[offset] >> 16) & 0xff;
		g = (m_paletteram32[offset] >> 8) & 0xff;
		b = (m_paletteram32[offset] >> 0) & 0xff;
	}

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

/******************************************************************************/

/*============================================================================*/

inline void taito_f3_state::alpha_set_level()
{
	const auto set_alpha_level = [](int &d, const u8 s)
	{
		if (s == 0)
		{
			d = 0;
		}
		else
		{
			d = s + 1;
		}
	};

//  set_alpha_level(m_alpha_s_1_1, m_alpha_level_2ad);
	set_alpha_level(m_alpha_s_1_1, 255 - m_alpha_level_2as);
//  set_alpha_level(m_alpha_s_1_2, m_alpha_level_2bd);
	set_alpha_level(m_alpha_s_1_2, 255 - m_alpha_level_2bs);
	set_alpha_level(m_alpha_s_1_4, m_alpha_level_3ad);
//  set_alpha_level(m_alpha_s_1_5, m_alpha_level_3ad*m_alpha_level_2ad / 255);
	set_alpha_level(m_alpha_s_1_5, m_alpha_level_3ad * (255 - m_alpha_level_2as) / 255);
//  set_alpha_level(m_alpha_s_1_6, m_alpha_level_3ad*m_alpha_level_2bd / 255);
	set_alpha_level(m_alpha_s_1_6, m_alpha_level_3ad * (255 - m_alpha_level_2bs) / 255);
	set_alpha_level(m_alpha_s_1_8, m_alpha_level_3bd);
//  set_alpha_level(m_alpha_s_1_9, m_alpha_level_3bd*m_alpha_level_2ad / 255);
	set_alpha_level(m_alpha_s_1_9, m_alpha_level_3bd * (255 - m_alpha_level_2as) / 255);
//  set_alpha_level(m_alpha_s_1_a, m_alpha_level_3bd*m_alpha_level_2bd / 255);
	set_alpha_level(m_alpha_s_1_a, m_alpha_level_3bd * (255 - m_alpha_level_2bs) / 255);

	set_alpha_level(m_alpha_s_2a_0, m_alpha_level_2as);
	set_alpha_level(m_alpha_s_2a_4, m_alpha_level_2as * m_alpha_level_3ad / 255);
	set_alpha_level(m_alpha_s_2a_8, m_alpha_level_2as * m_alpha_level_3bd / 255);

	set_alpha_level(m_alpha_s_2b_0, m_alpha_level_2bs);
	set_alpha_level(m_alpha_s_2b_4, m_alpha_level_2bs * m_alpha_level_3ad / 255);
	set_alpha_level(m_alpha_s_2b_8, m_alpha_level_2bs * m_alpha_level_3bd / 255);

	set_alpha_level(m_alpha_s_3a_0, m_alpha_level_3as);
	set_alpha_level(m_alpha_s_3a_1, m_alpha_level_3as * m_alpha_level_2ad / 255);
	set_alpha_level(m_alpha_s_3a_2, m_alpha_level_3as * m_alpha_level_2bd / 255);

	set_alpha_level(m_alpha_s_3b_0, m_alpha_level_3bs);
	set_alpha_level(m_alpha_s_3b_1, m_alpha_level_3bs * m_alpha_level_2ad / 255);
	set_alpha_level(m_alpha_s_3b_2, m_alpha_level_3bs * m_alpha_level_2bd / 255);
}

/*============================================================================*/

#define COLOR1 BYTE4_XOR_LE(0)
#define COLOR2 BYTE4_XOR_LE(1)
#define COLOR3 BYTE4_XOR_LE(2)

inline void taito_f3_state::alpha_blend32_s(int alphas, u32 s)
{
	u8 *sc = (u8 *)&s;
	u8 *dc = (u8 *)&m_dval;
	dc[COLOR1] = (alphas * sc[COLOR1]) >> 8;
	dc[COLOR2] = (alphas * sc[COLOR2]) >> 8;
	dc[COLOR3] = (alphas * sc[COLOR3]) >> 8;
}

inline void taito_f3_state::alpha_blend32_d(int alphas, u32 s)
{
	u8 *sc = (u8 *)&s;
	u8 *dc = (u8 *)&m_dval;
	dc[COLOR1] = std::min<unsigned>(dc[COLOR1] + ((alphas * sc[COLOR1]) >> 8), 255U);
	dc[COLOR2] = std::min<unsigned>(dc[COLOR2] + ((alphas * sc[COLOR2]) >> 8), 255U);
	dc[COLOR3] = std::min<unsigned>(dc[COLOR3] + ((alphas * sc[COLOR3]) >> 8), 255U);
}

/*============================================================================*/

inline void taito_f3_state::alpha_blend_1_1(u32 s) { alpha_blend32_d(m_alpha_s_1_1, s); }
inline void taito_f3_state::alpha_blend_1_2(u32 s) { alpha_blend32_d(m_alpha_s_1_2, s); }
inline void taito_f3_state::alpha_blend_1_4(u32 s) { alpha_blend32_d(m_alpha_s_1_4, s); }
inline void taito_f3_state::alpha_blend_1_5(u32 s) { alpha_blend32_d(m_alpha_s_1_5, s); }
inline void taito_f3_state::alpha_blend_1_6(u32 s) { alpha_blend32_d(m_alpha_s_1_6, s); }
inline void taito_f3_state::alpha_blend_1_8(u32 s) { alpha_blend32_d(m_alpha_s_1_8, s); }
inline void taito_f3_state::alpha_blend_1_9(u32 s) { alpha_blend32_d(m_alpha_s_1_9, s); }
inline void taito_f3_state::alpha_blend_1_a(u32 s) { alpha_blend32_d(m_alpha_s_1_a, s); }

inline void taito_f3_state::alpha_blend_2a_0(u32 s) { alpha_blend32_s(m_alpha_s_2a_0, s); }
inline void taito_f3_state::alpha_blend_2a_4(u32 s) { alpha_blend32_d(m_alpha_s_2a_4, s); }
inline void taito_f3_state::alpha_blend_2a_8(u32 s) { alpha_blend32_d(m_alpha_s_2a_8, s); }

inline void taito_f3_state::alpha_blend_2b_0(u32 s) { alpha_blend32_s(m_alpha_s_2b_0, s); }
inline void taito_f3_state::alpha_blend_2b_4(u32 s) { alpha_blend32_d(m_alpha_s_2b_4, s); }
inline void taito_f3_state::alpha_blend_2b_8(u32 s) { alpha_blend32_d(m_alpha_s_2b_8, s); }

inline void taito_f3_state::alpha_blend_3a_0(u32 s) { alpha_blend32_s(m_alpha_s_3a_0, s); }
inline void taito_f3_state::alpha_blend_3a_1(u32 s) { alpha_blend32_d(m_alpha_s_3a_1, s); }
inline void taito_f3_state::alpha_blend_3a_2(u32 s) { alpha_blend32_d(m_alpha_s_3a_2, s); }

inline void taito_f3_state::alpha_blend_3b_0(u32 s) { alpha_blend32_s(m_alpha_s_3b_0, s); }
inline void taito_f3_state::alpha_blend_3b_1(u32 s) { alpha_blend32_d(m_alpha_s_3b_1, s); }
inline void taito_f3_state::alpha_blend_3b_2(u32 s) { alpha_blend32_d(m_alpha_s_3b_2, s); }

/*============================================================================*/

inline bool taito_f3_state::dpix_1_noalpha(u32 s_pix) { m_dval = s_pix; return true; }
inline bool taito_f3_state::dpix_ret1(u32 s_pix) { return true; }
inline bool taito_f3_state::dpix_ret0(u32 s_pix) { return false; }
inline bool taito_f3_state::dpix_1_1(u32 s_pix) { if (s_pix) alpha_blend_1_1(s_pix); return true; }
inline bool taito_f3_state::dpix_1_2(u32 s_pix) { if (s_pix) alpha_blend_1_2(s_pix); return true; }
inline bool taito_f3_state::dpix_1_4(u32 s_pix) { if (s_pix) alpha_blend_1_4(s_pix); return true; }
inline bool taito_f3_state::dpix_1_5(u32 s_pix) { if (s_pix) alpha_blend_1_5(s_pix); return true; }
inline bool taito_f3_state::dpix_1_6(u32 s_pix) { if (s_pix) alpha_blend_1_6(s_pix); return true; }
inline bool taito_f3_state::dpix_1_8(u32 s_pix) { if (s_pix) alpha_blend_1_8(s_pix); return true; }
inline bool taito_f3_state::dpix_1_9(u32 s_pix) { if (s_pix) alpha_blend_1_9(s_pix); return true; }
inline bool taito_f3_state::dpix_1_a(u32 s_pix) { if (s_pix) alpha_blend_1_a(s_pix); return true; }

bool taito_f3_state::dpix_2a_0(u32 s_pix)
{
	if (s_pix) alpha_blend_2a_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_2a) { m_pval |= m_pdest_2a; return false; }
	return true;
}
bool taito_f3_state::dpix_2a_4(u32 s_pix)
{
	if (s_pix) alpha_blend_2a_4(s_pix);
	if (m_pdest_2a) { m_pval |= m_pdest_2a; return false; }
	return true;
}
bool taito_f3_state::dpix_2a_8(u32 s_pix)
{
	if (s_pix) alpha_blend_2a_8(s_pix);
	if (m_pdest_2a) { m_pval |= m_pdest_2a; return false; }
	return true;
}

bool taito_f3_state::dpix_3a_0(u32 s_pix)
{
	if (s_pix) alpha_blend_3a_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_3a) { m_pval |= m_pdest_3a; return false; }
	return true;
}
bool taito_f3_state::dpix_3a_1(u32 s_pix)
{
	if (s_pix) alpha_blend_3a_1(s_pix);
	if (m_pdest_3a) { m_pval |= m_pdest_3a; return false; }
	return true;
}
bool taito_f3_state::dpix_3a_2(u32 s_pix)
{
	if (s_pix) alpha_blend_3a_2(s_pix);
	if (m_pdest_3a) { m_pval |= m_pdest_3a; return false; }
	return true;
}

bool taito_f3_state::dpix_2b_0(u32 s_pix)
{
	if (s_pix) alpha_blend_2b_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_2b) { m_pval |= m_pdest_2b; return false; }
	return true;
}
bool taito_f3_state::dpix_2b_4(u32 s_pix)
{
	if (s_pix) alpha_blend_2b_4(s_pix);
	if (m_pdest_2b) { m_pval |= m_pdest_2b; return false; }
	return true;
}
bool taito_f3_state::dpix_2b_8(u32 s_pix)
{
	if (s_pix) alpha_blend_2b_8(s_pix);
	if (m_pdest_2b) { m_pval |= m_pdest_2b; return false; }
	return true;
}

bool taito_f3_state::dpix_3b_0(u32 s_pix)
{
	if (s_pix) alpha_blend_3b_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_3b) { m_pval |= m_pdest_3b; return false; }
	return true;
}
bool taito_f3_state::dpix_3b_1(u32 s_pix)
{
	if (s_pix) alpha_blend_3b_1(s_pix);
	if (m_pdest_3b) { m_pval |= m_pdest_3b; return false; }
	return true;
}
bool taito_f3_state::dpix_3b_2(u32 s_pix)
{
	if (s_pix) alpha_blend_3b_2(s_pix);
	if (m_pdest_3b) { m_pval |= m_pdest_3b; return false; }
	return true;
}

bool taito_f3_state::dpix_2_0(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_2b)      { alpha_blend_2b_0(s_pix); if (m_pdest_2b) m_pval |= m_pdest_2b; else return true; }
		else if (tr2 == m_tr_2a) { alpha_blend_2a_0(s_pix); if (m_pdest_2a) m_pval |= m_pdest_2a; else return true; }
	}
	else
	{
		if (tr2 == m_tr_2b)      { m_dval = 0; if (m_pdest_2b) m_pval |= m_pdest_2b; else return true; }
		else if (tr2 == m_tr_2a) { m_dval = 0; if (m_pdest_2a) m_pval |= m_pdest_2a; else return true; }
	}
	return false;
}
bool taito_f3_state::dpix_2_4(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_2b)      { alpha_blend_2b_4(s_pix); if (m_pdest_2b) m_pval |= m_pdest_2b; else return true; }
		else if (tr2 == m_tr_2a) { alpha_blend_2a_4(s_pix); if (m_pdest_2a) m_pval |= m_pdest_2a; else return true; }
	}
	else
	{
		if (tr2 == m_tr_2b)      { if (m_pdest_2b) m_pval |= m_pdest_2b; else return true; }
		else if (tr2 == m_tr_2a) { if (m_pdest_2a) m_pval |= m_pdest_2a; else return true; }
	}
	return false;
}
bool taito_f3_state::dpix_2_8(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_2b)      { alpha_blend_2b_8(s_pix); if (m_pdest_2b) m_pval |= m_pdest_2b; else return true; }
		else if (tr2 == m_tr_2a) { alpha_blend_2a_8(s_pix); if (m_pdest_2a) m_pval |= m_pdest_2a; else return true; }
	}
	else
	{
		if (tr2 == m_tr_2b)      { if (m_pdest_2b) m_pval |= m_pdest_2b; else return true; }
		else if (tr2 == m_tr_2a) { if (m_pdest_2a) m_pval |= m_pdest_2a; else return true; }
	}
	return false;
}

bool taito_f3_state::dpix_3_0(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_3b)      { alpha_blend_3b_0(s_pix); if (m_pdest_3b) m_pval |= m_pdest_3b; else return true; }
		else if (tr2 == m_tr_3a) { alpha_blend_3a_0(s_pix); if (m_pdest_3a) m_pval |= m_pdest_3a; else return true; }
	}
	else
	{
		if (tr2 == m_tr_3b)      { m_dval = 0; if (m_pdest_3b) m_pval |= m_pdest_3b; else return true; }
		else if (tr2 == m_tr_3a) { m_dval = 0; if (m_pdest_3a) m_pval |= m_pdest_3a; else return true; }
	}
	return false;
}
bool taito_f3_state::dpix_3_1(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_3b)      { alpha_blend_3b_1(s_pix); if (m_pdest_3b) m_pval |= m_pdest_3b; else return true; }
		else if (tr2 == m_tr_3a) { alpha_blend_3a_1(s_pix); if (m_pdest_3a) m_pval |= m_pdest_3a; else return true; }
	}
	else
	{
		if (tr2 == m_tr_3b)      { if (m_pdest_3b) m_pval |= m_pdest_3b; else return true; }
		else if (tr2 == m_tr_3a) { if (m_pdest_3a) m_pval |= m_pdest_3a; else return true; }
	}
	return false;
}
bool taito_f3_state::dpix_3_2(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_3b)      { alpha_blend_3b_2(s_pix); if (m_pdest_3b) m_pval |= m_pdest_3b; else return true; }
		else if (tr2 == m_tr_3a) { alpha_blend_3a_2(s_pix); if (m_pdest_3a) m_pval |= m_pdest_3a; else return true; }
	}
	else
	{
		if (tr2 == m_tr_3b)      { if (m_pdest_3b) m_pval |= m_pdest_3b; else return true; }
		else if (tr2 == m_tr_3a) { if (m_pdest_3a) m_pval |= m_pdest_3a; else return true; }
	}
	return false;
}

inline void taito_f3_state::dpix_1_sprite(u32 s_pix)
{
	if (s_pix)
	{
		const u8 p1 = m_pval & 0xf0;
		if      (p1 == 0x10) alpha_blend_1_1(s_pix);
		else if (p1 == 0x20) alpha_blend_1_2(s_pix);
		else if (p1 == 0x40) alpha_blend_1_4(s_pix);
		else if (p1 == 0x50) alpha_blend_1_5(s_pix);
		else if (p1 == 0x60) alpha_blend_1_6(s_pix);
		else if (p1 == 0x80) alpha_blend_1_8(s_pix);
		else if (p1 == 0x90) alpha_blend_1_9(s_pix);
		else if (p1 == 0xa0) alpha_blend_1_a(s_pix);
	}
}

inline void taito_f3_state::dpix_bg(u32 bgcolor)
{
	const u8 p1 = m_pval & 0xf0;
	if (!p1)         m_dval = bgcolor;
	else if (p1 == 0x10) alpha_blend_1_1(bgcolor);
	else if (p1 == 0x20) alpha_blend_1_2(bgcolor);
	else if (p1 == 0x40) alpha_blend_1_4(bgcolor);
	else if (p1 == 0x50) alpha_blend_1_5(bgcolor);
	else if (p1 == 0x60) alpha_blend_1_6(bgcolor);
	else if (p1 == 0x80) alpha_blend_1_8(bgcolor);
	else if (p1 == 0x90) alpha_blend_1_9(bgcolor);
	else if (p1 == 0xa0) alpha_blend_1_a(bgcolor);
}

/******************************************************************************/

void taito_f3_state::init_alpha_blend_func()
{
	m_dpix_n[0][0x0] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x1] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x2] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x3] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x4] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x5] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x6] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x7] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x8] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0x9] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0xa] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0xb] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0xc] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0xd] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0xe] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[0][0xf] = &taito_f3_state::dpix_1_noalpha;

	m_dpix_n[1][0x0] = &taito_f3_state::dpix_1_noalpha;
	m_dpix_n[1][0x1] = &taito_f3_state::dpix_1_1;
	m_dpix_n[1][0x2] = &taito_f3_state::dpix_1_2;
	m_dpix_n[1][0x3] = &taito_f3_state::dpix_ret1;
	m_dpix_n[1][0x4] = &taito_f3_state::dpix_1_4;
	m_dpix_n[1][0x5] = &taito_f3_state::dpix_1_5;
	m_dpix_n[1][0x6] = &taito_f3_state::dpix_1_6;
	m_dpix_n[1][0x7] = &taito_f3_state::dpix_ret1;
	m_dpix_n[1][0x8] = &taito_f3_state::dpix_1_8;
	m_dpix_n[1][0x9] = &taito_f3_state::dpix_1_9;
	m_dpix_n[1][0xa] = &taito_f3_state::dpix_1_a;
	m_dpix_n[1][0xb] = &taito_f3_state::dpix_ret1;
	m_dpix_n[1][0xc] = &taito_f3_state::dpix_ret1;
	m_dpix_n[1][0xd] = &taito_f3_state::dpix_ret1;
	m_dpix_n[1][0xe] = &taito_f3_state::dpix_ret1;
	m_dpix_n[1][0xf] = &taito_f3_state::dpix_ret1;

	m_dpix_n[2][0x0] = &taito_f3_state::dpix_2a_0;
	m_dpix_n[2][0x1] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0x2] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0x3] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0x4] = &taito_f3_state::dpix_2a_4;
	m_dpix_n[2][0x5] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0x6] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0x7] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0x8] = &taito_f3_state::dpix_2a_8;
	m_dpix_n[2][0x9] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0xa] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0xb] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0xc] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0xd] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0xe] = &taito_f3_state::dpix_ret0;
	m_dpix_n[2][0xf] = &taito_f3_state::dpix_ret0;

	m_dpix_n[3][0x0] = &taito_f3_state::dpix_3a_0;
	m_dpix_n[3][0x1] = &taito_f3_state::dpix_3a_1;
	m_dpix_n[3][0x2] = &taito_f3_state::dpix_3a_2;
	m_dpix_n[3][0x3] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0x4] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0x5] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0x6] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0x7] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0x8] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0x9] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0xa] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0xb] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0xc] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0xd] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0xe] = &taito_f3_state::dpix_ret0;
	m_dpix_n[3][0xf] = &taito_f3_state::dpix_ret0;

	m_dpix_n[4][0x0] = &taito_f3_state::dpix_2b_0;
	m_dpix_n[4][0x1] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0x2] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0x3] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0x4] = &taito_f3_state::dpix_2b_4;
	m_dpix_n[4][0x5] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0x6] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0x7] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0x8] = &taito_f3_state::dpix_2b_8;
	m_dpix_n[4][0x9] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0xa] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0xb] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0xc] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0xd] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0xe] = &taito_f3_state::dpix_ret0;
	m_dpix_n[4][0xf] = &taito_f3_state::dpix_ret0;

	m_dpix_n[5][0x0] = &taito_f3_state::dpix_3b_0;
	m_dpix_n[5][0x1] = &taito_f3_state::dpix_3b_1;
	m_dpix_n[5][0x2] = &taito_f3_state::dpix_3b_2;
	m_dpix_n[5][0x3] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0x4] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0x5] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0x6] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0x7] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0x8] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0x9] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0xa] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0xb] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0xc] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0xd] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0xe] = &taito_f3_state::dpix_ret0;
	m_dpix_n[5][0xf] = &taito_f3_state::dpix_ret0;

	m_dpix_n[6][0x0] = &taito_f3_state::dpix_2_0;
	m_dpix_n[6][0x1] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0x2] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0x3] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0x4] = &taito_f3_state::dpix_2_4;
	m_dpix_n[6][0x5] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0x6] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0x7] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0x8] = &taito_f3_state::dpix_2_8;
	m_dpix_n[6][0x9] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0xa] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0xb] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0xc] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0xd] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0xe] = &taito_f3_state::dpix_ret0;
	m_dpix_n[6][0xf] = &taito_f3_state::dpix_ret0;

	m_dpix_n[7][0x0] = &taito_f3_state::dpix_3_0;
	m_dpix_n[7][0x1] = &taito_f3_state::dpix_3_1;
	m_dpix_n[7][0x2] = &taito_f3_state::dpix_3_2;
	m_dpix_n[7][0x3] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0x4] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0x5] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0x6] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0x7] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0x8] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0x9] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0xa] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0xb] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0xc] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0xd] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0xe] = &taito_f3_state::dpix_ret0;
	m_dpix_n[7][0xf] = &taito_f3_state::dpix_ret0;
}

/******************************************************************************/

void taito_f3_state::get_pixmap_pointer(int skip_layer_num, const f3_playfield_line_inf **line_t, int y)
{
	for (int pf_num = skip_layer_num; pf_num < 5; ++pf_num)
	{
		const f3_playfield_line_inf *line_tmp = line_t[pf_num];
		m_src[pf_num] = line_tmp->src[y];
		m_src_s[pf_num] = line_tmp->src_s[y];
		m_src_e[pf_num] = line_tmp->src_e[y];
		m_tsrc[pf_num] = line_tmp->tsrc[y];
		m_tsrc_s[pf_num] = line_tmp->tsrc_s[y];
		m_x_count[pf_num] = line_tmp->x_count[y];
		m_x_zoom[pf_num] = line_tmp->x_zoom[y];
		m_clip_al[pf_num] = line_tmp->clip_in[y] & 0xffff;
		m_clip_ar[pf_num] = line_tmp->clip_in[y] >> 16;
		m_clip_bl[pf_num] = line_tmp->clip_ex[y] & 0xffff;
		m_clip_br[pf_num] = line_tmp->clip_ex[y] >> 16;
		m_pal_add[pf_num] = line_tmp->pal_add[y];
	}
}

void taito_f3_state::culc_pixmap_pointer(int skip_layer_num)
{
	for (int pf_num = skip_layer_num; pf_num < 5; ++pf_num)
	{
		m_x_count[pf_num] += m_x_zoom[pf_num];
		if (m_x_count[pf_num] >> 16)
		{
			m_x_count[pf_num] &= 0xffff;
			m_src[pf_num]++;
			m_tsrc[pf_num]++;
			if (m_src[pf_num] == m_src_e[pf_num])
			{
				m_src[pf_num] = m_src_s[pf_num];
				m_tsrc[pf_num] = m_tsrc_s[pf_num];
			}
		}
	}
}

#define UPDATE_PIXMAP_SP(pf_num) \
	if (cx > clip_als && cx < clip_ars && !(cx > clip_bls && cx < clip_brs)) \
	{ \
		sprite_pri = sprite[pf_num] & m_pval; \
		if (sprite_pri) \
		{ \
			if (sprite[pf_num] & 0x100) break; \
			if (!m_dpix_sp[sprite_pri]) \
			{ \
				if (!(m_pval & 0xf0)) break; \
				else { dpix_1_sprite(*dsti); *dsti = m_dval; break; } \
			} \
			if ((this->*m_dpix_sp[sprite_pri][m_pval >> 4])(*dsti)) { *dsti = m_dval; break; } \
		} \
	}

#define UPDATE_PIXMAP_LP(pf_num) \
	if (cx > m_clip_al[pf_num] && cx < m_clip_ar[pf_num] && !(cx > m_clip_bl[pf_num] && cx < m_clip_br[pf_num])) \
	{ \
		m_tval = *m_tsrc[pf_num]; \
		if (m_tval & 0xf0) \
			if ((this->*m_dpix_lp[pf_num][m_pval >> 4])(clut[(*m_src[pf_num] + m_pal_add[pf_num]) & 0x1fff])) { *dsti = m_dval; break; } \
	}


/*============================================================================*/

inline void taito_f3_state::draw_scanlines(
							bitmap_rgb32 &bitmap, int xsize, s16 *draw_line_num,
							const f3_playfield_line_inf **line_t,
							const u8 *sprite,
							u32 orient,
							int skip_layer_num)
{
	const pen_t *clut = &m_palette->pen(0);
	const u32 bgcolor = clut[0];

	const int x = 46;

	u16 clip_als = 0, clip_ars = 0, clip_bls = 0, clip_brs = 0;

	int yadv = bitmap.rowpixels();
	int yadvp = m_pri_alp_bitmap.rowpixels();
	int i = 0, y = draw_line_num[0];
	int ty = y;

	if (orient & ORIENTATION_FLIP_Y)
	{
		ty = bitmap.height() - 1 - ty;
		yadv = -yadv;
		yadvp = -yadvp;
	}

	u8 *dstp0 = &m_pri_alp_bitmap.pix(ty, x);

	m_pdest_2a = m_alpha_level_2ad ? 0x10 : 0;
	m_pdest_2b = m_alpha_level_2bd ? 0x20 : 0;
	m_tr_2a =(m_alpha_level_2as == 0 && m_alpha_level_2ad == 255) ? -1 : 0;
	m_tr_2b =(m_alpha_level_2bs == 0 && m_alpha_level_2bd == 255) ? -1 : 1;
	m_pdest_3a = m_alpha_level_3ad ? 0x40 : 0;
	m_pdest_3b = m_alpha_level_3bd ? 0x80 : 0;
	m_tr_3a =(m_alpha_level_3as == 0 && m_alpha_level_3ad == 255) ? -1 : 0;
	m_tr_3b =(m_alpha_level_3bs == 0 && m_alpha_level_3bd == 255) ? -1 : 1;

	{
		u32 *dsti0 = &bitmap.pix(ty, x);
		while (1)
		{
			int cx = 0;

			clip_als = m_sa_line_inf[0].sprite_clip_in[y] & 0xffff;
			clip_ars = m_sa_line_inf[0].sprite_clip_in[y] >> 16;
			clip_bls = m_sa_line_inf[0].sprite_clip_ex[y] & 0xffff;
			clip_brs = m_sa_line_inf[0].sprite_clip_ex[y] >> 16;

			int length = xsize;
			u32 *dsti = dsti0;
			u8 *dstp = dstp0;

			get_pixmap_pointer(skip_layer_num, line_t, y);

			while (1)
			{
				m_pval = *dstp;
				if (m_pval != 0xff)
				{
					u8 sprite_pri;
					switch (skip_layer_num)
					{
						case 0: UPDATE_PIXMAP_SP(0) UPDATE_PIXMAP_LP(0) [[fallthrough]];
						case 1: UPDATE_PIXMAP_SP(1) UPDATE_PIXMAP_LP(1) [[fallthrough]];
						case 2: UPDATE_PIXMAP_SP(2) UPDATE_PIXMAP_LP(2) [[fallthrough]];
						case 3: UPDATE_PIXMAP_SP(3) UPDATE_PIXMAP_LP(3) [[fallthrough]];
						case 4: UPDATE_PIXMAP_SP(4) UPDATE_PIXMAP_LP(4) [[fallthrough]];
						case 5: UPDATE_PIXMAP_SP(5)
								if (!bgcolor) { if (!(m_pval & 0xf0)) { *dsti = 0; break; } }
								else dpix_bg(bgcolor);
								*dsti = m_dval;
					}
				}

				if (!(--length)) break;
				dsti++;
				dstp++;
				cx++;

				culc_pixmap_pointer(skip_layer_num);
			}

			i++;
			if (draw_line_num[i] < 0) break;
			if (draw_line_num[i] == y + 1)
			{
				dsti0 += yadv;
				dstp0 += yadvp;
				y++;
				continue;
			}
			else
			{
				dsti0 += (draw_line_num[i] - y) * yadv;
				dstp0 += (draw_line_num[i] - y) * yadvp;
				y = draw_line_num[i];
			}
		}
	}
}

/******************************************************************************/

void taito_f3_state::visible_tile_check(
						f3_playfield_line_inf *line_t,
						int line,
						u32 x_index_fx,u32 y_index,
						const u16 *pf_data_n)
{
	const u8 alpha_mode = line_t->alpha_mode[line];
	if (!alpha_mode)
		return;

	const u32 total_elements = m_gfxdecode->gfx(3)->elements();

	int tile_index = x_index_fx >> 16;
	const int tile_num = (((line_t->x_zoom[line] * 320 + (x_index_fx & 0xffff) + 0xffff) >> 16) + (tile_index & 0xf) + 15) >> 4;
	tile_index >>= 4;

	const u16 *pf_base;
	if (m_flipscreen)
	{
		pf_base = pf_data_n + ((31 - (y_index >> 4)) << m_twidth_mask_bit);
		tile_index = (m_twidth_mask - tile_index) - tile_num + 1;
	}
	else pf_base = pf_data_n + ((y_index >> 4) << m_twidth_mask_bit);

	bool trans_all = true;
	bool opaque_all = true;
	u8 alpha_type = 0;
	for (int i = 0; i < tile_num; i++)
	{
		const u32 tile = (pf_base[(tile_index * 2 + 0) & m_twidth_mask] << 16) | (pf_base[(tile_index * 2 + 1) & m_twidth_mask]);
		const u8  extra_planes = (tile >> (16 + 10)) & 3;
		if (tile & 0xffff)
		{
			trans_all = false;
			if (opaque_all)
			{
				if (m_tile_opaque_pf[extra_planes][(tile & 0xffff) % total_elements] != 1) opaque_all = false;
			}

			if (alpha_mode == 1)
			{
				if (!opaque_all) return;
			}
			else
			{
				if (alpha_type != 3)
				{
					if ((tile >> (16 + 9)) & 1) alpha_type |= 2;
					else                        alpha_type |= 1;
				}
				else if (!opaque_all) break;
			}
		}
		else if (opaque_all) opaque_all = false;

		tile_index++;
	}

	if (trans_all) { line_t->alpha_mode[line] = 0; return; }

	if (alpha_mode > 1)
	{
		line_t->alpha_mode[line] |= alpha_type << 4;
	}

	if (opaque_all)
		line_t->alpha_mode[line] |= 0x80;
}

/******************************************************************************/

void taito_f3_state::calculate_clip(int y, u16 pri, u32 &clip_in, u32 &clip_ex, u8 &line_enable)
{
	const f3_spritealpha_line_inf *sa_line = &m_sa_line_inf[0];

	/* landmakr and quizhuhu use clip planes 2 and 3,
	   commandw enables all clip planes.
	   only up to 2 valid clips ever used in existing games? */
	u16 normal_planes = (pri >> 8) & (pri >> 4);
	u16 invert_planes = (pri >> 8) & ~(pri >> 4);
	// when bit 0x1000 set, invert bit ON is normal clip, OFF is inverted
	if (pri & 0x1000)
		std::swap(normal_planes, invert_planes);

	s16 clipl = 0, clipr = 0x7fff;

	const auto calc_clip =
		[&sa_line, y, &clipl, &clipr] (unsigned p)
		{
			clipl = std::max(sa_line->clip_l[p][y], clipl);
			clipr = std::min(sa_line->clip_r[p][y], clipr);
		};
	const auto calc_clip_inv =
		[&sa_line, y, &clipl, &clipr]
		(unsigned p)
		{
			clipl = std::min(sa_line->clip_l[p][y], clipl);
			clipr = std::max(sa_line->clip_r[p][y], clipr);
		};

	if (normal_planes & 0b0001) { calc_clip(0); };
	if (normal_planes & 0b0010) { calc_clip(1); };
	if (normal_planes & 0b0100) { calc_clip(2); };
	if (normal_planes & 0b1000) { calc_clip(3); };
	if (clipl > clipr)
		line_enable = 0;
	else
		clip_in = clipl | (clipr << 16);

	// reset temp clip sides for the inverted/excluded window
	clipl = 0x7fff; clipr = 0;
	if (invert_planes & 0b0001) { calc_clip_inv(0); };
	if (invert_planes & 0b0010) { calc_clip_inv(1); };
	if (invert_planes & 0b0100) { calc_clip_inv(2); };
	if (invert_planes & 0b1000) { calc_clip_inv(3); };
	if (clipl > clipr)
		clip_ex = 0;
	else
		clip_ex = clipl | (clipr << 16);
}

// line: [latched] line info from previous call, will modify in-place
// y should be called 0->255 for non-flipscreen, 255->0 for flipscreen
void taito_f3_state::read_line_ram(f3_line_inf &line, int y)
{
	const auto this_line = [=] (unsigned section)
	{
		return m_line_ram[section/2 + y];
	};
	// 4000 **********************************
	if (this_line(0x000) & 4) {
		u16 line_4400 = this_line(0x4400);
		line.pf[2].colscroll   = line_4400 & 0x1ff;
		line.pf[2].alt_tilemap = line_4400 & 0x200;
		line.clip[0].set_upper(BIT(line_4400, 12), BIT(line_4400, 13));
		line.clip[1].set_upper(BIT(line_4400, 14), BIT(line_4400, 15));
	}
	if (this_line(0x000) & 8) {
		u16 line_4600 = this_line(0x4600);
		line.pf[3].colscroll   = line_4600 & 0x1ff;
		line.pf[3].alt_tilemap = line_4600 & 0x200;
		line.clip[2].set_upper(BIT(line_4600, 12), BIT(line_4600, 13));
		line.clip[3].set_upper(BIT(line_4600, 14), BIT(line_4600, 15));
	}

	// 5000 **********************************
	// renderer needs to adjust by -48
	if (this_line(0x200) & 1) {
		u16 clip0_lows = this_line(0x5000);
		line.clip[0].set_lower(BIT(clip0_lows, 0, 8), BIT(clip0_lows, 8, 8));
	}
	if (this_line(0x200) & 2) {
		u16 clip1_lows = this_line(0x5200);
		line.clip[1].set_lower(BIT(clip1_lows, 0, 8), BIT(clip1_lows, 8, 8));
	}
	if (this_line(0x200) & 4) {
		u16 clip2_lows = this_line(0x5400);
		line.clip[2].set_lower(BIT(clip2_lows, 0, 8), BIT(clip2_lows, 8, 8));
	}
	if (this_line(0x200) & 8) {
		u16 clip3_lows = this_line(0x5600);
		line.clip[3].set_lower(BIT(clip3_lows, 0, 8), BIT(clip3_lows, 8, 8));
	}

	// 6000 **********************************
	if (this_line(0x400) & 1) {
		// first value is sync register, special handling unnecessary?
		u16 line_6000 = this_line(0x6000);

		line.pivot.pivot_control = BIT(line_6000, 8, 8);
		if (line.pivot.pivot_control & 0b01010111) // check if unknown pivot control bits set
			logerror("unknown pivot ctrl bits: %02x__ at %04x\n", line.pivot.pivot_control, 0x6000 + y*2);

		for (int sp_group = 0; sp_group < NUM_SPRITEGROUPS; sp_group++) {
			line.sp[sp_group].mix_value = (line.sp[sp_group].mix_value & 0x3fff)
				| BIT(line_6000, sp_group * 2, 2) << 14;
		}
	}
	if (this_line(0x400) & 2) {
		line.blend = this_line(0x6200);
	}
	if (this_line(0x400) & 4) {
		u16 x_mosaic = this_line(0x6400);

		for (int pf_num = 0; pf_num < NUM_PLAYFIELDS; pf_num++)
			line.pf[pf_num].x_sample_enable = BIT(x_mosaic, pf_num);

		line.x_sample = (x_mosaic & 0xf0) >> 4;

		line.fx_6400 = (x_mosaic & 0xff00) >> 8;
		if (line.fx_6400 && line.fx_6400 != 0x70) // check if unknown effect bits set
			logerror("unknown fx bits: %02x__ at %04x\n", line.fx_6400, 0x6400 + y*2);
	}
	if (this_line(0x400) & 8) {
		line.bg_palette = this_line(0x6600);
	}
	// bubblem writes these seemingly intentionally
	if (this_line(0x400) & 0x10) {
		u16 line_6800 = this_line(0x6800);
		if (line_6800) // check if mystery subsection 6800 used
			logerror("mystery subsection 6800: %04x at %04x\n", line_6800, 0x6800 + y*2);
	}
	if (this_line(0x400) & 0x20) {
		u16 line_6a00 = this_line(0x6a00);
		if (line_6a00) // check if mystery subsection 6a00 used
			logerror("mystery subsection 6a00: %04x at %04x\n", line_6a00, 0x6a00 + y*2);
	}

	// 7000 **********************************
	if (this_line(0x600) & 1) {
		u16 line_7000 = this_line(0x7000);
		line.pivot.pivot_enable = line_7000;
		if (line_7000) // check if confusing pivot enable bits are set
			logerror("unknown 'pivot enable' bits: %04x at %04x\n", line_7000, 0x7000 + y*2);
	}
	if (this_line(0x600) & 2) {
		line.pivot.mix_value = this_line(0x7200);
	}
	if (this_line(0x600) & 4) {
		u16 sprite_mix = this_line(0x7400);
		for (int group = 0; group < NUM_SPRITEGROUPS; group++) {
			// watch out for blend bit combine bugs here
			line.sp[group].mix_value = (line.sp[group].mix_value & 0xc00f)
				| sprite_mix << 4;
			line.sp[group].brightness = BIT(sprite_mix, 12 + group, 1);
		}
	}
	if (this_line(0x600) & 8) {
		u16 sprite_prio = this_line(0x7600);
		for (int group = 0; group < NUM_SPRITEGROUPS; group++) {
			line.sp[group].mix_value = (line.sp[group].mix_value & 0xfff0)
				| BIT(sprite_prio, group * 4, 4);
		}
	}

	if (this_line(0x600) & 0x10) {
		u16 line_7800 = this_line(0x7800);
		if (line_7800) // check if mystery subsection 6800 used
			logerror("mystery subsection 7800: %04x at %04x\n", line_7800, 0x7800 + y*2);
	}
	if (this_line(0x600) & 0x20) {
		u16 line_7a00 = this_line(0x7a00);
		if (line_7a00) // check if mystery subsection 6a00 used
			logerror("mystery subsection 7a00: %04x at %04x\n", line_7a00, 0x7a00 + y*2);
	}

	// 8000 **********************************
	// TODO: ignore first scaling offset, somewhere
	if (this_line(0x800) & 1) {
		u16 pf0x0y_scale = this_line(0x8000);
		line.pf[0].x_scale = BIT(pf0x0y_scale, 8, 8);
		line.pf[0].y_scale = BIT(pf0x0y_scale, 0, 8);
	}
	if (this_line(0x800) & 2) {
		u16 pf1x3y_scale = this_line(0x8200);
		line.pf[1].x_scale = BIT(pf1x3y_scale, 8, 8);
		line.pf[3].y_scale = BIT(pf1x3y_scale, 0, 8);
	}
	if (this_line(0x800) & 4) {
		u16 pf2x2y_scale = this_line(0x8400);
		line.pf[2].x_scale = BIT(pf2x2y_scale, 8, 8);
		line.pf[2].y_scale = BIT(pf2x2y_scale, 0, 8);
	}
	if (this_line(0x800) & 8) {
		u16 pf3x1y_scale = this_line(0x8600);
		line.pf[3].x_scale = BIT(pf3x1y_scale, 8, 8);
		line.pf[1].y_scale = BIT(pf3x1y_scale, 0, 8);
	}

	// 9000 **********************************
	if (this_line(0xa00) & 1) {
		u16 pf1_pal_add = this_line(0x9000);
		line.pf[0].pal_add = pf1_pal_add * 16;
	}
	if (this_line(0xa00) & 2) {
		u16 pf2_pal_add = this_line(0x9200);
		line.pf[1].pal_add = pf2_pal_add * 16;
	}
	if (this_line(0xa00) & 4) {
		u16 pf3_pal_add = this_line(0x9400);
		line.pf[2].pal_add = pf3_pal_add * 16;
	}
	if (this_line(0xa00) & 8) {
		u16 pf4_pal_add = this_line(0x9600);
		line.pf[3].pal_add = pf4_pal_add * 16;
	}

	// A000 **********************************
	if (this_line(0xc00) & 1) {
		u16 pf1_rowscroll = this_line(0xa000);
		line.pf[0].rowscroll = pf1_rowscroll << 10;
	}
	if (this_line(0xc00) & 2) {
		u16 pf2_rowscroll = this_line(0xa200);
		line.pf[1].rowscroll = pf2_rowscroll << 10;
	}
	if (this_line(0xc00) & 4) {
		u16 pf3_rowscroll = this_line(0xa400);
		line.pf[2].rowscroll = pf3_rowscroll << 10;
	}
	if (this_line(0xc00) & 8) {
		u16 pf4_rowscroll = this_line(0xa600);
		line.pf[3].rowscroll = pf4_rowscroll << 10;
	}
	// why is this shift here ... _x_offset is 16.16
	// lineram word must be 10.6 ?
	// i think to conform to the layer scroll regs which are 16.16


	// B000 **********************************
	if (this_line(0xe00) & 1) {
		line.pf[0].mix_value = this_line(0xb000);
	}
	if (this_line(0xe00) & 2) {
		line.pf[1].mix_value = this_line(0xb200);
	}
	if (this_line(0xe00) & 4) {
		line.pf[2].mix_value = this_line(0xb400);
	}
	if (this_line(0xe00) & 8) {
		line.pf[3].mix_value = this_line(0xb600);
	}
}

void taito_f3_state::get_pf_scroll(int pf_num, u32 &reg_sx, u32 &reg_sy)
{
	u32 sy = ((m_control_0[pf_num + 4] & 0xffff) <<  9) + (1 << 16);
	u32 sx = ((m_control_0[pf_num] & 0xffc0) << 10) - ((6 + 4 * pf_num) << 16);
	sx-= ((m_control_0[pf_num] & 0x003f) << 10) + 0x0400 - 0x10000;
	if (m_flipscreen) {
		sy =  0x3000000 - sy;
		sx = -0x1a00000 - sx;
	}
	reg_sx = sx;
	reg_sy = sy;
}

void taito_f3_state::draw_line(u32* dst, int y, int xs, int xe, sprite_inf* sp)
{
	if (!sp->layer_enable())
		return;
	const pen_t *clut = &m_palette->pen(0);
	for (int x = xs; x < xe; x++) {
		if (u16 col = sp->srcbitmap.pix(y, x)) // 0 = transparent
			dst[x] = clut[col];
	}
}
void taito_f3_state::draw_line(u32* dst, int y, int xs, int xe, playfield_inf* pf)
{
	if (!pf->layer_enable())
		return;
	const pen_t *clut = &m_palette->pen(0);
	int y_index = ((pf->reg_fx_y >> 16) + pf->colscroll) & 0x1ff;
	for (int x = xs; x < xe; x++) {
		int x_index = ((pf->reg_fx_x >> 16) + x) & m_width_mask;
		if (!(pf->flagsbitmap->pix(y_index, x_index) & 0xf0))
			continue;
		if (u16 col = pf->srcbitmap->pix(y_index, x_index))
			dst[x] = clut[col];
	}
}

void taito_f3_state::draw_line(u32* dst, int y, int xs, int xe, pivot_inf* pv)
{
	const pen_t *clut = &m_palette->pen(0);
	auto srcbitmap = pv->use_pix() ? pv->srcbitmap_pixel : pv->srcbitmap_vram;
	auto flagsbitmap = pv->use_pix() ? pv->flagsbitmap_pixel :
	pv->flagsbitmap_vram;
	const u16 height_mask = pv->use_pix() ? 0xff : 0x1ff;
	const u16 width_mask = 0x1ff;

	int y_index = (pv->reg_sy + y) & height_mask;

	for (int x = xs; x < xe; x++) {
		int x_index = (pv->reg_sx + x) & width_mask;
		if (!(flagsbitmap->pix(y_index, x_index) & 0xf0))
			continue;
		if (u16 col = srcbitmap->pix(y_index, x_index)) {
			dst[x] = clut[col];
		}
	}
}

void taito_f3_state::scanline_draw_TWO(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y_start, y_end, y_inc; // actual start and end, not same-efx grouping
	if (m_flipscreen) {
		y_start = 255;
		y_end = -1;
		y_inc = -1;
	} else {
		y_start = 0;
		y_end = 256;
		y_inc = 1;
	}

	// acquire sprite rendering layers, playfield tilemaps, playfield scroll
	f3_line_inf line_data{};
	for (int sp = 0; sp < NUM_SPRITEGROUPS; ++sp) {
		// aaa why are you making a new one every time...
		line_data.sp[sp].srcbitmap = bitmap_ind16(bitmap.width(), bitmap.height());
	}
	for (int pf = 0; pf < NUM_PLAYFIELDS; ++pf) {
		int tmap_number = pf + line_data.pf[pf].alt_tilemap * 2;
		line_data.pf[pf].srcbitmap = &m_tilemap[tmap_number]->pixmap();
		line_data.pf[pf].flagsbitmap = &m_tilemap[tmap_number]->flagsmap();
		get_pf_scroll(pf, line_data.pf[pf].reg_sx, line_data.pf[pf].reg_sy);
		if (m_flipscreen)
			line_data.pf[pf].reg_fx_y = -line_data.pf[pf].reg_sy - (256 << 16);
		else
			line_data.pf[pf].reg_fx_y = line_data.pf[pf].reg_sy;
	}
	line_data.pivot.srcbitmap_pixel = &m_pixel_layer->pixmap();
	line_data.pivot.flagsbitmap_pixel = &m_pixel_layer->flagsmap();
	line_data.pivot.srcbitmap_vram = &m_vram_layer->pixmap();
	line_data.pivot.flagsbitmap_vram = &m_vram_layer->flagsmap();
	if (m_flipscreen) {
		line_data.pivot.reg_sx = (m_control_1[4]) - 12;
		line_data.pivot.reg_sy = (m_control_1[5] & 0x1ff);
	} else {
		line_data.pivot.reg_sx = -(m_control_1[4]) - 5;
		line_data.pivot.reg_sy = -(m_control_1[5] & 0x1ff);
	}

	// draw sprite layers
	{
		rectangle myclip;
		/* KW 991012 -- Added code to force clip to bitmap boundary */
		myclip = cliprect;
		myclip &= m_pri_alp_bitmap.cliprect();

		const tempsprite *sprite_ptr;
		gfx_element *sprite_gfx = m_gfxdecode->gfx(2);

		sprite_ptr = m_sprite_end;
		while (sprite_ptr != &m_spritelist[0])
		{
			sprite_ptr--;

			f3_drawgfx(line_data.sp[sprite_ptr->pri].srcbitmap, myclip, sprite_gfx, *sprite_ptr);
		}
	}

	auto prio = [](const auto& obj) -> u8 { return obj->prio(); };

	for (int y = y_start; y != y_end; y += y_inc) {
		read_line_ram(line_data, y);

		// sort layers
		std::array<std::variant<pivot_inf*, sprite_inf*, playfield_inf*>,
				   NUM_SPRITEGROUPS + NUM_TILEMAPS> layers = {
			&line_data.sp[3], &line_data.sp[2], &line_data.sp[1], &line_data.sp[0],
			&line_data.pivot,
			&line_data.pf[3], &line_data.pf[2], &line_data.pf[1], &line_data.pf[0]
		};
		std::stable_sort(layers.begin(), layers.end(),
						 [prio](auto a, auto b) {
							 return std::visit(prio, a) < std::visit(prio, b);
						 });

		// draw layers to framebuffer, (for now, in bottom->top order)
		for (auto gfx : layers) {
			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, sprite_inf*>)
					draw_line(&bitmap.pix(y), y, 46, 46 + 320, arg);
				else if constexpr (std::is_same_v<T, playfield_inf*>)
					draw_line(&bitmap.pix(y), y, 46, 46 + 320, arg);
				else if constexpr (std::is_same_v<T, pivot_inf*>)
					draw_line(&bitmap.pix(y), y, 46, 46 + 320, arg);
			}, gfx);
		}

		// update registers
		for (int pf = 0; pf < NUM_PLAYFIELDS; ++pf) {
			auto p = &line_data.pf[pf];
			p->reg_fx_y += p->y_scale << 9;
			p->reg_fx_x = p->reg_sx + p->rowscroll + 10*p->x_scale;
			p->reg_fx_x &= (m_width_mask << 16) | 0xffff;
		}
	}
}


void taito_f3_state::get_spritealphaclip_info()
{
	f3_spritealpha_line_inf *line_t = &m_sa_line_inf[0];

	int y, y_end, y_inc;

	int spri_base, clip_base_low, clip_base_high, inc;

	u16 spri = 0;
	u16 sprite_clip = 0;
	u16 clip0_low = 0, clip0_high = 0, clip1_low = 0;
	u16 clip2_low = 0, clip2_high = 0, clip3_low = 0;
	u16 alpha_level = 0;
	u16 sprite_alpha = 0;

	if (m_flipscreen)
	{
		spri_base = 0x77fe;
		clip_base_low = 0x51fe;
		clip_base_high = 0x45fe;
		inc = -2;
		y = 255;
		y_end = -1;
		y_inc = -1;

	}
	else
	{
		spri_base = 0x7600;
		clip_base_low = 0x5000;
		clip_base_high = 0x4400;
		inc = 2;
		y = 0;
		y_end = 256;
		y_inc = 1;
	}

	while (y != y_end)
	{
		/* The zoom, column and row values can latch according to control ram */
		{
			if (m_line_ram[0x100 + y] & 1) // 5000 control playfield 1
				clip0_low = (m_line_ram[clip_base_low / 2] >> 0) & 0xffff;
			if (m_line_ram[0x000 + y] & 4) // 4000 control
				clip0_high = (m_line_ram[clip_base_high / 2] >> 0) & 0xffff;
			if (m_line_ram[0x100 + y] & 2) // 5000 control playfield 2
				clip1_low = (m_line_ram[(clip_base_low + 0x200) / 2] >> 0) & 0xffff;

			if (m_line_ram[0x100 + y] & 4) // 5000 control playfield 3
				clip2_low = (m_line_ram[(clip_base_low + 0x400) / 2] >> 0) & 0xffff;
			if (m_line_ram[0x000 + y] & 8) // 4000 control
				clip2_high = (m_line_ram[(clip_base_high + 0x200) / 2] >> 0) & 0xffff;
			if (m_line_ram[0x100 + y] & 8) // 5000 control playfield 4
				clip3_low = (m_line_ram[(clip_base_low + 0x600) / 2] >> 0) & 0xffff;

			if (m_line_ram[(0x0600 / 2) + y] & 0x8)
				spri = m_line_ram[spri_base / 2] & 0xffff;
			if (m_line_ram[(0x0600 / 2) + y] & 0x4)
				sprite_clip = m_line_ram[(spri_base-0x200) / 2] & 0xffff;
			if (m_line_ram[(0x0400 / 2) + y] & 0x1)
				sprite_alpha = m_line_ram[(spri_base-0x1600) / 2] & 0xffff;
			if (m_line_ram[(0x0400 / 2) + y] & 0x2)
				alpha_level = m_line_ram[(spri_base-0x1400) / 2] & 0xffff;
		}


		line_t->alpha_level[y] = alpha_level;
		line_t->spri[y] = spri;
		line_t->sprite_alpha[y] = sprite_alpha;
		line_t->clip_l[0][y] = ((clip0_low & 0xff) | ((clip0_high & 0x1000) >> 4)) - 48;
		line_t->clip_r[0][y] = (((clip0_low & 0xff00) >> 8) | ((clip0_high & 0x2000) >> 5)) - 48;
		line_t->clip_l[1][y] = ((clip1_low & 0xff) | ((clip0_high & 0x4000) >> 6)) - 48;
		line_t->clip_r[1][y] = (((clip1_low & 0xff00) >> 8) | ((clip0_high & 0x8000) >> 7)) - 48;
		line_t->clip_l[2][y] = ((clip2_low & 0xff) | ((clip2_high & 0x1000) >> 4)) - 48;
		line_t->clip_r[2][y] = (((clip2_low & 0xff00) >> 8) | ((clip2_high & 0x2000) >> 5)) - 48;
		line_t->clip_l[3][y] = ((clip3_low & 0xff) | ((clip2_high & 0x4000) >> 6)) - 48;
		line_t->clip_r[3][y] = (((clip3_low & 0xff00) >> 8) | ((clip2_high & 0x8000) >> 7)) - 48;
		if (line_t->clip_l[0][y] < 0) line_t->clip_l[0][y] = 0;
		if (line_t->clip_r[0][y] < 0) line_t->clip_r[0][y] = 0;
		if (line_t->clip_l[1][y] < 0) line_t->clip_l[1][y] = 0;
		if (line_t->clip_r[1][y] < 0) line_t->clip_r[1][y] = 0;
		if (line_t->clip_l[2][y] < 0) line_t->clip_l[2][y] = 0;
		if (line_t->clip_r[2][y] < 0) line_t->clip_r[2][y] = 0;
		if (line_t->clip_l[3][y] < 0) line_t->clip_l[3][y] = 0;
		if (line_t->clip_r[3][y] < 0) line_t->clip_r[3][y] = 0;

		/* Evaluate sprite clipping */
		if (sprite_clip & 0xf0)
		{
			u8 line_enable = 1;
			calculate_clip(y, ((sprite_clip & 0x1ff) << 4), line_t->sprite_clip_in[y], line_t->sprite_clip_ex[y], line_enable);
			if (line_enable == 0)
				line_t->sprite_clip_in[y] = 0x7fff7fff;
		}
		else
		{
			line_t->sprite_clip_in[y] = 0x7fff0000;
			line_t->sprite_clip_ex[y] = 0;
		}

		spri_base += inc;
		clip_base_low += inc;
		clip_base_high += inc;
		y += y_inc;
	}
}

/* sx and sy are 16.16 fixed point numbers */
void taito_f3_state::get_line_ram_info(tilemap_t *tmap, int sx, int sy, int pos, const u16 *pf_data_n)
{
	f3_playfield_line_inf *line_t = &m_pf_line_inf[pos];

	int y_start, y_end, y_inc;
	int y_index_fx;

	u16 colscroll = 0;
	u16 _colscroll[256];
	int x_offset = 0;
	u32 _x_offset[256];
	u8 line_zoom_x = 0, line_zoom_y = 0;
	u8 _y_zoom[256];
	u16 pri = 0, pal_add = 0;

	sx += ((46 << 16));

	if (m_flipscreen)
	{
		y_start = 255;
		y_end = -1;
		y_inc = -1;

		 /* Adjust for flipped scroll position */
		if (m_game_config->extend)
			sx = -sx + (((188 - 512) & 0xffff) << 16);
		else
			sx = -sx + (188 << 16);

		y_index_fx = -sy - (256 << 16); /* Adjust for flipped scroll position */
	}
	else
	{
		y_start = 0;
		y_end = 256;
		y_inc = 1;

		y_index_fx = sy;
	}

	int y = y_start;

	while (y != y_end)
	{
		const u16 col_base = (0x4000 + (pos << 9)) / 2;
		const u16 zoom_base = 0x8000 / 2;
		const u16 pal_add_base = (0x9000 + (pos << 9)) / 2;
		const u16 line_base = (0xa000 + (pos << 9)) / 2;
		const u16 pri_base = (0xb000 + (pos << 9)) / 2;

		const u8 bit_select = 1 << pos;

		/* The zoom, column and row values can latch according to control ram */
		{
			if (m_line_ram[0x600 + y] & bit_select)
				x_offset = m_line_ram[line_base + y] << 10;
			if (m_line_ram[0x700 + y] & bit_select)
				pri = m_line_ram[pri_base + y];

			// Zoom for playfields 1 & 3 is interleaved, as is the latch select
			switch (pos)
			{
			case 0:
				if (m_line_ram[0x400 + y] & bit_select)
				{
					line_zoom_x = m_line_ram[zoom_base + y + 0x000 / 2] >> 8;
					line_zoom_y = m_line_ram[zoom_base + y + 0x000 / 2] & 0xff;
				}
				break;
			case 1:
				if (m_line_ram[0x400 + y] & 0x2)
					line_zoom_x = m_line_ram[zoom_base + y + 0x200 / 2] >> 8;
				if (m_line_ram[0x400 + y] & 0x8)
					line_zoom_y = m_line_ram[zoom_base + y + 0x600 / 2] & 0xff;
				break;
			case 2:
				if (m_line_ram[0x400 + y] & bit_select)
				{
					line_zoom_x = m_line_ram[zoom_base + y + 0x400 / 2] >> 8;
					line_zoom_y = m_line_ram[zoom_base + y + 0x400 / 2] & 0xff;
				}
				break;
			case 3:
				if (m_line_ram[0x400 + y] & 0x8)
					line_zoom_x = m_line_ram[zoom_base + y + 0x600 / 2] >> 8;
				if (m_line_ram[0x400 + y] & 0x2)
					line_zoom_y = m_line_ram[zoom_base + y + 0x200 / 2] & 0xff;
				break;
			default:
				break;
			}

			// Column scroll only affects playfields 2 & 3
			if (pos >= 2 && m_line_ram[0x000 + y] & bit_select)
				colscroll = (m_line_ram[col_base + y] >> 0) & 0x3ff;

			if (m_line_ram[0x500 + y] & bit_select)
				pal_add = (m_line_ram[pal_add_base + y] & 0x1ff) * 16;
		}

		u8 line_enable;

		if (!pri || (!m_flipscreen && y < 24) || (m_flipscreen && y > 231) ||
			(pri & 0xc000) == 0xc000 || !(pri & 0x2000)/**/)
			line_enable = 0;
		else if (pri & 0x4000) //alpha1
			line_enable = 2;
		else if (pri & 0x8000) //alpha2
			line_enable = 3;
		else
			line_enable = 1;

		_colscroll[y] = colscroll;
		_x_offset[y] = (x_offset & 0xffff0000) - (x_offset & 0x0000ffff);
		_y_zoom[y] = line_zoom_y;

		/* Evaluate clipping */
		if (pri & 0x0f00)
		{
			//fast path todo - remove line enable
			calculate_clip(y, pri & 0x1ff0, line_t->clip_in[y], line_t->clip_ex[y], line_enable);
		}
		else
		{
			/* No clipping */
			line_t->clip_in[y] = 0x7fff0000;
			line_t->clip_ex[y] = 0;
		}

		line_t->x_zoom[y] = 0x10000 - (line_zoom_x << 8);
		line_t->alpha_mode[y] = line_enable;
		line_t->pri[y] = pri;
		line_t->pal_add[y] = pal_add;

		// zoom_base += inc;
		y += y_inc;
	}
	// ignore the first zoom value from ram and use the default
	_y_zoom[y_start] = 0;
	line_t->x_zoom[y_start] = 0x10000;

	tilemap_t* tm = tmap;
	const u16* pfdata = pf_data_n;

	y = y_start;
	while (y != y_end)
	{
		u32 x_index_fx;
		u32 y_index;

		/* lines with 0x0200 set in column scroll look up in alternate tilemaps
		   playfield 3 2000 -> 4000, playfield 4 3000 -> 5000 (non-extended only?)
		   used by kaiserkn (high scores), kirameki, football games (crowd, goals)

		   there's some seemingly unrelated issue with the timing of y scrolling,
		   causing the pitch to scroll ahead of crowd areas
		*/
		const u16 cs = _colscroll[y];
		if (cs & 0x200)
		{
			if (m_tilemap[4] && m_tilemap[5])
			{
				if (tmap == m_tilemap[2])
				{
					tmap = m_tilemap[4];
					pf_data_n = m_pf_data[4];
				}
				else if (tmap == m_tilemap[3])
				{
					tmap = m_tilemap[5];
					pf_data_n = m_pf_data[5];
				}
			}
		}
		else
		{
			tmap = tm;
			pf_data_n = pfdata;
		}

		/* set pixmap pointer */
		bitmap_ind16 &srcbitmap = tmap->pixmap();
		bitmap_ind8 &flagsbitmap = tmap->flagsmap();

		if (line_t->alpha_mode[y] != 0)
		{
			u16 *src_s;
			u8 *tsrc_s;

			x_index_fx = (sx+_x_offset[y]-(10*0x10000) + (10*line_t->x_zoom[y]))&((m_width_mask << 16)|0xffff);
			y_index = ((y_index_fx >> 16)+_colscroll[y]) & 0x1ff;

			/* check tile status */
			visible_tile_check(line_t, y, x_index_fx, y_index, pf_data_n);

			/* If clipping enabled for this line have to disable 'all opaque' optimisation */
			if (line_t->clip_in[y] != 0x7fff0000 || line_t->clip_ex[y] != 0)
				line_t->alpha_mode[y] &= ~0x80;

			/* set pixmap index */
			line_t->x_count[y]=x_index_fx & 0xffff; // Fractional part
			line_t->src_s[y] = src_s = &srcbitmap.pix(y_index);
			line_t->src_e[y] = &src_s[m_width_mask + 1];
			line_t->src[y] = &src_s[x_index_fx >> 16];

			line_t->tsrc_s[y]=tsrc_s = &flagsbitmap.pix(y_index);
			line_t->tsrc[y] = &tsrc_s[x_index_fx >> 16];
		}

		y_index_fx += _y_zoom[y] << 9;
		y += y_inc;
	}
}

void taito_f3_state::get_vram_info(tilemap_t *vram_tilemap, tilemap_t *pixel_tilemap, int sx, int sy)
{
	const f3_spritealpha_line_inf *sprite_alpha_line_t = &m_sa_line_inf[0];
	f3_playfield_line_inf *line_t = &m_pf_line_inf[4];

	int y_start, y_end, y_inc;
	int pri_base, inc;

	u8 line_enable;

	u16 pri = 0;

	const u16 vram_width_mask = 0x1ff;

	if (m_flipscreen)
	{
		pri_base = 0x73fe;
		inc = -2;
		y_start = 255;
		y_end = -1;
		y_inc = -1;
	}
	else
	{
		pri_base = 0x7200;
		inc = 2;
		y_start = 0;
		y_end = 256;
		y_inc = 1;

	}

	int y = y_start;
	while (y != y_end)
	{
		/* The zoom, column and row values can latch according to control ram */
		{
			if (m_line_ram[(0x0600 / 2) + y] & 0x2)
				pri = (m_line_ram[pri_base / 2] & 0xffff);
		}

		if (!pri || (!m_flipscreen && y < 24) || (m_flipscreen && y > 231) ||
			(pri & 0xc000) == 0xc000 || !(pri & 0x2000)/**/)
			line_enable = 0;
		else if (pri & 0x4000) //alpha1
			line_enable = 2;
		else if (pri & 0x8000) //alpha2
			line_enable = 3;
		else
			line_enable = 1;

		line_t->pri[y] = pri;

		/* Evaluate clipping */
		if (pri & 0x0f00)
		{
			//fast path todo - remove line enable
			calculate_clip(y, pri & 0x1ff0, line_t->clip_in[y], line_t->clip_ex[y], line_enable);
		}
		else
		{
			/* No clipping */
			line_t->clip_in[y] = 0x7fff0000;
			line_t->clip_ex[y] = 0;
		}

		line_t->x_zoom[y] = 0x10000;
		line_t->alpha_mode[y] = line_enable;
		if (line_t->alpha_mode[y] > 1)
			line_t->alpha_mode[y] |= 0x10;

		pri_base += inc;
		y += y_inc;
	}

	sx &= 0x1ff;

	/* set pixmap pointer */
	bitmap_ind16 &srcbitmap_pixel = pixel_tilemap->pixmap();
	bitmap_ind8 &flagsbitmap_pixel = pixel_tilemap->flagsmap();
	bitmap_ind16 &srcbitmap_vram = vram_tilemap->pixmap();
	bitmap_ind8 &flagsbitmap_vram = vram_tilemap->flagsmap();

	y = y_start;
	while (y != y_end)
	{
		if (line_t->alpha_mode[y] != 0)
		{
			u16 *src_s;
			u8 *tsrc_s;

			// These bits in control ram indicate whether the line is taken from
			// the VRAM tilemap layer or pixel layer.
			const bool usePixelLayer = ((sprite_alpha_line_t->sprite_alpha[y] & 0xa000) == 0xa000);

			/* set pixmap index */
			line_t->x_count[y] = 0xffff;
			if (usePixelLayer)
				line_t->src_s[y] = src_s = &srcbitmap_pixel.pix(sy & 0xff);
			else
				line_t->src_s[y] = src_s = &srcbitmap_vram.pix(sy & 0x1ff);
			line_t->src_e[y] = &src_s[vram_width_mask + 1];
			line_t->src[y] = &src_s[sx];

			if (usePixelLayer)
				line_t->tsrc_s[y]=tsrc_s = &flagsbitmap_pixel.pix(sy & 0xff);
			else
				line_t->tsrc_s[y]=tsrc_s = &flagsbitmap_vram.pix(sy & 0x1ff);
			line_t->tsrc[y] = &tsrc_s[sx];
		}

		sy++;
		y += y_inc;
	}
}

/******************************************************************************/

void taito_f3_state::scanline_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, ys, ye;
	int y_start, y_end, y_start_next, y_end_next;
	u8 draw_line[256] = {};
	s16 draw_line_num[256];

	u32 rot = 0;

	if (m_flipscreen)
	{
		rot = ORIENTATION_FLIP_Y;
		ys = 0;
		ye = 232;
	}
	else
	{
		ys = 24;
		ye = 256;
	}

	y_start = ys;
	y_end = ye;

	while (1)
	{
		u8 alpha_mode_flag[5];
		u8 sprite_alpha_check;
		u8 sprite_alpha_all_2a;
		f3_playfield_line_inf *pf_line_inf = m_pf_line_inf.get();
		f3_spritealpha_line_inf *sa_line_inf = m_sa_line_inf.get();
		u8 sprite[6] = {};
		const f3_playfield_line_inf *line_t[5];

		/* find same status of scanlines */
		u16 pri[5];
		pri[0] = pf_line_inf[0].pri[y_start];
		pri[1] = pf_line_inf[1].pri[y_start];
		pri[2] = pf_line_inf[2].pri[y_start];
		pri[3] = pf_line_inf[3].pri[y_start];
		pri[4] = pf_line_inf[4].pri[y_start];

		u8 alpha_mode[5];
		alpha_mode[0] = pf_line_inf[0].alpha_mode[y_start];
		alpha_mode[1] = pf_line_inf[1].alpha_mode[y_start];
		alpha_mode[2] = pf_line_inf[2].alpha_mode[y_start];
		alpha_mode[3] = pf_line_inf[3].alpha_mode[y_start];
		alpha_mode[4] = pf_line_inf[4].alpha_mode[y_start];
		const u16 alpha_level = sa_line_inf[0].alpha_level[y_start];
		const u16 spri = sa_line_inf[0].spri[y_start];
		const u16 sprite_alpha = sa_line_inf[0].sprite_alpha[y_start];

		draw_line[y_start] = 1;
		draw_line_num[i = 0] = y_start;
		y_start_next = -1;
		y_end_next = -1;
		for (int y = y_start + 1; y < y_end; y++)
		{
			if (!draw_line[y])
			{
				if (pri[0] != pf_line_inf[0].pri[y]) y_end_next = y + 1;
				else if (pri[1] != pf_line_inf[1].pri[y]) y_end_next = y + 1;
				else if (pri[2] != pf_line_inf[2].pri[y]) y_end_next = y + 1;
				else if (pri[3] != pf_line_inf[3].pri[y]) y_end_next = y + 1;
				else if (pri[4] != pf_line_inf[4].pri[y]) y_end_next = y + 1;
				else if (alpha_mode[0] != pf_line_inf[0].alpha_mode[y]) y_end_next = y + 1;
				else if (alpha_mode[1] != pf_line_inf[1].alpha_mode[y]) y_end_next = y + 1;
				else if (alpha_mode[2] != pf_line_inf[2].alpha_mode[y]) y_end_next = y + 1;
				else if (alpha_mode[3] != pf_line_inf[3].alpha_mode[y]) y_end_next = y + 1;
				else if (alpha_mode[4] != pf_line_inf[4].alpha_mode[y]) y_end_next = y + 1;
				else if (alpha_level!=sa_line_inf[0].alpha_level[y]) y_end_next = y + 1;
				else if (spri!=sa_line_inf[0].spri[y]) y_end_next = y + 1;
				else if (sprite_alpha!=sa_line_inf[0].sprite_alpha[y]) y_end_next = y + 1;
				else
				{
					draw_line[y] = 1;
					draw_line_num[++i] = y;
					continue;
				}

				if (y_start_next < 0) y_start_next = y;
			}
		}
		y_end = y_end_next;
		y_start = y_start_next;
		draw_line_num[++i] = -1;

		/* alpha blend */
		alpha_mode_flag[0] = alpha_mode[0] & ~3;
		alpha_mode_flag[1] = alpha_mode[1] & ~3;
		alpha_mode_flag[2] = alpha_mode[2] & ~3;
		alpha_mode_flag[3] = alpha_mode[3] & ~3;
		alpha_mode_flag[4] = alpha_mode[4] & ~3;
		alpha_mode[0] &= 3;
		alpha_mode[1] &= 3;
		alpha_mode[2] &= 3;
		alpha_mode[3] &= 3;
		alpha_mode[4] &= 3;
		if (alpha_mode[0] > 1 ||
			alpha_mode[1] > 1 ||
			alpha_mode[2] > 1 ||
			alpha_mode[3] > 1 ||
			alpha_mode[4] > 1 ||
			(sprite_alpha & 0xff) != 0xff)
		{
			/* set alpha level */
			if (alpha_level != m_alpha_level_last)
			{
				const u8 a = BIT(alpha_level, 12, 4);
				const u8 b = BIT(alpha_level,  8, 4);
				const u8 c = BIT(alpha_level,  4, 4);
				const u8 d = BIT(alpha_level,  0, 4);

				/* b000 7000 */
				u8 al_s = std::min(255, ((15 - d) * 256) / 8);
				u8 al_d = std::min(255, ((15 - b) * 256) / 8);
				m_alpha_level_3as = al_s;
				m_alpha_level_3ad = al_d;
				m_alpha_level_2as = al_d;
				m_alpha_level_2ad = al_s;

				al_s = std::min(255, ((15 - c) * 256) / 8);
				al_d = std::min(255, ((15 - a) * 256) / 8);
				m_alpha_level_3bs = al_s;
				m_alpha_level_3bd = al_d;
				m_alpha_level_2bs = al_d;
				m_alpha_level_2bd = al_s;

				alpha_set_level();
				m_alpha_level_last = alpha_level;
			}

			/* set sprite alpha mode */
			sprite_alpha_check = 0;
			sprite_alpha_all_2a = 1;
			m_dpix_sp[1] = nullptr;
			m_dpix_sp[2] = nullptr;
			m_dpix_sp[4] = nullptr;
			m_dpix_sp[8] = nullptr;
			for (i = 0; i < 4; i++)    /* i = sprite priority offset */
			{
				const u8 sprite_alpha_mode = (sprite_alpha >> (i * 2)) & 3;
				const u8 sftbit = 1 << i;
				if (m_sprite_pri_usage & sftbit)
				{
					if (sprite_alpha_mode == 1)
					{
						if (m_alpha_level_2as == 0 && m_alpha_level_2ad == 255)
							m_sprite_pri_usage &= ~sftbit;  // Disable sprite priority block
						else
						{
							m_dpix_sp[sftbit] = m_dpix_n[2];
							sprite_alpha_check |= sftbit;
						}
					}
					else if (sprite_alpha_mode == 2)
					{
						if (sprite_alpha & 0xff00)
						{
							if (m_alpha_level_3as == 0 && m_alpha_level_3ad == 255) m_sprite_pri_usage &= ~sftbit;
							else
							{
								m_dpix_sp[sftbit] = m_dpix_n[3];
								sprite_alpha_check |= sftbit;
								sprite_alpha_all_2a = 0;
							}
						}
						else
						{
							if (m_alpha_level_3bs == 0 && m_alpha_level_3bd == 255) m_sprite_pri_usage &= ~sftbit;
							else
							{
								m_dpix_sp[sftbit] = m_dpix_n[5];
								sprite_alpha_check |= sftbit;
								sprite_alpha_all_2a = 0;
							}
						}
					}
				}
			}

			/* check alpha level */
			for (i = 0; i < 5; i++)    /* i = playfield num (pos) */
			{
				const u8 alpha_type = (alpha_mode_flag[i] >> 4) & 3;

				if (alpha_mode[i] == 2)
				{
					if (alpha_type == 1)
					{
						/* if (m_alpha_level_2as == 0   && m_alpha_level_2ad == 255)
						 *     alpha_mode[i]=3; alpha_mode_flag[i] |= 0x80; }
						 * will display continue screen in gseeker (mt 00026) */
						if      (m_alpha_level_2as == 0   && m_alpha_level_2ad == 255) alpha_mode[i] = 0;
						else if (m_alpha_level_2as == 255 && m_alpha_level_2ad ==   0) alpha_mode[i] = 1;
					}
					else if (alpha_type == 2)
					{
						if      (m_alpha_level_2bs == 0   && m_alpha_level_2bd == 255) alpha_mode[i] = 0;
						else if (m_alpha_level_2as == 255 && m_alpha_level_2ad ==   0 &&
								 m_alpha_level_2bs == 255 && m_alpha_level_2bd ==   0) alpha_mode[i] = 1;
					}
					else if (alpha_type == 3)
					{
						if      (m_alpha_level_2as == 0   && m_alpha_level_2ad == 255 &&
								 m_alpha_level_2bs == 0   && m_alpha_level_2bd == 255) alpha_mode[i] = 0;
						else if (m_alpha_level_2as == 255 && m_alpha_level_2ad ==   0 &&
								 m_alpha_level_2bs == 255 && m_alpha_level_2bd ==   0) alpha_mode[i] = 1;
					}
				}
				else if (alpha_mode[i] == 3)
				{
					if (alpha_type == 1)
					{
						if      (m_alpha_level_3as == 0   && m_alpha_level_3ad == 255) alpha_mode[i] = 0;
						else if (m_alpha_level_3as == 255 && m_alpha_level_3ad ==   0) alpha_mode[i] = 1;
					}
					else if (alpha_type == 2)
					{
						if      (m_alpha_level_3bs == 0   && m_alpha_level_3bd == 255) alpha_mode[i] = 0;
						else if (m_alpha_level_3as == 255 && m_alpha_level_3ad ==   0 &&
								 m_alpha_level_3bs == 255 && m_alpha_level_3bd ==   0) alpha_mode[i] = 1;
					}
					else if (alpha_type == 3)
					{
						if      (m_alpha_level_3as == 0   && m_alpha_level_3ad == 255 &&
								 m_alpha_level_3bs == 0   && m_alpha_level_3bd == 255) alpha_mode[i] = 0;
						else if (m_alpha_level_3as == 255 && m_alpha_level_3ad ==   0 &&
								 m_alpha_level_3bs == 255 && m_alpha_level_3bd ==   0) alpha_mode[i] = 1;
					}
				}
			}

			if ((alpha_mode[0] == 1 || alpha_mode[0] == 2 || !alpha_mode[0]) &&
				(alpha_mode[1] == 1 || alpha_mode[1] == 2 || !alpha_mode[1]) &&
				(alpha_mode[2] == 1 || alpha_mode[2] == 2 || !alpha_mode[2]) &&
				(alpha_mode[3] == 1 || alpha_mode[3] == 2 || !alpha_mode[3]) &&
				(alpha_mode[4] == 1 || alpha_mode[4] == 2 || !alpha_mode[4]) &&
				sprite_alpha_all_2a)
			{
				const u8 alpha_type = (alpha_mode_flag[0] | alpha_mode_flag[1] | alpha_mode_flag[2] | alpha_mode_flag[3]) & 0x30;
				if ((alpha_type == 0x10 && m_alpha_level_2as == 255) ||
					(alpha_type == 0x20 && m_alpha_level_2as == 255 && m_alpha_level_2bs == 255) ||
					(alpha_type == 0x30 && m_alpha_level_2as == 255 && m_alpha_level_2bs == 255))
				{
					if (alpha_mode[0] > 1) alpha_mode[0] = 1;
					if (alpha_mode[1] > 1) alpha_mode[1] = 1;
					if (alpha_mode[2] > 1) alpha_mode[2] = 1;
					if (alpha_mode[3] > 1) alpha_mode[3] = 1;
					if (alpha_mode[4] > 1) alpha_mode[4] = 1;
					sprite_alpha_check = 0;
					m_dpix_sp[1] = nullptr;
					m_dpix_sp[2] = nullptr;
					m_dpix_sp[4] = nullptr;
					m_dpix_sp[8] = nullptr;
				}
			}
		}
		else
		{
			sprite_alpha_check = 0;
			m_dpix_sp[1] = nullptr;
			m_dpix_sp[2] = nullptr;
			m_dpix_sp[4] = nullptr;
			m_dpix_sp[8] = nullptr;
		}

		/* set scanline priority */
		u8 layer_tmp[5];
		int count_skip_layer = 0;
		{
			int pri_max_opa = -1;
			for (i = 0; i < 5; i++)    /* i = playfield num (pos) */
			{
				const u16 p0 = pri[i];
				const u8 pri_sl1 = p0 & 0x0f;

				layer_tmp[i] = i + (pri_sl1 << 3);

				if (!alpha_mode[i])
				{
					layer_tmp[i] |= 0x80;
					count_skip_layer++;
				}
				else if (alpha_mode[i] == 1 && (alpha_mode_flag[i] & 0x80))
				{
					if (layer_tmp[i] > pri_max_opa) pri_max_opa = layer_tmp[i];
				}
			}

			if (pri_max_opa != -1)
			{
				if (pri_max_opa > layer_tmp[0]) { layer_tmp[0] |= 0x80; count_skip_layer++; }
				if (pri_max_opa > layer_tmp[1]) { layer_tmp[1] |= 0x80; count_skip_layer++; }
				if (pri_max_opa > layer_tmp[2]) { layer_tmp[2] |= 0x80; count_skip_layer++; }
				if (pri_max_opa > layer_tmp[3]) { layer_tmp[3] |= 0x80; count_skip_layer++; }
				if (pri_max_opa > layer_tmp[4]) { layer_tmp[4] |= 0x80; count_skip_layer++; }
			}
		}

		/* sort layer_tmp */
		std::sort(std::begin(layer_tmp), std::end(layer_tmp), std::greater<u8>());

		/* check sprite & layer priority */
		{
			u8 pri_sp[5];

			const u8 l0 = layer_tmp[0] >> 3;
			const u8 l1 = layer_tmp[1] >> 3;
			const u8 l2 = layer_tmp[2] >> 3;
			const u8 l3 = layer_tmp[3] >> 3;
			const u8 l4 = layer_tmp[4] >> 3;

			pri_sp[0] =  spri & 0xf;
			pri_sp[1] = (spri >> 4) & 0xf;
			pri_sp[2] = (spri >> 8) & 0xf;
			pri_sp[3] =  spri >> 12;

			for (i = 0; i < 4; i++)    /* i = sprite priority offset */
			{
				const u8 sflg = 1 << i;
				if (!(m_sprite_pri_usage & sflg)) continue;
				u8 sp = pri_sp[i];

				/*
				    sprite priority==playfield priority
				        GSEEKER (plane leaving hangar) --> sprite
				        BUBSYMPH (title)       ---> sprite
				        DARIUSG (ZONE V' BOSS) ---> playfield
				*/

				if (m_game == BUBSYMPH) sp++;        //BUBSYMPH (title)
				if (m_game == GSEEKER) sp++;     //GSEEKER (plane leaving hangar)

				if      (            sp > l0) sprite[0] |= sflg;
				else if (sp <= l0 && sp > l1) sprite[1] |= sflg;
				else if (sp <= l1 && sp > l2) sprite[2] |= sflg;
				else if (sp <= l2 && sp > l3) sprite[3] |= sflg;
				else if (sp <= l3 && sp > l4) sprite[4] |= sflg;
				else if (sp <= l4           ) sprite[5] |= sflg;
			}
		}


		/* draw scanlines */
		bool alpha = false;
		for (i = count_skip_layer; i < 5; i++)
		{
			const u8 pos = layer_tmp[i] & 7;
			line_t[i] = &pf_line_inf[pos];

			if (sprite[i] & sprite_alpha_check) alpha = true;
			else if (!alpha) sprite[i] |= 0x100;

			if (alpha_mode[pos] > 1)
			{
				const u8 alpha_type = (((alpha_mode_flag[pos] >> 4) & 3) - 1) * 2;
				m_dpix_lp[i] = m_dpix_n[alpha_mode[pos]+alpha_type];
				alpha = true;
			}
			else
			{
				if (alpha) m_dpix_lp[i] = m_dpix_n[1];
				else       m_dpix_lp[i] = m_dpix_n[0];
			}
		}
		if (sprite[5] & sprite_alpha_check) alpha = true;
		else if (!alpha) sprite[5] |= 0x100;

		draw_scanlines(bitmap, 320, draw_line_num, line_t, sprite, rot, count_skip_layer);
		if (y_start < 0) break;
	}
}

/******************************************************************************/

inline void taito_f3_state::f3_drawgfx(bitmap_ind16 &dest_bmp, const rectangle &myclip, gfx_element *gfx, const tempsprite &sprite)
{
	
	if (gfx)
	{
		const u8 *code_base = gfx->get_data(sprite.code % gfx->elements());

		//logerror("sprite draw at %f %f size %f %f\n", sprite.x/16.0, sprite.y/16.0, sprite.zoomx/16.0, sprite.zoomy/16.0);
		u8 flipx = sprite.flipx ? 0xF : 0;
		u8 flipy = sprite.flipy ? 0xF : 0;
		
		fixed8 dy8 = (sprite.y << 4);
		for (u8 y=0; y<16; y++) {
			int dy = dy8 >> 8;
			dy8 += sprite.zoomy;
			if (dy < myclip.min_y || dy > myclip.max_y)
				continue;
			u8 *pri = &m_pri_alp_bitmap.pix(dy);
			u16* dest = &dest_bmp.pix(dy);
			auto src = &code_base[(y ^ flipy)*16];

			fixed8 dx8 = (sprite.x << 4) + 128; // 128 is ½ in fixed.8
			for (u8 x=0; x<16; x++) {
				int dx = dx8 >> 8;
				dx8 += sprite.zoomx;
				if (dx < myclip.min_x || dx > myclip.max_x)
					continue;
				if (dx == dx8 >> 8) // if the next pixel would be in the same column, skip this one
					continue;
				int c = src[(x ^ flipx)] & m_sprite_pen_mask;
				if (c && !pri[dx]) {
					dest[dx] = gfx->colorbase() + (sprite.color<<4 | c);
					pri[dx] = 1;
				}
			}
		}
	}
}

void taito_f3_state::get_sprite_info(const u16 *spriteram16_ptr)
{
	struct sprite_axis
	{
		fixed8 block_size = 0x100;
		fixed4 pos = 0, block_pos = 0;
		s16 global = 0, subglobal = 0;
		void update(u8 scroll, u16 posw, bool lock, u8 block_ctrl, u8 new_zoom)
		{
			s16 new_pos = util::sext(posw, 12);
			// set scroll offsets
			if (BIT(scroll, 0))
				subglobal = new_pos;
			if (BIT(scroll, 1))
				global = new_pos;
			// add scroll offsets
			if (!BIT(scroll, 2))
				new_pos += subglobal;
			if (!BIT(scroll, 3))
				new_pos += global;
		
			switch (block_ctrl)
			{
			case 0b00:
				if (!lock)
				{
					block_pos = new_pos << 4;
					block_size = (0x100 - new_zoom);
				}
				[[fallthrough]];
			case 0b10:
				pos = block_pos;
				break;
			case 0b11:
				pos += block_size;
				break;
			}
		};
	};
	sprite_axis x, y;
	u8 color = 0;
	//bool multi = false;
	
	const rectangle &visarea = m_screen->visible_area();
	
	tempsprite *sprite_ptr = &m_spritelist[0];
	int total_sprites = 0;
	int bank = 0;
	for (int offs = 0; offs < 0x400 && (total_sprites < 0x400); offs++)
	{
		const u16 *spr = &spriteram16_ptr[bank + (offs * 8)];

		/* Check if the sprite list jump command bit is set */
		if (BIT(spr[6], 15))
		{
			const u32 new_offs = BIT(spr[6], 0, 10);
			if (new_offs <= offs) // could this be ≤ ?
			{
				if (new_offs < offs)
					logerror("backwards long jump (sprite 0x%x to 0x%x)\n", offs, new_offs);
				break;
			}
			offs = new_offs - 1; // subtract because we increment in the for loop
		}

		/* Check if special command bit is set */
		if (BIT(spr[3], 15))
		{
			u16 cntrl = spr[5];
			m_flipscreen = BIT(cntrl, 13);

			/*  cntrl bit 12(0x1000) = disabled?  (From F2 driver, doesn't seem used anywhere)
			    cntrl bit 4 (0x0010) = ???
			    cntrl bit 5 (0x0020) = ???
			    cntrl bit 1 (0x0002) = enabled when Darius Gaiden sprite trail effect should occur (MT #1922)
			                     Notice that sprites also completely disappear due of a bug/missing feature in the alpha routines.
			*/

			m_sprite_extra_planes = BIT(cntrl, 8, 2);   // 00 = 4bpp, 01 = 5bpp, 10 = unused?, 11 = 6bpp
			m_sprite_pen_mask = (m_sprite_extra_planes << 4) | 0x0f;

			/* Sprite bank select */
			if (BIT(cntrl, 0))
				bank = 0x4000;
		}

		u8 spritecont = spr[4] >> 8;
		bool lock = BIT(spritecont, 2);
		if (!lock)
			color = spr[4] & 0xFF;
		u8 scroll_mode = BIT(spr[2], 12, 4);
		u16 zooms = spr[1];
		x.update(scroll_mode, spr[2] & 0xFFF, lock, BIT(spritecont, 4+2, 2), zooms & 0xFF);
		y.update(scroll_mode, spr[3] & 0xFFF, lock, BIT(spritecont, 4+0, 2), zooms >> 8);
		
		const int tile = spr[0] | (BIT(spr[5], 0) << 16);
		if (!tile) continue;
		
		fixed4 tx = m_flipscreen ? (512<<4) - x.block_size - x.pos : x.pos;
		fixed4 ty = m_flipscreen ? (256<<4) - y.block_size - y.pos : y.pos;

		if (tx + x.block_size <= visarea.min_x<<4 || tx > visarea.max_x<<4 || ty + y.block_size <= visarea.min_y<<4 || ty > visarea.max_y<<4)
			continue;
		
		bool flipx = BIT(spritecont, 0);
		bool flipy = BIT(spritecont, 1);
		//multi = BIT(spritecont, 3);
		
		sprite_ptr->x = tx;
		sprite_ptr->y = ty;
		sprite_ptr->flipx = m_flipscreen ? !flipx : flipx;
		sprite_ptr->flipy = m_flipscreen ? !flipy : flipy;
		sprite_ptr->code = tile;
		sprite_ptr->color = color;
		sprite_ptr->zoomx = x.block_size;
		sprite_ptr->zoomy = y.block_size;
		sprite_ptr->pri = BIT(color, 6, 2);
		sprite_ptr++;
		total_sprites++;
	}
	m_sprite_end = sprite_ptr;
}

void taito_f3_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/*	const tempsprite *sprite_ptr;
	gfx_element *sprite_gfx = m_gfxdecode->gfx(2);

	sprite_ptr = m_sprite_end;
	m_sprite_pri_usage = 0;
	
	while (sprite_ptr != &m_spritelist[0])
	{
		sprite_ptr--;

		const u8 pri = sprite_ptr->pri;
		m_sprite_pri_usage |= 1 << pri;

		f3_drawgfx(bitmap, cliprect, sprite_gfx, *sprite_ptr);
		}*/
}

/******************************************************************************/
u32 taito_f3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u32 sy_fix[5], sx_fix[5];

	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Setup scroll */
	for (int i = 0; i < 4; ++i)
		get_pf_scroll(i, sx_fix[i], sy_fix[i]);

	sx_fix[4] = -(m_control_1[4]) + 41;
	sy_fix[4] = -(m_control_1[5] & 0x1ff);

	if (m_flipscreen)
	{
		sx_fix[4] = -sx_fix[4] + 75;
		sy_fix[4] = -sy_fix[4];
	}

	bitmap.fill(0, cliprect);
	m_pri_alp_bitmap.fill(0, cliprect);

	/* sprites */
	if (m_sprite_lag == 0)
		get_sprite_info(m_spriteram.target());

	/* Update sprite buffer */
	//draw_sprites(bitmap, cliprect);

	/* Parse sprite, alpha & clipping parts of lineram */
	get_spritealphaclip_info();

	/* Parse playfield effects */
	get_line_ram_info(m_tilemap[0], sx_fix[0], sy_fix[0], 0, m_pf_data[0]);
	get_line_ram_info(m_tilemap[1], sx_fix[1], sy_fix[1], 1, m_pf_data[1]);
	get_line_ram_info(m_tilemap[2], sx_fix[2], sy_fix[2], 2, m_pf_data[2]);
	get_line_ram_info(m_tilemap[3], sx_fix[3], sy_fix[3], 3, m_pf_data[3]);
	get_vram_info(m_vram_layer, m_pixel_layer, sx_fix[4], sy_fix[4]);

	/* Draw final framebuffer */
	//scanline_draw(bitmap, cliprect);
	scanline_draw_TWO(bitmap, cliprect);

	if (VERBOSE)
		print_debug_info(bitmap);
	return 0;
}
