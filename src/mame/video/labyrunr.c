#include "driver.h"
#include "video/konamiic.h"

UINT8 *labyrunr_videoram1,*labyrunr_videoram2,*labyrunr_scrollram;
static tilemap *layer0, *layer1;
static rectangle clip0, clip1;


PALETTE_INIT( labyrunr )
{
	int i,pal;

	for (pal = 0;pal < 8;pal++)
	{
		if (pal & 1)	/* chars, no lookup table */
		{
			for (i = 0;i < 256;i++)
				*(colortable++) = 16 * pal + (i & 0x0f);
		}
		else	/* sprites */
		{
			for (i = 0;i < 256;i++)
				if (color_prom[i] == 0)
					*(colortable++) = 0;
				else
					*(colortable++) = 16 * pal + color_prom[i];
		}
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	int attr = labyrunr_videoram1[tile_index];
	int code = labyrunr_videoram1[tile_index + 0x400];
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
			((K007121_ctrlram[0][6]&0x30)*2+16)+(attr&7),
			0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	int attr = labyrunr_videoram2[tile_index];
	int code = labyrunr_videoram2[tile_index + 0x400];
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
			((K007121_ctrlram[0][6]&0x30)*2+16)+(attr&7),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( labyrunr )
{
	layer0 = tilemap_create(get_tile_info0,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	layer1 = tilemap_create(get_tile_info1,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(layer0,0);
	tilemap_set_transparent_pen(layer1,0);

	clip0 = machine->screen[0].visarea;
	clip0.min_x += 40;

	clip1 = machine->screen[0].visarea;
	clip1.max_x = 39;
	clip1.min_x = 0;

	tilemap_set_scroll_cols(layer0,32);
}



/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_HANDLER( labyrunr_vram1_w )
{
	labyrunr_videoram1[offset] = data;
	tilemap_mark_tile_dirty(layer0,offset & 0x3ff);
}

WRITE8_HANDLER( labyrunr_vram2_w )
{
	labyrunr_videoram2[offset] = data;
	tilemap_mark_tile_dirty(layer1,offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( labyrunr )
{
	rectangle finalclip0, finalclip1;

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	if(~K007121_ctrlram[0][3] & 0x20)
	{
		int i;

		finalclip0 = clip0;
		finalclip1 = clip1;

		sect_rect(&finalclip0, cliprect);
		sect_rect(&finalclip1, cliprect);

		tilemap_set_scrollx(layer0,0,K007121_ctrlram[0][0x00] - 40);
		tilemap_set_scrollx(layer1,0,0);

		for(i = 0; i < 32; i++)
		{
			/* enable colscroll */
			if((K007121_ctrlram[0][1] & 6) == 6) // it's probably just one bit, but it's only used once in the game so I don't know which it's
				tilemap_set_scrolly(layer0,(i+2) & 0x1f,K007121_ctrlram[0][0x02] + labyrunr_scrollram[i]);
			else
				tilemap_set_scrolly(layer0,(i+2) & 0x1f,K007121_ctrlram[0][0x02]);
		}

		tilemap_draw(bitmap,&finalclip0,layer0,TILEMAP_DRAW_OPAQUE,0);
		K007121_sprites_draw(machine,0,bitmap,cliprect,spriteram,(K007121_ctrlram[0][6]&0x30)*2,40,0,(K007121_ctrlram[0][3] & 0x40) >> 5);
		/* we ignore the transparency because layer1 is drawn only at the top of the screen also covering sprites */
		tilemap_draw(bitmap,&finalclip1,layer1,TILEMAP_DRAW_OPAQUE,0);
	}
	else
	{
		int use_clip3[2] = { 0, 0 };
		rectangle finalclip3;

		/* custom cliprects needed for the weird effect used in the endinq sequence to hide and show the needed part of text */
		finalclip0.min_y = finalclip1.min_y = cliprect->min_y;
		finalclip0.max_y = finalclip1.max_y = cliprect->max_y;

		if(K007121_ctrlram[0][1] & 1)
		{
			finalclip0.min_x = cliprect->max_x - K007121_ctrlram[0][0x00] + 8;
			finalclip0.max_x = cliprect->max_x;

			if(K007121_ctrlram[0][0x00] >= 40)
			{
				finalclip1.min_x = cliprect->min_x;
			}
			else
			{
				use_clip3[0] = 1;

				finalclip1.min_x = 40 - K007121_ctrlram[0][0x00];
			}

			finalclip1.max_x = cliprect->max_x - K007121_ctrlram[0][0x00] + 8;

		}
		else
		{
			if(K007121_ctrlram[0][0x00] >= 40)
			{
				finalclip0.min_x = cliprect->min_x;
			}
			else
			{
				use_clip3[1] = 1;

				finalclip0.min_x = 40 - K007121_ctrlram[0][0x00];
			}

			finalclip0.max_x = cliprect->max_x - K007121_ctrlram[0][0x00] + 8;

			finalclip1.min_x = cliprect->max_x - K007121_ctrlram[0][0x00] + 8;
			finalclip1.max_x = cliprect->max_x;
		}

		if(use_clip3[0] || use_clip3[1])
		{
			finalclip3.min_y = cliprect->min_y;
			finalclip3.max_y = cliprect->max_y;
			finalclip3.min_x = cliprect->min_x;
			finalclip3.max_x = 40 - K007121_ctrlram[0][0x00] - 8;
		}

		tilemap_set_scrollx(layer0,0,K007121_ctrlram[0][0x00] - 40);
		tilemap_set_scrollx(layer1,0,K007121_ctrlram[0][0x00] - 40);

		tilemap_draw(bitmap,&finalclip0,layer0,0,1);
		if(use_clip3[0]) tilemap_draw(bitmap,&finalclip3,layer0,0,1);

		tilemap_draw(bitmap,&finalclip1,layer1,0,1);
		if(use_clip3[1]) tilemap_draw(bitmap,&finalclip3,layer1,0,1);

		K007121_sprites_draw(machine,0,bitmap,cliprect,spriteram,(K007121_ctrlram[0][6]&0x30)*2,40,0,(K007121_ctrlram[0][3] & 0x40) >> 5);
	}
	return 0;
}
