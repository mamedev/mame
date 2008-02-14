#include "driver.h"
#include "video/konamiic.h"

static int sprite_colorbase;
static int layer_colorbase[4], layerpri[4];
static int cur_alpha;

void xexex_set_alpha(int on)
{
	cur_alpha = on;
}

static void xexex_sprite_callback(int *code, int *color, int *priority_mask)
{
	int pri, c = *color;

	// Xexex doesn't seem to use bit8 and 9 as effect selectors so this should be safe.
	// (pdrawgfx() still needs change to fix Elaine's end-game graphics)
	pri = (c & 0x3e0) >> 4;

	if (pri <= layerpri[3])                       *priority_mask = 0; else
	if (pri >  layerpri[3] && pri <= layerpri[2]) *priority_mask = 0xff00; else
	if (pri >  layerpri[2] && pri <= layerpri[1]) *priority_mask = 0xff00|0xf0f0; else
	if (pri >  layerpri[1] && pri <= layerpri[0]) *priority_mask = 0xff00|0xf0f0|0xcccc; else
	*priority_mask = 0xff00|0xf0f0|0xcccc|0xaaaa;

	*color = sprite_colorbase | (c & 0x001f);
}

static void xexex_tile_callback(int layer, int *code, int *color, int *flags)
{
	*color = layer_colorbase[layer] | (*color>>2 & 0x0f);
}

VIDEO_START( xexex )
{
	int region = REGION_GFX3;

	assert(machine->screen[0].format == BITMAP_FORMAT_RGB32);

	cur_alpha = 0;

	K053251_vh_start();
	K054338_vh_start();
	K053250_vh_start(1, &region);

	K056832_vh_start(machine,REGION_GFX1, K056832_BPP_4, 1, NULL, xexex_tile_callback, 0);
	K053247_vh_start(machine,REGION_GFX2, -48, 32, NORMAL_PLANE_ORDER, xexex_sprite_callback);

	// Xexex has relative plane offsets of -2,2,4,6 vs. -2,0,2,3 in MW and GX.
	K056832_set_LayerOffset(0, -2, 16);
	K056832_set_LayerOffset(1,  2, 16);
	K056832_set_LayerOffset(2,  4, 16);
	K056832_set_LayerOffset(3,  6, 16);

	K053250_set_LayerOffset(0, -5,-16);

	K054338_invert_alpha(0);
}

/* useful function to sort the four tile layers by priority order */
/* suboptimal, but for such a size who cares ? */
static void sortlayers(int *layer, int *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
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

VIDEO_UPDATE( xexex )
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[4];
	int bg_colorbase, new_colorbase, plane, alpha;

	sprite_colorbase   = K053251_get_palette_index(K053251_CI0);
	bg_colorbase       = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = 0x70;

	for (plane=1; plane<4; plane++)
	{
		new_colorbase = K053251_get_palette_index(K053251_CI[plane]);
		if (layer_colorbase[plane] != new_colorbase)
		{
			layer_colorbase[plane] = new_colorbase;
			K056832_mark_plane_dirty(plane);
		}
	}

	layer[0] = 1;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	layer[1] = 2;
	layerpri[1] = K053251_get_priority(K053251_CI3);
	layer[2] = 3;
	layerpri[2] = K053251_get_priority(K053251_CI4);
	layer[3] = -1;
	layerpri[3] = K053251_get_priority(K053251_CI1);

	sortlayers(layer, layerpri);

	K054338_update_all_shadows(machine);
	K054338_fill_backcolor(machine, bitmap, 0);

	fillbitmap(priority_bitmap, 0, cliprect);

	for (plane=0; plane<4; plane++)
	{
		if (layer[plane] < 0)
		{
			K053250_draw(machine, bitmap, cliprect, 0, bg_colorbase, 0, 1<<plane);
		}
		else if (!cur_alpha || layer[plane] != 1)
		{
			K056832_tilemap_draw(machine, bitmap, cliprect, layer[plane], 0, 1<<plane);
		}
	}

	K053247_sprites_draw(machine, bitmap, cliprect);

	if (cur_alpha)
	{
		alpha = K054338_set_alpha_level(1);

		if (alpha > 0)
		{
			K056832_tilemap_draw(machine, bitmap, cliprect, 1, (alpha >= 255) ? 0 : TILEMAP_DRAW_ALPHA, 0);
		}
	}

	K056832_tilemap_draw(machine, bitmap, cliprect, 0, 0, 0);
	return 0;
}
