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
        0x03ff: x repeat / mosaic - each tile collapses to 16 single colour lines [unemulated]
          Bits: ??ps mmmm 4321
            4321 = enable effect for respective playfield
            mmmm = x repeat - 0 = repeat 1st pixel 16 times (sample every 16)
                              1 = repeat 1st pixel 15 times (sample every 15)
                              f = repeat 1st pixel  1 times (sample every pixel)
               s = enable effect for sprites
               p = enable effect for pivot layer ?
            (spcinvdj title screen, riding fight)

       0xff00: previous pixel response? and palette ram format?? [unemulated]
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
	const u16* m_spriteram16_buffered = m_spriteram.target();
	
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
	u16* tilep = &m_pf_data[Layer][tile_index * 2];
	// tile info:
	// [yx?? ddac cccc cccc]
	// yx: x/y flip
	// ?: upper bits of tile number?
	// d: bpp
	// a: alpha blend mode
	// c: color
	
	const u16 palette_code = BIT(tilep[0],  0, 9);
	const u8 abtype        = BIT(tilep[0],  9, 1);
	const u8 extra_planes  = BIT(tilep[0], 10, 2); // 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp

	tileinfo.set(3,
			tilep[1],
			palette_code,
			TILE_FLIPYX(BIT(tilep[0], 14, 2)));

	tileinfo.category = abtype & 1; // alpha blending type
	// gfx extra planes and palette code set the same bits of color address
	// we need to account for tilemap.h combining using "+" instead of "|"
	tileinfo.pen_mask = ((extra_planes & ~palette_code) << 4) | 0x0f;
}


TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info_text)
{
	const u16 vram_tile = m_textram[tile_index];
	// text tile info:
	// [yccc cccx tttt tttt]
	// y: y flip
	// c: palette
	// x: x flip
	// t: tile number

	u8 flags = 0;
	if (BIT(vram_tile,  8)) flags |= TILE_FLIPX;
	if (BIT(vram_tile, 15)) flags |= TILE_FLIPY;

	tileinfo.set(0,
			vram_tile & 0xff,
			BIT(vram_tile, 9, 6),
			flags);
}

TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info_pixel)
{
	int y_offs = BIT(m_control_1[5], 0, 9);
	if (m_flipscreen)
		y_offs += 0x100;

	/* Colour is shared with VRAM layer */
	// lllllllhhhhh ??
	//+ hhhhh
	//+     lllllll
	int col_off = (BIT(tile_index, 0, 5) << 6) + BIT(tile_index, 5, 7);
	if (((BIT(tile_index, 0, 5) * 8 + y_offs) & 0x1ff) > 0xff)
		col_off += 0x800;
	// edits here are untested
	
	const u16 vram_tile = m_textram[col_off];

	u8 flags = 0;
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

void taito_f3_state::set_extend(bool state) {
	m_extend = state;
	// TODO: we need to free these if this is called multiple times
	// for (int i=0; i<8; i++) {
	// 	delete m_tilemap[i];
	// }
	if (m_extend) {
		m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[4] = m_tilemap[5] = m_tilemap[6] = m_tilemap[7] = nullptr;
	} else {
		m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<4>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<5>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[6] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<6>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[7] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info<7>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	}
	if (m_extend) {
		m_pf_data[0] = m_pf_ram + (0x0000 / 2);
		m_pf_data[1] = m_pf_ram + (0x2000 / 2);
		m_pf_data[2] = m_pf_ram + (0x4000 / 2);
		m_pf_data[3] = m_pf_ram + (0x6000 / 2);
	} else {
		m_pf_data[0] = m_pf_ram + (0x0000 / 2);
		m_pf_data[1] = m_pf_ram + (0x1000 / 2);
		m_pf_data[2] = m_pf_ram + (0x2000 / 2);
		m_pf_data[3] = m_pf_ram + (0x3000 / 2);
		m_pf_data[4] = m_pf_ram + (0x4000 / 2);
		m_pf_data[5] = m_pf_ram + (0x5000 / 2);
		m_pf_data[6] = m_pf_ram + (0x6000 / 2);
		m_pf_data[7] = m_pf_ram + (0x7000 / 2);
	}
	if (m_extend) {
		m_width_mask = 0x3ff;
		m_twidth_mask = 0x7f;
		m_twidth_mask_bit = 7;
	} else {
		m_width_mask = 0x1ff;
		m_twidth_mask = 0x3f;
		m_twidth_mask_bit = 6;
	}
	if (m_extend) {
		m_tilemap[0]->set_transparent_pen(0);
		m_tilemap[1]->set_transparent_pen(0);
		m_tilemap[2]->set_transparent_pen(0);
		m_tilemap[3]->set_transparent_pen(0);
	} else {
		m_tilemap[0]->set_transparent_pen(0);
		m_tilemap[1]->set_transparent_pen(0);
		m_tilemap[2]->set_transparent_pen(0);
		m_tilemap[3]->set_transparent_pen(0);
		m_tilemap[4]->set_transparent_pen(0);
		m_tilemap[5]->set_transparent_pen(0);
		m_tilemap[6]->set_transparent_pen(0);
		m_tilemap[7]->set_transparent_pen(0);
	}
}

void taito_f3_state::video_start()
{
	const F3config *pCFG = &f3_config_table[0];

	m_spritelist = nullptr;
	//m_spriteram16_buffered = nullptr;
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
	
	set_extend(m_game_config->extend);
	
	//m_spriteram16_buffered = std::make_unique<u16[]>(0x10000 / 2);
	m_spritelist = std::make_unique<tempsprite[]>(0x400);
	m_sprite_end = &m_spritelist[0];
	m_vram_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info_text)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_pixel_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taito_f3_state::get_tile_info_pixel)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
	//m_line_inf = std::make_unique<f3_line_inf[]>(256);
	m_screen->register_screen_bitmap(m_pri_alp_bitmap);
	for (auto &sp_bitmap : m_sprite_framebuffers) {
		m_screen->register_screen_bitmap(sp_bitmap);
	}
	m_tile_opaque_sp = std::make_unique<u8[]>(m_gfxdecode->gfx(2)->elements());
	for (auto &tile_opaque : m_tile_opaque_pf)
		tile_opaque = std::make_unique<u8[]>(m_gfxdecode->gfx(3)->elements());

	m_vram_layer->set_transparent_pen(0);
	m_pixel_layer->set_transparent_pen(0);

	// Palettes have 4 bpp indexes despite up to 6 bpp data. The unused top bits in the gfx data are cleared later.
	m_gfxdecode->gfx(2)->set_granularity(16);
	m_gfxdecode->gfx(3)->set_granularity(16);

	m_flipscreen = false;
	//memset(m_spriteram16_buffered.get(), 0, 0x10000);
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
	//logerror("@@ scroll command: %04x at: %04x vpos: %d\n", data, offset*2, m_screen->vpos());

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
		//logerror("sprite write: %04x at: %04x vpos: %d\n", data, offset*2, m_screen->vpos());
	}
	if (offset % 8 == 5) {
		if (BIT(m_spriteram[(offset & ~0b111) + 3], 15)) {
			//logerror("!sprite command: %04x at: %04x vpos: %d\n", data, offset*2, m_screen->vpos());
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

	// is this supposed to be mirroring?
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

	// .... .... .... .... rrrr gggg bbbb ....
	// .... .... rrrr rrrr gggg gggg bbbb bbbb
	
	/* 12 bit palette games - there has to be a palette select bit somewhere */
	if (m_game == SPCINVDX || m_game == RIDINGF || m_game == ARABIANM || m_game == RINGRAGE)
	{
		b = 15 * ((m_paletteram32[offset] >> 4) & 0xf);
		g = 15 * ((m_paletteram32[offset] >> 8) & 0xf);
		r = 15 * ((m_paletteram32[offset] >> 12) & 0xf);
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
	// "can you use
	// arrow syntax here"
	const auto latched_addr = [=] (u8 section, u8 subsection)
	{
		u16 latches = m_line_ram[(section * 0x200)/2 + y];
		offs_t base = 0x400 * BIT(latches, 8, 8) + 0x200 * subsection;
		if (BIT(latches, subsection + 4))
			return (base + 0x800) / 2 + y;
		else if (BIT(latches, subsection))
			return (base) / 2 + y;
		return 0;
	};
	// 4000 **********************************
	for (int i : {2, 3}) {
		if (offs_t where = latched_addr(0, i)) {
			u16 colscroll = m_line_ram[where];
			line.pf[i].colscroll   = colscroll & 0x1ff;
			line.pf[i].alt_tilemap = colscroll & 0x200;
			line.clip[2*(i-2) + 0].set_upper(BIT(colscroll, 12), BIT(colscroll, 13));
			line.clip[2*(i-2) + 1].set_upper(BIT(colscroll, 14), BIT(colscroll, 15));
		}
	}

	// 5000 **********************************
	// renderer needs to adjust by -48
	for (int i : {0, 1, 2, 3}) {
		if (offs_t where = latched_addr(1, i)) {
			u16 clip_lows = m_line_ram[where];
			line.clip[i].set_lower(BIT(clip_lows, 0, 8), BIT(clip_lows, 8, 8));
		}
	}

	// 6000 **********************************
	if (offs_t where = latched_addr(2, 0)) {
		// first value is sync register, special handling unnecessary?
		u16 line_6000 = m_line_ram[where];

		line.pivot.pivot_control = BIT(line_6000, 8, 8);
		if (line.pivot.pivot_control & 0b01010101) // check if unknown pivot control bits set
			logerror("unknown pivot ctrl bits: %02x__ at %04x\n", line.pivot.pivot_control, 0x6000 + y*2);

		for (int sp_group = 0; sp_group < NUM_SPRITEGROUPS; sp_group++) {
			line.sp[sp_group].mix_value = (line.sp[sp_group].mix_value & 0x3fff)
				| BIT(line_6000, sp_group * 2, 2) << 14;
		}
	}
	if (offs_t where = latched_addr(2, 1)) {
		u16 blend_vals = m_line_ram[where];
		for (int idx = 0; idx < 4; idx++) {
			u8 a = BIT(blend_vals, 4 * idx, 4);
			line.blend[idx] = 0xf - a;
		}
	}
	if (offs_t where = latched_addr(2, 2)) {
		u16 x_mosaic = m_line_ram[where];

		for (int pf_num = 0; pf_num < NUM_PLAYFIELDS; pf_num++) {
			if ((line.pf[pf_num].x_sample_enable = BIT(x_mosaic, pf_num))) {
				line.pf[pf_num].x_sample = (x_mosaic & 0xf0) >> 4;
			}
		}

		line.x_sample = (x_mosaic & 0xf0) >> 4;

		for (int sp_num = 0; sp_num < NUM_SPRITEGROUPS; sp_num++) {
			line.sp[sp_num].x_sample_enable = BIT(x_mosaic, 9);
		}
		line.pivot.x_sample_enable = BIT(x_mosaic, 10);

		line.fx_6400 = (x_mosaic & 0xfc00) >> 8;
		if (line.fx_6400 && line.fx_6400 != 0x70) // check if unknown effect bits set
			logerror("unknown fx bits: %02x__ at %04x\n", line.fx_6400, 0x6400 + y*2);
	}
	if (offs_t where = latched_addr(2, 3)) {
		line.bg_palette = m_line_ram[where];
	}

	// 7000 **********************************
	if (offs_t where = latched_addr(3, 0)) {
		u16 line_7000 = m_line_ram[where];
		line.pivot.pivot_enable = line_7000;
		//if (line_7000) // check if confusing pivot enable bits are set
		//	logerror("unknown 'pivot enable' bits: %04x at %04x\n", line_7000, 0x7000 + y*2);
	}
	if (offs_t where = latched_addr(3, 1)) {
		line.pivot.mix_value = m_line_ram[where];
	}
	if (offs_t where = latched_addr(3, 2)) {
		u16 sprite_mix = m_line_ram[where];
		
		u16 unknown = BIT(sprite_mix, 10, 2);
		if (unknown)
			logerror("unknown sprite mix bits: _%01x__ at %04x\n", unknown << 2, 0x7400 + y*2);
		
		for (int group = 0; group < NUM_SPRITEGROUPS; group++) {
			line.sp[group].mix_value = (line.sp[group].mix_value & 0xc00f)
				| BIT(sprite_mix, 0, 10) << 4;
			line.sp[group].brightness = BIT(sprite_mix, 12 + group, 1);
		}
	}
	if (offs_t where = latched_addr(3, 3)) {
		u16 sprite_prio = m_line_ram[where];
		for (int group = 0; group < NUM_SPRITEGROUPS; group++) {
			line.sp[group].mix_value = (line.sp[group].mix_value & 0xfff0)
				| BIT(sprite_prio, group * 4, 4);
		}
	}

	// 8000 **********************************
	for (int i : { 0, 1, 2, 3 }) {
		if (offs_t where = latched_addr(4, i)) {
			u16 pf_scale = m_line_ram[where];
			// y zooms are interleaved
			const int FIX_Y = { 0, 3, 2, 1 };
			line.pf[i].x_scale = 256-BIT(pf_scale, 8, 8);
			line.pf[FIX_Y[i]].y_scale = BIT(pf_scale, 0, 8)<<1;
		}
	}

	// 9000 **********************************
	for (int i : { 0, 1, 2, 3 }) {
		if (offs_t where = latched_addr(5, i)) {
			u16 pf_pal_add = m_line_ram[where];
			line.pf[i].pal_add = pf_pal_add * 16;
		}
	}

	// A000 **********************************
	// iiii iiii iiff ffff
	// fractional part is negative (allegedly). i wonder if it's supposed to be inverted instead? and then we just subtract (1<<8) to get almost the same value..
	for (int i : { 0, 1, 2, 3 }) {
		if (offs_t where = latched_addr(6, i)) {
			fixed8 rowscroll = m_line_ram[where] << (8-6);
			line.pf[i].rowscroll = (rowscroll & 0xffffff00) - (rowscroll & 0x000000ff);
			// ((i ^ 0b111111) - 0b111111) << (8-6);
		}
	}

	// B000 **********************************
	for (int i : { 0, 1, 2, 3 }) {
		if (offs_t where = latched_addr(7, i)) {
			line.pf[i].mix_value = m_line_ram[where];
		}
	}
}

void taito_f3_state::get_pf_scroll(int pf_num, fixed8 &reg_sx, fixed8 &reg_sy)
{
	// x: iiii iiii iiFF FFFF
	// y: iiii iiii ifff ffff

	// x scroll is stored as fixed10.6, with fractional bits inverted.
	// we convert this to regular fixed24.8
	fixed8 sx = (m_control_0[pf_num] ^ 0b111111) << (8-6);
	fixed8 sy = (m_control_0[pf_num + 4]) << (8-7); // fixed11.7 to fixed24.8

	sx -= (6 + 4 * pf_num) << 8;
	sy += 1 << 8;

	if (m_flipscreen) {
		sx += (416+188) << 8;
		sy -= 1024 << 8;

		if (m_game_config->extend)
			sx -= 512 << 8;
	}

	reg_sx = sx;
	reg_sy = sy;
}

std::vector<taito_f3_state::clip_plane_inf>
taito_f3_state::calc_clip(const clip_plane_inf (&clip)[NUM_CLIPPLANES],
						  const mixable *line)
{
	using clip_range = clip_plane_inf;
	const s16 INF_L = 46;
	const s16 INF_R = 320 + 46;

	std::bitset<4> normal_planes = line->clip_enable() & ~line->clip_inv();
	std::bitset<4> invert_planes = line->clip_enable() & line->clip_inv();
	if (!line->clip_inv_mode())
		std::swap(normal_planes, invert_planes);

	// start with a visible region spanning the entire space
	std::vector<clip_range> ranges{1, clip_range{INF_L, INF_R}};
	for (int plane = 0; plane < NUM_CLIPPLANES; plane++) {
		s16 clip_l = clip[plane].l - 1;
		s16 clip_r = clip[plane].r - 2;
		if (normal_planes[plane]) {
			// check and clip all existing ranges
			for (auto it = ranges.begin(); it != ranges.end(); it++) {
				// if this clip is <1 px wide, clip entire line
				// remove ranges outside normal clip intersection
				if (clip_l > clip_r || it->r < clip_l || it->l > clip_r) {
					ranges.erase(it); --it;
				} else { // otherwise intersect normally
					it->l = std::max(it->l, clip_l);
					it->r = std::min(it->r, clip_r);
				}
			}
		} else if (invert_planes[plane] && (clip_l <= clip_r)) {
			// ASSUMING: only up to two clip settings legal at a time,
			// can get up to 3 ranges; figure out which one it *isn't* later
			std::vector<clip_range> new_ranges{};
			new_ranges.reserve(2 * ranges.size());
			new_ranges.insert(new_ranges.end(), ranges.size(), clip_range{INF_L, clip_l});
			new_ranges.insert(new_ranges.end(), ranges.size(), clip_range{clip_r, INF_R});

			for (auto it = new_ranges.begin(); it != new_ranges.end(); it++) {
				for (const auto &range : ranges) {
					it->l = std::max(range.l, it->l);
					it->r = std::max(range.l, it->r);
					if (it->l >= it->r) {
						new_ranges.erase(it); --it;
						break; // goto...
					}
				}
			}
			ranges = new_ranges;
		}
	}
	return ranges;
}

static int mosaic(int x, int sample) {
	int x_count = (x - 46 + 114);
	x_count = x_count >= 432 ? x_count - 432 : x_count;
	return x - (x_count % sample);
}

void taito_f3_state::blend_s(u8 blend_mode, bool sel, u8 prio, const u8 *blendvals, pri_alpha &pri_alp, u32 &dst, u32 src)
{
	/*if (blendvals[(2 - (blend_mode & 0b10)) + sel] > 8) {
		dst = src;
		pri_alp.pri = prio;
		pri_alp.active_alpha = blend_mode;
		pri_alp.alpha = 0;
		return;
		}*/
	int al_a = blendvals[sel] * 32;
	int al_b = blendvals[2 + sel] * 32;
	//logerror("b%d a%d", b, a);
	if (blend_mode & 0b10)
		std::swap(al_a, al_b);
	rgb_t rgb{src};
	dst = rgb.scale8(std::min(255, al_b)).set_a(255);
	pri_alp.pri = prio;
	pri_alp.active_alpha = blend_mode;
	pri_alp.alpha = std::min(255, al_a);	
}
void taito_f3_state::blend_o(u8 blend_mode, bool sel, u8 prio, const u8 *blendvals, pri_alpha &pri_alp, u32 &dst, u32 src)
{
	const int al_a = std::min(255, blendvals[sel] * 32);
	const int al_b = std::min(255, blendvals[2 + sel] * 32);
	rgb_t rgb1{src};
	rgb_t rgb2{src};
	if (pri_alp.active_alpha) {
		const int al_c = std::min<u8>(255, pri_alp.alpha);
		dst = rgb_t{dst} + (rgb1.scale8(al_b) + rgb2.scale8(al_a)).scale8(al_c).set_a(255);
		// this is a blend dest, don't update prio
	} else {
		dst = (rgb1.scale8(al_b) + rgb2.scale8(al_a)).set_a(255);
		pri_alp.pri = prio;
	}
	pri_alp.active_alpha = 0;
	pri_alp.alpha = 0;
}
void taito_f3_state::blend_d(u8 blend_mode, bool sel, u8 prio, const u8 *blendvals, pri_alpha &pri_alp, u32 &dst, u32 src)
{
	//const int al_a = std::min(255, blendvals[sel] * 32);
	//const int al_b = std::min(255, blendvals[2 + sel] * 32);
	const int al_c = std::min<u8>(255, pri_alp.alpha);
	rgb_t rgb1{src};
	dst = rgb_t{dst} + rgb1.scale8(al_c).set_a(255);
	//rgb_t rgb2{src};
	//dst = rgb_t{dst} + (rgb1.scale8(al_b) + rgb2.scale8(al_a)).scale8(al_c).set_a(255);
	//pri_alp.active_alpha = blend_mode;
	pri_alp.active_alpha = 0; // no more blending !
	pri_alp.alpha = 0; // no more blending !
}

void taito_f3_state::blend_dispatch(u8 blend_mode, bool sel, u8 prio, const u8 *blendvals, pri_alpha &pri_alp, u32 &dst, u32 src)
{
	if (blend_mode == 0b00 || blend_mode == 0b11) { // opaque case
		blend_o(0, sel, prio, blendvals, pri_alp, dst, src);
	} else if (pri_alp.active_alpha) { // alt alpha above us
		blend_d(blend_mode, sel, prio, blendvals, pri_alp, dst, src);
	} else if (blend_mode) { // first alpha
		blend_s(blend_mode, sel, prio, blendvals, pri_alp, dst, src);
	}
}


void taito_f3_state::draw_line(pen_t* dst, f3_line_inf &line, int xs, int xe, playfield_inf* pf)
{
	const pen_t *clut = &m_palette->pen(0);
	const int y_index = ((pf->reg_fx_y >> 8) + pf->colscroll) & 0x1ff;
	const u16 *src = &pf->srcbitmap->pix(y_index);
	const u8 *flags = &pf->flagsbitmap->pix(y_index);

	fixed8 fx_x = pf->reg_sx + pf->rowscroll;
	fx_x += 10*((pf->x_scale)-(1<<8));
	fx_x &= (m_width_mask << 8) | 0xff;

	//logerror("pri/alp: %X %X\n", line.pri_alp[180].pri, line.pri_alp[180].alpha);
	for (int x = xs; x < xe; x++) {
		auto pri_alp = line.pri_alp[x];
		if (pf->prio() > pri_alp.pri
			|| (pri_alp.alpha && (pf->blend_mask() != pri_alp.active_alpha))) {

			int real_x = pf->x_sample_enable ? mosaic(x, 16 - pf->x_sample) : x;
			int x_index = (((fx_x + (real_x - 46) * pf->x_scale)>>8) + 46) & m_width_mask;

			if (!(flags[x_index] & 0xf0))
				continue;
			if (const u16 col = src[x_index]) {
				const bool sel = flags[x_index] & 0x1;
				blend_dispatch(pf->blend_mask(), sel, pf->prio(),
							   line.blend, line.pri_alp[x], dst[x], clut[col]);
			}
		}
	}
}

void taito_f3_state::draw_line(pen_t* dst, f3_line_inf &line, int xs, int xe, sprite_inf* sp)
{
	const pen_t *clut = &m_palette->pen(0);
	const u16 *src = &sp->srcbitmap->pix(line.y);

	const u8 blend_mode = sp->blend_mask();
	//logerror("pri/alp: %X %X\n", line.pri_alp[180].pri, line.pri_alp[180].alpha);	
	for (int x = xs; x < xe; x++) {
		auto pri_alp = line.pri_alp[x];
		if (sp->prio() > line.pri_alp[x].pri
			|| (pri_alp.alpha && (blend_mode != pri_alp.active_alpha))) {

			if (const u16 col = src[x]) { // 0 = transparent
				const bool sel = sp->brightness;
				blend_dispatch(sp->blend_mask(), sel, sp->prio(),
							   line.blend, line.pri_alp[x], dst[x], clut[col]);
			}
		}
	}
}



void taito_f3_state::draw_line(pen_t* dst, f3_line_inf &line, int xs, int xe, pivot_inf* pv)
{
	const pen_t *clut = &m_palette->pen(0);
	const u16 height_mask = pv->use_pix() ? 0xff : 0x1ff;
	const u16 width_mask = 0x1ff;

	const int y_index = (pv->reg_sy + line.y) & height_mask;
	const u16 *srcbitmap = pv->use_pix() ? &pv->srcbitmap_pixel->pix(y_index) : &pv->srcbitmap_vram->pix(y_index);
	const u8 *flagsbitmap = pv->use_pix() ? &pv->flagsbitmap_pixel->pix(y_index) : &pv->flagsbitmap_vram->pix(y_index);

	for (int x = xs; x < xe; x++) {
		auto pri_alp = line.pri_alp[x];
		if (pv->prio() > pri_alp.pri
			|| (pri_alp.alpha && (pv->blend_mask() != pri_alp.active_alpha))) {

			int x_index = (pv->reg_sx + x) & width_mask;
			if (!(flagsbitmap[x_index] & 0xf0))
				continue;
			if (u16 col = srcbitmap[x_index]) {
				const bool sel = false;
				blend_dispatch(pv->blend_mask(), sel, pv->prio(),
							   line.blend, line.pri_alp[x], dst[x], clut[col]);
			}
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
	for (int i=0; i<NUM_SPRITEGROUPS; i++) {
		line_data.sp[i].srcbitmap = &m_sprite_framebuffers[i];
	}
	for (int pf = 0; pf < NUM_PLAYFIELDS; ++pf) {
		int tmap_number = pf + line_data.pf[pf].alt_tilemap * 2;
		line_data.pf[pf].srcbitmap = &m_tilemap[tmap_number]->pixmap();
		line_data.pf[pf].flagsbitmap = &m_tilemap[tmap_number]->flagsmap();

		get_pf_scroll(pf, line_data.pf[pf].reg_sx, line_data.pf[pf].reg_sy);
		line_data.pf[pf].reg_fx_y = line_data.pf[pf].reg_sy;
		line_data.pf[pf].x_scale = (256-0);
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

	auto prio = [](const auto& obj) -> u8 { return obj->prio(); };

	int ys = m_flipscreen ? 24 : 0;
	for (int y = y_start; y != y_end; y += y_inc) {
		read_line_ram(line_data, y);
		line_data.y = y + ys;
		for (auto &pri : line_data.pri_alp) {
			pri.pri = 0;
			pri.active_alpha = 0;
			pri.alpha = 0;
		}

		// sort layers
		std::array<std::variant<pivot_inf*, sprite_inf*, playfield_inf*>,
				   NUM_SPRITEGROUPS + NUM_TILEMAPS> layers = {
			&line_data.sp[3], &line_data.sp[2], &line_data.sp[1], &line_data.sp[0],
			&line_data.pivot,
			&line_data.pf[3], &line_data.pf[2], &line_data.pf[1], &line_data.pf[0]
		};
		std::stable_sort(layers.begin(), layers.end(),
						 [prio](auto a, auto b) {
							 return std::visit(prio, a) > std::visit(prio, b); // was <
						 });

		// draw layers to framebuffer (currently top to bottom)
		for (auto gfx : layers) {
			// bool last = gfx == layers.back();
			std::visit([&](auto&& arg) {
				if (arg->layer_enable()) {
					// if (last)
					// 	arg->mix_value &= 0x3fff; // treat last layer as opaque mode ?
					std::vector<clip_plane_inf> clip_ranges = calc_clip(line_data.clip, arg);
					for (const auto &clip : clip_ranges) {
						draw_line(&bitmap.pix(y+ys), line_data, clip.l, clip.r, arg);
					}
				}
			}, gfx);
		}
		//logerror("-----------\n");

		int dbgx = 46;
		for (auto &pf : line_data.pf) {
			bool bonus_d = false;
			for (int xx = 46; xx < 320+46; xx++) {
				bool temp_bonus = pf.flagsbitmap->pix(y+ys, xx) & 0x1;
				if (temp_bonus != bonus_d) {
					bitmap.pix(y+ys, xx) = temp_bonus ? 0x00FFFF : 0x00FF00;
					bonus_d = temp_bonus;
				}
			}
			bitmap.pix(y+ys, dbgx++) = pf.blend_b() ? 0x0000FF : 0x000000;
			bitmap.pix(y+ys, dbgx++) = pf.blend_a() ? 0xFF0000 : 0x000000;
			bitmap.pix(y+ys, dbgx++) = pf.flagsbitmap->pix(y+ys, 0) & 0x1 ? 0x00FFFF : 0x000000;
			dbgx++;
		}
		for (auto &sp : line_data.sp) {
			bitmap.pix(y+ys, dbgx++) = sp.blend_b() ? 0x0000FF : 0x000000;
			bitmap.pix(y+ys, dbgx++) = sp.blend_a() ? 0xFF0000 : 0x000000;
			bitmap.pix(y+ys, dbgx++) = sp.brightness ? 0x00FFFF : 0x000000;
			dbgx++;
		}
		bitmap.pix(y+ys, dbgx++) = line_data.pivot.blend_b() ? 0x0000FF : 0x000000;
		bitmap.pix(y+ys, dbgx++) = line_data.pivot.blend_a() ? 0xFF0000 : 0x000000;


		if (y != y_start) {
			// update registers
			for (auto &pf : line_data.pf) {
				pf.reg_fx_y += pf.y_scale;
			}
		}
	}
}

/******************************************************************************/

inline void taito_f3_state::f3_drawgfx(const tempsprite &sprite, const rectangle &cliprect)
{
	bitmap_ind16 &dest_bmp = m_sprite_framebuffers[sprite.pri];
	
	gfx_element *gfx = m_gfxdecode->gfx(2);
	const u8 *code_base = gfx->get_data(sprite.code % gfx->elements());

	//logerror("sprite draw at %f %f size %f %f\n", sprite.x/16.0, sprite.y/16.0, sprite.zoomx/16.0, sprite.zoomy/16.0);
	const u8 flipx = sprite.flip_x ? 0xF : 0;
	const u8 flipy = sprite.flip_y ? 0xF : 0;

	fixed8 dy8 = (sprite.y);
	if (!m_flipscreen)
		dy8 += 255; // round up in non-flipscreen mode?    mayybe flipscreen coordinate adjustments should be done after all this math, during final rendering?. anyway:  testcases for vertical scaling: elvactr mission # text (non-flipscreen), kaiserknj attract mode first text line (flipscreen)
	for (u8 y = 0; y < 16; y++) {
		const int dy = dy8 >> 8;
		dy8 += sprite.scale_y;
		if (dy < cliprect.min_y || dy > cliprect.max_y)
			continue;
		u8 *pri = &m_pri_alp_bitmap.pix(dy);
		u16* dest = &dest_bmp.pix(dy);
		auto src = &code_base[(y ^ flipy) * 16];

		fixed8 dx8 = (sprite.x) + 128; // 128 is ½ in fixed.8
		for (u8 x = 0; x < 16; x++) {
			const int dx = dx8 >> 8;
			dx8 += sprite.scale_x;
			// is this necessary with the large margins outside visarea?
			if (dx < cliprect.min_x || dx > cliprect.max_x)
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
		fixed8 block_scale = 1 << 8;
		fixed8 pos = 0, block_pos = 0;
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
					block_pos = new_pos << 8;
					block_scale = (0x100 - new_zoom);
				}
				[[fallthrough]];
			case 0b10:
				pos = block_pos;
				break;
			case 0b11:
				pos += block_scale * 16;
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

	for (int offs = 0; offs < 0x400 && (total_sprites < 0x400); offs++)
	{
		total_sprites++; // prevent infinite loops
		int bank = m_sprite_bank ? 0x4000 : 0;
		const u16 *spr = &spriteram16_ptr[bank + (offs * 8)];

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

		/* Check if the sprite list jump bit is set */
		// we have to check this AFTER processing sprite commands because recalh uses a sprite command and jump in the same sprite
		if (BIT(spr[6], 15))
		{
			const int new_offs = BIT(spr[6], 0, 10);
			if (new_offs == offs) // could this be ≤ ? -- NO! RECALH USES BACKWARDS JUMPS!!
			{
				break; // optimization, edge cases to watch for: looped sprite block commands?
			}
			offs = new_offs - 1; // subtract because we increment in the for loop
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

		const fixed8 tx = m_flipscreen ? (512<<8) - x.block_scale*16 - x.pos : x.pos;
		const fixed8 ty = m_flipscreen ? (256<<8) - y.block_scale*16 - y.pos : y.pos;

		if (tx + x.block_scale*16 <= visarea.min_x<<8 || tx > visarea.max_x<<8 || ty + y.block_scale*16 <= visarea.min_y<<8 || ty > visarea.max_y<<8)
			continue;

		const bool flip_x = BIT(spritecont, 0);
		const bool flip_y = BIT(spritecont, 1);
		//multi = BIT(spritecont, 3);

		sprite_ptr->x = tx;
		sprite_ptr->y = ty;
		sprite_ptr->flip_x = m_flipscreen ? !flip_x : flip_x;
		sprite_ptr->flip_y = m_flipscreen ? !flip_y : flip_y;
		sprite_ptr->code = tile;
		sprite_ptr->color = color;
		sprite_ptr->scale_x = x.block_scale;
		sprite_ptr->scale_y = y.block_scale;
		sprite_ptr->pri = BIT(color, 6, 2);
		sprite_ptr++;
	}
	m_sprite_end = sprite_ptr;
}

void taito_f3_state::draw_sprites(const rectangle &cliprect)
{
	// todo: don't clear these if trails are enabled
	m_pri_alp_bitmap.fill(0);
	for (auto &sp_bitmap : m_sprite_framebuffers) {
		sp_bitmap.fill(0);
	}
	
	rectangle myclip = cliprect;
	myclip &= m_pri_alp_bitmap.cliprect();
	
	for (auto* spr = m_sprite_end; spr-- != &m_spritelist[0]; ) {
		f3_drawgfx(*spr, myclip);
	}
	
	//m_sprite_pri_usage = 0;
	//m_sprite_pri_usage |= 1 << pri; ?? do we still need this?
}

/******************************************************************************/
u32 taito_f3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	logerror("vpos screen update now: %d\n", m_screen->vpos());;

	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(0, cliprect);
	
	/* sprites */
	//if (m_sprite_lag == 0)
	// these are getted during vblank now
	//get_sprite_info(m_spriteram.target());


	/* Update sprite buffer */
	//draw_sprites(bitmap, cliprect);

	/* Draw final framebuffer */

	scanline_draw_TWO(bitmap, cliprect);

	// need to swap these to get more sprite lag sometimes ?
	get_sprite_info(m_spriteram.target());
	// draw sprite layers
	draw_sprites(cliprect);
	
	if (VERBOSE)
		print_debug_info(bitmap);
	return 0;
}
