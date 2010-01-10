/* Aquarium */

#include "emu.h"
#include "includes/aquarium.h"

/* gcpinbal.c modified */
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int y_offs )
{
	aquarium_state *state = (aquarium_state *)machine->driver_data;
	int offs, chain_pos;
	int x, y, curx, cury;
	UINT8 col, flipx, flipy, chain;
	UINT16 code;

	for (offs = 0; offs < state->spriteram_size / 2; offs += 8)
	{
		code = ((state->spriteram[offs + 5]) & 0xff) + (((state->spriteram[offs + 6]) & 0xff) << 8);
		code &= 0x3fff;

		if (!(state->spriteram[offs + 4] &0x80))	/* active sprite ? */
		{
			x = ((state->spriteram[offs + 0]) &0xff) + (((state->spriteram[offs + 1]) & 0xff) << 8);
			y = ((state->spriteram[offs + 2]) &0xff) + (((state->spriteram[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col = ((state->spriteram[offs + 7]) & 0x0f);
			chain = (state->spriteram[offs + 4]) & 0x07;
			flipy = (state->spriteram[offs + 4]) & 0x10;
			flipx = (state->spriteram[offs + 4]) & 0x20;

			curx = x;
			cury = y;

			if (((state->spriteram[offs + 4]) & 0x08) && flipy)
				cury += (chain * 16);

			if (!(((state->spriteram[offs + 4]) & 0x08)) && flipx)
				curx += (chain * 16);


			for (chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				drawgfx_transpen(bitmap, cliprect,machine->gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury,0);

				/* wrap around y */
				drawgfx_transpen(bitmap, cliprect,machine->gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury + 256,0);

				code++;

				if ((state->spriteram[offs + 4]) &0x08)	/* Y chain */
				{
					if (flipy)
						cury -= 16;
					else
						cury += 16;
				}
				else	/* X chain */
				{
					if (flipx)
						curx -= 16;
					else
						curx += 16;
				}
			}
		}
	}
#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf, "sprite rotate offs %04x ?", rotate);
		popmessage(buf);
	}
#endif
}

/* TXT Layer */
static TILE_GET_INFO( get_aquarium_txt_tile_info )
{
	aquarium_state *state = (aquarium_state *)machine->driver_data;
	int tileno, colour;

	tileno = (state->txt_videoram[tile_index] & 0x0fff);
	colour = (state->txt_videoram[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO(2, tileno, colour, 0);
}

WRITE16_HANDLER( aquarium_txt_videoram_w )
{
	aquarium_state *state = (aquarium_state *)space->machine->driver_data;
	state->txt_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->txt_tilemap, offset);
}

/* MID Layer */
static TILE_GET_INFO( get_aquarium_mid_tile_info )
{
	aquarium_state *state = (aquarium_state *)machine->driver_data;
	int tileno, colour, flag;

	tileno = (state->mid_videoram[tile_index * 2] & 0x0fff);
	colour = (state->mid_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((state->mid_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO(1, tileno, colour, flag);

	tileinfo->category = (state->mid_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_HANDLER( aquarium_mid_videoram_w )
{
	aquarium_state *state = (aquarium_state *)space->machine->driver_data;
	state->mid_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->mid_tilemap, offset / 2);
}

/* BAK Layer */
static TILE_GET_INFO( get_aquarium_bak_tile_info )
{
	aquarium_state *state = (aquarium_state *)machine->driver_data;
	int tileno, colour, flag;

	tileno = (state->bak_videoram[tile_index * 2] & 0x0fff);
	colour = (state->bak_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((state->bak_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO(3, tileno, colour, flag);

	tileinfo->category = (state->bak_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_HANDLER( aquarium_bak_videoram_w )
{
	aquarium_state *state = (aquarium_state *)space->machine->driver_data;
	state->bak_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bak_tilemap, offset / 2);
}

VIDEO_START(aquarium)
{
	aquarium_state *state = (aquarium_state *)machine->driver_data;
	state->txt_tilemap = tilemap_create(machine, get_aquarium_txt_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->bak_tilemap = tilemap_create(machine, get_aquarium_bak_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->mid_tilemap = tilemap_create(machine, get_aquarium_mid_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->txt_tilemap, 0);
	tilemap_set_transparent_pen(state->mid_tilemap, 0);
}

VIDEO_UPDATE(aquarium)
{
	aquarium_state *state = (aquarium_state *)screen->machine->driver_data;
	tilemap_set_scrollx(state->mid_tilemap, 0, state->scroll[0]);
	tilemap_set_scrolly(state->mid_tilemap, 0, state->scroll[1]);
	tilemap_set_scrollx(state->bak_tilemap, 0, state->scroll[2]);
	tilemap_set_scrolly(state->bak_tilemap, 0, state->scroll[3]);
	tilemap_set_scrollx(state->txt_tilemap, 0, state->scroll[4]);
	tilemap_set_scrolly(state->txt_tilemap, 0, state->scroll[5]);

	tilemap_draw(bitmap, cliprect, state->bak_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->mid_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect, 16);

	tilemap_draw(bitmap, cliprect, state->bak_tilemap, 1, 0);
	tilemap_draw(bitmap, cliprect, state->mid_tilemap, 1, 0);
	tilemap_draw(bitmap, cliprect, state->txt_tilemap, 0, 0);
	return 0;
}
