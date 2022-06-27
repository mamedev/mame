// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Taito F3 Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

Brief overview:

    4 scrolling layers (512x512 or 1024x512) of 4/5/6 bpp tiles.
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
    controlling each effect - effects can be selectively applied to individual
    playfields or only certain lines out of the 256 can be active - in which
    case the last allowed value can be latched (typically used so a game can
    use one zoom or row value over the whole playfield).

    The programmers of some of these games made strange use of flipscreen -
    some games have all their graphics flipped in ROM, and use the flipscreen
    bit to display them correctly!.

    Most games display 232 scanlines, but some use lineram effects to clip
    themselves to 224 or less.

****************************************************************************

Line ram memory map:

    Here 'playfield 1' refers to the first playfield in memory, etc

    0x0000: Column line control ram (256 lines)
        100x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4
    0x0200: Line control ram for 0x5000 section.
    0x0400: Line control ram for 0x6000 section.
        180x:   Where bit 0 of x latches sprite alpha value
                Where bit 1 of x latches tilemap alpha value
    0x0600: Sprite control ram
        1c0x:   Where x enables sprite control word for that line
    0x0800: Zoom line control ram (256 lines)
        200x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4
    0x0a00: Assumed unused.
    0x0c00: Rowscroll line control ram (256 lines)
        280x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4
    0x0e00: Priority line control ram (256 lines)
        2c0x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4

    0x4000: Column scroll & clipping info

    0x5000: Clip plane 0 (low bits)
    0x5200: Clip plane 1 (low bits)
    0x5400: Unused?
    0x5600: Used by quizhuhu (taito logo and ingame text), and landmakr (winning text),
            special clip used in tandem with b*00 bit 11? [unemulated]

    0x6000: Sync register
    0x6004: Sprite alpha control
        0xff00: VRAM/Pixel layer control
            0xa000 enables pixel layer for this line (Disables VRAM layer)
            0x2000 enables garbage pixels for this line (maybe another pixel bank?) [unemulated]
            0x0800 seems to set the vram layer to be opaque [unemulated]
            Effect of other bits is unknown.
        0x00c0: Alpha mode for sprites with pri value 0x00
        0x0030: Alpha mode for sprites with pri value 0x00
        0x000c: Alpha mode for sprites with pri value 0x00
        0x0003: Alpha mode for sprites with pri value 0x00
    0x6200: Alpha blending control

    0x6400 - controls X zoom of tile - each tile collapses to 16 single colour lines
        xxx1 - affects bottom playfield
        xxx2 -
        xxx4 -
        xxx8 - affects top playfield
        xxfx - x zoom - 0 = 1st pixel 16 times
                        1 = 1st pixel 8, then 2nd 8
                        8 = 2 per pixel

        (these effects only known to be used on spcinvdj title screen and riding fight - not emulated)

        1xxx = interpret palette ram for this playfield line as 15 bit and bilinear filter framebuffer(??)
        3xxx = interpret palette ram for this playfield line as 15 bit
        7xxx = interpret palette ram for this playfield line as 24 bit palette
        8xxx = interpret palette ram for this playfield line as 21 bit palette

        (effect not emulated)

    0x6600: Background - background palette entry (under all tilemaps) (takes part in alpha blending)

        (effect not emulated)

    0x7000: Pivot/vram layer enable
    0x7200: Cram layer priority
    0x7400: Sprite clipping
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

    0x9000: Palette add (can affect opacity) [ not emulated ]

    0xa000: Playfield 1 rowscroll (1 word per line, 256 lines)
    0xa200: Playfield 2 rowscroll
    0xa400: Playfield 3 rowscroll
    0xa600: Playfield 4 rowscroll

    0xb000: Playfield 1 priority (1 word per line, 256 lines)
    0xb200: Playfield 2 priority
    0xb400: Playfield 3 priority
    0xb600: Playfield 4 priority
        0xf000 = Blend mode, 0x3000 = Normal, 0x7000 = Alpha A, 0xb000 = Alpha B, others disable line
        0x0800 = Clip line (totally disable line)
        0x0400 = Assumed unused
        0x0200 = If set enable clip plane 1
        0x0100 = If set enable clip plane 0
        0x00c0 = Assumed unused
        0x0020 = Enable clip inverse mode for plane 1
        0x0010 = Enable clip inverse mode for plane 0
        0x000f = Layer priority

    0xc000 - 0xffff: Unused.

    When sprite priority==playfield priority sprite takes precedence (Bubble Symph title)

****************************************************************************

    F3 sprite format:

    Word 0: 0xffff      Tile number (LSB)
    Word 1: 0xff00      X zoom
            0x00ff      Y zoom
    Word 2: 0x03ff      X position
    Word 3: 0x03ff      Y position
    Word 4: 0xf000      Sprite block controls
            0x0800      Sprite block start
            0x0400      Use same colour on this sprite as block start
            0x0200      Y flip
            0x0100      X flip
            0x00ff      Colour
    Word 5: 0xffff      Tile number (MSB), probably only low bits used
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

#define VERBOSE 0
#define DARIUSG_KLUDGE
//#define DEBUG_F3 1


/* Game specific data, some of this can be
removed when the software values are figured out */
struct F3config
{
	int name;
	int extend;
	int sprite_lag;
};

static const F3config f3_config_table[] =
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
/*  u16 *line_ram = m_line_ram;
    int l[16];
    char buf[64*16];
    char *bufptr = buf;

    bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n", m_control_0[0] >> 6, m_control_0[1] >> 6, m_control_0[2] >> 6, m_control_0[3] >> 6);
    bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n", m_control_0[4] >> 7, m_control_0[5] >> 7, m_control_0[6] >> 7, m_control_0[7] >> 7);
    bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n", m_control_1[0], m_control_1[1], m_control_1[2], m_control_1[3]);
    bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n", m_control_1[4], m_control_1[5], m_control_1[6], m_control_1[7]);

    bufptr += sprintf(bufptr,"%04X %04X %04X %04X %04X %04X %04X %04X\n", m_spriteram16_buffered[0], m_spriteram16_buffered[1], m_spriteram16_buffered[2], m_spriteram16_buffered[3], m_spriteram16_buffered[4], m_spriteram16_buffered[5], m_spriteram16_buffered[6], m_spriteram16_buffered[7]);
    bufptr += sprintf(bufptr,"%04X %04X %04X %04X %04X %04X %04X %04X\n", m_spriteram16_buffered[8], m_spriteram16_buffered[9], m_spriteram16_buffered[10], m_spriteram16_buffered[11], m_spriteram16_buffered[12], m_spriteram16_buffered[13], m_spriteram16_buffered[14], m_spriteram16_buffered[15]);
    bufptr += sprintf(bufptr,"%04X %04X %04X %04X %04X %04X %04X %04X\n", m_spriteram16_buffered[16], m_spriteram16_buffered[17], m_spriteram16_buffered[18], m_spriteram16_buffered[19], m_spriteram16_buffered[20], m_spriteram16_buffered[21], m_spriteram16_buffered[22], m_spriteram16_buffered[23]);

    l[0] = line_ram[0x0040*2] & 0xffff;
    l[1] = line_ram[0x00c0*2] & 0xffff;
    l[2] = line_ram[0x0140*2] & 0xffff;
    l[3] = line_ram[0x01c0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Ctr1: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x0240*2] & 0xffff;
    l[1] = line_ram[0x02c0*2] & 0xffff;
    l[2] = line_ram[0x0340*2] & 0xffff;
    l[3] = line_ram[0x03c0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Ctr2: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x2c60*2] & 0xffff;
    l[1] = line_ram[0x2ce0*2] & 0xffff;
    l[2] = line_ram[0x2d60*2] & 0xffff;
    l[3] = line_ram[0x2de0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Pri : %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x2060*2] & 0xffff;
    l[1] = line_ram[0x20e0*2] & 0xffff;
    l[2] = line_ram[0x2160*2] & 0xffff;
    l[3] = line_ram[0x21e0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Zoom: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x2860*2] & 0xffff;
    l[1] = line_ram[0x28e0*2] & 0xffff;
    l[2] = line_ram[0x2960*2] & 0xffff;
    l[3] = line_ram[0x29e0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Line: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x1c60*2] & 0xffff;
    l[1] = line_ram[0x1ce0*2] & 0xffff;
    l[2] = line_ram[0x1d60*2] & 0xffff;
    l[3] = line_ram[0x1de0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Sprt: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x1860*2] & 0xffff;
    l[1] = line_ram[0x18e0*2] & 0xffff;
    l[2] = line_ram[0x1960*2] & 0xffff;
    l[3] = line_ram[0x19e0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Pivt: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x1060*2] & 0xffff;
    l[1] = line_ram[0x10e0*2] & 0xffff;
    l[2] = line_ram[0x1160*2] & 0xffff;
    l[3] = line_ram[0x11e0*2] & 0xffff;
    bufptr += sprintf(bufptr,"Colm: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    l[0] = line_ram[0x1460*2] & 0xffff;
    l[1] = line_ram[0x14e0*2] & 0xffff;
    l[2] = line_ram[0x1560*2] & 0xffff;
    l[3] = line_ram[0x15e0*2] & 0xffff;
    bufptr += sprintf(bufptr,"5000: %04x %04x %04x %04x\n", l[0], l[1], l[2], l[3]);

    machine().ui().draw_text(&machine().render().ui_container(), buf, 60, 40); */
}

/******************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info)
{
	const u32 tile = (m_pf_data[Layer][tile_index * 2 + 0] << 16) | (m_pf_data[Layer][tile_index * 2 + 1] & 0xffff);
	const u8 abtype = (tile >> (16 + 9)) & 1;
	// tiles can be configured to use 4, 5, or 6 bpp data.
	// if tiles use more than 4bpp, the bottom bits of the color code must be masked out.
	// This fixes (at least) the rain in round 6 of Arabian Magic.
	const u8 extra_planes = ((tile >> (16 + 10)) & 3); // 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp

	tileinfo.set(3,
			tile & 0xffff,
			(tile >> 16) & 0x1ff & (~extra_planes),
			TILE_FLIPYX(tile >> 30));
	tileinfo.category =  abtype & 1;      /* alpha blending type */
	tileinfo.pen_mask = (extra_planes << 4) | 0x0f;
}


TILE_GET_INFO_MEMBER(taito_f3_state::get_tile_info_text)
{
	int flags = 0;

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
	int flags = 0;
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

WRITE_LINE_MEMBER(taito_f3_state::screen_vblank)
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

	m_flipscreen = 0;
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
#if 0
	/* DariusGX has an interesting bug at the start of Round D - the clearing of lineram
	(0xa000->0x0xa7ff) overflows into priority RAM (0xb000) and creates garbage priority
	values.  I'm not sure what the real machine would do with these values, and this
	emulation certainly doesn't like it, so I've chosen to catch the bug here, and prevent
	the trashing of priority ram.  If anyone has information on what the real machine does,
	please let me know! */
	/* Update: this doesn't seem to occur anymore, I'll leave this snippet in but commented out.
	 *         fwiw PC=0x1768a0/0x1768a4 is where the game clears lineram in round D, which is a
	 *         move.w Dn, (An,D7.w*2) , a kind of opcode that could've been bugged back then.
	 */
	if (m_game == DARIUSG)
	{
		if (m_f3_skip_this_frame)
			return;
		if (offset == 0xb000 / 2 && data == 0x003f)
		{
			m_f3_skip_this_frame = 1;
			return;
		}
	}
#endif

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

#define SET_ALPHA_LEVEL(d, s)           \
{                                       \
	int level = s;                      \
	if (level == 0) level = -1;         \
	d = level + 1;                      \
}

inline void taito_f3_state::alpha_set_level()
{
//  SET_ALPHA_LEVEL(m_alpha_s_1_1, m_alpha_level_2ad)
	SET_ALPHA_LEVEL(m_alpha_s_1_1, 255 - m_alpha_level_2as)
//  SET_ALPHA_LEVEL(m_alpha_s_1_2, m_alpha_level_2bd)
	SET_ALPHA_LEVEL(m_alpha_s_1_2, 255 - m_alpha_level_2bs)
	SET_ALPHA_LEVEL(m_alpha_s_1_4, m_alpha_level_3ad)
//  SET_ALPHA_LEVEL(m_alpha_s_1_5, m_alpha_level_3ad*m_alpha_level_2ad / 255)
	SET_ALPHA_LEVEL(m_alpha_s_1_5, m_alpha_level_3ad * (255 - m_alpha_level_2as) / 255)
//  SET_ALPHA_LEVEL(m_alpha_s_1_6, m_alpha_level_3ad*m_alpha_level_2bd / 255)
	SET_ALPHA_LEVEL(m_alpha_s_1_6, m_alpha_level_3ad * (255 - m_alpha_level_2bs) / 255)
	SET_ALPHA_LEVEL(m_alpha_s_1_8, m_alpha_level_3bd)
//  SET_ALPHA_LEVEL(m_alpha_s_1_9, m_alpha_level_3bd*m_alpha_level_2ad / 255)
	SET_ALPHA_LEVEL(m_alpha_s_1_9, m_alpha_level_3bd * (255 - m_alpha_level_2as) / 255)
//  SET_ALPHA_LEVEL(m_alpha_s_1_a, m_alpha_level_3bd*m_alpha_level_2bd / 255)
	SET_ALPHA_LEVEL(m_alpha_s_1_a, m_alpha_level_3bd * (255 - m_alpha_level_2bs) / 255)

	SET_ALPHA_LEVEL(m_alpha_s_2a_0, m_alpha_level_2as)
	SET_ALPHA_LEVEL(m_alpha_s_2a_4, m_alpha_level_2as * m_alpha_level_3ad / 255)
	SET_ALPHA_LEVEL(m_alpha_s_2a_8, m_alpha_level_2as * m_alpha_level_3bd / 255)

	SET_ALPHA_LEVEL(m_alpha_s_2b_0, m_alpha_level_2bs)
	SET_ALPHA_LEVEL(m_alpha_s_2b_4, m_alpha_level_2bs * m_alpha_level_3ad / 255)
	SET_ALPHA_LEVEL(m_alpha_s_2b_8, m_alpha_level_2bs * m_alpha_level_3bd / 255)

	SET_ALPHA_LEVEL(m_alpha_s_3a_0, m_alpha_level_3as)
	SET_ALPHA_LEVEL(m_alpha_s_3a_1, m_alpha_level_3as * m_alpha_level_2ad / 255)
	SET_ALPHA_LEVEL(m_alpha_s_3a_2, m_alpha_level_3as * m_alpha_level_2bd / 255)

	SET_ALPHA_LEVEL(m_alpha_s_3b_0, m_alpha_level_3bs)
	SET_ALPHA_LEVEL(m_alpha_s_3b_1, m_alpha_level_3bs * m_alpha_level_2ad / 255)
	SET_ALPHA_LEVEL(m_alpha_s_3b_2, m_alpha_level_3bs * m_alpha_level_2bd / 255)
}
#undef SET_ALPHA_LEVEL

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
	dc[COLOR1] = m_add_sat[dc[COLOR1]][(alphas * sc[COLOR1]) >> 8];
	dc[COLOR2] = m_add_sat[dc[COLOR2]][(alphas * sc[COLOR2]) >> 8];
	dc[COLOR3] = m_add_sat[dc[COLOR3]][(alphas * sc[COLOR3]) >> 8];
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

int taito_f3_state::dpix_1_noalpha(u32 s_pix) { m_dval = s_pix; return 1; }
int taito_f3_state::dpix_ret1(u32 s_pix) { return 1; }
int taito_f3_state::dpix_ret0(u32 s_pix) { return 0; }
int taito_f3_state::dpix_1_1(u32 s_pix) { if (s_pix) alpha_blend_1_1(s_pix); return 1; }
int taito_f3_state::dpix_1_2(u32 s_pix) { if (s_pix) alpha_blend_1_2(s_pix); return 1; }
int taito_f3_state::dpix_1_4(u32 s_pix) { if (s_pix) alpha_blend_1_4(s_pix); return 1; }
int taito_f3_state::dpix_1_5(u32 s_pix) { if (s_pix) alpha_blend_1_5(s_pix); return 1; }
int taito_f3_state::dpix_1_6(u32 s_pix) { if (s_pix) alpha_blend_1_6(s_pix); return 1; }
int taito_f3_state::dpix_1_8(u32 s_pix) { if (s_pix) alpha_blend_1_8(s_pix); return 1; }
int taito_f3_state::dpix_1_9(u32 s_pix) { if (s_pix) alpha_blend_1_9(s_pix); return 1; }
int taito_f3_state::dpix_1_a(u32 s_pix) { if (s_pix) alpha_blend_1_a(s_pix); return 1; }

int taito_f3_state::dpix_2a_0(u32 s_pix)
{
	if (s_pix) alpha_blend_2a_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_2a) { m_pval |= m_pdest_2a; return 0; }
	return 1;
}
int taito_f3_state::dpix_2a_4(u32 s_pix)
{
	if (s_pix) alpha_blend_2a_4(s_pix);
	if (m_pdest_2a) { m_pval |= m_pdest_2a; return 0; }
	return 1;
}
int taito_f3_state::dpix_2a_8(u32 s_pix)
{
	if (s_pix) alpha_blend_2a_8(s_pix);
	if (m_pdest_2a) { m_pval |= m_pdest_2a; return 0; }
	return 1;
}

int taito_f3_state::dpix_3a_0(u32 s_pix)
{
	if (s_pix) alpha_blend_3a_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_3a) { m_pval |= m_pdest_3a; return 0; }
	return 1;
}
int taito_f3_state::dpix_3a_1(u32 s_pix)
{
	if (s_pix) alpha_blend_3a_1(s_pix);
	if (m_pdest_3a) { m_pval |= m_pdest_3a; return 0; }
	return 1;
}
int taito_f3_state::dpix_3a_2(u32 s_pix)
{
	if (s_pix) alpha_blend_3a_2(s_pix);
	if (m_pdest_3a) { m_pval |= m_pdest_3a; return 0; }
	return 1;
}

int taito_f3_state::dpix_2b_0(u32 s_pix)
{
	if (s_pix) alpha_blend_2b_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_2b) { m_pval |= m_pdest_2b; return 0; }
	return 1;
}
int taito_f3_state::dpix_2b_4(u32 s_pix)
{
	if (s_pix) alpha_blend_2b_4(s_pix);
	if (m_pdest_2b) { m_pval |= m_pdest_2b; return 0; }
	return 1;
}
int taito_f3_state::dpix_2b_8(u32 s_pix)
{
	if (s_pix) alpha_blend_2b_8(s_pix);
	if (m_pdest_2b) { m_pval |= m_pdest_2b; return 0; }
	return 1;
}

int taito_f3_state::dpix_3b_0(u32 s_pix)
{
	if (s_pix) alpha_blend_3b_0(s_pix);
	else      m_dval = 0;
	if (m_pdest_3b) { m_pval |= m_pdest_3b; return 0; }
	return 1;
}
int taito_f3_state::dpix_3b_1(u32 s_pix)
{
	if (s_pix) alpha_blend_3b_1(s_pix);
	if (m_pdest_3b) { m_pval |= m_pdest_3b; return 0; }
	return 1;
}
int taito_f3_state::dpix_3b_2(u32 s_pix)
{
	if (s_pix) alpha_blend_3b_2(s_pix);
	if (m_pdest_3b) { m_pval |= m_pdest_3b; return 0; }
	return 1;
}

int taito_f3_state::dpix_2_0(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_2b)      { alpha_blend_2b_0(s_pix); if (m_pdest_2b) m_pval |= m_pdest_2b; else return 1; }
		else if (tr2 == m_tr_2a) { alpha_blend_2a_0(s_pix); if (m_pdest_2a) m_pval |= m_pdest_2a; else return 1; }
	}
	else
	{
		if (tr2 == m_tr_2b)      { m_dval = 0; if (m_pdest_2b) m_pval |= m_pdest_2b; else return 1; }
		else if (tr2 == m_tr_2a) { m_dval = 0; if (m_pdest_2a) m_pval |= m_pdest_2a; else return 1; }
	}
	return 0;
}
int taito_f3_state::dpix_2_4(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_2b)      { alpha_blend_2b_4(s_pix); if (m_pdest_2b) m_pval |= m_pdest_2b; else return 1; }
		else if (tr2 == m_tr_2a) { alpha_blend_2a_4(s_pix); if (m_pdest_2a) m_pval |= m_pdest_2a; else return 1; }
	}
	else
	{
		if (tr2 == m_tr_2b)      { if (m_pdest_2b) m_pval |= m_pdest_2b; else return 1; }
		else if (tr2 == m_tr_2a) { if (m_pdest_2a) m_pval |= m_pdest_2a; else return 1; }
	}
	return 0;
}
int taito_f3_state::dpix_2_8(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_2b)      { alpha_blend_2b_8(s_pix); if (m_pdest_2b) m_pval |= m_pdest_2b; else return 1; }
		else if (tr2 == m_tr_2a) { alpha_blend_2a_8(s_pix); if (m_pdest_2a) m_pval |= m_pdest_2a; else return 1; }
	}
	else
	{
		if (tr2 == m_tr_2b)      { if (m_pdest_2b) m_pval |= m_pdest_2b; else return 1; }
		else if (tr2 == m_tr_2a) { if (m_pdest_2a) m_pval |= m_pdest_2a; else return 1; }
	}
	return 0;
}

int taito_f3_state::dpix_3_0(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_3b)      { alpha_blend_3b_0(s_pix); if (m_pdest_3b) m_pval |= m_pdest_3b; else return 1; }
		else if (tr2 == m_tr_3a) { alpha_blend_3a_0(s_pix); if (m_pdest_3a) m_pval |= m_pdest_3a; else return 1; }
	}
	else
	{
		if (tr2 == m_tr_3b)      { m_dval = 0; if (m_pdest_3b) m_pval |= m_pdest_3b; else return 1; }
		else if (tr2 == m_tr_3a) { m_dval = 0; if (m_pdest_3a) m_pval |= m_pdest_3a; else return 1; }
	}
	return 0;
}
int taito_f3_state::dpix_3_1(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_3b)      { alpha_blend_3b_1(s_pix); if (m_pdest_3b) m_pval |= m_pdest_3b; else return 1; }
		else if (tr2 == m_tr_3a) { alpha_blend_3a_1(s_pix); if (m_pdest_3a) m_pval |= m_pdest_3a; else return 1; }
	}
	else
	{
		if (tr2 == m_tr_3b)      { if (m_pdest_3b) m_pval |= m_pdest_3b; else return 1; }
		else if (tr2 == m_tr_3a) { if (m_pdest_3a) m_pval |= m_pdest_3a; else return 1; }
	}
	return 0;
}
int taito_f3_state::dpix_3_2(u32 s_pix)
{
	const u8 tr2 = m_tval & 1;
	if (s_pix)
	{
		if (tr2 == m_tr_3b)      { alpha_blend_3b_2(s_pix); if (m_pdest_3b) m_pval |= m_pdest_3b; else return 1; }
		else if (tr2 == m_tr_3a) { alpha_blend_3a_2(s_pix); if (m_pdest_3a) m_pval |= m_pdest_3a; else return 1; }
	}
	else
	{
		if (tr2 == m_tr_3b)      { if (m_pdest_3b) m_pval |= m_pdest_3b; else return 1; }
		else if (tr2 == m_tr_3a) { if (m_pdest_3a) m_pval |= m_pdest_3a; else return 1; }
	}
	return 0;
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

	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			m_add_sat[i][j] = std::min(i + j, 255);
}

/******************************************************************************/

#define GET_PIXMAP_POINTER(pf_num) \
{ \
	const f3_playfield_line_inf *line_tmp = line_t[pf_num]; \
	m_src[pf_num] = line_tmp->src[y]; \
	m_src_s[pf_num] = line_tmp->src_s[y]; \
	m_src_e[pf_num] = line_tmp->src_e[y]; \
	m_tsrc[pf_num] = line_tmp->tsrc[y]; \
	m_tsrc_s[pf_num] = line_tmp->tsrc_s[y]; \
	m_x_count[pf_num] = line_tmp->x_count[y]; \
	m_x_zoom[pf_num] = line_tmp->x_zoom[y]; \
	m_clip_al[pf_num] = line_tmp->clip0[y] & 0xffff; \
	m_clip_ar[pf_num] = line_tmp->clip0[y] >> 16; \
	m_clip_bl[pf_num] = line_tmp->clip1[y] & 0xffff; \
	m_clip_br[pf_num] = line_tmp->clip1[y] >> 16; \
}

#define CULC_PIXMAP_POINTER(pf_num) \
{ \
	m_x_count[pf_num] += m_x_zoom[pf_num]; \
	if (m_x_count[pf_num] >> 16) \
	{ \
		m_x_count[pf_num] &= 0xffff; \
		m_src[pf_num]++; \
		m_tsrc[pf_num]++; \
		if (m_src[pf_num] == m_src_e[pf_num]) { m_src[pf_num] = m_src_s[pf_num]; m_tsrc[pf_num] = m_tsrc_s[pf_num]; } \
	} \
}

#define UPDATE_PIXMAP_SP(pf_num) \
	if (cx >= clip_als && cx < clip_ars && !(cx >= clip_bls && cx < clip_brs)) \
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
	if (cx >= m_clip_al[pf_num] && cx < m_clip_ar[pf_num] && !(cx >= m_clip_bl[pf_num] && cx < m_clip_br[pf_num])) \
	{ \
		m_tval = *m_tsrc[pf_num]; \
		if (m_tval & 0xf0) \
			if ((this->*m_dpix_lp[pf_num][m_pval >> 4])(clut[*m_src[pf_num]])) { *dsti = m_dval; break; } \
	}


/*============================================================================*/

inline void taito_f3_state::draw_scanlines(
							bitmap_rgb32 &bitmap, int xsize, s16 *draw_line_num,
							const f3_playfield_line_inf **line_t,
							const int *sprite,
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

			clip_als = m_sa_line_inf[0].sprite_clip0[y] & 0xffff;
			clip_ars = m_sa_line_inf[0].sprite_clip0[y] >> 16;
			clip_bls = m_sa_line_inf[0].sprite_clip1[y] & 0xffff;
			clip_brs = m_sa_line_inf[0].sprite_clip1[y] >> 16;

			int length = xsize;
			u32 *dsti = dsti0;
			u8 *dstp = dstp0;

			switch (skip_layer_num)
			{
				case 0: GET_PIXMAP_POINTER(0) [[fallthrough]];
				case 1: GET_PIXMAP_POINTER(1) [[fallthrough]];
				case 2: GET_PIXMAP_POINTER(2) [[fallthrough]];
				case 3: GET_PIXMAP_POINTER(3) [[fallthrough]];
				case 4: GET_PIXMAP_POINTER(4)
			}

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

				switch (skip_layer_num)
				{
					case 0: CULC_PIXMAP_POINTER(0) [[fallthrough]];
					case 1: CULC_PIXMAP_POINTER(1) [[fallthrough]];
					case 2: CULC_PIXMAP_POINTER(2) [[fallthrough]];
					case 3: CULC_PIXMAP_POINTER(3) [[fallthrough]];
					case 4: CULC_PIXMAP_POINTER(4)
				}
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
#undef GET_PIXMAP_POINTER
#undef CULC_PIXMAP_POINTER

/******************************************************************************/

void taito_f3_state::visible_tile_check(
						f3_playfield_line_inf *line_t,
						int line,
						u32 x_index_fx,u32 y_index,
						u16 *pf_data_n)
{
	u16 *pf_base;

	int alpha_mode = line_t->alpha_mode[line];
	if (!alpha_mode) return;

	const u32 total_elements = m_gfxdecode->gfx(3)->elements();

	int tile_index = x_index_fx >> 16;
	const int tile_num = (((line_t->x_zoom[line] * 320 + (x_index_fx & 0xffff) + 0xffff) >> 16) + (tile_index & 0xf) + 15) >> 4;
	tile_index >>= 4;

	if (m_flipscreen)
	{
		pf_base = pf_data_n + ((31 - (y_index >> 4)) << m_twidth_mask_bit);
		tile_index = (m_twidth_mask - tile_index) - tile_num + 1;
	}
	else pf_base = pf_data_n + ((y_index >> 4) << m_twidth_mask_bit);

	bool trans_all = true;
	bool opaque_all = true;
	int alpha_type = 0;
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

void taito_f3_state::calculate_clip(int y, u16 pri, u32 *clip0, u32 *clip1, int *line_enable)
{
	const f3_spritealpha_line_inf *sa_line_t = &m_sa_line_inf[0];

	switch (pri)
	{
	case 0x0100: /* Clip plane 1 enable */
		{
			if (sa_line_t->clip0_l[y] > sa_line_t->clip0_r[y])
				*line_enable = 0;
			else
				*clip0 = (sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y] << 16);
			*clip1 = 0;
		}
		break;
	case 0x0110: /* Clip plane 1 enable, inverted */
		{
			*clip1 = (sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y] << 16);
			*clip0 = 0x7fff0000;
		}
		break;
	case 0x0200: /* Clip plane 2 enable */
		{
			if (sa_line_t->clip1_l[y] > sa_line_t->clip1_r[y])
				*line_enable = 0;
			else
				*clip0 = (sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y] << 16);
			*clip1 = 0;
		}
		break;
	case 0x0220: /* Clip plane 2 enable, inverted */
		{
			*clip1 = (sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y] << 16);
			*clip0 = 0x7fff0000;
		}
		break;
	case 0x0300: /* Clip plane 1 & 2 enable */
		{
			int clipl = 0, clipr = 0;

			if (sa_line_t->clip1_l[y] > sa_line_t->clip0_l[y])
				clipl = sa_line_t->clip1_l[y];
			else
				clipl = sa_line_t->clip0_l[y];

			if (sa_line_t->clip1_r[y] < sa_line_t->clip0_r[y])
				clipr = sa_line_t->clip1_r[y];
			else
				clipr = sa_line_t->clip0_r[y];

			if (clipl > clipr)
				*line_enable = 0;
			else
				*clip0 = (clipl) | (clipr << 16);
			*clip1 = 0;
		}
		break;
	case 0x0310: /* Clip plane 1 & 2 enable, plane 1 inverted */
		{
			if (sa_line_t->clip1_l[y] > sa_line_t->clip1_r[y])
				line_enable = nullptr;
			else
				*clip0 = (sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y] << 16);

			*clip1 = (sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y] << 16);
		}
		break;
	case 0x0320: /* Clip plane 1 & 2 enable, plane 2 inverted */
		{
			if (sa_line_t->clip0_l[y] > sa_line_t->clip0_r[y])
				line_enable = nullptr;
			else
				*clip0 = (sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y] << 16);

			*clip1 = (sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y] << 16);
		}
		break;
	case 0x0330: /* Clip plane 1 & 2 enable, both inverted */
		{
			int clipl = 0, clipr = 0;

			if (sa_line_t->clip1_l[y] < sa_line_t->clip0_l[y])
				clipl = sa_line_t->clip1_l[y];
			else
				clipl = sa_line_t->clip0_l[y];

			if (sa_line_t->clip1_r[y] > sa_line_t->clip0_r[y])
				clipr = sa_line_t->clip1_r[y];
			else
				clipr = sa_line_t->clip0_r[y];

			if (clipl > clipr)
				*line_enable = 0;
			else
				*clip1 = (clipl) | (clipr << 16);
			*clip0 = 0x7fff0000;
		}
		break;
	default:
		// popmessage("Illegal clip mode");
		break;
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
	int alpha_level = 0;
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
			if (m_line_ram[0x100 + y] & 1)
				clip0_low = (m_line_ram[clip_base_low / 2] >> 0) & 0xffff;
			if (m_line_ram[0x000 + y] & 4)
				clip0_high = (m_line_ram[clip_base_high / 2] >> 0) & 0xffff;
			if (m_line_ram[0x100 + y] & 2)
				clip1_low = (m_line_ram[(clip_base_low + 0x200) / 2] >> 0) & 0xffff;

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
		line_t->clip0_l[y] = ((clip0_low & 0xff) | ((clip0_high & 0x1000) >> 4)) - 47;
		line_t->clip0_r[y] = (((clip0_low & 0xff00) >> 8) | ((clip0_high & 0x2000) >> 5)) - 48;
		line_t->clip1_l[y] = ((clip1_low & 0xff) | ((clip0_high & 0x4000) >> 6)) - 47;
		line_t->clip1_r[y] = (((clip1_low & 0xff00) >> 8) | ((clip0_high & 0x8000) >> 7)) - 48;
		if (line_t->clip0_l[y] < 0) line_t->clip0_l[y] = 0;
		if (line_t->clip0_r[y] < 0) line_t->clip0_r[y] = 0;
		if (line_t->clip1_l[y] < 0) line_t->clip1_l[y] = 0;
		if (line_t->clip1_r[y] < 0) line_t->clip1_r[y] = 0;

		/* Evaluate sprite clipping */
		if (sprite_clip & 0x080)
		{
			line_t->sprite_clip0[y] = 0x7fff7fff;
			line_t->sprite_clip1[y] = 0;
		}
		else if (sprite_clip & 0x33)
		{
			int line_enable = 1;
			calculate_clip(y, ((sprite_clip & 0x33) << 4), &line_t->sprite_clip0[y], &line_t->sprite_clip1[y], &line_enable);
			if (line_enable == 0)
				line_t->sprite_clip0[y] = 0x7fff7fff;
		}
		else
		{
			line_t->sprite_clip0[y] = 0x7fff0000;
			line_t->sprite_clip1[y] = 0;
		}

		spri_base += inc;
		clip_base_low += inc;
		clip_base_high += inc;
		y += y_inc;
	}
}

/* sx and sy are 16.16 fixed point numbers */
void taito_f3_state::get_line_ram_info(tilemap_t *tmap, int sx, int sy, int pos, u16 *pf_data_n)
{
	f3_playfield_line_inf *line_t = &m_pf_line_inf[pos];

	int y_start, y_end, y_inc;
	int line_base, zoom_base, col_base, pri_base, inc;

	int line_enable;
	int colscroll = 0, x_offset = 0, line_zoom = 0;
	u32 _y_zoom[256];
	u16 pri = 0;
	int bit_select = 1 << pos;

	int _colscroll[256];
	u32 _x_offset[256];
	int y_index_fx;

	sx += ((46 << 16));

	if (m_flipscreen)
	{
		line_base = 0xa1fe + (pos << 9);
		zoom_base = 0x81fe;// + (pos << 9);
		col_base = 0x41fe + (pos << 9);
		pri_base = 0xb1fe + (pos << 9);
		inc = -2;
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
		line_base = 0xa000 + (pos << 9);
		zoom_base = 0x8000;// + (pos << 9);
		col_base = 0x4000 + (pos << 9);
		pri_base = 0xb000 + (pos << 9);
		inc = 2;
		y_start = 0;
		y_end = 256;
		y_inc = 1;

		y_index_fx = sy;
	}

	int y = y_start;

	while (y != y_end)
	{
		/* The zoom, column and row values can latch according to control ram */
		{
			if (m_line_ram[0x600 + y] & bit_select)
				x_offset = (m_line_ram[line_base / 2] & 0xffff) << 10;
			if (m_line_ram[0x700 + y] & bit_select)
				pri = m_line_ram[pri_base / 2] & 0xffff;

			// Zoom for playfields 1 & 3 is interleaved, as is the latch select
			switch (pos)
			{
			case 0:
				if (m_line_ram[0x400 + y] & bit_select)
					line_zoom = m_line_ram[(zoom_base + 0x000) / 2] & 0xffff;
				break;
			case 1:
				if (m_line_ram[0x400 + y] & 0x2)
					line_zoom = ((m_line_ram[(zoom_base + 0x200) / 2] & 0xffff) & 0xff00) | (line_zoom & 0x00ff);
				if (m_line_ram[0x400 + y] & 0x8)
					line_zoom = ((m_line_ram[(zoom_base + 0x600) / 2] & 0xffff) & 0x00ff) | (line_zoom & 0xff00);
				break;
			case 2:
				if (m_line_ram[0x400 + y] & bit_select)
					line_zoom = m_line_ram[(zoom_base + 0x400) / 2] & 0xffff;
				break;
			case 3:
				if (m_line_ram[0x400 + y] & 0x8)
					line_zoom = ((m_line_ram[(zoom_base + 0x600) / 2] & 0xffff) & 0xff00) | (line_zoom & 0x00ff);
				if (m_line_ram[0x400 + y] & 0x2)
					line_zoom = ((m_line_ram[(zoom_base + 0x200) / 2] & 0xffff) & 0x00ff) | (line_zoom & 0xff00);
				break;
			default:
				break;
			}

			// Column scroll only affects playfields 2 & 3
			if (pos >= 2 && m_line_ram[0x000 + y] & bit_select)
				colscroll = (m_line_ram[col_base / 2] >> 0) & 0x3ff;
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

		_colscroll[y] = colscroll;
		_x_offset[y] = (x_offset & 0xffff0000) - (x_offset & 0x0000ffff);
		_y_zoom[y] = (line_zoom & 0xff) << 9;

		/* Evaluate clipping */
		if (pri & 0x0800)
			line_enable = 0;
		else if (pri & 0x0330)
		{
			//fast path todo - remove line enable
			calculate_clip(y, pri & 0x0330, &line_t->clip0[y], &line_t->clip1[y], &line_enable);
		}
		else
		{
			/* No clipping */
			line_t->clip0[y] = 0x7fff0000;
			line_t->clip1[y] = 0;
		}

		line_t->x_zoom[y] = 0x10000 - (line_zoom & 0xff00);
		line_t->alpha_mode[y] = line_enable;
		line_t->pri[y] = pri;

		zoom_base += inc;
		line_base += inc;
		col_base += inc;
		pri_base += inc;
		y += y_inc;
	}

	tilemap_t* tm = tmap;

	y = y_start;
	while (y != y_end)
	{
		u32 x_index_fx;
		u32 y_index;

		/* The football games use values in the range 0x200 - 0x3ff where the crowd should be drawn - !?

		   This appears to cause it to reference outside of the normal tilemap RAM area into the unused
		   area on the 32x32 tilemap configuration.. but exactly how isn't understood

		    Until this is understood we're creating additional tilemaps for the otherwise unused area of
		    RAM and forcing it to look up there.

		    the crowd area still seems to 'lag' behind the pitch area however.. but these are the values
		    in ram??
		*/
		const int cs = _colscroll[y];

		if (cs & 0x200)
		{
			if (m_tilemap[4] && m_tilemap[5])
			{
				if (tmap == m_tilemap[2]) tmap = m_tilemap[4]; // pitch -> crowd
				if (tmap == m_tilemap[3]) tmap = m_tilemap[5]; // corruption on goals -> blank (hthero94)
			}
		}
		else
		{
			tmap = tm;
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
			if (line_t->clip0[y] != 0x7fff0000 || line_t->clip1[y] != 0)
				line_t->alpha_mode[y] &= ~0x80;

			/* set pixmap index */
			line_t->x_count[y]=x_index_fx & 0xffff; // Fractional part
			line_t->src_s[y] = src_s = &srcbitmap.pix(y_index);
			line_t->src_e[y] = &src_s[m_width_mask + 1];
			line_t->src[y] = &src_s[x_index_fx >> 16];

			line_t->tsrc_s[y]=tsrc_s = &flagsbitmap.pix(y_index);
			line_t->tsrc[y] = &tsrc_s[x_index_fx >> 16];
		}

		y_index_fx += _y_zoom[y];
		y += y_inc;
	}
}

void taito_f3_state::get_vram_info(tilemap_t *vram_tilemap, tilemap_t *pixel_tilemap, int sx, int sy)
{
	const f3_spritealpha_line_inf *sprite_alpha_line_t = &m_sa_line_inf[0];
	f3_playfield_line_inf *line_t = &m_pf_line_inf[4];

	int y_start, y_end, y_inc;
	int pri_base, inc;

	int line_enable;

	u16 pri = 0;

	const int vram_width_mask = 0x3ff;

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
		if (pri & 0x0800)
			line_enable = 0;
		else if (pri & 0x0330)
		{
			//fast path todo - remove line enable
			calculate_clip(y, pri & 0x0330, &line_t->clip0[y], &line_t->clip1[y], &line_enable);
		}
		else
		{
			/* No clipping */
			line_t->clip0[y] = 0x7fff0000;
			line_t->clip1[y] = 0;
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
	u8 draw_line[256];
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
	std::fill(std::begin(draw_line), std::end(draw_line), 0);

	while (1)
	{
		int pos;
		int pri[5],alpha_mode[5],alpha_mode_flag[5];
		u8 sprite_alpha_check;
		u8 sprite_alpha_all_2a;
		int layer_tmp[5];
		f3_playfield_line_inf *pf_line_inf = m_pf_line_inf.get();
		f3_spritealpha_line_inf *sa_line_inf = m_sa_line_inf.get();
		int count_skip_layer = 0;
		int sprite[6] = {0, 0, 0, 0, 0, 0};
		const f3_playfield_line_inf *line_t[5];

		/* find same status of scanlines */
		pri[0] = pf_line_inf[0].pri[y_start];
		pri[1] = pf_line_inf[1].pri[y_start];
		pri[2] = pf_line_inf[2].pri[y_start];
		pri[3] = pf_line_inf[3].pri[y_start];
		pri[4] = pf_line_inf[4].pri[y_start];
		alpha_mode[0] = pf_line_inf[0].alpha_mode[y_start];
		alpha_mode[1] = pf_line_inf[1].alpha_mode[y_start];
		alpha_mode[2] = pf_line_inf[2].alpha_mode[y_start];
		alpha_mode[3] = pf_line_inf[3].alpha_mode[y_start];
		alpha_mode[4] = pf_line_inf[4].alpha_mode[y_start];
		const int alpha_level = sa_line_inf[0].alpha_level[y_start];
		const int spri = sa_line_inf[0].spri[y_start];
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
				int a = alpha_level;
				const int b = (a >> 8) & 0xf;
				const int c = (a >> 4) & 0xf;
				const int d = (a >> 0) & 0xf;
				a >>= 12;

				/* b000 7000 */
				int al_s = std::min(255, ((15 - d) * 256) / 8);
				int al_d = std::min(255, ((15 - b) * 256) / 8);
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
			sprite_alpha_all_2a=1;
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
				const int alpha_type = (alpha_mode_flag[i] >> 4) & 3;

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
				int alpha_type = (alpha_mode_flag[0] | alpha_mode_flag[1] | alpha_mode_flag[2] | alpha_mode_flag[3]) & 0x30;
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
		{
			int pri_max_opa = -1;
			for (i = 0; i < 5; i++)    /* i = playfield num (pos) */
			{
				const int p0 = pri[i];
				const int pri_sl1 = p0 & 0x0f;

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
		for (i = 0; i < 4; i++)
		{
			for (int j = i + 1; j < 5; j++)
			{
				if (layer_tmp[i] < layer_tmp[j])
				{
					const int temp = layer_tmp[i];
					layer_tmp[i] = layer_tmp[j];
					layer_tmp[j] = temp;
				}
			}
		}

		/* check sprite & layer priority */
		{
			int pri_sp[5];

			const int l0 = layer_tmp[0] >> 3;
			const int l1 = layer_tmp[1] >> 3;
			const int l2 = layer_tmp[2] >> 3;
			const int l3 = layer_tmp[3] >> 3;
			const int l4 = layer_tmp[4] >> 3;

			pri_sp[0] =  spri & 0xf;
			pri_sp[1] = (spri >> 4) & 0xf;
			pri_sp[2] = (spri >> 8) & 0xf;
			pri_sp[3] =  spri >> 12;

			for (i = 0; i < 4; i++)    /* i = sprite priority offset */
			{
				const int sflg = 1 << i;
				if (!(m_sprite_pri_usage & sflg)) continue;
				int sp = pri_sp[i];

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
			pos = layer_tmp[i] & 7;
			line_t[i] = &pf_line_inf[pos];

			if (sprite[i] & sprite_alpha_check) alpha = true;
			else if (!alpha) sprite[i] |= 0x100;

			if (alpha_mode[pos] > 1)
			{
				int alpha_type = (((alpha_mode_flag[pos] >> 4) & 3) - 1) * 2;
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

#define PSET_T                  \
	c = *source & m_sprite_pen_mask; \
	if (c)                       \
	{                           \
		p = *pri;                 \
		if (!p || p == 0xff)       \
		{                       \
			*dest = pal[c];     \
			*pri = pri_dst;     \
		}                       \
	}

#define PSET_O                  \
	p = *pri;                     \
	if (!p || p == 0xff)           \
	{                           \
		*dest = pal[*source & m_sprite_pen_mask];    \
		*pri = pri_dst;         \
	}

#define NEXT_P                  \
	source += dx;               \
	dest++;                     \
	pri++;

inline void taito_f3_state::f3_drawgfx(bitmap_rgb32 &dest_bmp, const rectangle &clip,
		gfx_element *gfx,
		int code,
		int color,
		int flipx, int flipy,
		int sx, int sy,
		u8 pri_dst)
{
	rectangle myclip;

	pri_dst = 1 << pri_dst;

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();

	if (gfx)
	{
		const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
		const u8 *code_base = gfx->get_data(code % gfx->elements());

		{
			/* compute sprite increment per screen pixel */
			int dx = 1;
			int dy = 1;

			int ex = sx + 16;
			int ey = sy + 16;

			int x_index_base;
			int y_index;

			if (flipx)
			{
				x_index_base = 15;
				dx = -1;
			}
			else
			{
				x_index_base = 0;
			}

			if (flipy)
			{
				y_index = 15;
				dy = -1;
			}
			else
			{
				y_index = 0;
			}

			if (sx < myclip.min_x)
			{ /* clip left */
				int pixels = myclip.min_x - sx;
				sx += pixels;
				x_index_base += pixels * dx;
			}
			if (sy < myclip.min_y)
			{ /* clip top */
				int pixels = myclip.min_y - sy;
				sy += pixels;
				y_index += pixels * dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if (ex > myclip.max_x + 1)
			{ /* clip right */
				int pixels = ex - myclip.max_x - 1;
				ex -= pixels;
			}
			if (ey > myclip.max_y + 1)
			{ /* clip bottom */
				int pixels = ey - myclip.max_y - 1;
				ey -= pixels;
			}

			if (ex > sx && ey > sy)
			{ /* skip if inner loop doesn't draw anything */
//              if (dest_bmp.bpp == 32)
				{
					int y = ey - sy;
					const int x = (ex - sx - 1) | (m_tile_opaque_sp[code % gfx->elements()] << 4);
					const u8 *source0 = code_base + y_index * 16 + x_index_base;
					u32 *dest0 = &dest_bmp.pix(sy, sx);
					u8 *pri0 = &m_pri_alp_bitmap.pix(sy, sx);
					const int yadv = dest_bmp.rowpixels();
					const int yadvp = m_pri_alp_bitmap.rowpixels();
					dy = dy * 16;
					while (1)
					{
						const u8 *source = source0;
						u32 *dest = dest0;
						u8 *pri = pri0;

						switch (x)
						{
							int c;
							u8 p;
							case 31: PSET_O NEXT_P [[fallthrough]];
							case 30: PSET_O NEXT_P [[fallthrough]];
							case 29: PSET_O NEXT_P [[fallthrough]];
							case 28: PSET_O NEXT_P [[fallthrough]];
							case 27: PSET_O NEXT_P [[fallthrough]];
							case 26: PSET_O NEXT_P [[fallthrough]];
							case 25: PSET_O NEXT_P [[fallthrough]];
							case 24: PSET_O NEXT_P [[fallthrough]];
							case 23: PSET_O NEXT_P [[fallthrough]];
							case 22: PSET_O NEXT_P [[fallthrough]];
							case 21: PSET_O NEXT_P [[fallthrough]];
							case 20: PSET_O NEXT_P [[fallthrough]];
							case 19: PSET_O NEXT_P [[fallthrough]];
							case 18: PSET_O NEXT_P [[fallthrough]];
							case 17: PSET_O NEXT_P [[fallthrough]];
							case 16: PSET_O break;

							case 15: PSET_T NEXT_P [[fallthrough]];
							case 14: PSET_T NEXT_P [[fallthrough]];
							case 13: PSET_T NEXT_P [[fallthrough]];
							case 12: PSET_T NEXT_P [[fallthrough]];
							case 11: PSET_T NEXT_P [[fallthrough]];
							case 10: PSET_T NEXT_P [[fallthrough]];
							case  9: PSET_T NEXT_P [[fallthrough]];
							case  8: PSET_T NEXT_P [[fallthrough]];
							case  7: PSET_T NEXT_P [[fallthrough]];
							case  6: PSET_T NEXT_P [[fallthrough]];
							case  5: PSET_T NEXT_P [[fallthrough]];
							case  4: PSET_T NEXT_P [[fallthrough]];
							case  3: PSET_T NEXT_P [[fallthrough]];
							case  2: PSET_T NEXT_P [[fallthrough]];
							case  1: PSET_T NEXT_P [[fallthrough]];
							case  0: PSET_T
						}

						if (!(--y)) break;
						source0 += dy;
						dest0 += yadv;
						pri0 += yadvp;
					}
				}
			}
		}
	}
}
#undef PSET_T
#undef PSET_O
#undef NEXT_P


inline void taito_f3_state::f3_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip,
		gfx_element *gfx,
		int code,
		int color,
		int flipx, int flipy,
		int sx, int sy,
		int scalex, int scaley,
		u8 pri_dst)
{
	rectangle myclip;

	pri_dst = 1 << pri_dst;

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();

	if (gfx)
	{
		const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
		const u8 *code_base = gfx->get_data(code % gfx->elements());

		{
			/* compute sprite increment per screen pixel */
			int dx = (16 << 16) / scalex;
			int dy = (16 << 16) / scaley;

			int ex = sx + scalex;
			int ey = sy + scaley;

			int x_index_base;
			int y_index;

			if (flipx)
			{
				x_index_base = (scalex - 1) * dx;
				dx = -dx;
			}
			else
			{
				x_index_base = 0;
			}

			if (flipy)
			{
				y_index = (scaley - 1) * dy;
				dy = -dy;
			}
			else
			{
				y_index = 0;
			}

			if (sx < myclip.min_x)
			{ /* clip left */
				int pixels = myclip.min_x - sx;
				sx += pixels;
				x_index_base += pixels * dx;
			}
			if (sy < myclip.min_y)
			{ /* clip top */
				int pixels = myclip.min_y - sy;
				sy += pixels;
				y_index += pixels * dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if (ex > myclip.max_x + 1)
			{ /* clip right */
				int pixels = ex - myclip.max_x - 1;
				ex -= pixels;
			}
			if (ey > myclip.max_y + 1)
			{ /* clip bottom */
				int pixels = ey - myclip.max_y - 1;
				ey -= pixels;
			}

			if (ex > sx)
			{ /* skip if inner loop doesn't draw anything */
//              if (dest_bmp.bpp == 32)
				{
					for (int y = sy; y < ey; y++)
					{
						const u8 *source = code_base + (y_index >> 16) * 16;
						u32 *dest = &dest_bmp.pix(y);
						u8 *pri = &m_pri_alp_bitmap.pix(y);

						int x_index = x_index_base;
						for (int x = sx; x < ex; x++)
						{
							int c = source[x_index >> 16] & m_sprite_pen_mask;
							if (c)
							{
								const u8 p = pri[x];
								if (p == 0 || p == 0xff)
								{
									dest[x] = pal[c];
									pri[x] = pri_dst;
								}
							}
							x_index += dx;
						}
						y_index += dy;
					}
				}
			}
		}
	}
}


#define CALC_ZOOM(p)    {                                       \
	p##_addition = 0x100 - block_zoom_##p + p##_addition_left;  \
	p##_addition_left = p##_addition & 0xf;                     \
	p##_addition = p##_addition >> 4;                           \
	/*zoom##p = p##_addition << 12; */                           \
}

void taito_f3_state::get_sprite_info(const u16 *spriteram16_ptr)
{
	const rectangle &visarea = m_screen->visible_area();
	const int min_x = visarea.min_x, max_x = visarea.max_x;
	const int min_y = visarea.min_y, max_y = visarea.max_y;
	int global_x = 0,global_y = 0, subglobal_x = 0, subglobal_y = 0;
	int block_x = 0, block_y = 0;
	int last_color = 0, last_x = 0, last_y = 0,block_zoom_x = 0,block_zoom_y = 0;
	int this_x, this_y;
	int y_addition = 16, x_addition = 16;
	int multi = 0;

	int x_addition_left = 8, y_addition_left = 8;

	tempsprite *sprite_ptr = &m_spritelist[0];

	int total_sprites = 0;

	int color = 0;
	int flipx = 0, flipy = 0;
	//int old_x = 0;
	int y = 0, x = 0;

	int sprite_top = 0x2000;
	for (int offs = 0; offs < sprite_top && (total_sprites < 0x400); offs += 8)
	{
		const int current_offs = offs; /* Offs can change during loop, current_offs cannot */

		/* Check if the sprite list jump command bit is set */
		if ((spriteram16_ptr[current_offs + 6 + 0]) & 0x8000)
		{
			const u32 jump = (spriteram16_ptr[current_offs + 6 + 0]) & 0x3ff;

			const u32 new_offs = ((offs & 0x4000) | ((jump << 4) / 2));
			if (new_offs == offs)
				break;
			offs = new_offs - 8;
		}

		/* Check if special command bit is set */
		if (spriteram16_ptr[current_offs + 2 + 1] & 0x8000)
		{
			const u32 cntrl = (spriteram16_ptr[current_offs + 4 + 1]) & 0xffff;
			m_flipscreen = cntrl & 0x2000;

			/*  cntrl & 0x1000 = disabled?  (From F2 driver, doesn't seem used anywhere)
			    cntrl & 0x0010 = ???
			    cntrl & 0x0020 = ???
			    cntrl & 0x0002 = enabled when Darius Gaiden sprite trail effect should occur (MT #1922)
			                     Notice that sprites also completely disappear due of a bug/missing feature in the
			             alpha routines.
			*/

			m_sprite_extra_planes = (cntrl & 0x0300) >> 8;   // 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp
			m_sprite_pen_mask = (m_sprite_extra_planes << 4) | 0x0f;

			/* Sprite bank select */
			if (cntrl & 1)
			{
				offs = offs | 0x4000;
				sprite_top = sprite_top | 0x4000;
			}
		}

		/* Set global sprite scroll */
		if (((spriteram16_ptr[current_offs + 2 + 0]) & 0xf000) == 0xa000)
		{
			global_x = (spriteram16_ptr[current_offs + 2 + 0]) & 0xfff;
			if (global_x >= 0x800) global_x -= 0x1000;
			global_y = spriteram16_ptr[current_offs + 2 + 1] & 0xfff;
			if (global_y >= 0x800) global_y -= 0x1000;
		}

		/* And sub-global sprite scroll */
		if (((spriteram16_ptr[current_offs + 2 + 0]) & 0xf000) == 0x5000)
		{
			subglobal_x = (spriteram16_ptr[current_offs + 2 + 0]) & 0xfff;
			if (subglobal_x >= 0x800) subglobal_x -= 0x1000;
			subglobal_y = spriteram16_ptr[current_offs + 2 + 1] & 0xfff;
			if (subglobal_y >= 0x800) subglobal_y -= 0x1000;
		}

		if (((spriteram16_ptr[current_offs + 2 + 0]) & 0xf000) == 0xb000)
		{
			subglobal_x = (spriteram16_ptr[current_offs + 2 + 0]) & 0xfff;
			if (subglobal_x >= 0x800) subglobal_x -= 0x1000;
			subglobal_y = spriteram16_ptr[current_offs + 2 + 1] & 0xfff;
			if (subglobal_y >= 0x800) subglobal_y -= 0x1000;
			global_y = subglobal_y;
			global_x = subglobal_x;
		}

		/* A real sprite to process! */
		const int sprite = (spriteram16_ptr[current_offs + 0 + 0]) | ((spriteram16_ptr[current_offs + 4 + 1] & 1) << 16);
		const int spritecont = spriteram16_ptr[current_offs + 4 + 0] >> 8;

/* These games either don't set the XY control bits properly (68020 bug?), or
    have some different mode from the others */
#ifdef DARIUSG_KLUDGE
		if (m_game == DARIUSG || m_game == GEKIRIDO || m_game == CLEOPATR || m_game == RECALH)
			multi = spritecont & 0xf0;
#endif

		/* Check if this sprite is part of a continued block */
		if (multi)
		{
			/* Bit 0x4 is 'use previous colour' for this block part */
			if (spritecont & 0x4) color=last_color;
			else color = (spriteram16_ptr[current_offs + 4 + 0]) & 0xff;

#ifdef DARIUSG_KLUDGE
			if (m_game == DARIUSG || m_game == GEKIRIDO || m_game == CLEOPATR || m_game == RECALH)
			{
				/* Adjust X Position */
				if ((spritecont & 0x40) == 0)
				{
					if (spritecont & 0x4)
						x = block_x;
					else
					{
						this_x = spriteram16_ptr[current_offs + 2 + 0];
						if (this_x & 0x800) this_x = 0 - (0x800 - (this_x & 0x7ff)); else this_x &= 0x7ff;

						if ((spriteram16_ptr[current_offs + 2 + 0]) & 0x8000)
							this_x += 0;
						else if ((spriteram16_ptr[current_offs + 2 + 0]) & 0x4000) /* Ignore subglobal (but apply global) */
							this_x += global_x;
						else /* Apply both scroll offsets */
							this_x += global_x + subglobal_x;

						x = block_x = this_x;
					}
					x_addition_left = 8;
					CALC_ZOOM(x)
				}
				else if ((spritecont & 0x80) != 0)
				{
					x = last_x + x_addition;
					CALC_ZOOM(x)
				}

				/* Adjust Y Position */
				if ((spritecont & 0x10) == 0)
				{
					if (spritecont & 0x4)
						y = block_y;
					else
					{
						this_y = spriteram16_ptr[current_offs + 2 + 1] & 0xffff;
						if (this_y & 0x800) this_y = 0 - (0x800 - (this_y & 0x7ff)); else this_y &= 0x7ff;

						if ((spriteram16_ptr[current_offs + 2 + 0]) & 0x8000)
							this_y += 0;
						else if ((spriteram16_ptr[current_offs + 2 + 0]) & 0x4000) /* Ignore subglobal (but apply global) */
							this_y += global_y;
						else /* Apply both scroll offsets */
							this_y += global_y + subglobal_y;

						y = block_y = this_y;
					}
					y_addition_left = 8;
					CALC_ZOOM(y)
				}
				else if ((spritecont & 0x20) != 0)
				{
					y = last_y + y_addition;
					CALC_ZOOM(y)
				}
			}
			else
#endif
			{
				/* Adjust X Position */
				if ((spritecont & 0x40) == 0)
				{
					x = block_x;
					x_addition_left = 8;
					CALC_ZOOM(x)
				}
				else if ((spritecont & 0x80) != 0)
				{
					x = last_x + x_addition;
					CALC_ZOOM(x)
				}
				/* Adjust Y Position */
				if ((spritecont & 0x10) == 0)
				{
					y = block_y;
					y_addition_left = 8;
					CALC_ZOOM(y)
				}
				else if ((spritecont & 0x20) != 0)
				{
					y = last_y + y_addition;
					CALC_ZOOM(y)
				}
				/* Both zero = reread block latch? */
			}
		}
		/* Else this sprite is the possible start of a block */
		else
		{
			color = (spriteram16_ptr[current_offs + 4 + 0]) & 0xff;
			last_color = color;

			/* Sprite positioning */
			this_y = spriteram16_ptr[current_offs + 2 + 1] & 0xffff;
			this_x = spriteram16_ptr[current_offs + 2 + 0] & 0xffff;
			if (this_y & 0x800) this_y = 0 - (0x800 - (this_y & 0x7ff)); else this_y &= 0x7ff;
			if (this_x & 0x800) this_x = 0 - (0x800 - (this_x & 0x7ff)); else this_x &= 0x7ff;

			/* Ignore both scroll offsets for this block */
			if ((spriteram16_ptr[current_offs + 2 + 0]) & 0x8000)
			{
				this_x += 0;
				this_y += 0;
			}
			else if ((spriteram16_ptr[current_offs + 2 + 0]) & 0x4000) /* Ignore subglobal (but apply global) */
			{
				this_x += global_x;
				this_y += global_y;
			}
			else /* Apply both scroll offsets */
			{
				this_x += global_x + subglobal_x;
				this_y += global_y + subglobal_y;
			}

			block_y = y = this_y;
			block_x = x = this_x;

			block_zoom_x = spriteram16_ptr[current_offs + 0 + 1];
			block_zoom_y = (block_zoom_x >> 8) & 0xff;
			block_zoom_x &= 0xff;

			x_addition_left = 8;
			CALC_ZOOM(x)

			y_addition_left = 8;
			CALC_ZOOM(y)
		}

		/* These features are common to sprite and block parts */
		flipx = spritecont & 0x1;
		flipy = spritecont & 0x2;
		multi = spritecont & 0x8;
		last_x=x;
		last_y = y;

		if (!sprite) continue;
		if (!x_addition || !y_addition) continue;

		if (m_flipscreen)
		{
			const int tx = 512 - x_addition - x;
			const int ty = 256 - y_addition - y;

			if (tx + x_addition <= min_x || tx > max_x || ty + y_addition <= min_y || ty > max_y) continue;
			sprite_ptr->x = tx;
			sprite_ptr->y = ty;
			sprite_ptr->flipx = !flipx;
			sprite_ptr->flipy = !flipy;
		}
		else
		{
			if (x + x_addition <= min_x || x > max_x || y + y_addition <= min_y || y > max_y) continue;
			sprite_ptr->x = x;
			sprite_ptr->y = y;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
		}

		sprite_ptr->code = sprite;
		sprite_ptr->color = color;
		sprite_ptr->zoomx = x_addition;
		sprite_ptr->zoomy = y_addition;
		sprite_ptr->pri = (color & 0xc0) >> 6;
		sprite_ptr++;
		total_sprites++;
	}
	m_sprite_end = sprite_ptr;
}
#undef CALC_ZOOM


void taito_f3_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const tempsprite *sprite_ptr;
	gfx_element *sprite_gfx = m_gfxdecode->gfx(2);

	sprite_ptr = m_sprite_end;
	m_sprite_pri_usage = 0;

	// if sprites use more than 4bpp, the bottom bits of the color code must be masked out.
	// This fixes (at least) stage 1 battle ships and attract mode explosions in Ray Force.

	while (sprite_ptr != &m_spritelist[0])
	{
		sprite_ptr--;

		const int pri = sprite_ptr->pri;
		m_sprite_pri_usage |= 1 << pri;

		if (sprite_ptr->zoomx == 16 && sprite_ptr->zoomy == 16)
			f3_drawgfx(
					bitmap, cliprect, sprite_gfx,
					sprite_ptr->code,
					sprite_ptr->color & (~m_sprite_extra_planes),
					sprite_ptr->flipx, sprite_ptr->flipy,
					sprite_ptr->x, sprite_ptr->y,
					pri);
		else
			f3_drawgfxzoom(
					bitmap, cliprect, sprite_gfx,
					sprite_ptr->code,
					sprite_ptr->color & (~m_sprite_extra_planes),
					sprite_ptr->flipx, sprite_ptr->flipy,
					sprite_ptr->x, sprite_ptr->y,
					sprite_ptr->zoomx, sprite_ptr->zoomy,
					pri);
	}
}

/******************************************************************************/

u32 taito_f3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u32 sy_fix[5], sx_fix[5];

	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Setup scroll */
	sy_fix[0] = ((m_control_0[4] & 0xffff) <<  9) + (1 << 16);
	sy_fix[1] = ((m_control_0[5] & 0xffff) <<  9) + (1 << 16);
	sy_fix[2] = ((m_control_0[6] & 0xffff) <<  9) + (1 << 16);
	sy_fix[3] = ((m_control_0[7] & 0xffff) <<  9) + (1 << 16);
	sx_fix[0] = ((m_control_0[0] & 0xffc0) << 10) - (6 << 16);
	sx_fix[1] = ((m_control_0[1] & 0xffc0) << 10) - (10 << 16);
	sx_fix[2] = ((m_control_0[2] & 0xffc0) << 10) - (14 << 16);
	sx_fix[3] = ((m_control_0[3] & 0xffc0) << 10) - (18 << 16);
	sx_fix[4] = -(m_control_1[4]) + 41;
	sy_fix[4] = -(m_control_1[5] & 0x1ff);

	sx_fix[0]-=((m_control_0[0] & 0x003f) << 10) + 0x0400 - 0x10000;
	sx_fix[1]-=((m_control_0[1] & 0x003f) << 10) + 0x0400 - 0x10000;
	sx_fix[2]-=((m_control_0[2] & 0x003f) << 10) + 0x0400 - 0x10000;
	sx_fix[3]-=((m_control_0[3] & 0x003f) << 10) + 0x0400 - 0x10000;

	if (m_flipscreen)
	{
		sy_fix[0] =  0x3000000 - sy_fix[0];
		sy_fix[1] =  0x3000000 - sy_fix[1];
		sy_fix[2] =  0x3000000 - sy_fix[2];
		sy_fix[3] =  0x3000000 - sy_fix[3];
		sx_fix[0] = -0x1a00000 - sx_fix[0];
		sx_fix[1] = -0x1a00000 - sx_fix[1];
		sx_fix[2] = -0x1a00000 - sx_fix[2];
		sx_fix[3] = -0x1a00000 - sx_fix[3];
		sx_fix[4] = -sx_fix[4] + 75;
		sy_fix[4] = -sy_fix[4];
	}

	m_pri_alp_bitmap.fill(0, cliprect);

	/* sprites */
	if (m_sprite_lag == 0)
		get_sprite_info(m_spriteram.target());

	/* Update sprite buffer */
	draw_sprites(bitmap, cliprect);

	/* Parse sprite, alpha & clipping parts of lineram */
	get_spritealphaclip_info();

	/* Parse playfield effects */
	get_line_ram_info(m_tilemap[0], sx_fix[0], sy_fix[0], 0, m_pf_data[0]);
	get_line_ram_info(m_tilemap[1], sx_fix[1], sy_fix[1], 1, m_pf_data[1]);
	get_line_ram_info(m_tilemap[2], sx_fix[2], sy_fix[2], 2, m_pf_data[2]);
	get_line_ram_info(m_tilemap[3], sx_fix[3], sy_fix[3], 3, m_pf_data[3]);
	get_vram_info(m_vram_layer, m_pixel_layer, sx_fix[4], sy_fix[4]);

	/* Draw final framebuffer */
	scanline_draw(bitmap, cliprect);

	if (VERBOSE)
		print_debug_info(bitmap);
	return 0;
}
