/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku
    (c)1998 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

******************************************************************************/

#include "emu.h"
#include "includes/inufuku.h"


/******************************************************************************

    Memory handlers

******************************************************************************/

WRITE16_HANDLER( inufuku_palettereg_w )
{
	inufuku_state *state = space->machine().driver_data<inufuku_state>();
	switch (offset)
	{
		case 0x02:	state->m_bg_palettebank = (data & 0xf000) >> 12;
				tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
				break;
		case 0x03:	state->m_tx_palettebank = (data & 0xf000) >> 12;
				tilemap_mark_all_tiles_dirty(state->m_tx_tilemap);
				break;
	}
}

WRITE16_HANDLER( inufuku_scrollreg_w )
{
	inufuku_state *state = space->machine().driver_data<inufuku_state>();
	switch (offset)
	{
		case 0x00:	state->m_bg_scrollx = data + 1; break;
		case 0x01:	state->m_bg_scrolly = data + 0; break;
		case 0x02:	state->m_tx_scrollx = data - 3; break;
		case 0x03:	state->m_tx_scrolly = data + 1; break;
		case 0x04:	state->m_bg_raster = (data & 0x0200) ? 0 : 1; break;
	}
}


/******************************************************************************

    Sprite routines

******************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	inufuku_state *state = machine.driver_data<inufuku_state>();
	int offs;

	for (offs = (state->m_spriteram1_size / 16) - 1; offs >= 0; offs--)
	{
		if ((state->m_spriteram1[offs] & 0x8000) == 0x0000)
		{
			int attr_start;
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
			int priority, priority_mask;

			attr_start = 4 * (state->m_spriteram1[offs] & 0x03ff);

			/*
                attr_start + 0x0000
                ---- ---x xxxx xxxx oy
                ---- xxx- ---- ---- ysize
                xxxx ---- ---- ---- zoomy

                attr_start + 0x0001
                ---- ---x xxxx xxxx ox
                ---- xxx- ---- ---- xsize
                xxxx ---- ---- ---- zoomx

                attr_start + 0x0002
                -x-- ---- ---- ---- flipx
                x--- ---- ---- ---- flipy
                --xx xxxx ---- ---- color
                --xx ---- ---- ---- priority?
                ---- ---- xxxx xxxx unused?

                attr_start + 0x0003
                -xxx xxxx xxxx xxxx map start
                x--- ---- ---- ---- unused?
            */

			ox = (state->m_spriteram1[attr_start + 1] & 0x01ff) + 0;
			xsize = (state->m_spriteram1[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (state->m_spriteram1[attr_start + 1] & 0xf000) >> 12;
			oy = (state->m_spriteram1[attr_start + 0] & 0x01ff) + 1;
			ysize = (state->m_spriteram1[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (state->m_spriteram1[attr_start + 0] & 0xf000) >> 12;
			flipx = state->m_spriteram1[attr_start + 2] & 0x4000;
			flipy = state->m_spriteram1[attr_start + 2] & 0x8000;
			color = (state->m_spriteram1[attr_start + 2] & 0x3f00) >> 8;
			priority = (state->m_spriteram1[attr_start + 2] & 0x3000) >> 12;
			map_start = (state->m_spriteram1[attr_start + 3] & 0x7fff) << 1;

			switch (priority)
			{
				default:
				case 0:	priority_mask = 0x00; break;
				case 3:	priority_mask = 0xfe; break;
				case 2:	priority_mask = 0xfc; break;
				case 1:	priority_mask = 0xf0; break;
			}

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = (oy + zoomy * (ysize - y) / 2 + 16) & 0x1ff;
				else
					sy = (oy + zoomy * y / 2 + 16) & 0x1ff;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = (ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff;
					else
						sx = (ox + zoomx * x / 2 + 16) & 0x1ff;

					code  = ((state->m_spriteram2[map_start] & 0x0007) << 16) + state->m_spriteram2[map_start + 1];

					pdrawgfxzoom_transpen(bitmap, cliprect, machine.gfx[2],
							code,
							color,
							flipx, flipy,
							sx - 16, sy - 16,
							zoomx << 11, zoomy << 11,
							machine.priority_bitmap,priority_mask, 15);

					map_start += 2;
				}
			}
		}
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

static TILE_GET_INFO( get_inufuku_bg_tile_info )
{
	inufuku_state *state = machine.driver_data<inufuku_state>();
	SET_TILE_INFO(
			0,
			state->m_bg_videoram[tile_index],
			state->m_bg_palettebank,
			0);
}

static TILE_GET_INFO( get_inufuku_tx_tile_info )
{
	inufuku_state *state = machine.driver_data<inufuku_state>();
	SET_TILE_INFO(
			1,
			state->m_tx_videoram[tile_index],
			state->m_tx_palettebank,
			0);
}

READ16_HANDLER( inufuku_bg_videoram_r )
{
	inufuku_state *state = space->machine().driver_data<inufuku_state>();
	return state->m_bg_videoram[offset];
}

WRITE16_HANDLER( inufuku_bg_videoram_w )
{
	inufuku_state *state = space->machine().driver_data<inufuku_state>();
	COMBINE_DATA(&state->m_bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

READ16_HANDLER( inufuku_tx_videoram_r )
{
	inufuku_state *state = space->machine().driver_data<inufuku_state>();
	return state->m_tx_videoram[offset];
}

WRITE16_HANDLER( inufuku_tx_videoram_w )
{
	inufuku_state *state = space->machine().driver_data<inufuku_state>();
	COMBINE_DATA(&state->m_tx_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_tx_tilemap, offset);
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( inufuku )
{
	inufuku_state *state = machine.driver_data<inufuku_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_inufuku_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tx_tilemap = tilemap_create(machine, get_inufuku_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->m_bg_tilemap, 255);
	tilemap_set_transparent_pen(state->m_tx_tilemap, 255);
}


/******************************************************************************

    Display refresh

******************************************************************************/

SCREEN_UPDATE( inufuku )
{
	inufuku_state *state = screen->machine().driver_data<inufuku_state>();
	int i;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));
	bitmap_fill(screen->machine().priority_bitmap, NULL, 0);

	if (state->m_bg_raster)
	{
		tilemap_set_scroll_rows(state->m_bg_tilemap, 512);
		for (i = 0; i < 256; i++)
			tilemap_set_scrollx(state->m_bg_tilemap, (state->m_bg_scrolly + i) & 0x1ff, state->m_bg_rasterram[i]);
	}
	else
	{
		tilemap_set_scroll_rows(state->m_bg_tilemap, 1);
		tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_bg_scrollx);
	}
	tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_bg_scrolly);
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);

	tilemap_set_scrollx(state->m_tx_tilemap, 0, state->m_tx_scrollx);
	tilemap_set_scrolly(state->m_tx_tilemap, 0, state->m_tx_scrolly);
	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 4);

	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
