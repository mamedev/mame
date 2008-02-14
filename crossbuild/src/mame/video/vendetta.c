#include "driver.h"
#include "video/konamiic.h"


static int layer_colorbase[3],bg_colorbase,sprite_colorbase;
static int layerpri[3];


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void vendetta_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x30) << 6) |
			((*color & 0x0c) << 10) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

static void esckids_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) |
			((*color & 0x0c) <<  9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >>  5);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority_mask)
{
	int pri = (*color & 0x03e0) >> 4;	/* ??????? */
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x001f);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( vendetta )
{
	K053251_vh_start();

	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,vendetta_tile_callback);
	K053247_vh_start(machine,REGION_GFX2,53,6,NORMAL_PLANE_ORDER,sprite_callback);
}

VIDEO_START( esckids )
{
    K053251_vh_start();

    K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,esckids_tile_callback);
	K053247_vh_start(machine,REGION_GFX2,101,6,NORMAL_PLANE_ORDER,sprite_callback);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* useful function to sort the three tile layers by priority order */
static void sortlayers(int *layer,int *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

VIDEO_UPDATE( vendetta )
{
	int layer[3];


	bg_colorbase       = K053251_get_palette_index(K053251_CI0);
	sprite_colorbase   = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = K053251_get_palette_index(K053251_CI2);
	layer_colorbase[1] = K053251_get_palette_index(K053251_CI3);
	layer_colorbase[2] = K053251_get_palette_index(K053251_CI4);

	K052109_tilemap_update();

	layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI3);
	layer[2] = 2;
	layerpri[2] = K053251_get_priority(K053251_CI4);

	sortlayers(layer,layerpri);

	fillbitmap(priority_bitmap,0,cliprect);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[layer[0]],TILEMAP_DRAW_OPAQUE,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[layer[1]],0,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[layer[2]],0,4);

	K053247_sprites_draw(machine, bitmap,cliprect);
	return 0;
}
