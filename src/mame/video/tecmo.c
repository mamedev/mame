/***************************************************************************

  video hardware for Tecmo games

***************************************************************************/

#include "driver.h"

UINT8 *tecmo_txvideoram,*tecmo_fgvideoram,*tecmo_bgvideoram;

int tecmo_video_type = 0;
/*
   video_type is used to distinguish Rygar, Silkworm and Gemini Wing.
   This is needed because there is a difference in the tile and sprite indexing.
*/

static tilemap *tx_tilemap,*fg_tilemap,*bg_tilemap;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 attr = tecmo_bgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			3,
			tecmo_bgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 attr = tecmo_fgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			2,
			tecmo_fgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

static TILE_GET_INFO( gemini_get_bg_tile_info )
{
	UINT8 attr = tecmo_bgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			3,
			tecmo_bgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

static TILE_GET_INFO( gemini_get_fg_tile_info )
{
	UINT8 attr = tecmo_fgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			2,
			tecmo_fgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	UINT8 attr = tecmo_txvideoram[tile_index+0x400];
	SET_TILE_INFO(
			0,
			tecmo_txvideoram[tile_index] + ((attr & 0x03) << 8),
			attr >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( tecmo )
{
	if (tecmo_video_type == 2)	/* gemini */
	{
		bg_tilemap = tilemap_create(gemini_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,16);
		fg_tilemap = tilemap_create(gemini_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,16);
	}
	else	/* rygar, silkworm */
	{
		bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,16);
		fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,16);
	}
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,32,32);

	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(tx_tilemap,0);

	tilemap_set_scrolldx(bg_tilemap,-48,256+48);
	tilemap_set_scrolldx(fg_tilemap,-48,256+48);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( tecmo_txvideoram_w )
{
	tecmo_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( tecmo_fgvideoram_w )
{
	tecmo_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x1ff);
}

WRITE8_HANDLER( tecmo_bgvideoram_w )
{
	tecmo_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x1ff);
}

WRITE8_HANDLER( tecmo_fgscroll_w )
{
	static UINT8 scroll[3];

	scroll[offset] = data;

	tilemap_set_scrollx(fg_tilemap,0,scroll[0] + 256 * scroll[1]);
	tilemap_set_scrolly(fg_tilemap,0,scroll[2]);
}

WRITE8_HANDLER( tecmo_bgscroll_w )
{
	static UINT8 scroll[3];

	scroll[offset] = data;

	tilemap_set_scrollx(bg_tilemap,0,scroll[0] + 256 * scroll[1]);
	tilemap_set_scrolly(bg_tilemap,0,scroll[2]);
}

WRITE8_HANDLER( tecmo_flipscreen_w )
{
	flip_screen_set(data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;
	static const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = spriteram_size-8;offs >= 0;offs -= 8)
	{
		int flags = spriteram[offs+3];
		int priority = flags>>6;
		int bank = spriteram[offs+0];
		if (bank & 4)
		{ /* visible */
			int which = spriteram[offs+1];
			int code,xpos,ypos,flipx,flipy,priority_mask,x,y;
			int size = spriteram[offs + 2] & 3;

			if (tecmo_video_type != 0)	/* gemini, silkworm */
			  code = which + ((bank & 0xf8) << 5);
			else						/* rygar */
			  code = which + ((bank & 0xf0) << 4);

			code &= ~((1 << (size*2)) - 1);
			size = 1 << size;

			xpos = spriteram[offs + 5] - ((flags & 0x10) << 4);
			ypos = spriteram[offs + 4] - ((flags & 0x20) << 3);
			flipx = bank & 1;
			flipy = bank & 2;

			if (flip_screen)
			{
				xpos = 256 - (8 * size) - xpos;
				ypos = 256 - (8 * size) - ypos;
				flipx = !flipx;
				flipy = !flipy;
			}

			/* bg: 1; fg:2; text: 4 */
			switch (priority)
			{
				default:
				case 0x0: priority_mask = 0; break;
				case 0x1: priority_mask = 0xf0; break; /* obscured by text layer */
				case 0x2: priority_mask = 0xf0|0xcc; break;	/* obscured by foreground */
				case 0x3: priority_mask = 0xf0|0xcc|0xaa; break; /* obscured by bg and fg */
			}

			for (y = 0;y < size;y++)
			{
				for (x = 0;x < size;x++)
				{
					int sx = xpos + 8*(flipx?(size-1-x):x);
					int sy = ypos + 8*(flipy?(size-1-y):y);
					pdrawgfx(bitmap,machine->gfx[1],
							code + layout[y][x],
							flags & 0xf,
							flipx,flipy,
							sx,sy,
							cliprect,TRANSPARENCY_PEN,0,
							priority_mask);
				}
			}
		}
	}
}


VIDEO_UPDATE( tecmo )
{
	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0x100],cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,1);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,2);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,4);

	draw_sprites(machine, bitmap,cliprect);
	return 0;
}
