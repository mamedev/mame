// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  Gaelco Type 1 Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/gaelco.h"
#include "screen.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -------x | flip x
      0  | -------- ------x- | flip y
      0  | xxxxxxxx xxxxxx-- | code
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | xxxxxxxx -------- | not used
*/

TILE_GET_INFO_MEMBER(gaelco_state::get_tile_info_gaelco_screen0)
{
	int data = m_videoram[tile_index << 1];
	int data2 = m_videoram[(tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2);

	tileinfo.category = (data2 >> 6) & 0x03;

	SET_TILE_INFO_MEMBER(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}


TILE_GET_INFO_MEMBER(gaelco_state::get_tile_info_gaelco_screen1)
{
	int data = m_videoram[(0x1000 / 2) + (tile_index << 1)];
	int data2 = m_videoram[(0x1000 / 2) + (tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2);

	tileinfo.category = (data2 >> 6) & 0x03;

	SET_TILE_INFO_MEMBER(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(gaelco_state::gaelco_vram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tilemap[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(gaelco_state,bigkarnk)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco_state::get_tile_info_gaelco_screen0),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco_state::get_tile_info_gaelco_screen1),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[0]->set_transmask(0, 0xff01, 0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
	m_tilemap[1]->set_transmask(0, 0xff01, 0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
}

VIDEO_START_MEMBER(gaelco_state,maniacsq)
{
	m_screen->register_screen_bitmap(m_temp_bitmap_bg0);
	m_screen->register_screen_bitmap(m_temp_bitmap_bg1);
	m_screen->register_screen_bitmap(m_temp_bitmap_sprites);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco_state::get_tile_info_gaelco_screen0),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gaelco_state::get_tile_info_gaelco_screen1),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (8x8 quadrant)
      3  | xxxxxxxx xxxxxx-- | sprite code
*/

void gaelco_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i, x, y, ex, ey;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	static const int x_offset[2] = {0x0,0x2};
	static const int y_offset[2] = {0x0,0x1};

	for (i = 3; i < 0x800; i += 4)
	{
		int sx = m_spriteram[i + 2] & 0x01ff;
		int sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int color = (m_spriteram[i + 2] & 0x7e00) >> 9;
		int attr = (m_spriteram[i] & 0xfe00) >> 9;
		int priority = (m_spriteram[i] & 0x1000) >> 12;
		int priority2 = (m_spriteram[i] & 0x2000) >> 13;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size;

		// these do appear to be in reverse order
		color |= priority2 << 6;
		color |= priority << 7;

		if (attr & 0x04)
			spr_size = 1;
		else
		{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++)
		{
			for (x = 0; x < spr_size; x++)
			{
				ex = xflip ? (spr_size - 1 - x) : x;
				ey = yflip ? (spr_size - 1 - y) : y;

				gfx->transpen_raw(bitmap,cliprect,number + x_offset[ex] + y_offset[ey],
						color<<4,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,
						0);
			}
		}
	}
}

// Big Karnak still relies on sprites with palettes 0x38 - 0x3f being forced to higher priority
// I have a feeling this isn't entirely correct but works for this game, however the mixing
// etc. does not appear to work the same way as the other games
void gaelco_state::draw_sprites_bigkarnk( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i, x, y, ex, ey;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	static const int x_offset[2] = {0x0,0x2};
	static const int y_offset[2] = {0x0,0x1};

	for (i = 0x800 - 4 - 1; i >= 3; i -= 4)
	{
		int sx = m_spriteram[i + 2] & 0x01ff;
		int sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int color = (m_spriteram[i + 2] & 0x7e00) >> 9;
		int attr = (m_spriteram[i] & 0xfe00) >> 9;
		int priority = (m_spriteram[i] & 0x3000) >> 12;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size, pri_mask;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38)
			priority = 4;

		switch (priority)
		{
			case 0: pri_mask = 0xff00; break;
			case 1: pri_mask = 0xff00 | 0xf0f0; break;
			case 2: pri_mask = 0xff00 | 0xf0f0 | 0xcccc; break;
			case 3: pri_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa; break;
			default:
			case 4: pri_mask = 0; break;
		}

		if (attr & 0x04)
			spr_size = 1;
		else
		{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++)
		{
			for (x = 0; x < spr_size; x++)
			{
				ex = xflip ? (spr_size - 1 - x) : x;
				ey = yflip ? (spr_size - 1 - y) : y;

				gfx->prio_transpen(bitmap,cliprect,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,
						screen.priority(),pri_mask,0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t gaelco_state::screen_update_maniacsq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_temp_bitmap_bg0.fill(0, cliprect);
	m_temp_bitmap_bg1.fill(0, cliprect);
	m_temp_bitmap_sprites.fill(0, cliprect);

	draw_sprites(screen, m_temp_bitmap_sprites, cliprect);

	const bitmap_ind16 &bg0_pixmap = m_tilemap[0]->pixmap();
	const bitmap_ind8 &bg0_flagsmap = m_tilemap[0]->flagsmap();
	const bitmap_ind16 &bg1_pixmap = m_tilemap[1]->pixmap();
	const bitmap_ind8 &bg1_flagsmap = m_tilemap[1]->flagsmap();

	for (int y = cliprect.min_y;y < cliprect.max_y;y++)
	{
		int yoff;
		yoff = (y + m_vregs[0])&(m_tilemap[0]->height() - 1);
		uint16_t *src1 = &bg0_pixmap.pix16(yoff);
		uint8_t *src1flags = &bg0_flagsmap.pix8(yoff);
		yoff = (y + m_vregs[2])&(m_tilemap[1]->height() - 1);
		uint16_t *src2 = &bg1_pixmap.pix16(yoff);
		uint8_t *src2flags = &bg1_flagsmap.pix8(yoff);

		uint16_t *src3 = &m_temp_bitmap_sprites.pix16(y);

		uint16_t *dst = &bitmap.pix16(y);

		for (int x = cliprect.min_x;x < cliprect.max_x;x++)
		{
			int xoff;
			xoff = (x + m_vregs[1] + 4)&(m_tilemap[0]->width() - 1);
			uint16_t src1dat = src1[xoff] | (src1flags[xoff] & 0x03) << 10;
			xoff = (x + m_vregs[3])&(m_tilemap[0]->width() - 1);
			uint16_t src2dat = src2[xoff] | (src2flags[xoff] & 0x03) << 10;;

			uint16_t src3dat = src3[x];

			dst[x] = src1dat & 0x3ff; // so we have a bgpen

			uint16_t lastdrawn = 0xffff;

			if (src1dat & 0xf)
			{
				dst[x] = src1dat & 0x3ff;
				lastdrawn = src1dat & 0xfff;
			}

			if (src2dat & 0xf)
			{
				if (((src2dat & 0xfff) <= lastdrawn)) // could be <= or <
				{
					dst[x] = src2dat & 0x3ff;
					lastdrawn = src2dat & 0xfff;
				}
			}

			if (src3dat & 0xf)
			{
				if (((src3dat & 0xfff) <= lastdrawn))
				{
					dst[x] = src3dat & 0x3ff;
					lastdrawn = src3dat & 0xfff;
				}
			}
		}
	}

	return 0;
}

uint32_t gaelco_state::screen_update_bigkarnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_tilemap[0]->set_scrolly(0, m_vregs[0]);
	m_tilemap[0]->set_scrollx(0, m_vregs[1] + 4);
	m_tilemap[1]->set_scrolly(0, m_vregs[2]);
	m_tilemap[1]->set_scrollx(0, m_vregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 1);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 1);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 2);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 2);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 4);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 4);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);

	draw_sprites_bigkarnk(screen, bitmap, cliprect);
	return 0;
}
