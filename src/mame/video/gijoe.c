#include "driver.h"
#include "video/konamiic.h"


static int AVAC_bits[4], AVAC_occupancy[4];
static int layer_colorbase[4], layer_pri[4];
static int AVAC_vrc, sprite_colorbase;

static void gijoe_sprite_callback(int *code, int *color, int *priority_mask)
{
	int pri, c = *color;

	pri = (c & 0x03e0) >> 4;

	if (pri <= layer_pri[3])                        *priority_mask = 0; else
	if (pri >  layer_pri[3] && pri <= layer_pri[2]) *priority_mask = 0xff00; else
	if (pri >  layer_pri[2] && pri <= layer_pri[1]) *priority_mask = 0xff00|0xf0f0; else
	if (pri >  layer_pri[1] && pri <= layer_pri[0]) *priority_mask = 0xff00|0xf0f0|0xcccc; else
	*priority_mask = 0xff00|0xf0f0|0xcccc|0xaaaa;

	*color = sprite_colorbase | (c & 0x001f);
}

static void gijoe_tile_callback(int layer, int *code, int *color, int *flags)
{
	int tile = *code;

	if (tile >= 0xf000 && tile <= 0xf4ff)
	{
		tile &= 0x0fff;
		if (tile < 0x0310) { AVAC_occupancy[layer] |= 0x0f00; tile |= AVAC_bits[0]; } else
		if (tile < 0x0470) { AVAC_occupancy[layer] |= 0xf000; tile |= AVAC_bits[1]; } else
		                   { AVAC_occupancy[layer] |= 0x00f0; tile |= AVAC_bits[2]; }
		*code = tile;
	}

	*color = (*color>>2 & 0x0f) | layer_colorbase[layer];
}

VIDEO_START( gijoe )
{
	int i;

	K053251_vh_start();
	K056832_vh_start(machine, REGION_GFX1, K056832_BPP_4, 1, NULL, gijoe_tile_callback, 0);

	K056832_linemap_enable(1);

	K053247_vh_start(machine, REGION_GFX2, -37, 20, NORMAL_PLANE_ORDER, gijoe_sprite_callback);

	for (i=0; i<4; i++) AVAC_occupancy[i] = 0;
	AVAC_vrc = 0xffff;
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
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(2, 3)
}

VIDEO_UPDATE( gijoe )
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[4];
	int vrc_mode, vrc_new, colorbase_new, primode, dirty, i;
	int mask = 0;

	// update tile offsets
	K056832_read_AVAC(&vrc_mode, &vrc_new);

	if (vrc_mode)
	{
		for (dirty=0xf000; dirty; dirty>>=4) if ((AVAC_vrc & dirty) != (vrc_new & dirty)) mask |= dirty;

		AVAC_vrc = vrc_new;
		AVAC_bits[0] = vrc_new<<4  & 0xf000;
		AVAC_bits[1] = vrc_new     & 0xf000;
		AVAC_bits[2] = vrc_new<<8  & 0xf000;
		AVAC_bits[3] = vrc_new<<12 & 0xf000;
	}
	else
		AVAC_bits[3] = AVAC_bits[2] = AVAC_bits[1] = AVAC_bits[0] = 0xf000;

	// update color info and refresh tilemaps
	sprite_colorbase = K053251_get_palette_index(K053251_CI0);

	for (i=0; i<4; i++)
	{
		dirty = 0;

		colorbase_new = K053251_get_palette_index(K053251_CI[i]);
		if (layer_colorbase[i] != colorbase_new) { layer_colorbase[i] = colorbase_new; dirty = 1; }
		if (AVAC_occupancy[i] & mask) dirty = 1;

		if (dirty) { AVAC_occupancy[i] = 0; K056832_mark_plane_dirty(i); }
	}

	/*
        Layer A is supposed to be a non-scrolling status display with static X-offset.
        The weird thing is tilemap alignment only follows the 832 standard when 2 is
        written to the layer's X-scroll register otherwise the chip expects totally
        different alignment values.
    */
	if (K056832_read_register(0x14) == 2)
	{
		K056832_set_LayerOffset(0,  2, 0);
		K056832_set_LayerOffset(1,  4, 0);
		K056832_set_LayerOffset(2,  6, 0); // 7?
		K056832_set_LayerOffset(3,  8, 0);
	}
	else
	{
		K056832_set_LayerOffset(0,  0, 0);
		K056832_set_LayerOffset(1,  8, 0);
		K056832_set_LayerOffset(2, 14, 0);
		K056832_set_LayerOffset(3, 16, 0); // smaller?
	}

	// seems to switch the K053251 between different priority modes, detail unknown
	primode = K053251_get_priority(K053251_CI1);

	layer[0] = 0;
	layer_pri[0] = 0; // not sure
	layer[1] = 1;
	layer_pri[1] = K053251_get_priority(K053251_CI2);
	layer[2] = 2;
	layer_pri[2] = K053251_get_priority(K053251_CI3);
	layer[3] = 3;
	layer_pri[3] = K053251_get_priority(K053251_CI4);

	sortlayers(layer, layer_pri);

	fillbitmap(bitmap, get_black_pen(machine), cliprect);
	fillbitmap(priority_bitmap, 0, cliprect);

	K056832_tilemap_draw(machine, bitmap,cliprect, layer[0], 0, 1);
	K056832_tilemap_draw(machine, bitmap,cliprect, layer[1], 0, 2);
	K056832_tilemap_draw(machine, bitmap,cliprect, layer[2], 0, 4);
	K056832_tilemap_draw(machine, bitmap,cliprect, layer[3], 0, 8);

	K053247_sprites_draw(machine, bitmap, cliprect);
	return 0;
}
