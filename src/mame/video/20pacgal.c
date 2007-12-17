#include "driver.h"



UINT8 *pacgal_charram,*pacgal_sprram;
UINT8 *pacgal_videoram,*pacgal_videoram2;

static mame_bitmap *chr_bitmap;
static int palbank;
static int active_game;



static void switch_palette(running_machine *machine)
{
	int i;
	UINT8 *color_prom = memory_region(REGION_PROMS) + 0x1000 * palbank;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

PALETTE_INIT( 20pacgal )
{
	palbank = 0;
	switch_palette(machine);
}



VIDEO_START( 20pacgal )
{
	chr_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
}



WRITE8_HANDLER( pacgal_lookup_w )
{
	/* palette hacks! */
	((UINT16 *)Machine->game_colortable)[offset] = data & 0x0f;
	((pen_t *)Machine->remapped_colortable)[offset] = Machine->pens[data & 0x0f];
}

WRITE8_HANDLER( pacgal_active_game_w )
{
	active_game = data & 1;

	if (!active_game)
		memcpy(pacgal_videoram2,memory_region(REGION_CPU1)+0x8000,0x2000);

	if (palbank != active_game)
	{
		palbank = active_game;
		switch_palette(Machine);
	}
}

WRITE8_HANDLER( pacgal_videoram2_w )
{
	if (active_game)
		pacgal_videoram2[offset] = data;
}

WRITE8_HANDLER( pacgal_charram_w )
{
	pacgal_charram[offset] = data;

	decodechar(Machine->gfx[0],offset/16,pacgal_charram,Machine->drv->gfxdecodeinfo[0].gfxlayout);
}

WRITE8_HANDLER( pacgal_sprram_w )
{
	offset = (offset & 0x1f83) | ((offset & 0x078) >> 1) | ((offset & 0x004) << 4);
	pacgal_sprram[offset] = data;

	decodechar(Machine->gfx[1],offset/64,pacgal_sprram,Machine->drv->gfxdecodeinfo[1].gfxlayout);
}



static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;


	for (offs = 0x80-2;offs >= 0;offs-=2)
	{
		static int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs + 0x000] & 0x7f;
		int color = spriteram[offs + 0x001] & 0x3f;
		int sx = spriteram[offs + 0x081] - 41 + 0x100*(spriteram[offs + 0x101] & 3);
		int sy = 256 - spriteram[offs + 0x080] + 1;
		int flipx = (spriteram[offs + 0x100] & 0x01);
		int flipy = (spriteram[offs + 0x100] & 0x02) >> 1;
		int sizex = (spriteram[offs + 0x100] & 0x04) >> 2;
		int sizey = (spriteram[offs + 0x100] & 0x08) >> 3;
		int x,y;

		if (flip_screen)
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;	// fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				drawgfx(bitmap,machine->gfx[1],
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x, sy + 16*y,
					cliprect,TRANSPARENCY_COLOR,0);
			}
		}
	}
}

VIDEO_UPDATE( 20pacgal )
{
	int x,y;

	for (y = 0;y < 32;y++)
	{
		for (x = 0;x < 32;x++)
		{
			int sx,sy;
			if (y < 2)
			{
				sy = x-2;
				sx = 34+y;
			}
			else if (y >= 30)
			{
				sy = x-2;
				sx = y-30;
			}
			else
			{
				sy = y-2;
				sx = x+2;
			}

			if (active_game)
			{
				drawgfx(chr_bitmap,machine->gfx[0],
						pacgal_videoram2[y*32 + x] + 0xa00,
						(pacgal_videoram2[0x400 + y*32 + x] & 0x3f) * 4,
						0,0,
						8*sx,8*sy,
						cliprect,TRANSPARENCY_NONE_RAW,0);
			}
			else
			{
				drawgfx(chr_bitmap,machine->gfx[0],
						pacgal_videoram[y*32 + x] + 0xa00,
						(pacgal_videoram[0x400 + y*32 + x] & 0x3f) * 4,
						0,0,
						8*sx,8*sy,
						cliprect,TRANSPARENCY_NONE_RAW,0);
			}
		}
	}

	fillbitmap(bitmap,0,NULL);

	draw_sprites(machine,bitmap,cliprect);

	copybitmap(bitmap, chr_bitmap, flip_screen, flip_screen, 0, 0, cliprect, TRANSPARENCY_BLEND_RAW, 4);

	return 0;
}
