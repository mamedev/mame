#include "driver.h"
#include "video/konamiic.h"


static int sprite_colorbase;
static int layer_colorbase[4], layerpri[3];
static UINT16 spritebank;
static int tilebanks[4];
static int spritebanks[4];

static void reset_spritebank(void)
{
	K053244_bankselect(0, spritebank & 7);
	spritebanks[0] = (spritebank << 12) & 0x7000;
	spritebanks[1] = (spritebank <<  9) & 0x7000;
	spritebanks[2] = (spritebank <<  6) & 0x7000;
	spritebanks[3] = (spritebank <<  3) & 0x7000;
}

WRITE16_HANDLER( asterix_spritebank_w )
{
	COMBINE_DATA(&spritebank);
	reset_spritebank();
}

static void asterix_sprite_callback(int *code, int *color, int *priority_mask)
{
	int pri = (*color & 0x00e0) >> 2;
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;
	*color = sprite_colorbase | (*color & 0x001f);
	*code = (*code & 0xfff) | spritebanks[(*code >> 12) & 3];
}


static void asterix_tile_callback(int layer, int *code, int *color, int *flags)
{
	*flags = *code & 0x1000 ? TILE_FLIPX : 0;
	*color = (layer_colorbase[layer] + ((*code & 0xe000) >> 13)) & 0x7f;
	*code = (*code & 0x03ff) | tilebanks[(*code >> 10) & 3];
}

VIDEO_START( asterix )
{
	K053251_vh_start(machine);
	K056832_vh_start(machine, "gfx1", K056832_BPP_4, 1, NULL, asterix_tile_callback, 1);
	K053245_vh_start(machine,0, "gfx2",NORMAL_PLANE_ORDER, asterix_sprite_callback);

	K053245_set_SpriteOffset(0,-3,-1);
}

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

VIDEO_UPDATE( asterix )
{
	static const int K053251_CI[4] = { K053251_CI0, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[3], plane, new_colorbase;

	/* Layer offsets are different if horizontally flipped */
	if (K056832_read_register(0x0) & 0x10)
	{
		K056832_set_LayerOffset(0, 89-176, 0);
		K056832_set_LayerOffset(1, 91-176, 0);
		K056832_set_LayerOffset(2, 89-176, 0);
		K056832_set_LayerOffset(3, 95-176, 0);
	}
	else
	{
		K056832_set_LayerOffset(0, 89, 0);
		K056832_set_LayerOffset(1, 91, 0);
		K056832_set_LayerOffset(2, 89, 0);
		K056832_set_LayerOffset(3, 95, 0);
	}


	tilebanks[0] = (K056832_get_lookup(0) << 10);
	tilebanks[1] = (K056832_get_lookup(1) << 10);
	tilebanks[2] = (K056832_get_lookup(2) << 10);
	tilebanks[3] = (K056832_get_lookup(3) << 10);

	// update color info and refresh tilemaps
	sprite_colorbase = K053251_get_palette_index(K053251_CI1);

	for (plane=0; plane<4; plane++)
	{
		new_colorbase = K053251_get_palette_index(K053251_CI[plane]);
		if (layer_colorbase[plane] != new_colorbase)
		{
			layer_colorbase[plane] = new_colorbase;
			K056832_mark_plane_dirty(plane);
		}
	}

	layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI0);
	layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI2);
	layer[2] = 3;
	layerpri[2] = K053251_get_priority(K053251_CI4);

	sortlayers(layer, layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	K056832_tilemap_draw(screen->machine, bitmap, cliprect, layer[0], 0, 1);
	K056832_tilemap_draw(screen->machine, bitmap, cliprect, layer[1], 0, 2);
	K056832_tilemap_draw(screen->machine, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows in front of feet */
	K053245_sprites_draw(screen->machine, 0, bitmap, cliprect);

	K056832_tilemap_draw(screen->machine, bitmap, cliprect, 2, 0, 0);
	return 0;
}
