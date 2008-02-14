/***************************************************************************

 Bishi Bashi Champ Mini Game Senshuken
 (c) 1996 Konami

 Video hardware emulation.

***************************************************************************/

#include "driver.h"
#include "video/konamiic.h"
#include "machine/konamigx.h"

static int layer_colorbase[4], layerpri[4];

static void bishi_tile_callback(int layer, int *code, int *color, int *flags)
{
//  *code -= '0';
//  *color = layer_colorbase[layer] | (*color>>2 & 0x0f);
//  K055555GX_decode_vmixcolor(layer, color);
//  if (*color) mame_printf_debug("plane %x col %x [55 %x %x]\n", layer, *color, layer_colorbase[layer], K055555_get_palette_index(layer));

	*color = layer_colorbase[layer] + ((*color & 0xf0));
}

VIDEO_START(bishi)
{
	assert(machine->screen[0].format == BITMAP_FORMAT_RGB32);

	K055555_vh_start();
	K054338_vh_start();

	K056832_vh_start(machine, REGION_GFX1, K056832_BPP_8, 1, NULL, bishi_tile_callback, 0);

	K056832_set_LayerAssociation(0);

	K056832_set_LayerOffset(0, -2, 0);
	K056832_set_LayerOffset(1,  2, 0);
	K056832_set_LayerOffset(2,  4, 0);
	K056832_set_LayerOffset(3,  6, 0);

	// the 55555 is set to "0x10, 0x11, 0x12, 0x13", but these values are almost correct...
	layer_colorbase[0] = 0x00;
	layer_colorbase[1] = 0x40;	// this one is wrong
	layer_colorbase[2] = 0x80;
	layer_colorbase[3] = 0xc0;
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
	SWAP(0,3)
	SWAP(1,2)
	SWAP(1,3)
	SWAP(2,3)
}

VIDEO_UPDATE(bishi)
{
	int layers[4], i;/*, old;*/
/*  int bg_colorbase, new_colorbase, plane, dirty; */
	static const int pris[4] = { K55_PRIINP_0, K55_PRIINP_3, K55_PRIINP_6, K55_PRIINP_7 };
	static const int enables[4] = { K55_INP_VRAM_A, K55_INP_VRAM_B, K55_INP_VRAM_C, K55_INP_VRAM_D };

	K054338_update_all_shadows(machine);
	K054338_fill_backcolor(machine, bitmap, 0);

	for (i = 0; i < 4; i++)
	{
		layers[i] = i;
		layerpri[i] = K055555_read_register(pris[i]);
	}

	sortlayers(layers, layerpri);

	fillbitmap(priority_bitmap,0,cliprect);

	for (i = 0; i < 4; i++)
	{
		if (K055555_read_register(K55_INPUT_ENABLES) & enables[layers[i]])
		{
			K056832_tilemap_draw(machine, bitmap, cliprect, layers[i], 0, 1<<i);
		}
	}
	return 0;
}
