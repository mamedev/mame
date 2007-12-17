/*
 *  Beatmania DJ Main Board (GX753)
 *  emulate video hardware
 */

#include "driver.h"
#include "video/konamiic.h"

#define NUM_SPRITES	(0x800 / 16)
#define NUM_LAYERS	2

UINT32 *djmain_obj_ram;


static void draw_sprites(running_machine* machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs, pri_code;
	int sortedlist[NUM_SPRITES];

	machine->gfx[0]->color_base = K055555_read_register(K55_PALBASE_SUB2) * 0x400;

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0; offs < NUM_SPRITES * 4; offs += 4)
	{
		if (djmain_obj_ram[offs] & 0x00008000)
		{
			if (djmain_obj_ram[offs] & 0x80000000)
				continue;

			pri_code = djmain_obj_ram[offs] & (NUM_SPRITES - 1);
			sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		static int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		static int sizetab[4] =  { 1, 2, 4, 8 };
		int x, y;
		int ox, oy;
		int flipx, flipy;
		int xscale, yscale;
		int code;
		int color;
		int size;

		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		code = djmain_obj_ram[offs] >> 16;
		flipx = (djmain_obj_ram[offs] >> 10) & 1;
		flipy = (djmain_obj_ram[offs] >> 11) & 1;
		size = sizetab[(djmain_obj_ram[offs] >> 8) & 3];

		ox = (INT16)(djmain_obj_ram[offs + 1] & 0xffff);
		oy = (INT16)(djmain_obj_ram[offs + 1] >> 16);

		xscale = djmain_obj_ram[offs + 2] >> 16;
		yscale = djmain_obj_ram[offs + 2] & 0xffff;

		if (!xscale || !yscale)
			continue;

		xscale = (0x40 << 16) / xscale;
		yscale = (0x40 << 16) / yscale;
		ox -= (size * xscale) >> 13;
		oy -= (size * yscale) >> 13;

		color = (djmain_obj_ram[offs + 3] >> 16) & 15;

		for (x = 0; x < size; x++)
			for (y = 0; y < size; y++)
			{
				int c = code;

				if (flipx)
					c += xoffset[size - x - 1];
				else
					c += xoffset[x];

				if (flipy)
					c += yoffset[size - y - 1];
				else
					c += yoffset[y];

				if (xscale != 0x10000 || yscale != 0x10000)
				{
					int sx = ox + ((x * xscale + (1 << 11)) >> 12);
					int sy = oy + ((y * yscale + (1 << 11)) >> 12);
					int zw = ox + (((x + 1) * xscale + (1 << 11)) >> 12) - sx;
					int zh = oy + (((y + 1) * yscale + (1 << 11)) >> 12) - sy;

					drawgfxzoom(bitmap,
					            machine->gfx[0],
					            c,
					            color,
					            flipx,
					            flipy,
					            sx,
					            sy,
					            cliprect,
					            TRANSPARENCY_PEN,
					            0,
					            (zw << 16) / 16,
					            (zh << 16) / 16);
				}
				else
				{
					int sx = ox + (x << 4);
					int sy = oy + (y << 4);

					drawgfx(bitmap,
					        machine->gfx[0],
					        c,
					        color,
					        flipx,
					        flipy,
					        sx,
					        sy,
					        cliprect,
					        TRANSPARENCY_PEN,
					        0);
				}
			}
	}
}


static void game_tile_callback(int layer, int *code, int *color, int *flags)
{
}

VIDEO_START( djmain )
{
	static int scrolld[NUM_LAYERS][4][2] = {
	 	{{ 0, 0}, {0, 0}, {0, 0}, {0, 0}},
	 	{{ 0, 0}, {0, 0}, {0, 0}, {0, 0}}
	};

	K056832_vh_start(machine, REGION_GFX2, K056832_BPP_4dj, 1, scrolld, game_tile_callback, 1);
	K055555_vh_start();

	K056832_set_LayerOffset(0, -92, -27);
	// K056832_set_LayerOffset(1, -87, -27);
	K056832_set_LayerOffset(1, -88, -27);
}

VIDEO_UPDATE( djmain )
{
	int enables = K055555_read_register(K55_INPUT_ENABLES);
	int pri[NUM_LAYERS + 1];
	int order[NUM_LAYERS + 1];
	int i, j;

	for (i = 0; i < NUM_LAYERS; i++)
		pri[i] = K055555_read_register(K55_PRIINP_0 + i * 3);
	pri[i] = K055555_read_register(K55_PRIINP_10);

	for (i = 0; i < NUM_LAYERS + 1; i++)
		order[i] = i;

	for (i = 0; i < NUM_LAYERS; i++)
		for (j = i + 1; j < NUM_LAYERS + 1; j++)
			if (pri[order[i]] > pri[order[j]])
			{
				int temp = order[i];

				order[i] = order[j];
				order[j] = temp;
			}

	fillbitmap(bitmap, machine->remapped_colortable[0], cliprect);

	for (i = 0; i < NUM_LAYERS + 1; i++)
	{
		int layer = order[i];

		if (layer == NUM_LAYERS)
		{
			if (enables & K55_INP_SUB2)
				draw_sprites(machine, bitmap, cliprect);
		}
		else
		{
			if (enables & (K55_INP_VRAM_A << layer))
				K056832_tilemap_draw_dj(machine, bitmap, cliprect, layer, 0, 1 << i);
		}
	}
	return 0;
}
