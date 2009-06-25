#include "driver.h"
#include "video/konamiic.h"

UINT8 *fastlane_k007121_regs,*fastlane_videoram1,*fastlane_videoram2;
static tilemap *layer0, *layer1;
static rectangle clip0, clip1;


PALETTE_INIT( fastlane )
{
	int pal;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x400);

	for (pal = 0; pal < 0x10; pal++)
	{
		int i;

		for (i = 0; i < 0x400; i++)
		{
			UINT8 ctabentry = (i & 0x3f0) | color_prom[(pal << 4) | (i & 0x0f)];
			colortable_entry_set_value(machine->colortable, (pal << 10) | i, ctabentry);
		}
	}
}


static void set_pens(colortable_t *colortable)
{
	int i;

	for (i = 0x00; i < 0x800; i += 2)
	{
		UINT16 data = paletteram[i | 1] | (paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(colortable, i >> 1, color);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	int attr = fastlane_videoram1[tile_index];
	int code = fastlane_videoram1[tile_index + 0x400];
	int bit0 = (K007121_ctrlram[0][0x05] >> 0) & 0x03;
	int bit1 = (K007121_ctrlram[0][0x05] >> 2) & 0x03;
	int bit2 = (K007121_ctrlram[0][0x05] >> 4) & 0x03;
	int bit3 = (K007121_ctrlram[0][0x05] >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((K007121_ctrlram[0][0x03] & 0x01) << 5);
	int mask = (K007121_ctrlram[0][0x04] & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((K007121_ctrlram[0][0x04] & mask) << 1);

	SET_TILE_INFO(
			0,
			code+bank*256,
			1 + 64 * (attr & 0x0f),
			0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	int attr = fastlane_videoram2[tile_index];
	int code = fastlane_videoram2[tile_index + 0x400];
	int bit0 = (K007121_ctrlram[0][0x05] >> 0) & 0x03;
	int bit1 = (K007121_ctrlram[0][0x05] >> 2) & 0x03;
	int bit2 = (K007121_ctrlram[0][0x05] >> 4) & 0x03;
	int bit3 = (K007121_ctrlram[0][0x05] >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((K007121_ctrlram[0][0x03] & 0x01) << 5);
	int mask = (K007121_ctrlram[0][0x04] & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((K007121_ctrlram[0][0x04] & mask) << 1);

	SET_TILE_INFO(
			0,
			code+bank*256,
			0 + 64 * (attr & 0x0f),
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( fastlane )
{
	layer0 = tilemap_create(machine, get_tile_info0,tilemap_scan_rows,8,8,32,32);
	layer1 = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,8,8,32,32);

	tilemap_set_scroll_rows( layer0, 32 );

	clip0 = *video_screen_get_visible_area(machine->primary_screen);
	clip0.min_x += 40;

	clip1 = *video_screen_get_visible_area(machine->primary_screen);
	clip1.max_x = 39;
	clip1.min_x = 0;
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_HANDLER( fastlane_vram1_w )
{
	fastlane_videoram1[offset] = data;
	tilemap_mark_tile_dirty(layer0,offset & 0x3ff);
}

WRITE8_HANDLER( fastlane_vram2_w )
{
	fastlane_videoram2[offset] = data;
	tilemap_mark_tile_dirty(layer1,offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( fastlane )
{
	rectangle finalclip0 = clip0, finalclip1 = clip1;
	int i, xoffs;

	sect_rect(&finalclip0, cliprect);
	sect_rect(&finalclip1, cliprect);

	set_pens(screen->machine->colortable);

	/* set scroll registers */
	xoffs = K007121_ctrlram[0][0x00];
	for( i=0; i<32; i++ ){
		tilemap_set_scrollx(layer0, i, fastlane_k007121_regs[0x20 + i] + xoffs - 40);
	}
	tilemap_set_scrolly( layer0, 0, K007121_ctrlram[0][0x02] );

	tilemap_draw(bitmap,&finalclip0,layer0,0,0);
	K007121_sprites_draw(0,bitmap,cliprect,screen->machine->gfx,screen->machine->colortable,spriteram,0,40,0,-1);
	tilemap_draw(bitmap,&finalclip1,layer1,0,0);
	return 0;
}
