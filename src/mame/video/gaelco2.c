// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  Gaelco Type CG-1V/GAE1 Video Hardware

  Functions to emulate the video hardware of the machine

  CG-1V/GAE1 (Gaelco custom GFX & Sound chip):
    The CG-1V works with 16x16, 5 bpp gfx.
    It can handle:
        * 2 1024x512 tilemaps with linescroll.
        * 2 banks of 512 sprites (sprites can be grouped up to 16x16).
    Sprites can make the background darker or brighter.

    Memory map:
    ===========
        0x000000-0x000fff   Sprite bank #1  (1)
        0x001000-0x001fff   Sprite bank #2  (1)
        0x002000-0x0023ff   Linescroll tilemap #1 (2)
        0x002400-0x0027ff   Linescroll tilemap #2 (2)
            Linescroll entries are like this:
                Word | Bit(s)            | Description
                -----+-FEDCBA98-76543210-+--------------------------
                 i   | xxxxxx-- -------- | not used?
                 i   | ------xx xxxxxxxx | line i x scroll register

        0x002800-0x002807   Scroll registers
                Word | Bit(s)            | Description
                -----+-FEDCBA98-76543210-+--------------------------
                  0  | xxxxxxx- -------- | not used?
                  0  | -------x xxxxxxxx | tilemap #1 y scroll register
                  1  | xxxxxx-- -------- | not used?
                  1  | ------xx xxxxxxxx | tilemap #1 x scroll register
                  2  | xxxxxxx- -------- | not used?
                  2  | -------x xxxxxxxx | tilemap #2 y scroll register
                  3  | xxxxxx-- -------- | not used?
                  3  | ------xx xxxxxxxx | tilemap #2 x scroll register
        0x002890-0x0028ff   Sound registers (3)
        0x000000-0x00ffff   Video RAM
        0x010000-0x011fff   Palette (xRRRRRGGGGGBBBBB)
        0x018004-0x018007   Video Registers
                Word | Bit(s)            | Description
                -----+-FEDCBA98-76543210-+--------------------------
                  0  | x------- -------- | tilemap #1 linescroll enable
                  0  | -xxx---- -------- | not used?
                  0  | ----xxx- -------- | tilemap #1 video RAM bank? (4, 5)
                  0  | -------x -------- | not used?
                  0  | -------- x------- | unknown
                  0  | -------- -x------ | not used?
                  0  | -------- --xx---- | visible area size? (=0,480x240;=1,384x240;=2,320x240)
                  0  | -------- ----xxxx | not used?
                  1  | x------- -------- | tilemap #2 linescroll enable
                  1  | -xxx---- -------- | not used?
                  1  | ----xxx- -------- | tilemap #2 video RAM bank? (4, 5)
                  1  | -------x xxx----- | not used?
                  1  | -------- ---x---- | sprite bank select
                  1  | -------- ----xxxx | not used?
        0x018008-0x018009   Clear video int?

Notes:
    (1) See sprite format in the sprite section
    (2) x scroll register is not taken into account when doing line scroll
    (3) See sound/gaelco.c for the sound register layout
    (4) tilemaps use the memory [0x2000*bank .. 0x2000*bank + 0x1fff]
    (5) See tile format in the tilemap section

Multi monitor notes:
    Some games have two RGB outputs to allow two or four simultaneous
    players linking two cabinets (World Rally 2, Touch & Go).

    In 2 monitors mode, the hardware maps one tilemap to a monitor and the
    other tilemap to the other monitor. The game palette is splitted, using
    the first half for one monitor and the second half for the other monitor.
    The sprite RAM has one bit that selects the sprite's target monitor. The
    sound is splitted too, right channel for cabinet 1 and the left channel
    for the other cabinet.

***************************************************************************/

#include "emu.h"
#include "includes/gaelco2.h"


/***************************************************************************

    Callbacks for the TileMap code (single monitor games)

    Tile format
    -----------

    Screen 0 & 1: (1024*512, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -----xxx | code (bits 18-16)
      0  | -------- --xxx--- | not used?
      0  | -------- -x------ | flip y
      0  | -------- x------- | flip x
      0  | -------x -------- | not used?
      0  | xxxxxxx- -------- | color
      1  | xxxxxxxx xxxxxxxx | code (bits 15-0)

***************************************************************************/

TILE_GET_INFO_MEMBER(gaelco2_state::get_tile_info_gaelco2_screen0)
{
	int data = m_videoram[(((m_vregs[0] >> 9) & 0x07)*0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(((m_vregs[0] >> 9) & 0x07)*0x2000/2) + ((tile_index << 1) + 1)];
	int code = ((data & 0x07) << 16) | (data2 & 0xffff);

	SET_TILE_INFO_MEMBER(0, code, ((data >> 9) & 0x7f), TILE_FLIPXY((data >> 6) & 0x03));
}

TILE_GET_INFO_MEMBER(gaelco2_state::get_tile_info_gaelco2_screen1)
{
	int data = m_videoram[(((m_vregs[1] >> 9) & 0x07)*0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(((m_vregs[1] >> 9) & 0x07)*0x2000/2) + ((tile_index << 1) + 1)];
	int code = ((data & 0x07) << 16) | (data2 & 0xffff);

	SET_TILE_INFO_MEMBER(0, code, ((data >> 9) & 0x7f), TILE_FLIPXY((data >> 6) & 0x03));
}


/***************************************************************************

    Callbacks for the TileMap code (dual monitor games)

    Tile format
    -----------

    Screen 0 & 1: (1024*512, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -----xxx | code (bits 18-16)
      0  | -------- --xxx--- | not used?
      0  | -------- -x------ | flip y
      0  | -------- x------- | flip x
      0  | -------x -------- | not used?
      0  | -xxxxxx- -------- | color
      0  | x------- -------- | unknown
      1  | xxxxxxxx xxxxxxxx | code (bits 15-0)

***************************************************************************/

TILE_GET_INFO_MEMBER(gaelco2_state::get_tile_info_gaelco2_screen0_dual)
{
	int data = m_videoram[(((m_vregs[0] >> 9) & 0x07)*0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(((m_vregs[0] >> 9) & 0x07)*0x2000/2) + ((tile_index << 1) + 1)];
	int code = ((data & 0x07) << 16) | (data2 & 0xffff);

	SET_TILE_INFO_MEMBER(0, code, ((data >> 9) & 0x3f), TILE_FLIPXY((data >> 6) & 0x03));
}

TILE_GET_INFO_MEMBER(gaelco2_state::get_tile_info_gaelco2_screen1_dual)
{
	int data = m_videoram[(((m_vregs[1] >> 9) & 0x07)*0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(((m_vregs[1] >> 9) & 0x07)*0x2000/2) + ((tile_index << 1) + 1)];
	int code = ((data & 0x07) << 16) | (data2 & 0xffff);

	SET_TILE_INFO_MEMBER(0, code, 0x40 + ((data >> 9) & 0x3f), TILE_FLIPXY((data >> 6) & 0x03));
}


/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(gaelco2_state::gaelco2_vram_w)
{
	int pant0_start = ((m_vregs[0] >> 9) & 0x07)*0x1000;
	int pant0_end = pant0_start + 0x1000;
	int pant1_start = ((m_vregs[1] >> 9) & 0x07)*0x1000;
	int pant1_end = pant1_start + 0x1000;

	COMBINE_DATA(&m_videoram[offset]);

	/* tilemap 0 writes */
	if ((offset >= pant0_start) && (offset < pant0_end)){
		m_pant[0]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);
	}

	/* tilemap 1 writes */
	if ((offset >= pant1_start) && (offset < pant1_end)){
		m_pant[1]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);
	}
}

/***************************************************************************

    Palette (paletteram_xRRRRRGGGGGBBBBB_word_w)

    The game's palette uses colors 0-4095, but we need 15 aditional palettes
    to handle shadows and highlights properly. After a color write to the
    game's palette we update the other palettes with a darker/brighter color.

    Sprites use last palette entry for shadows and highlights
    (in order to make some pixels darker or brighter).

    The sprite's pens define the color adjustment:

    0x00 -> Transparent
    0x01-0x07 -> Shadow level (0x01 = min, 0x07 = max)
    0x08-0x0f -> Highlight level (0x08 = max, 0x0f = min)
    0x10-0x1f -> not used?

***************************************************************************/

#define RGB_CHG     0x08
#define ADJUST_COLOR(c) ((c < 0) ? 0 : ((c > 255) ? 255 : c))

/* table used for color adjustment */
static const int pen_color_adjust[16] = {
	+RGB_CHG*0, -RGB_CHG*1, -RGB_CHG*2, -RGB_CHG*3, -RGB_CHG*4, -RGB_CHG*5, -RGB_CHG*6, -RGB_CHG*7,
	+RGB_CHG*8, +RGB_CHG*7, +RGB_CHG*6, +RGB_CHG*5, +RGB_CHG*4, +RGB_CHG*3, +RGB_CHG*2, +RGB_CHG*1
};


WRITE16_MEMBER(gaelco2_state::gaelco2_palette_w)
{
	int i, color, r, g, b, auxr, auxg, auxb;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	color = m_generic_paletteram_16[offset];

	/* extract RGB components */
	r = (color >> 10) & 0x1f;
	g = (color >>  5) & 0x1f;
	b = (color >>  0) & 0x1f;

	r = pal5bit(r);
	g = pal5bit(g);
	b = pal5bit(b);

	/* update game palette */
	m_palette->set_pen_color(4096*0 + offset, rgb_t(r, g, b));

	/* update shadow/highligh palettes */
	for (i = 1; i < 16; i++){
		/* because the last palette entry is reserved for shadows and highlights, we
		don't use it and that way we save some colors so the UI looks fine ;-) */
		if ((offset >= 0xff0) && (offset <= 0xfff)) return;

		auxr = ADJUST_COLOR(r + pen_color_adjust[i]);
		auxg = ADJUST_COLOR(g + pen_color_adjust[i]);
		auxb = ADJUST_COLOR(b + pen_color_adjust[i]);

		m_palette->set_pen_color(4096*i + offset, rgb_t(auxr, auxg, auxb));
	}
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(gaelco2_state,gaelco2)
{
	m_videoram = m_spriteram->live();

	/* create tilemaps */
	m_pant[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco2_state::get_tile_info_gaelco2_screen0),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_pant[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco2_state::get_tile_info_gaelco2_screen1),this),TILEMAP_SCAN_ROWS,16,16,64,32);

	/* set tilemap properties */
	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);

	m_pant[0]->set_scroll_rows(512);
	m_pant[0]->set_scroll_cols(1);
	m_pant[1]->set_scroll_rows(512);
	m_pant[1]->set_scroll_cols(1);

	m_dual_monitor = 0;
}

VIDEO_START_MEMBER(gaelco2_state,gaelco2_dual)
{
	m_videoram = m_spriteram->live();

	/* create tilemaps */
	m_pant[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco2_state::get_tile_info_gaelco2_screen0_dual),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_pant[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco2_state::get_tile_info_gaelco2_screen1_dual),this),TILEMAP_SCAN_ROWS,16,16,64,32);

	/* set tilemap properties */
	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);

	m_pant[0]->set_scroll_rows(512);
	m_pant[0]->set_scroll_cols(1);
	m_pant[1]->set_scroll_rows(512);
	m_pant[1]->set_scroll_cols(1);

	m_dual_monitor = 1;
}

/***************************************************************************

    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------x xxxxxxxx | sprite bank (sprite number bits 18-10)
      0  | xxxxxxx- -------- | sprite color (bits 6-0)
      1  | -------x xxxxxxxx | y position
      1  | ------x- -------- | sprite enable
      1  | -----x-- -------- | flipy
      1  | ----x--- -------- | flipx
      1  | xxxx---- -------- | sprite y size
      2  | ------xx xxxxxxxx | x position
      2  | ----xx-- -------- | not used?
      2  | xxxx---- -------- | sprite x size
      3  | xxxxxxxx xxxxxxxx | pointer to more sprite data

      Each sprite entry has a pointer to more sprite data.
      The length of data depends on the sprite size (xsize*ysize).
      Each entry has the following format:

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | ----xxxx xxxxxxxx | sprite number offset (sprite number bits 11-0)
      0  | xxxx---- -------- | sprite color offset (bits 3-0)

      In dual monitor games, the configuration is the same, but MSB of
      word 0 is used to select target monitor for the sprite, and the
      palette is splitted for each monitor.

      Last sprite color entry is used for shadows/highlights

***************************************************************************/

void gaelco2_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int mask, int xoffs)
{
	UINT16 *buffered_spriteram16 = m_spriteram->buffer();
	int j, x, y, ex, ey, px, py;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	/* get sprite ram start and end offsets */
	int start_offset = (m_vregs[1] & 0x10)*0x100;
	int end_offset = start_offset + 0x1000;

	/* sprite offset is based on the visible area */
	int spr_x_adjust = (screen.visible_area().max_x - 320 + 1) - (511 - 320 - 1) - ((m_vregs[0] >> 4) & 0x01) + xoffs;

	for (j = start_offset; j < end_offset; j += 8){
		int data = buffered_spriteram16[(j/2) + 0];
		int data2 = buffered_spriteram16[(j/2) + 1];
		int data3 = buffered_spriteram16[(j/2) + 2];
		int data4 = buffered_spriteram16[(j/2) + 3];

		int sx = data3 & 0x3ff;
		int sy = data2 & 0x1ff;

		int xflip = data2 & 0x800;
		int yflip = data2 & 0x400;

		int xsize = ((data3 >> 12) & 0x0f) + 1;
		int ysize = ((data2 >> 12) & 0x0f) + 1;

		if (m_dual_monitor && ((data & 0x8000) != mask)) continue;

		/* if it's enabled, draw it */
		if ((data2 & 0x0200) != 0){
			for (y = 0; y < ysize; y++){
				for (x = 0; x < xsize; x++){
					/* for each x,y of the sprite, fetch the sprite data */
					int data5 = buffered_spriteram16[((data4/2) + (y*xsize + x)) & 0x7fff];
					int number = ((data & 0x1ff) << 10) + (data5 & 0x0fff);
					int color = ((data >> 9) & 0x7f) + ((data5 >> 12) & 0x0f);
					int color_effect = m_dual_monitor ? ((color & 0x3f) == 0x3f) : (color == 0x7f);

					ex = xflip ? (xsize - 1 - x) : x;
					ey = yflip ? (ysize - 1 - y) : y;

					/* normal sprite, pen 0 transparent */
					if (color_effect == 0){
							gfx->transpen(bitmap,cliprect, number,
							color, xflip, yflip,
							((sx + ex*16) & 0x3ff) + spr_x_adjust,
							((sy + ey*16) & 0x1ff), 0);

					} else { /* last palette entry is reserved for shadows and highlights */

						/* get a pointer to the current sprite's gfx data */
						const UINT8 *gfx_src = gfx->get_data(number % gfx->elements());

						for (py = 0; py < gfx->height(); py++){
							/* get a pointer to the current line in the screen bitmap */
							int ypos = ((sy + ey*16 + py) & 0x1ff);
							UINT16 *srcy = &bitmap.pix16(ypos);

							int gfx_py = yflip ? (gfx->height() - 1 - py) : py;

							if ((ypos < cliprect.min_y) || (ypos > cliprect.max_y)) continue;

							for (px = 0; px < gfx->width(); px++){
								/* get current pixel */
								int xpos = (((sx + ex*16 + px) & 0x3ff) + spr_x_adjust) & 0x3ff;
								UINT16 *pixel = srcy + xpos;
								int src_color = *pixel;

								int gfx_px = xflip ? (gfx->width() - 1 - px) : px;

								/* get asociated pen for the current sprite pixel */
								int gfx_pen = gfx_src[gfx->rowbytes()*gfx_py + gfx_px];

								if ((gfx_pen == 0) || (gfx_pen >= 16)) continue;

								if ((xpos < cliprect.min_x) || (xpos > cliprect.max_x)) continue;

								/* make background color darker or brighter */
								*pixel = src_color + 4096*gfx_pen;
							}
						}
					}
				}
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 gaelco2_state::screen_update_gaelco2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	/* read scroll values */
	int scroll0x = m_videoram[0x2802/2] + 0x14;
	int scroll1x = m_videoram[0x2806/2] + 0x10;
	int scroll0y = m_videoram[0x2800/2] + 0x01;
	int scroll1y = m_videoram[0x2804/2] + 0x01;

	/* set y scroll registers */
	m_pant[0]->set_scrolly(0, scroll0y & 0x1ff);
	m_pant[1]->set_scrolly(0, scroll1y & 0x1ff);

	/* set x linescroll registers */
	for (i = 0; i < 512; i++){
		m_pant[0]->set_scrollx(i & 0x1ff, (m_vregs[0] & 0x8000) ? (m_videoram[(0x2000/2) + i] + 0x14) & 0x3ff : scroll0x & 0x3ff);
		m_pant[1]->set_scrollx(i & 0x1ff, (m_vregs[1] & 0x8000) ? (m_videoram[(0x2400/2) + i] + 0x10) & 0x3ff : scroll1x & 0x3ff);
	}

	/* draw screen */
	bitmap.fill(0, cliprect);

	m_pant[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_pant[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 gaelco2_state::dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index)
{
	int i;

	/* read scroll values */
	int scroll0x = m_videoram[0x2802/2] + 0x14;
	int scroll1x = m_videoram[0x2806/2] + 0x10;
	int scroll0y = m_videoram[0x2800/2] + 0x01;
	int scroll1y = m_videoram[0x2804/2] + 0x01;

	/* set y scroll registers */
	m_pant[0]->set_scrolly(0, scroll0y & 0x1ff);
	m_pant[1]->set_scrolly(0, scroll1y & 0x1ff);

	/* set x linescroll registers */
	for (i = 0; i < 512; i++){
		m_pant[0]->set_scrollx(i & 0x1ff, (m_vregs[0] & 0x8000) ? (m_videoram[(0x2000/2) + i] + 0x14) & 0x3ff : scroll0x & 0x3ff);
		m_pant[1]->set_scrollx(i & 0x1ff, (m_vregs[1] & 0x8000) ? (m_videoram[(0x2400/2) + i] + 0x10) & 0x3ff : scroll1x & 0x3ff);
	}

	/* draw screen */
	bitmap.fill(0, cliprect);

	m_pant[index]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(screen,bitmap,cliprect, 0x8000 * index, 0);

	return 0;
}

UINT32 gaelco2_state::screen_update_gaelco2_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return dual_update(screen, bitmap, cliprect, 0); }
UINT32 gaelco2_state::screen_update_gaelco2_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return dual_update(screen, bitmap, cliprect, 1); }
