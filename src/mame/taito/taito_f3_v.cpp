// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, ywy, 12Me21
/***************************************************************************

   Taito F3 Video emulation
   Thanks to original driver and research by Bryan McPhail (2001)
   and additional information from David Haywood and others

   (2024) major rewrite and updates by ywy and 12

****************************************************************************

Brief overview:

    4 scrolling layers (512x512 or 1024x512) of 4, 5, or 6 bpp tiles.
    1 scrolling text layer (512x512, characters generated in vram), 4bpp chars.
    1 scrolling pixel layer (512x256 pixels generated in pivot ram), 4bpp pixels.
    2 sprite banks (for double buffering of sprites)
    Sprites can be 4, 5 or 6 bpp
    Sprite scaling.
    Line by line effects on all playfields
    Line by line control of priority/mixing on sprites and text/pixel layer
    2 of the 4 playfields have (line by line) access to alternate tilemaps

Notes:
    All special effects are controlled by an area in 'line ram'.  Typically
    there are 256 values, one for each line of the screen (including clipped
    lines at the top of the screen).  For example, at 0x8000 in lineram,
    there are 4 sets of 256 values (one for each playfield) and each value
    is the scale control for that line in the destination bitmap (screen).
    Therefore each line can have a different zoom value for special effects.

    This also applies to playfield priority, rowscroll, column scroll, clipping,
    palette interpretation, sprite priority, VRAM/pivot layers, and so on...

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
    In rare cases (e.g. bubblem), the second set of subsections
    is latched on using bits 4,5,6,7 for ..800,a00,c00,e00, respectively.

    0x0000: Line set ram for 4000 (column scroll, alt tilemap, clip) section
        10xx
    0x0200: Line set ram for 5000 (clip planes) section
        14xx
    0x0400: Line set ram for 6000 (blending) section
        18xx
    0x0600: Line set ram for 7000 (pivot and sprite layer mixing) section
        1cxx
    0x0800: Line set ram for 8000 (zoom) section
        20xx
    0x0a00: Line set ram for 9000 (palette add) section
        24xx
    0x0c00: Line set ram for a000 (row scroll) section
        28xx
    0x0e00: Line set ram for b000 (playfield mixing info) section
        2cxx

  "Pivot port" (0x1000-2fff) has only one known used address.
    0x1000: Unknown control word?
        (usually 0x00f0; gseeker, spcinvdj, twinqix, puchicar set 0x0000)


  Line data ram (8 sections, 4 normal and 4 alt subsections of 256 lines each):

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
          Bits: B?p? o?A?
            A = alpha blend value select
            o = ?
              "0x0800 seems to set the vram layer to be opaque [unemulated]"
            p = enable pixel layer
              "0x2000 enables garbage pixels for this line (maybe another pixel bank?) [unemulated]"
            B = pixel bank ?
            ? = unknown
        0x00ff: Bits: DdCc BbAa
            Dd = Alpha mode for sprites with pri value 0xc0
            Cc = Alpha mode for sprites with pri value 0x80
            Bb = Alpha mode for sprites with pri value 0x40
            Aa = Alpha mode for sprites with pri value 0x00

    0x6200: Alpha blend values
        Bits: BBBB AAAA bbbb aaaa
            A = contribution (alpha) value A for SOURCE when alpha mode is not reversed
            B = contribution (alpha) value B for SOURCE when alpha mode is not reversed
            a = contribution (alpha) value A for DEST when alpha mode is not reversed
            b = contribution (alpha) value B for DEST when alpha mode is not reversed

    0x6400: forward repeat and palette
        0x03ff: x repeat / mosaic - each tile collapses to 16 single colour lines
          Bits: ??ps mmmm 4321
            4321 = enable effect for respective playfield
            mmmm = x repeat - 0 = repeat 1st pixel 16 times (sample every 16)
                              1 = repeat 1st pixel 15 times (sample every 15)
                              f = repeat 1st pixel  1 times (sample every pixel)
               s = enable effect for sprites
               p = enable effect for pivot layer
            (spcinvdj title screen, riding fight, command war)

          x4xx = ??? seems to display garbage pixels on the pivot layer (unused?)
          x8xx = ??? seems to affect the palette of a single layer(??) (gunlock)

       0xf000: palette ram format? [unemulated]
         Bits: ?wBu
           ? = ? possibly "interpret palette ram for this as 21-bit palette"
           w = 1 = interpret palette entries as 12-bit RGB
           B = 0 = enable horizontal forward blur (1 = don't blur)
           u = ??? (normally 1)

    0x6600: Background - background palette entry (under all tilemaps) (takes part in alpha blending)

    0x7000: ? [unemulated]
        "Pivot/vram layer enable" ?
    0x7200: VRAM layer mixing info (incl. priority)
        Bits: BAEI cccc iiii pppp  (see 0xb000)
    0x7400: Sprite mixing info (without priority, like playfield priority clip bits but shifted)
        Bits: AAAA ??EI cccc iiii
                   | ^^ ^^^^ ^^^^ line enable/clip/inverse clip (see 0xb000)
                   ^ 0x0800 set by arabianm, bubsymph, bubblem, cleopatr, commandw, cupfinal, gseeker, spcinv95, twinqix, kirameki...
              A = Alpha blend value select for sprites with pri value 0xc0/80/40/00
    0x7600: Sprite priority values
        0xf000: Relative priority for sprites with pri value 0xc0
        0x0f00: Relative priority for sprites with pri value 0x80
        0x00f0: Relative priority for sprites with pri value 0x40
        0x000f: Relative priority for sprites with pri value 0x00

    0x8000: Playfield 1 scale (1 word per line, 256 lines, 0x0080 = default no scale)
    0x8200: Playfield 2/4 scale
    0x8400: Playfield 3 scale
    0x8600: Playfield 2/4 scale
        0x00ff = Y scale (0x80 = no scale, > 0x80 is zoom in, < 0x80 is zoom out [0xc0 is half height of 0x80])
                 source pixels per screen pixel in fixed1.7
        0xff00 = X scale (0 = no scale, > 0 = zoom in, [0x8000 = double length, i.e., 0.5 src step per screen px])
                 1.0 − fixed0.8 source pixels per screen pixel?

        Playfield 2 & 4 registers seem to be interleaved, playfield 2 Y zoom is stored where you would
        expect playfield 4 y zoom to be and vice versa.

    0x9000: Palette add (can affect blend output)
      9200: Playfield 2 palette add
      9400: Playfield 3 palette add
      9600: Playfield 4 palette add

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
          A = Alpha mode A - alpha enable
          B = Alpha mode B - alpha with reversed source/destination contribution

    0xc000 - 0xffff: Unused.

****************************************************************************

    F3 sprite format:

    word 0: [tttt tttt tttt tttt]
     t: lower 16 bits of tile number

    word 1: [yyyy yyyy xxxx xxxx]
     y: y zoom (scale factor is: (256-zoom)/256)
     x: x zoom

    word 2: [iiss xxxx xxxx xxxx]
     i: ignore global/subglobal scroll
     s: set global/subglobal scroll
     x: x position (signed 12 bits)

    word 3: [c.BA yyyy yyyy yyyy]
     c: special command (parameters are in word 5)
     B: ? set by gseeker on a special command (also sets word 3 to FFEE: probably a position overflow again)
     A: ? set by ridingf on ACTUAL SPRITES during gameplay - probably just the position overflowing
     y: y position (signed 12 bits)
     (???) ridingf sets this word to 0x9000 at 0x3710, and 0x9001 at 0xB710

    word 4: [bbbb mlyx cccc cccc]
     b: block position controls
     m: "multi" - set to 0 on the sprite before a new sprite block
     l: "lock" - reuse palette from previous sprite
     y: y flip
     x: x flip
     c: color palette

    word 5: [.... .... .... ...h] (normal sprite)
     h: upper bit of tile number
    word 5: [..fA ..pp ..?? ..tb] (if special command bit set)
     f: enable flipscreen
     A: ??? set by ridingf
     p: enable extra planes (00 = 4bpp, 01 = 5bpp, 11 = 6bpp)
     t: enable sprite trails (don't clear framebuffer)
     b: sprite bank to switch to
     (???) ridingf sets this word to 1000/1001 at 0x3710 and 0xB710

    word 6: [j... ..ii iiii iiii]
     j: jump command if set
     i: index to jump to (0-1023)

    word 7: [.... .... .... ....]
     (unused)

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
    0x0200 0000 - Blend value select
    0x01ff 0000 - Colour


***************************************************************************
    blending seems to work something like:
    Blend values:
     Bits: [BBBB AAAA bbbb aaaa]
     opacity is (15-N)/8, clamped to (0, 1.0)

    each layer (each playfield, each sprite priority group, and pivot):
     1) blend enable bit ("blend mode A" historically)
     2) reverse blend enable bit ("blend mode B" historically)
        always grouped together, usually with other mixing-related bits
        - these select between the a/b and A/B blend value pairs
        - when both 0 or both 1 (sprites), the layer is opaque.
          opaque layers use two contribution values. (e.g. A*x + a*x)

     3) blend value select bit ("blend select")
        per-line for pivot and sprite groups, per-tile for playfields
        this selects between the two sets of blend contribution values.

     a lower priority non-blank pixel with *different* blend mode from source:
       is combined by saturating addition in the blending circuit using
       a contribution amount according to the opposite of the source blend mode,
       and according to its own blend select bit

     -  should only be (up to) 2 contributing layers to each final pixel (?)
     -  there's a HW "feature" (bug?) when layers have a prio conflict.
          in this case, for a given pixel, it seems like the second layer
          can reset part of the state... (dariusg stage V' clouds)

***************************************************************************/

#include "emu.h"
#include "taito_f3.h"

#include <algorithm>
#include <variant>

void taito_f3_state::device_post_load()
{
	/* force a reread of the dynamic tiles in the pixel layer */
	m_fdp->gfx(0)->mark_all_dirty();
	m_fdp->gfx(1)->mark_all_dirty();
}

/******************************************************************************/

void taito_f3_state::screen_vblank(int state)
{
	if (state) {
		//get_sprite_info();
	}
}

void taito_f3_state::video_start()
{
	// Game specific data - some of this can be removed when the software values are figured out
	struct F3config {
		const char *name;
		int extend;     // playfield control 0x1F bit 7
		int sprite_lag;
	};

	F3config f3_config_table[] = {
		/* Name    Extend  Lag */
		{ "ringrage",  0,     2 },
		{ "arabianm",  0,     2 },
		{ "ridingf",   1,     1 },
		{ "gseeker",   0,     1 },
		{ "commandw",  1,     1 },
		{ "cupfinal",  0,     1 },
		{ "trstar",    1,     0 },
		{ "gunlock",   1,     2 },
		{ "scfinals",  0,     1 },
		{ "lightbr",   1,     2 },
		{ "intcup94",  0,     1 },
		{ "kaiserkn",  0,     2 },
		{ "dankuga",   0,     2 },
		{ "dariusg",   0,     2 },
		{ "bublbob2",  1,     1 },
		{ "spacedx",   1,     1 },
		{ "pwrgoal",   0,     1 },
		{ "qtheater",  1,     1 },
		{ "elvactr",   1,     2 },
		{ "recalh",    1,     1 },
		{ "spcinv95",  0,     1 },
		{ "twinqix",   1,     1 },
		{ "quizhuhu",  1,     1 },
		{ "pbobble2",  0,     1 },
		{ "gekiridn",  0,     1 },
		{ "tcobra2",   0,     0 },
		{ "bubblem",   1,     1 },
		{ "cleopatr",  0,     1 },
		{ "pbobble3",  0,     1 },
		{ "arkretrn",  1,     1 },
		{ "kirameki",  0,     1 },
		{ "puchicar",  1,     1 },
		{ "pbobble4",  0,     1 },
		{ "popnpop",   1,     1 },
		{ "landmakr",  1,     1 },
		{ "2mindril",  1,     0 },
		{ nullptr,     1,     1 },
	};

	const game_driver &game = machine().system();
	
	const F3config *config;
	for (config = &f3_config_table[0]; config->name; config++) {
		if (!strcmp(config->name, game.name) || !strcmp(config->name, game.parent))
			break;
	}
	
	m_fdp->create_tilemaps(config->extend);
	
	m_fdp->m_sprite_lag = config->sprite_lag;
}

/******************************************************************************/

void taito_f3_state::palette_24bit_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_paletteram32[offset]);
	const u32 color = m_paletteram32[offset];

	// .... .... .... .... RRRR GGGG BBBB ....
	// .... .... RRRR rrrr GGGG gggg BBBB bbbb
	m_fdp->m_palette_12bit->set_pen_color(offset, rgb_t(BIT(color, 12, 4) * 16, BIT(color, 8, 4) * 16, BIT(color, 4, 4) * 16));
	m_fdp->m_palette->set_pen_color(offset, rgb_t(color).set_a(255));
}

/******************************************************************************/

u32 taito_f3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_fdp->machine().tilemap().set_flip_all(m_fdp->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(0, cliprect);

	// TODO: presumably "sprite lag" is timing of sprite ram/framebuffer access.
	if (m_fdp->m_sprite_lag == 0) {
		m_fdp->read_sprite_info();
		m_fdp->draw_sprites();
		m_fdp->scanline_draw(bitmap, cliprect);
	} else if (m_fdp->m_sprite_lag == 1) {
		m_fdp->scanline_draw(bitmap, cliprect);
		m_fdp->read_sprite_info();
		m_fdp->draw_sprites();
	} else { // 2
		m_fdp->scanline_draw(bitmap, cliprect);
		m_fdp->draw_sprites();
		m_fdp->read_sprite_info();
	}

	return 0;
}
