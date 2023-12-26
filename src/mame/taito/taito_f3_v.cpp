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

	if (BIT(vram_tile,  8)) flags |= TILE_FLIPX;
	if (BIT(vram_tile, 15)) flags |= TILE_FLIPY;

	tileinfo.set(0,
			vram_tile & 0xff,
			BIT(vram_tile, 9, 6),
			flags);
}

TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info_pixel)
{

	u8 flags = 0;
	int y_offs = (m_control_1[5] & 0x1ff);
	if (m_flipscreen) y_offs += 0x100;

	/* Colour is shared with VRAM layer */
	int col_off;
	if ((((tile_index & 0x1f) * 8 + y_offs) & 0x1ff) > 0xff)
		col_off = 0x800 + ((tile_index & 0x1f) << 6) + ((tile_index & 0xfe0) >> 5);
	else
		col_off = ((tile_index & 0x1f) << 6) + ((tile_index & 0xfe0) >> 5);

	const u16 vram_tile = (m_textram[col_off] & 0xffff);

	if (BIT(vram_tile,  8)) flags |= TILE_FLIPX;
	if (BIT(vram_tile, 15)) flags |= TILE_FLIPY;

	tileinfo.set(1,
			tile_index,
			BIT(vram_tile, 9, 6),
			flags);
}

/******************************************************************************/

void taito_f3_state::screen_vblank(int state)
{
	if (state) {
		//get_sprite_info(m_spriteram.target());
	}
}

void taito_f3_state::video_start()
{
	const F3config *pCFG = &f3_config_table[0];

	m_spritelist = nullptr;
	m_spriteram16_buffered = nullptr;
	//m_line_inf = nullptr;
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
	//m_line_inf = std::make_unique<f3_line_inf[]>(256);
	m_screen->register_screen_bitmap(m_pri_alp_bitmap);
	m_tile_opaque_sp = std::make_unique<u8[]>(m_gfxdecode->gfx(2)->elements());
	for (auto &tile_opaque : m_tile_opaque_pf)
		tile_opaque = std::make_unique<u8[]>(m_gfxdecode->gfx(3)->elements());

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
	logerror("@@ scroll command: %04x at: %04x vpos: %d\n", data, offset*2, m_screen->vpos());

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
	if (offset % 8 == 3) {
		logerror("sprite write: %04x at: %04x vpos: %d\n", data, offset*2, m_screen->vpos());
	}
	if (offset % 8 == 5) {
		if (BIT(m_spriteram[(offset & ~0b111) + 3], 15)) {
			logerror("!sprite command: %04x at: %04x vpos: %d\n", data, offset*2, m_screen->vpos());
		}
	}

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
	u8 r, g, b;

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
		if (line.pivot.pivot_control & 0b01010101) // check if unknown pivot control bits set
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
	// iiii iiii iiff ffff
	// fractional part inverted (like scroll regs)
	if (this_line(0xc00) & 1) {
		u32 rowscroll = this_line(0xa000) << 10;
		line.pf[0].rowscroll = (rowscroll & 0xffff0000) - (rowscroll & 0x0000ffff);
	}
	if (this_line(0xc00) & 2) {
		u32 rowscroll = this_line(0xa200) << 10;
		line.pf[1].rowscroll = (rowscroll & 0xffff0000) - (rowscroll & 0x0000ffff);
	}
	if (this_line(0xc00) & 4) {
		u32 rowscroll = this_line(0xa400) << 10;
		line.pf[2].rowscroll = (rowscroll & 0xffff0000) - (rowscroll & 0x0000ffff);
	}
	if (this_line(0xc00) & 8) {
		u32 rowscroll = this_line(0xa600) << 10;
		line.pf[3].rowscroll = (rowscroll & 0xffff0000) - (rowscroll & 0x0000ffff);
	}

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

void taito_f3_state::get_pf_scroll(int pf_num, u16_16 &reg_sx, u16_16 &reg_sy)
{
	// x: iiii iiii iiFF FFFF
	// y: iiii iiii ifff ffff

	// x scroll is stored as fixed10.6, with fractional bits inverted.
	// we convert this to regular fixed16.16
	u16_16 sx = (m_control_0[pf_num] ^ 0b111111) << (16-6);
	u16_16 sy = (m_control_0[pf_num + 4]) << (16-7); // fixed11.7 to fixed16.16

	sx -= (6 + 4 * pf_num) << 16;
	sy += 1 << 16;

	if (m_flipscreen) {
		sx += (416+188) << 16;
		sy -= 1024 << 16;

		if (m_game_config->extend)
			sx -= 512 << 16;
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
		if (const u16 col = sp->srcbitmap.pix(y, x)) // 0 = transparent
			dst[x] = clut[col];
	}
}
void taito_f3_state::draw_line(u32* dst, int y, int xs, int xe, playfield_inf* pf)
{
	if (!pf->layer_enable())
		return;
	const pen_t *clut = &m_palette->pen(0);
	const int y_index = ((pf->reg_fx_y >> 16) + pf->colscroll) & 0x1ff;
	u16_16 fx_x = pf->reg_sx + pf->rowscroll + 10*pf->x_scale;
	fx_x &= (m_width_mask << 16) | 0xffff;

	for (int x = xs; x < xe; x++) {
		int x_index = ((fx_x >> 16) + x) & m_width_mask;
		if (!(pf->flagsbitmap->pix(y_index, x_index) & 0xf0))
			continue;
		if (const u16 col = pf->srcbitmap->pix(y_index, x_index))
			dst[x] = clut[col];
	}
}

void taito_f3_state::draw_line(u32* dst, int y, int xs, int xe, pivot_inf* pv)
{
	const pen_t *clut = &m_palette->pen(0);
	const auto *srcbitmap = pv->use_pix() ? pv->srcbitmap_pixel : pv->srcbitmap_vram;
	const auto *flagsbitmap = pv->use_pix() ? pv->flagsbitmap_pixel : pv->flagsbitmap_vram;
	const u16 height_mask = pv->use_pix() ? 0xff : 0x1ff;
	const u16 width_mask = 0x1ff;

	const int y_index = (pv->reg_sy + y) & height_mask;

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
	for (auto &spgroup : line_data.sp) {
		// aaa why are you making a new one every time...
		spgroup.srcbitmap = bitmap_ind16(bitmap.width(), bitmap.height());
	}
	for (int pf = 0; pf < NUM_PLAYFIELDS; ++pf) {
		int tmap_number = pf + line_data.pf[pf].alt_tilemap * 2;
		line_data.pf[pf].srcbitmap = &m_tilemap[tmap_number]->pixmap();
		line_data.pf[pf].flagsbitmap = &m_tilemap[tmap_number]->flagsmap();

		get_pf_scroll(pf, line_data.pf[pf].reg_sx, line_data.pf[pf].reg_sy);
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
		myclip = cliprect;
		myclip &= m_pri_alp_bitmap.cliprect();

		gfx_element *sprite_gfx = m_gfxdecode->gfx(2);

		const tempsprite *sprite_ptr = m_sprite_end;
		while (sprite_ptr != &m_spritelist[0])
		{
			sprite_ptr--;

			f3_drawgfx(line_data.sp[sprite_ptr->pri].srcbitmap, myclip, sprite_gfx, *sprite_ptr);
		}
	}

	auto prio = [](const auto& obj) -> u8 { return obj->prio(); };

	int ys = m_flipscreen ? 24 : 0;
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
				draw_line(&bitmap.pix(y+ys), y+ys, 46, 46 + 320, arg);
			}, gfx);
		}

		if (y != y_start) {
			// update registers
			for (auto &pf : line_data.pf) {
				pf.reg_fx_y += pf.y_scale << (16-7);
			}
		}
	}
}


/******************************************************************************/

inline void taito_f3_state::f3_drawgfx(bitmap_ind16 &dest_bmp, const rectangle &myclip, gfx_element *gfx, const tempsprite &sprite)
{
	if (gfx == nullptr)
		return;

	const u8 *code_base = gfx->get_data(sprite.code % gfx->elements());

	//logerror("sprite draw at %f %f size %f %f\n", sprite.x/16.0, sprite.y/16.0, sprite.zoomx/16.0, sprite.zoomy/16.0);
	const u8 flipx = sprite.flipx ? 0xF : 0;
	const u8 flipy = sprite.flipy ? 0xF : 0;

	fixed8 dy8 = (sprite.y << 4);
	if (!m_flipscreen)
		dy8 += 255; // round up in non-flipscreen mode?    mayybe flipscreen coordinate adjustments should be done after all this math, during final rendering?. anyway:  testcases for vertical scaling: elvactr mission # text (non-flipscreen), kaiserknj attract mode first text line (flipscreen)
	for (u8 y = 0; y < 16; y++) {
		const int dy = dy8 >> 8;
		dy8 += sprite.zoomy;
		if (dy < myclip.min_y || dy > myclip.max_y)
			continue;
		u8 *pri = &m_pri_alp_bitmap.pix(dy);
		u16* dest = &dest_bmp.pix(dy);
		auto src = &code_base[(y ^ flipy) * 16];

		fixed8 dx8 = (sprite.x << 4) + 128; // 128 is ½ in fixed.8
		for (u8 x = 0; x < 16; x++) {
			const int dx = dx8 >> 8;
			dx8 += sprite.zoomx;
			// is this necessary with the large margins outside visarea?
			if (dx < myclip.min_x || dx > myclip.max_x)
				continue;
			if (dx == dx8 >> 8) // if the next pixel would be in the same column, skip this one
				continue;
			const auto c = src[(x ^ flipx)] & m_sprite_pen_mask;
			if (c && !pri[dx]) {
				dest[dx] = gfx->colorbase() + (sprite.color<<4 | c);
				pri[dx] = 1;
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

	for (u32 offs = 0; offs < 0x400 && (total_sprites < 0x400); offs++)
	{
		int bank = m_sprite_bank ? 0x4000 : 0;
		const u16 *spr = &spriteram16_ptr[bank + (offs * 8)];

		/* Check if the sprite list jump command bit is set */
		if (BIT(spr[6], 15))
		{
			const int new_offs = BIT(spr[6], 0, 10);
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

			/*
			    ??f? ??dd ???? ??tb
			    
			    
			    cntrl bit 12(0x1000) = disabled?  (From F2 driver, doesn't seem used anywhere)
			    cntrl bit 4 (0x0010) = ???
			    cntrl bit 5 (0x0020) = ???
			    cntrl bit 1 (0x0002) = enabled when Darius Gaiden sprite trail effect should occur (MT #1922)
			                     Notice that sprites also completely disappear due of a bug/missing feature in the alpha routines.
			*/

			m_sprite_extra_planes = BIT(cntrl, 8, 2); // 00 = 4bpp, 01 = 5bpp, 10 = unused?, 11 = 6bpp
			m_sprite_pen_mask = (m_sprite_extra_planes << 4) | 0x0f;

			/* Sprite bank select */
			m_sprite_bank = BIT(cntrl, 0);
		}

		const u8 spritecont = spr[4] >> 8;
		const bool lock = BIT(spritecont, 2);
		if (!lock)
			color = spr[4] & 0xFF;
		const u8 scroll_mode = BIT(spr[2], 12, 4);
		const u16 zooms = spr[1];
		x.update(scroll_mode, spr[2] & 0xFFF, lock, BIT(spritecont, 4+2, 2), zooms & 0xFF);
		y.update(scroll_mode, spr[3] & 0xFFF, lock, BIT(spritecont, 4+0, 2), zooms >> 8);

		const int tile = spr[0] | (BIT(spr[5], 0) << 16);
		if (!tile) continue;

		const fixed4 tx = m_flipscreen ? (512<<4) - x.block_size - x.pos : x.pos;
		const fixed4 ty = m_flipscreen ? (256<<4) - y.block_size - y.pos : y.pos;

		if (tx + x.block_size <= visarea.min_x<<4 || tx > visarea.max_x<<4 || ty + y.block_size <= visarea.min_y<<4 || ty > visarea.max_y<<4)
			continue;

		const bool flipx = BIT(spritecont, 0);
		const bool flipy = BIT(spritecont, 1);
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
	logerror("vpos screen update now: %d\n", m_screen->vpos());;

	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(0, cliprect);
	m_pri_alp_bitmap.fill(0, cliprect);

	/* sprites */
	//if (m_sprite_lag == 0)
	// these are getted during vblank now
	//get_sprite_info(m_spriteram.target());


	/* Update sprite buffer */
	//draw_sprites(bitmap, cliprect);

	/* Draw final framebuffer */

	scanline_draw_TWO(bitmap, cliprect);

	get_sprite_info(m_spriteram.target());

	if (VERBOSE)
		print_debug_info(bitmap);
	return 0;
}
