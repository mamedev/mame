#include "emu.h"
#include "includes/playmark.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( bigtwin_get_tx_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	UINT16 code = state->m_videoram1[2 * tile_index];
	UINT16 color = state->m_videoram1[2 * tile_index + 1];
	SET_TILE_INFO(
			2,
			code,
			color,
			0);
}

static TILE_GET_INFO( bigtwin_get_fg_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	UINT16 code = state->m_videoram2[2 * tile_index];
	UINT16 color = state->m_videoram2[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			code,
			color,
			0);
}

static TILE_GET_INFO( wbeachvl_get_tx_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	UINT16 code = state->m_videoram1[2 * tile_index];
	UINT16 color = state->m_videoram1[2 * tile_index + 1];

	SET_TILE_INFO(
			2,
			code,
			color / 4,
			0);
}

static TILE_GET_INFO( wbeachvl_get_fg_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	UINT16 code = state->m_videoram2[2 * tile_index];
	UINT16 color = state->m_videoram2[2 * tile_index + 1];

	SET_TILE_INFO(
			1,
			code & 0x7fff,
			color / 4 + 8,
			(code & 0x8000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( wbeachvl_get_bg_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	UINT16 code = state->m_videoram3[2 * tile_index];
	UINT16 color = state->m_videoram3[2 * tile_index + 1];

	SET_TILE_INFO(
			1,
			code & 0x7fff,
			color / 4,
			(code & 0x8000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( hrdtimes_get_tx_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int code = state->m_videoram1[tile_index] & 0x03ff;
	int colr = state->m_videoram1[tile_index] & 0xe000;

	SET_TILE_INFO(2,code + state->m_txt_tile_offset, colr >> 13, 0);
}

static TILE_GET_INFO( bigtwinb_get_tx_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int code = state->m_videoram1[tile_index] & 0x0fff;
	int colr = state->m_videoram1[tile_index] & 0xf000;

	SET_TILE_INFO(2,code + state->m_txt_tile_offset, colr >> 12, 0);
}

static TILE_GET_INFO( hrdtimes_get_fg_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int code = state->m_videoram2[tile_index] & 0x1fff;
	int colr = state->m_videoram2[tile_index] & 0xe000;

	SET_TILE_INFO(1,code + 0x2000,(colr >> 13) + 8,0);
}

static TILE_GET_INFO( hrdtimes_get_bg_tile_info )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int code = state->m_videoram3[tile_index] & 0x1fff;
	int colr = state->m_videoram3[tile_index] & 0xe000;

	SET_TILE_INFO(1, code, colr >> 13, 0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bigtwin )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_tx_tilemap = tilemap_create(machine, bigtwin_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, bigtwin_get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(0);

	state->m_xoffset = 0;
	state->m_yoffset = 0;
	state->m_txt_tile_offset = 0;

	state->m_pri_masks[0] = 0;
	state->m_pri_masks[1] = 0;
	state->m_pri_masks[2] = 0;
}


VIDEO_START( bigtwinb )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_tx_tilemap = tilemap_create(machine, bigtwinb_get_tx_tile_info,tilemap_scan_rows, 8, 8, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, hrdtimes_get_fg_tile_info,tilemap_scan_rows, 16, 16, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, hrdtimes_get_bg_tile_info,tilemap_scan_rows, 16, 16, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap->set_scrolldx(-4, -4);

	state->m_xoffset = 1;
	state->m_yoffset = 0;
	state->m_txt_tile_offset = 0x8000;

	state->m_pri_masks[0] = 0;
	state->m_pri_masks[1] = 0;
	state->m_pri_masks[2] = 0;
}


VIDEO_START( wbeachvl )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_tx_tilemap = tilemap_create(machine, wbeachvl_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, wbeachvl_get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_bg_tilemap = tilemap_create(machine, wbeachvl_get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);

	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_xoffset = 0;
	state->m_yoffset = 0;
	state->m_txt_tile_offset = 0;

	state->m_pri_masks[0] = 0xfff0;
	state->m_pri_masks[1] = 0xfffc;
	state->m_pri_masks[2] = 0;
}

VIDEO_START( excelsr )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_tx_tilemap = tilemap_create(machine, bigtwin_get_tx_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, bigtwin_get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(0);

	state->m_xoffset = 0;
	state->m_yoffset = 0;
	state->m_txt_tile_offset = 0;

	state->m_pri_masks[0] = 0;
	state->m_pri_masks[1] = 0xfffc;
	state->m_pri_masks[2] = 0xfff0;
}

VIDEO_START( hotmind )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_tx_tilemap = tilemap_create(machine, hrdtimes_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, hrdtimes_get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, hrdtimes_get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_tx_tilemap->set_scrolldx(-14, -14);
	state->m_fg_tilemap->set_scrolldx(-14, -14);
	state->m_bg_tilemap->set_scrolldx(-14, -14);

	state->m_xoffset = -9;
	state->m_yoffset = -8;
	state->m_txt_tile_offset = 0x9000;

	state->m_pri_masks[0] = 0xfff0;
	state->m_pri_masks[1] = 0xfffc;
	state->m_pri_masks[2] = 0;
}

VIDEO_START( hrdtimes )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_tx_tilemap = tilemap_create(machine, hrdtimes_get_tx_tile_info,tilemap_scan_rows, 8, 8, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, hrdtimes_get_fg_tile_info,tilemap_scan_rows, 16, 16, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, hrdtimes_get_bg_tile_info,tilemap_scan_rows, 16, 16, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_tx_tilemap->set_scrolldx(-14, -14);
	state->m_fg_tilemap->set_scrolldx(-10, -10);
	state->m_bg_tilemap->set_scrolldx(-12, -12);

	state->m_xoffset = -8;
	state->m_yoffset = -8;
	state->m_txt_tile_offset = 0xfc00;

	state->m_pri_masks[0] = 0xfff0;
	state->m_pri_masks[1] = 0xfffc;
	state->m_pri_masks[2] = 0;
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(playmark_state::wbeachvl_txvideoram_w)
{

	COMBINE_DATA(&m_videoram1[offset]);
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(playmark_state::wbeachvl_fgvideoram_w)
{

	COMBINE_DATA(&m_videoram2[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(playmark_state::wbeachvl_bgvideoram_w)
{

	COMBINE_DATA(&m_videoram3[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(playmark_state::hrdtimes_txvideoram_w)
{

	COMBINE_DATA(&m_videoram1[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(playmark_state::hrdtimes_fgvideoram_w)
{

	COMBINE_DATA(&m_videoram2[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(playmark_state::hrdtimes_bgvideoram_w)
{

	COMBINE_DATA(&m_videoram3[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(playmark_state::bigtwin_paletteram_w)
{
	int r, g, b, val;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);

	val = m_generic_paletteram_16[offset];
	r = (val >> 11) & 0x1e;
	g = (val >>  7) & 0x1e;
	b = (val >>  3) & 0x1e;

	r |= ((val & 0x08) >> 3);
	g |= ((val & 0x04) >> 2);
	b |= ((val & 0x02) >> 1);

	palette_set_color_rgb(machine(), offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE16_MEMBER(playmark_state::bigtwin_scroll_w)
{

	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: 	m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1: 	m_tx_tilemap->set_scrolly(0, data);   break;
		case 2: 	m_bgscrollx = -(data + 4);                    break;
		case 3: 	m_bgscrolly = (-data) & 0x1ff;
				m_bg_enable = data & 0x0200;
				m_bg_full_size = data & 0x0400;
				break;
		case 4: 	m_fg_tilemap->set_scrollx(0, data + 6); break;
		case 5: 	m_fg_tilemap->set_scrolly(0, data);   break;
	}
}

WRITE16_MEMBER(playmark_state::wbeachvl_scroll_w)
{

	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: 	m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1: 	m_tx_tilemap->set_scrolly(0, data);   break;
		case 2: 	m_fgscrollx = data + 4;break;
		case 3: 	m_fg_tilemap->set_scrolly(0, data & 0x3ff);
				m_fg_rowscroll_enable = data & 0x0800;
				break;
		case 4: 	m_bg_tilemap->set_scrollx(0, data + 6); break;
		case 5: 	m_bg_tilemap->set_scrolly(0, data);   break;
	}
}

WRITE16_MEMBER(playmark_state::excelsr_scroll_w)
{

	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0:	m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1: 	m_tx_tilemap->set_scrolly(0, data);   break;
		case 2: 	m_bgscrollx = -data;	break;
		case 3: 	m_bgscrolly = (-data + 2)& 0x1ff;
				m_bg_enable = data & 0x0200;
				m_bg_full_size = data & 0x0400;
				break;
		case 4:	m_fg_tilemap->set_scrollx(0, data + 6); break;
		case 5:	m_fg_tilemap->set_scrolly(0, data);   break;
	}
}

WRITE16_MEMBER(playmark_state::hrdtimes_scroll_w)
{

	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_tx_tilemap->set_scrollx(0, data); break;
		case 1: m_tx_tilemap->set_scrolly(0, data); break;
		case 2: m_fg_tilemap->set_scrollx(0, data); break;
		case 3: m_fg_tilemap->set_scrolly(0, data); break;
		case 4: m_bg_tilemap->set_scrollx(0, data); break;
		case 5: m_bg_tilemap->set_scrolly(0, data); break;
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int offs, start_offset = state->m_spriteram_size / 2 - 4;
	int height = machine.gfx[0]->height;
	int colordiv = machine.gfx[0]->color_granularity / 16;
	UINT16 *spriteram = state->m_spriteram;

	// find the "end of list" to draw the sprites in reverse order
	for (offs = 4; offs < state->m_spriteram_size / 2; offs += 4)
	{
		if (spriteram[offs + 3 - 4] == 0x2000) /* end of list marker */
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (offs = start_offset; offs >= 4; offs -= 4)
	{
		int sx, sy, code, color, flipx, pri;

		sy = spriteram[offs + 3 - 4];	/* -4? what the... ??? */

		flipx = sy & 0x4000;
		sx = (spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		code = spriteram[offs + 2] >> codeshift;
		color = ((spriteram[offs + 1] & 0x3e00) >> 9) / colordiv;
		pri = (spriteram[offs + 1] & 0x8000) >> 15;

		if(!pri && (color & 0x0c) == 0x0c)
			pri = 2;

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				 code,
				 color,
				 flipx,0,
				 sx + state->m_xoffset,sy + state->m_yoffset,
				 machine.priority_bitmap,state->m_pri_masks[pri],0);
	}
}


static void bigtwinb_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int offs, start_offset = state->m_spriteram_size / 2 - 4;
	int height = machine.gfx[0]->height;
	UINT16 *spriteram = state->m_spriteram;

	// find the "end of list" to draw the sprites in reverse order
	for (offs = 4; offs < state->m_spriteram_size / 2; offs += 4)
	{
		if (spriteram[offs + 3 - 4] == 0x2000) /* end of list marker */
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (offs = start_offset; offs >= 4; offs -= 4)
	{
		int sx, sy, code, color, flipx;

		sy = spriteram[offs + 3 - 4];	/* -4? what the... ??? */

		flipx = sy & 0x4000;
		sx = (spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		code = spriteram[offs + 2] >> codeshift;
		color = ((spriteram[offs + 1] & 0xf000) >> 12);

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				 code,
				 color,
				 flipx,0,
				 sx + state->m_xoffset,sy + state->m_yoffset, 0);
	}
}

static void draw_bitmap( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	int x, y, count;
	int color;
	UINT8 *pri;

	count = 0;
	for (y = 0; y < 512; y++)
	{
		for (x = 0; x < 512; x++)
		{
			color = state->m_bgvideoram[count] & 0xff;

			if (color)
			{
				if (state->m_bg_full_size)
				{
					bitmap.pix16((y + state->m_bgscrolly) & 0x1ff, (x + state->m_bgscrollx) & 0x1ff) = 0x100 + color;

					pri = &machine.priority_bitmap.pix8((y + state->m_bgscrolly) & 0x1ff);
					pri[(x + state->m_bgscrollx) & 0x1ff] |= 2;
				}
				else
				{
					/* 50% size */
					if(!(x % 2) && !(y % 2))
					{
						bitmap.pix16((y / 2 + state->m_bgscrolly) & 0x1ff, (x / 2 + state->m_bgscrollx) & 0x1ff) = 0x100 + color;

						pri = &machine.priority_bitmap.pix8((y / 2 + state->m_bgscrolly) & 0x1ff);
						pri[(x / 2 + state->m_bgscrollx) & 0x1ff] |= 2;
					}
				}
			}

			count++;
		}
	}
}

SCREEN_UPDATE_IND16( bigtwin )
{
	playmark_state *state = screen.machine().driver_data<playmark_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	if (state->m_bg_enable)
		draw_bitmap(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 4);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


SCREEN_UPDATE_IND16( bigtwinb )
{
	playmark_state *state = screen.machine().driver_data<playmark_state>();

	// video enabled
	if (state->m_scroll[6] & 1)
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
		bigtwinb_draw_sprites(screen.machine(), bitmap, cliprect, 4);
		state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( excelsr )
{
	playmark_state *state = screen.machine().driver_data<playmark_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 1);
	if (state->m_bg_enable)
		draw_bitmap(screen.machine(), bitmap, cliprect);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 4);
	draw_sprites(screen.machine(), bitmap, cliprect, 2);
	return 0;
}

SCREEN_UPDATE_IND16( wbeachvl )
{
	playmark_state *state = screen.machine().driver_data<playmark_state>();

	if (state->m_fg_rowscroll_enable)
	{
		int i;

		state->m_fg_tilemap->set_scroll_rows(512);
		for (i = 0; i < 256; i++)
			state->m_fg_tilemap->set_scrollx(i + 1, state->m_rowscroll[8 * i]);
	}
	else
	{
		state->m_fg_tilemap->set_scroll_rows(1);
		state->m_fg_tilemap->set_scrollx(0, state->m_fgscrollx);
	}

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 1);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 2);
	draw_sprites(screen.machine(), bitmap, cliprect, 0);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( hrdtimes )
{
	playmark_state *state = screen.machine().driver_data<playmark_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	// video enabled
	if (state->m_scroll[6] & 1)
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 1);
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 2);
		draw_sprites(screen.machine(), bitmap, cliprect, 2);
		state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
	return 0;
}
