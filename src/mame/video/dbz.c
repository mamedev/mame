/*
  Dragonball Z
  (c) 1993 Banpresto
  Dragonball Z 2 Super Battle
  (c) 1994 Banpresto

  Video hardware emulation.
*/


#include "driver.h"
#include "video/konamiic.h"

static tilemap *dbz_bg1_tilemap, *dbz_bg2_tilemap;
UINT16 *dbz_bg1_videoram, *dbz_bg2_videoram;
static int sprite_colorbase, layer_colorbase[6], layer[5], layerpri[5];

static void dbz_tile_callback(int layer, int *code, int *color, int *flags)
{
	*color = (layer_colorbase[layer] << 1) + ((*color & 0x3c) >> 2);
}

static void dbz_sprite_callback(int *code, int *color, int *priority_mask)
{
	int pri = (*color & 0x3c0) >> 5;

	if(pri <= layerpri[3])							*priority_mask = 0xff00;
	else if (pri > layerpri[3] && pri <= layerpri[2])	*priority_mask = 0xfff0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xfffc;
	else												*priority_mask = 0xfffe;

	*color = (sprite_colorbase << 1) + (*color & 0x1f);
}

/* Background Tilemaps */

WRITE16_HANDLER( dbz_bg2_videoram_w )
{
	COMBINE_DATA(&dbz_bg2_videoram[offset]);
	tilemap_mark_tile_dirty(dbz_bg2_tilemap,offset/2);
}

static TILE_GET_INFO( get_dbz_bg2_tile_info )
{
	int tileno, colour, flag;

	tileno = dbz_bg2_videoram[tile_index*2+1] & 0x7fff;
	colour = (dbz_bg2_videoram[tile_index*2] & 0x000f);
	flag = (dbz_bg2_videoram[tile_index*2] & 0x0080) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0,tileno,colour + (layer_colorbase[5] << 1),flag);
}

WRITE16_HANDLER( dbz_bg1_videoram_w )
{
	COMBINE_DATA(&dbz_bg1_videoram[offset]);
	tilemap_mark_tile_dirty(dbz_bg1_tilemap,offset/2);
}

static TILE_GET_INFO( get_dbz_bg1_tile_info )
{
	int tileno, colour, flag;

	tileno = dbz_bg1_videoram[tile_index*2+1] & 0x7fff;
	colour = (dbz_bg1_videoram[tile_index*2] & 0x000f);
	flag = (dbz_bg1_videoram[tile_index*2] & 0x0080) ? TILE_FLIPX : 0;

	SET_TILE_INFO(1,tileno,colour + (layer_colorbase[4] << 1),flag);
}

/* useful function to sort the four tile layers by priority order */
static void sortlayers(int *layer, int *pri)
{
#define SWAP(a,b) \
	if (pri[a] <= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0, 1)
	SWAP(0, 2)
	SWAP(0, 3)
	SWAP(0, 4)
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(1, 4)
	SWAP(2, 3)
	SWAP(2, 4)
	SWAP(3, 4)
}

VIDEO_START( dbz )
{
	K053251_vh_start();
	K056832_vh_start(machine, REGION_GFX1, K056832_BPP_4, 1, NULL, dbz_tile_callback, 1);
	K053247_vh_start(machine, REGION_GFX2, -52, 16, NORMAL_PLANE_ORDER, dbz_sprite_callback);

	dbz_bg1_tilemap = tilemap_create(get_dbz_bg1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,64,32);
	dbz_bg2_tilemap = tilemap_create(get_dbz_bg2_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,64,32);

	tilemap_set_transparent_pen(dbz_bg1_tilemap,0);
	tilemap_set_transparent_pen(dbz_bg2_tilemap,0);

	K053936_wraparound_enable(0, 1);
	K053936_set_offset(0, -46, -16);

	K053936_wraparound_enable(1, 1);
	K053936_set_offset(1, -46, -16);

	if (!strcmp(machine->gamedrv->name, "dbz"))
		K056832_set_LayerOffset(0, -34, -16);
	else
		K056832_set_LayerOffset(0, -35, -16);

	K056832_set_LayerOffset(1, -31, -16);
	K056832_set_LayerOffset(3, -31, -16); //?

	K053247_set_SpriteOffset(-87,32);
}

VIDEO_UPDATE( dbz )
{
	static const int K053251_CI[6] = { K053251_CI3, K053251_CI4, K053251_CI4, K053251_CI4, K053251_CI2, K053251_CI1 };
	int plane, new_colorbase;

	sprite_colorbase = K053251_get_palette_index(K053251_CI0);

	for (plane=0; plane<6; plane++)
	{
		new_colorbase = K053251_get_palette_index(K053251_CI[plane]);
		if (layer_colorbase[plane] != new_colorbase)
		{
			layer_colorbase[plane] = new_colorbase;
			if(plane <= 3)
				K056832_mark_plane_dirty(plane);
			else if(plane == 4)
				tilemap_mark_all_tiles_dirty(dbz_bg1_tilemap);
			else if(plane == 5)
				tilemap_mark_all_tiles_dirty(dbz_bg2_tilemap);
		}
	}

	//layers priority

	layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI3);
	layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI4);
	layer[2] = 3;
	layerpri[2] = K053251_get_priority(K053251_CI0);
	layer[3] = 4;
	layerpri[3] = K053251_get_priority(K053251_CI2);
	layer[4] = 5;
	layerpri[4] = K053251_get_priority(K053251_CI1);

	sortlayers(layer, layerpri);

	fillbitmap(priority_bitmap, 0, cliprect);

	for(plane = 0; plane < 5; plane++)
	{
		int flag, pri;

		if(plane == 0)
		{
			flag = TILEMAP_DRAW_OPAQUE;
			pri = 0;
		}
		else
		{
			flag = 0;
			pri = 1 << (plane - 1);
		}

		if(layer[plane] == 4)
		{
			K053936_1_zoom_draw(bitmap,cliprect,dbz_bg1_tilemap,flag,pri);
		}
		else if(layer[plane] == 5)
		{
			K053936_0_zoom_draw(bitmap,cliprect,dbz_bg2_tilemap,flag,pri);
		}
		else
		{
			K056832_tilemap_draw(machine, bitmap,cliprect,layer[plane],flag,pri);
		}
	}

	K053247_sprites_draw(machine, bitmap, cliprect);
	return 0;
}

