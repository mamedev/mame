// license:BSD-3-Clause
// copyright-holders:Luca Elia, Roberto Zandona, David Haywood
/***************************************************************************

                    -= Seta, Sammy, Visco (SSV) System =-

                    driver by   Luca Elia (l.elia@tin.it)

    This hardware only generates sprites. But they're of various types,
    including some large "floating tilemap" ones.

    Sprites RAM is 0x40000 bytes long. The first 0x2000 bytes hold a list of
    sprites to display (the list can be made shorter using an end-of-list marker).

    Each entry in the list (8 bytes) is a multi-sprite (e.g it tells the
    hardware to display up to 32 single-sprites).

    The list looks like this:

    Offset:     Bits:                   Value:

        0.h     f--- ---- ---- ----     Shadow
                -edc ---- ---- ----     Each bit enables 2 bitplanes*
                ---- ba-- ---- ----     X Size (1,2,4,8 tiles)
                ---- --98 ---- ----     Y Size (1,2,4,8 tiles)
                ---- ---- 765- ----     Index of a scroll to apply to the single-sprite(s)
                ---- ---- ---4 3210     Number of single-sprites, minus 1

        2.h     f--- ---- ---- ----     List end
                -edc ba98 7654 3210     Offset of the single-sprite(s) data

        4.h     fedc ba-- ---- ----
                ---- --98 7654 3210     X displacement (ignored by tilemap sprites?)

        6.h     fedc ba-- ---- ----
                ---- --98 7654 3210     Y displacement (ignored by tilemap sprites?)


* bit c, which enables/disables the 2 high order bitplanes (256 / 64 color tiles)
  is needed by keithlcy (logo), drifto94 (wheels).
  eaglshot masks even more bits, needed for 'birdie' text etc.

    A single-sprite can be:

    1. a rectangle of tiles (only 1 tile code needs to be specified)
    2. a row of tiles of a tilemap in ram. The row is (always?) as wide
       as the screen and 64 pixels tall.

    Rectangle case(1):
    Offset:     Bits:                   Value:

        0.h                             Code (low bits)

        2.h     f--- ---- ---- ----     Flip X
                -e-- ---- ---- ----     Flip Y
                --dc ba-- ---- ----     Code (high bits)
                ---- --9- ---- ----     Code? Color?
                ---- ---8 7654 3210     Color code (64 color steps)

        4.h     f--- ---- ---- ----     Shadow
                -edc ---- ---- ----     Each bit enables 2 bitplanes*
                ---- ba-- ---- ----     X Size (1,2,4,8 tiles)
                ---- --98 7654 3210     X

        6.h     fedc ---- ---- ----
                ---- ba-- ---- ----     Y Size (1,2,4 tiles) **
                ---- --98 7654 3210     Y


    Tilemap case(2):
    Offset:     Bits:                   Value:

        0.h     fedc ba98 7654 3---
                ---- ---- ---- -210     Scroll index (see below)

        2.h                             Always 0

        4.h     fedc ba-- ---- ----
                ---- --98 7654 3210     X?

        6.h     fedc ---- ---- ----
                ---- ba-- ---- ----     **
                ---- --98 7654 3210     Y

** ? both bits set means "Row Sprite" if the single-sprite type hasn't been
     specified in the sprites list ?

    There are 8 scroll values for the tilemap sprites, in the
    1c0000-1c003f area (each scroll value uses 8 bytes):

    Offset:     Bits:                   Value:

        0.h                             Scroll X

        2.h                             Scroll Y

        4.h                             Priority ? (0000, 0401, 0440, 057f, 05ff) (seems to control offsets too)

        6.h     fed- ---- ---- ----     Tilemap width (games only use 1 -> $200, 2 -> $400)
                ---c ---- ---- ----     Rowscroll enable
                ---- b--- ---- ----     Shadow mode
                ---- -a98 ---- ----     Each bit enables 2 bitplanes*
                ---- ---- 7654 3210     Rowscroll base (multiply by 0x400)

    Where scroll x&y refer to a virtual $8000 x $200 tilemap (filling the
    whole spriteram) made of 16x16 tiles. A tile uses 4 bytes:

    Offset:     Bits:                   Value:

        0.h                             Code (low bits)***

        2.h     f--- ---- ---- ----     Flip X
                -e-- ---- ---- ----     Flip Y
                --dc ba-- ---- ----     Code (high bits)
                ---- --9- ---- ----     Code? Color?
                ---- ---8 7654 3210     Color code (64 color steps)

    The tilemap is stored in ram by column.

*** The tiles size is (always?) 16x16 so Code and Code+1 are used.


    Note that there is a "background layer": a series of tilemap sprites
    that fill up the screen and use scroll 0 as the source tilemap are
    always displayed before the sprites in the sprites list.

    Shadows:

    The low bits of the pens from a "shadowing" tile (regardless of color code)
    substitute the top bits of the color index (0-7fff) in the frame buffer.
    The number of low bits from the "shadowing tile" is 4 or 2, depending on
    bit 7 of 1c0076.

***************************************************************************/

#include "emu.h"
#include "ssv.h"


void ssv_state::drawgfx_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, uint32_t code, uint32_t color, bool flipx, bool flipy, int base_sx, int base_sy, int shadow, int realline, int line)
{
	gfx_element *gfxelement = m_gfxdecode->gfx(0);

	const uint8_t *const addr = gfxelement->get_data(code  % gfxelement->elements());
	const uint32_t realcolor = gfxelement->granularity() * (color % gfxelement->colors());

	const uint8_t *const source = flipy ? addr + (7 - line) * gfxelement->rowbytes() : addr + line * gfxelement->rowbytes();

	if (realline >= cliprect.min_y && realline <= cliprect.max_y)
	{
		struct drawmodes
		{
			int gfx_mask;
			int gfx_shift;
		};

		// comments at top suggest that each bit of 'gfx' enables 2 bitplanes, but ultrax case disagrees, also that would require 4 bits to cover all cases, and we only have 3
		// see also seta2.cpp where the same logic is used
		static constexpr drawmodes BPP_MASK_TABLE[8] = {
			{ 0x3f,0 },   // 0: ultrax, twineag2 text - is there a local / global mixup somewhere, or is this an 'invalid' setting that just enables all planes?
			{ 0xff,0 },   // 1: unverified case, mimic old driver behavior of only using lowest bit
			{ 0x3f,0 },   // 2: unverified case, mimic old driver behavior of only using lowest bit
			{ 0xff,0 },   // 3: unverified case, mimic old driver behavior of only using lowest bit (pastelis sets this on shadows, but seems to need behavior beyond what we currently emulate, maybe also changes number of shadow bits in addition to global shadow mask/shift?)
			{ 0x0f,0 },   // 4: eagle shot 4bpp birdie text
			{ 0xf0,4 },   // 5: eagle shot 4bpp Japanese text
			{ 0x3f,0 },   // 6: common 6bpp case + keithlcy (logo), drifto94 (wheels) masking
			{ 0xff,0 }    // 7: common 8bpp case
		};

		const uint8_t gfxbppmask = BPP_MASK_TABLE[gfx & 0x07].gfx_mask;
		const uint8_t gfxshift = BPP_MASK_TABLE[gfx & 0x07].gfx_shift;

		uint16_t *const dest = &bitmap.pix(realline);

		const int x0 = flipx ? (base_sx + gfxelement->width() - 1) : base_sx;
		const int x1 = flipx ? (base_sx - 1) : (x0 + gfxelement->width());
		const int dx = flipx ? -1 : 1;

		int column = 0;
		for (int sx = x0; sx != x1; sx += dx)
		{
			const uint8_t pen = (source[column] & gfxbppmask) >> gfxshift;

			if (pen && sx >= cliprect.min_x && sx <= cliprect.max_x)
			{
				if (shadow)
					dest[sx] = ((dest[sx] & m_shadow_pen_mask) | (pen << m_shadow_pen_shift)) & 0x7fff;
				else
					dest[sx] = (realcolor + pen) & 0x7fff;
			}
			column++;
		}
	}
}

void ssv_state::drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, uint32_t code, uint32_t color, bool flipx, bool flipy, int base_sx, int base_sy, int shadow)
{
	for (int line = 0; line < 8; line++)
		drawgfx_line(bitmap, cliprect, gfx, code, color, flipx, flipy, base_sx, base_sy, shadow, base_sy+line, line);
}


void ssv_state::video_start()
{
	m_gfxdecode->gfx(0)->set_granularity(64); // 256 colour sprites with palette selectable on 64 colour boundaries

	save_item(NAME(m_enable_video));
	save_item(NAME(m_shadow_pen_mask));
	save_item(NAME(m_shadow_pen_shift));
}

void eaglshot_state::video_start()
{
	ssv_state::video_start();

	m_gfxram = std::make_unique<uint16_t[]>(16 * 0x40000 / 2);

	m_gfxdecode->gfx(0)->set_source((uint8_t *)m_gfxram.get());

	save_pointer(NAME(m_gfxram), 16 * 0x40000 / 2);
}

TILE_GET_INFO_MEMBER(gdfs_state::get_tile_info)
{
	const uint16_t tile = m_tmapram[tile_index];

	tileinfo.set(1, tile, 0, TILE_FLIPXY( tile >> 14 ));
}

void gdfs_state::tmapram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tmapram[offset]);
	m_tmap->mark_tile_dirty(offset);
}

void gdfs_state::video_start()
{
	ssv_state::video_start();

	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gdfs_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 0x100,0x100);

	m_tmap->set_transparent_pen(0);
}

/***************************************************************************

    CRT controller, registers that are read
    (vblank etc.?)

            1c0000 (wait for bit .. to become ..)

    keithlcy:   bit D, 0 -> 1

    mslider:    bit A, 0

    hypreact:
    meosism:
    srmp7:
    sxyreact:
    ultrax: bit F, 0

    twineag2:
    hypreac2:   bit C, 1 -> 0
            bit F, 0

    janjans1:
    srmp4:
    survarts:   no checks

    ryorioh:
    drifto94:   bit D, 0 -> 1
            bit A, 0


    CRT controller, registers that are written
    (resolution, visible area, flipping etc.)

    1c0060-61   ---- ---- ---- ---- ?                           21 or 2b    for all games
    1c0062-63   fedc ba98 7654 3210 x start visible area
    1c0064-65   fedc ba98 7654 3210 x end visible area
    1c0066-67   ---- ---- ---- ---- ?                           1c6     for all games
    1c0068-69   ---- ---- ---- ---- ?                           1       for all games
    1c006a-6b   fedc ba98 7654 3210 y start visible area
    1c006c-6d   fedc ba98 7654 3210 y end visible area
    1c006e-6f   ---- ---- ---- ---- ?                           106     for all games
    1c0070-71   fedc ba98 7654 3210 signed y global tilemap x offset
    1c0072-73   ---- ---- ---- ---- ?
    1c0074-75   ---- ---- ---- ---- ?
                -e-- ---- ---- ---- y flipscreen
                ---c ---- ---- ---- x flipscreen
                ---- ba98 ---- ---- ?                           0101 for all games
                ---- ---- 7654 3210 signed global tilemap x offset
    1c0076-77   -e-- ---- ---- ---- global/local sprites coordinates
                ---- ---- 7--- ---- shadow (0 = 2 bits, 1 = 4 bits)
    1c0078-79   ---- ---- ---- ---- ?
    1c007a-7b   ---- ---- ---- ---- ?
                ---- b--- ---- ---- sprite coordinate mode

            1c0060-7f:

    drifto94:   0000 0025 00cd 01c6 - 0001 0013 0101 0106
            0300 0711 0500 0000 - 0015 5940
            03ea      5558  (flip)

    dynagear:   002b 002c 00d4 01c6 - 0001 0012 0102 0106
            02fd 0000 0500 0000 - 0015 5940
            03ed      5558  (flip)

    eaglshot:   0021 002a 00ca 01c6 - 0001 0016 00f6 0106
            0301 0000 0500 d000 - 0015 5940
            03f1      5560 d00f  (flip)

    gdfs:       002b 002c 00d5 01c6 - 0001 0012 0102 0106
            03ec 0711 0500 0000 - 00d5 5950
            03ec      1557  (flip)

    hypreact:   0021 0022 00cb 01c6 - 0001 000e 00fe 0106
            0301 0000 0500 c000 - 0015 5140
            03f0      5558  (flip)

    hypreac2:   0021 0022 00cb 01c6 - 0001 000e 00fe 0106
            0301 0000 05ff c000 - 0015 5140
            03ea      5558  (flip)

    janjans1:   0021 0023 00cb 01c6 - 0001 000f 00fe 0106
            0300 0000 0500 c000 - 0015 5140
            0300      0500  (flip)

    keithlcy:   002b 0025 00cd 01c6 - 0001 0013 0101 0106
            0300 0711 0500 0000 - 0015 5940
            03ea      5558  (flip)

    meosism:    002b 002c 00d5 01c6 - 0001 0012 00fe 0106
            0301 0000 0500 c000 - 0015 5140
            (no flip)

    mslider:    0021 0026 00d6 01c6 - 0001 000e 00fe 0106
            03f1 0711 5550 c080 - 0015 5940
            0301      0500  (flip)

    ryorioh:    0021 0023*00cb 01c6 - 0001 000f 00fe 0106
            0300 0000 0500 c000 - 0015 5140
            03ed      5558  (flip) *0025

    srmp4:  002b 002c 00d4 01c6 - 0001 0012 0102 0106
            0301 0711 0500 0000 - 0015 4940
            ffe8      5557  (flip)

    srmp7:  002b 002c 00d4 01c6 - 0001 000e 00fd 0106
            0000 0000 e500 0000 - 0015 7140
            02f2      b558  (flip)

    stmblade:   0021 0026 00d6 01c6 - 0001 000e 00fe 0106
            03f1 0711 5550 c080 - 0015 5940         <- 711 becomes 0 during gameplay
            0301      0500  (flip)

    survarts:   002b 002c 00d4 01c6 - 0001 0012 0102 0106
            0301 0000 0500 0000 - 0015 5140
            03e9      5558  (flip)

    sxyreact:   0021 0022 00cb 01c6 - 0001 000e 00fe 0106
            0301 0000 0500 c000 - 0015 5140
            03ef      5558  (flip)

    sxyreac2:   0021 0023 00cb 01c6 - 0001 000e 00fe 0106
            0301 0000 0500 c000 - 0015 5140
            ????      ????  (flip)

    twineag2:   002b 002c 00d4 01c6 - 0001 0012 0102 0106
            ffec 0000 e500 4000 - 0315 7940
            ????      ????  (flip)

    ultrax: 002b 002c 00d4 01c6 - 0001 0012 0102 0106
            ffec 0000 e500 4000 - 0315 7940
            02fe      b558  (flip)

    vasara &    0021 0024 00cc 01c6 - 0001 000e 00fe 0106
    vasara2:    03f1 0000 6500 c000 - 0015 5140
            0301      3558  (flip)


***************************************************************************/

uint16_t ssv_state::vblank_r()
{
	// maybe reads scanline counter? pastelis reads this on the 'for use in Japan' screen + end credits
	// and adjusts y position to do a polled raster effect by changing scroll values midscreen
	uint16_t ret = 0x0000;

	if (m_screen->vblank())
		ret |= 0x3000;

	if (m_screen->hblank())
		ret |= 0x0800;

	return ret;
}

void ssv_state::scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_screen->update_partial(m_screen->vpos() - 1); // pastelis FOR USE IN JAPAN screen polls the vblank / hblank to do a raster effect
	COMBINE_DATA(m_scroll + offset);

//  offsets 60-7f: CRT Controller
}

/***************************************************************************


                                Sprites Drawing

[mslider]
"Insert coins"          101f60: 0006 0825 00b0 000c
                        104128: 1a3a 0000 63d4 0400 (16x16)

Character tilemap       100080: 000f 0410 0000 0000
                        102080: 0002 0000 6200 0c00
                                0002 0000 6200 0c40
                                0002 0000 6200 0c80
                                0002 0000 6200 0cc0
                                0002 0000 6200 0d00
                                ..
                                0002 0000 6200 0fc0
                        1c0010: 73f0 00f1 05ff 4600


[hypreact]
intro                   100140: 6003 04ca 0000 0000 (tilemap sprite)
                        102650: 0003 0000 0000 0c00
                                0003 0000 0000 0c40
                                0003 0000 0000 0c80
                                0003 0000 0000 0cc0
                        1c0018: 15f8 00e8 057f 2600

game                    100058: 6003 00db 0050 0040 (normal sprite)
                        1006d8: 037c 001b 6400 0400

tiles                   100060: 6019 0120 0014 000a (sometimes 6619!)
                        100900: 0a8c 0012 6400 0420 (32x16)
                                0400 0012 6400 0800 (32x32)
                                ...

[hypreac2]
"warning"               100f60: 6106 3893 0068 00c8 (128x32)
                        11c498: 00e0 00b2 6c00 0800
                                0000 0000 0000 0000
text below "warning"    100f70: 6016 389b 0048 00b0 ((16 or 8)x8)
                        11c4d8: 0054 0007 6000 0000

black regions           100af8: 6303 2d64 0000 03f8 (yoffs should be used)
                        116b20: 0004 0000 0000 0fa0 <- move up and down
                                0004 0000 0000 0fe0
                                0004 0000 0000 0ce0
                                0004 0000 0000 0d20
                        1c0020: 6c00 00ef 057f 2600

"credits"               1012f0: 6007 440e 00e0 0008 (16x8)
                        122070: 0043 0006 6000 0000

tiles                   100ce8: 6111 3464 0008 0030 (16x32,empty tile frames)
                        11a320: 0460 006b 6110 0800
                                0460 006b 6100 0800
                                ..
                        100d68: 6205 34de 0008 0030 (16x32, tiles on frames)
                        11a6f0: 050c 0059 6000 0800
                                04c0 005a 6010 0800
                                04d4 005a 6020 0800
                                0500 0059 6030 0800
                                ..
                        100cf8: 6105 348a 0008 fff8 (16x16 x 2x3, big tiles below)
                        11a450: 61d6 0059 6000 0420
                                61f2 0059 6000 0410
                                6212 0059 6000 0400
                                61d8 0059 6010 0420
                                61f4 0059 6010 0410
                                6214 0059 6010 0400

[keithlcy]
high scores             101030: 717f 40c0 0010 0000
                        120600: 0000 0000 00c0 0025 (16x16)
                                0000 0000 00d0 0025
                                ..
                                0000 0000 0000 ffff (ysize should be 16!)
                                0000 0000 0010 ffff
                                ..

K of "KEITH"            101180: 610e 4600 0020 0090
                        123000: 4ef4 016a 0000 0040 (16x16)
                                4ef6 016a 0010 0040
                                ..

floating robot (demo)   101000: 713d 4000 03b0 0088
                        120000: ..
                                71f6 0020 0000 0030
                                71f8 0020 0010 0030
                                ..

cityscape               100030: 7304 1600 0000 007c (yoffs should not be used)
                        10b000: 0001 0000 0200 0000
                                0001 0000 0200 0040
                                0001 0000 0200 0080
                                0001 0000 0200 00c0
                        1c0008: 0800 00f2 05ff 5729


[meosism]
shadows                 100100: 701f 051b 0041 0020 (16x16 shadow)
                        1028d8: 05aa 0030 f000 0470

[srmp4]
logo                    100000: 6303 1680 0180 0078 (yoffs?)
                        10b400: 0001 0000 0000 0060
                                0001 0000 0000 0020
                                0001 0000 0000 ffe0
                                0001 0000 0000 ffa0
                        1c0008: 0800 00ec 05ff 5629

tiles                   100088: 6103 25b7 0028 000a (16x16)
                        112db8: f4aa 0009 03f8 0018
                                f4a8 0009 0008 0018
                                f4ba 0009 03f8 0008
                                f4bc 0009 0008 0008

[survarts]
tilemap                 100030: 6303 2048 0000 0000
                        110240: 0002 0000 0160 03fc
                                0002 0000 0160 003c
                                0002 0000 0160 007c
                                0002 0000 0160 00bc
                        1c0010: 0c2e 01f5 05ff 4628

player                  100030: 611f 1da0 0000 fffc
                        10ed00: eb16 200f 005c fffa (16x16)

"push start"            100130: 601d 1920 0048 0000
                        10c900: 0441 0004 0000 004c (16x8)

[drifto94]
car shadow              100010: 8137 0640 0080 0030
                        103200: ..
                                544a 21e4 0030 0030 (16x16)

writings on finish      100130: 6109 4840 004e 0058 "good work"
                        124200: ee6e 0135 0024 0000 (16x16)
                        100158: 611e 4860 0058 0020 "you have proved yOur"..
                        124300: ee92 0137 0024 0014 (16x16)
                                ..
                                ee7e 0137 fff4 0000 (16x16!!)   ; 'O'
                                ..

[ultrax]
sprite begin of lev1    100010: 6b60 4280 0016 00a0
                        121400: 51a0 0042 6800 0c00 (64x64)

[eaglshot]
title logo              100040: 001b 2920 0048 00e0
                        114900: 2130 0060 7018 0fd0 (16x64)

play                    100020: 0003 290c 0000 0000
                        114860: 0003 0000 03f0 0ce0 (tilemap)

sammy logo              100020: 0003 1000 0000 0000
                        108000: 0001 0000 4380 0ce0 (tilemap)

From the above some noteworthy cases are:

            101f60: 0006 0825 00b0 000c
            104128: 1a3a 0000 63d4 0400     consider y size

            101030: 717f 40c0 0010 0000
            120600: 0000 0000 0000 ffff     ignore y size & depth

            100158: 611e 4860 0058 0020
            124300  ee7e 0137 fff4 0000     ignore x size & depth

            100f60: 6106 3893 0068 00c8
            11c498: 00e0 00b2 6c00 0800     consider x size & y size

            100100: 701f 051b 0041 0020
            1028d8: 05aa 0030 f000 0470     consider shadow (16x16 shadow)

            100010: 6b60 4280 0016 00a0
            121400: 51a0 0042 6800 0c00     (64x64)

            100140: 6003 04ca 0000 0000     tilemap
            102650: 0003 0000 0000 0c00

            100080: 000f 0410 0000 0000     tilemap
            102080: 0002 0000 6200 0c00

**************************************************************************/

// Draw a tilemap sprite


void ssv_state::draw_16x16_tile_line(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flipx, bool flipy, int mode, int code, int color, int sx, int sy, int realline, int line)
{
	// Force 16x16 tiles ?
	int realcode;
	if (flipy)
	{
		if (line & 8)
			realcode = code ;
		else
			realcode = code + 1;
	}
	else
	{
		if (line & 8)
			realcode = code + 1;
		else
			realcode = code;
	}
	const int tileline = line & 7;

	const int shadow = BIT(mode, 11);
	// Select 256 or 64 color tiles
	const int gfx = ((mode & 0x0700) >> 8);

	drawgfx_line(bitmap, cliprect, gfx, realcode, color, flipx, flipy, sx, sy, shadow, realline, tileline);

}

inline void ssv_state::get_tile(int x, int y, int size, int page, int& code, int& attr, bool& flipx, bool& flipy)
{
	const uint16_t *const s3 = &m_spriteram[page * (size * ((0x1000 / 0x200) / 2)) +
		((x & ((size - 1) & ~0xf)) << 2) +
		((y & ((0x200 - 1) & ~0xf)) >> 3)];

	code = s3[0];  // code high bits
	attr = s3[1];  // code low  bits + color

	// Code's high bits are scrambled
	code += m_tile_code[(attr & 0x3c00) >> 10];
	flipy = BIT(attr, 14);
	flipx = BIT(attr, 15);

	if (BIT(m_scroll[0x74 / 2], 12) && BIT(~m_scroll[0x74 / 2], 13))
		flipx = !flipx;

	if (BIT(m_scroll[0x74 / 2], 14) && BIT(~m_scroll[0x74 / 2], 13))
		flipy = !flipy;

}

void ssv_state::draw_row_64pixhigh(bitmap_ind16 &bitmap, const rectangle &cliprect, int in_sy, int scrollreg)
{
	scrollreg &= 0x7;      // scroll register index

	// in_sy will always be 0x00, 0x40, 0x80, 0xc0 in 'draw layer'
	in_sy = (in_sy & 0x1ff) - (in_sy & 0x200);

	// Set up a clipping region for the tilemap slice ..
	rectangle outclip;
	outclip.set(0, 0x20/*width in tiles*/ * 0x10, in_sy, in_sy + 0x8/*height in tiles, always 64 pixels*/ * 0x8);

	// .. and clip it against the visible screen

	if (outclip.min_x > cliprect.max_x)    return;
	if (outclip.min_y > cliprect.max_y)    return;

	if (outclip.max_x < cliprect.min_x)    return;
	if (outclip.max_y < cliprect.min_y)    return;

	outclip &= cliprect;

	for (int line = outclip.min_y; line <= outclip.max_y; line++)
	{
		rectangle clip;
		clip.set(outclip.min_x, outclip.max_x, line, line);

		// Get the scroll data
		int tilemap_scrollx = m_scroll[scrollreg * 4 + 0];    // x scroll
		int tilemap_scrolly = m_scroll[scrollreg * 4 + 1];    // y scroll
		const int unknown = m_scroll[scrollreg * 4 + 2];    // ???
		const int mode = m_scroll[scrollreg * 4 + 3];    // layer disabled, shadow, depth etc.

		// Background layer disabled
		if ((mode & 0xe000) == 0)
			return;

		// Decide the actual size of the tilemap
		const int size = 1 << (8 + ((mode & 0xe000) >> 13));
		const int page = (tilemap_scrollx & 0x7fff) / size;

		// Given a fixed scroll value, the portion of tilemap displayed changes with the sprite position
		tilemap_scrolly += in_sy;

		// Tweak the scroll values
		tilemap_scrolly += ((m_scroll[0x70 / 2] & 0x1ff) - (m_scroll[0x70 / 2] & 0x200) + m_scroll[0x6a / 2] + 2);

		// Kludge for eaglshot
		if ((unknown & 0x05ff) == 0x0440) tilemap_scrollx += -0x10;
		if ((unknown & 0x05ff) == 0x0401) tilemap_scrollx += -0x20;

		const int realy = tilemap_scrolly + (line - in_sy);

		if (BIT(mode, 12))
		{
			const uint32_t scrolltable_base = ((mode & 0x00ff) * 0x400) / 2;
			//logerror("line %d realy %04x: scrolltable base is %08x\n", line,realy&0x1ff, scrolltable_base*2);
			tilemap_scrollx += m_spriteram[(scrolltable_base + (realy & 0x1ff)) & 0x1ffff];
		}

		// Draw the rows
		const int sx1 = 0 - (tilemap_scrollx & 0xf);
		int x = tilemap_scrollx;
		for (int sx = sx1; sx <= clip.max_x; sx += 0x10)
		{
			int code, attr;
			bool flipx, flipy;
			get_tile(x, realy, size, page, code, attr, flipx, flipy);
			draw_16x16_tile_line(bitmap, clip, flipx, flipy, mode, code, attr, sx, realy, line,realy & 0xf);
			x += 0x10;
		} // sx
	} // line
}

// Draw the "background layer" using multiple tilemap sprites

void ssv_state::draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int nr)
{
	for (int sy = 0; sy <= m_screen->visible_area().max_y; sy += 0x40)
		draw_row_64pixhigh(bitmap, cliprect, sy, nr);
}

void ssv_state::draw_sprites_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect, int code, bool flipx, bool flipy, int gfx, int shadow, int color, int sx, int sy, int xnum, int ynum)
{
	int xstart, xend, xinc;
	int ystart, yend, yinc;

	// Draw the tiles
	if (flipx) { xstart = xnum - 1;  xend = -1;    xinc = -1; }
	else       { xstart = 0;         xend = xnum;  xinc = +1; }

	if (flipy) { ystart = ynum - 1;  yend = -1;    yinc = -1; }
	else       { ystart = 0;         yend = ynum;  yinc = +1; }

	for (int x = xstart; x != xend; x += xinc)
	{
		for (int y = ystart; y != yend; y += yinc)
		{
			drawgfx(bitmap, cliprect, gfx,
				code++,
				color,
				flipx, flipy,
				sx + x * 16, sy + y * 8,
				shadow);
		}
	}
}


// Draw sprites in the sprites list

void ssv_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Sprites list
	const uint16_t *spritelist_global = m_spriteram;
	const uint16_t *spritelist_global_end = m_spriteram + 0x02000 / 2;

	for (; spritelist_global < spritelist_global_end; spritelist_global += 4)
	{
		const int sprite = spritelist_global[1];

		// Last sprite
		if (BIT(sprite, 15)) break;

		// Single-sprite address
		const uint16_t *spritelist_local = &m_spriteram[(sprite & 0x7fff) * 4];
		int tilemaps_offsy = ((spritelist_local[3] & 0x1ff) - (spritelist_local[3] & 0x200));

		const int mode = spritelist_global[0];

		// Number of single-sprites int local list (1-32)
		const int local_num = (mode & 0x001f);

		for (int count = 0; count <= local_num; count++, spritelist_local += 4)
		{
			const uint16_t *spritelist_local_end = m_spriteram + 0x40000 / 2;

			if (spritelist_local >= spritelist_local_end) break;

			int sx = spritelist_local[2];
			int sy = spritelist_local[3];

			// do we use local sizes (set here) or global ones (set in previous list)
			const int use_local = BIT(m_scroll[0x76 / 2], 14);

			int xnum = use_local ? (sx & 0x0c00) : (mode & 0x0c00);
			int ynum = use_local ? (sy & 0x0c00) : (mode & 0x0300) << 2;
			const int depth = use_local ? (sx & 0xf000) : (mode & 0xf000);

			if (spritelist_local[0] <= 7 && spritelist_local[1] == 0 && xnum == 0 && ynum == 0x0c00)
			{
				// Tilemap Sprite
				const int scroll = spritelist_local[0];    // scroll index

				if (BIT(m_scroll[0x76 / 2], 12))
					sy -= 0x20;                     // eaglshot
				else
				{
					if (BIT(m_scroll[0x7a / 2], 11))
					{
						if (BIT(m_scroll[0x7a / 2], 12))    // drifto94, dynagear, keithlcy, mslider, stmblade, gdfs, ultrax, twineag2
							sy -= tilemaps_offsy;
						else                        // srmp4
							sy += tilemaps_offsy;
					}
				}

				if (local_num != 0)
				{
					if (spritelist_local[0] != 0) // index 0 is the 'always automatically drawn background layer' so don't draw it twice even if it's specified later?
						draw_row_64pixhigh(bitmap, cliprect, sy, scroll);
				}
			}
			else
			{
				// "Normal" Sprite
				/*
				    hot spots:
				    "warning" in hypreac2 has mode & 0x0100 and is not 16x16
				    keithlcy high scores has mode & 0x0100 and y & 0x0c00 can be 0x0c00
				    drifto94 "you have proved yOur".. has mode & 0x0100 and x & 0x0c00 can be 0x0c00
				    ultrax (begin of lev1): 100010: 6b60 4280 0016 00a0
				                            121400: 51a0 0042 6800 0c00 needs to be a normal sprite
				*/

				int code = spritelist_local[0];  // code high bits
				const int attr = spritelist_local[1];  // code low  bits + color

				// Code's high bits are scrambled
				code += m_tile_code[(attr & 0x3c00) >> 10];
				bool flipy = BIT(attr, 14);
				bool flipx = BIT(attr, 15);

				if (BIT(m_scroll[0x74 / 2], 12) && BIT(~m_scroll[0x74 / 2], 13))
					flipx = !flipx;

				if (BIT(m_scroll[0x74 / 2], 14) && BIT(~m_scroll[0x74 / 2], 13))
					flipy = !flipy;

				// Select 256 or 64 color tiles
				const int gfx = (depth & 0x7000) >> 12;
				const int shadow = BIT(depth, 15);

				/* Every single sprite is offset by x & yoffs, and additionally
				by one of the 8 x & y offsets in the 1c0040-1c005f area   */

				// Apply global offsets
				const int scrollreg = ((mode & 0x00e0) >> 4);

				sx += spritelist_global[2] + m_scroll[scrollreg + (0x40 / 2)];
				sy += spritelist_global[3] + m_scroll[scrollreg + (0x42 / 2)];

				// Sign extend the position
				sx = (sx & 0x1ff) - (sx & 0x200);
				sy = (sy & 0x1ff) - (sy & 0x200);

				const int sprites_offsx = ((m_scroll[0x74 / 2] & 0x7f) - (m_scroll[0x74 / 2] & 0x80));

				const int sprites_offsy = -((m_scroll[0x70 / 2] & 0x1ff) - (m_scroll[0x70 / 2] & 0x200) + m_scroll[0x6a / 2] + 1);

				if (BIT(m_scroll[0x74 / 2], 14)) // flipscreen y
				{
					sy = -sy;
					if (BIT(m_scroll[0x74 / 2], 15))
						sy += 0x00;         //
					else
						sy -= 0x10;         // vasara (hack)
				}

				if (BIT(m_scroll[0x74 / 2], 12)) // flipscreen x
					sx = -sx + 0x100;

				// Single-sprite tile size
				xnum = 1 << (xnum >> 10);   // 1, 2, 4 or 8 tiles
				ynum = 1 << (ynum >> 10);   // 1, 2, 4 tiles (8 means tilemap sprite?)

				// sprites can be relative to a side, the other side or the center

				if (m_scroll[0x7a / 2] == 0x7140)
				{
					// srmp7
					sx = sprites_offsx + sx;
					sy = sprites_offsy - sy;
				}
				else if (BIT(m_scroll[0x7a / 2], 11))
				{
					// dynagear, drifto94, eaglshot, keithlcy, mslider, srmp4, stmblade, twineag2, ultrax
					sx = sprites_offsx + sx - (xnum * 8);
					sy = sprites_offsy - sy - (ynum * 8) / 2;
				}
				else
				{
					// hypreact, hypreac2, janjans1, meosism, ryorioh, survarts, sxyreact, sxyreac2, vasara, vasara2
					sx = sprites_offsx + sx;
					sy = sprites_offsy - sy - (ynum * 8);
				}


				// Sprite code masking
				if (xnum == 2 && ynum == 4) // needed by hypreact
					code &= ~7;

				draw_sprites_tiles(bitmap, cliprect, code, flipx, flipy, gfx, shadow, attr, sx, sy, xnum, ynum);
			}       // sprite type
		}   // single-sprites
	}   // sprites list
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

uint32_t eaglshot_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return ssv_state::screen_update(screen, bitmap, cliprect);
}

uint32_t gdfs_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	ssv_state::screen_update(screen, bitmap, cliprect);

	// draw zooming sprites
	m_st0020->update_screen(screen, bitmap, cliprect, false);

	m_tmap->set_scrollx(0, m_tmapscroll[0x0c/2]);
	m_tmap->set_scrolly(0, m_tmapscroll[0x10/2]);
	m_tmap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void ssv_state::enable_video(bool enable)
{
	m_enable_video = enable;
}

uint32_t ssv_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;

	// Shadow
	if (m_scroll[0x76/2] & 0x0080)
	{
		// 4 bit shadows (mslider, stmblade)
		m_shadow_pen_shift = 15-4;
	}
	else
	{
		// 2 bit shadows
		m_shadow_pen_shift = 15-2;
	}
	m_shadow_pen_mask = (1 << m_shadow_pen_shift) - 1;

	// The background color is the first one in the palette
	bitmap.fill(0, cliprect);

	// used by twineag2 and ultrax
	clip.min_x = (cliprect.max_x / 2 + m_scroll[0x62/2]) * 2 - m_scroll[0x64/2] * 2 + 2;
	clip.max_x = (cliprect.max_x / 2 + m_scroll[0x62/2]) * 2 - m_scroll[0x62/2] * 2 + 1;
	clip.min_y = (cliprect.max_y     + m_scroll[0x6a/2])     - m_scroll[0x6c/2]     + 1;
	clip.max_y = (cliprect.max_y     + m_scroll[0x6a/2])     - m_scroll[0x6a/2]        ;

//  printf("%04x %04x %04x %04x\n",clip.min_x, clip.max_x, clip.min_y, clip.max_y);

	clip &= cliprect;

	if (!m_enable_video)
		return 0;

	draw_layer(bitmap, clip, 0); // "background layer"

	draw_sprites(bitmap, clip);  // sprites list

	return 0;
}
