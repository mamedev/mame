#include "driver.h"
#include "video/konicdev.h"


UINT16 *tail2nos_bgvideoram;


static tilemap *bg_tilemap;

static int charbank,charpalette,video_enable;
static UINT16 *zoomdata;

#define TOTAL_CHARS 0x400


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT16 code = tail2nos_bgvideoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x1fff) + (charbank << 13),
			((code & 0xe000) >> 13) + charpalette * 16,
			0);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void tail2nos_zoom_callback(running_machine *machine, int *code,int *color,int *flags)
{
	*code |= ((*color & 0x03) << 8);
	*color = 32 + ((*color & 0x38) >> 3);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( tail2nos )
{
	bg_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(bg_tilemap,15);

	zoomdata = (UINT16 *)memory_region(machine, "gfx3");
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( tail2nos_bgvideoram_w )
{
	COMBINE_DATA(&tail2nos_bgvideoram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

READ16_HANDLER( tail2nos_zoomdata_r )
{
	return zoomdata[offset];
}

WRITE16_HANDLER( tail2nos_zoomdata_w )
{
	int oldword = zoomdata[offset];
	COMBINE_DATA(&zoomdata[offset]);
	if (oldword != zoomdata[offset])
		gfx_element_mark_dirty(space->machine->gfx[2], offset / 64);
}

WRITE16_HANDLER( tail2nos_gfxbank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		int bank;

		/* bits 0 and 2 select char bank */
		if (data & 0x04) bank = 2;
		else if (data & 0x01) bank = 1;
		else bank = 0;

		if (charbank != bank)
		{
			charbank = bank;
			tilemap_mark_all_tiles_dirty(bg_tilemap);
		}

		/* bit 5 seems to select palette bank (used on startup) */
		if (data & 0x20) bank = 7;
		else bank = 3;

		if (charpalette != bank)
		{
			charpalette = bank;
			tilemap_mark_all_tiles_dirty(bg_tilemap);
		}

		/* bit 4 seems to be video enable */
		video_enable = data & 0x10;
	}
}


/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;


	for (offs = 0;offs < machine->generic.spriteram_size/2;offs += 4)
	{
		int sx,sy,flipx,flipy,code,color;

		sx = spriteram16[offs + 1];
		if (sx >= 0x8000) sx -= 0x10000;
		sy = 0x10000 - spriteram16[offs + 0];
		if (sy >= 0x8000) sy -= 0x10000;
		code = spriteram16[offs + 2] & 0x07ff;
		color = (spriteram16[offs + 2] & 0xe000) >> 13;
		flipx = spriteram16[offs + 2] & 0x1000;
		flipy = spriteram16[offs + 2] & 0x0800;

		drawgfx_transpen(bitmap,/* placement relative to zoom layer verified on the real thing */
				cliprect,machine->gfx[1],
				code,
				40 + color,
				flipx,flipy,
				sx+3,sy+1,15);
	}
}

VIDEO_UPDATE( tail2nos )
{
	const device_config *k051316 = devtag_get_device(screen->machine, "k051316");

	if (video_enable)
	{
		k051316_zoom_draw(k051316, bitmap,cliprect, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect);
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	}
	else
		bitmap_fill(bitmap, cliprect, 0);

	return 0;
}
