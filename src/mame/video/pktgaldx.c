#include "emu.h"
#include "video/deco16ic.h"
#include "includes/pktgaldx.h"

static void draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	pktgaldx_state *state = machine->driver_data<pktgaldx_state>();
	UINT16 *spriteram = state->spriteram;
	int offs;
	int flipscreen = !flip_screen_get(machine);

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = spriteram[offs+1];
		if (!sprite)
			continue;

		y = spriteram[offs];
		flash = y & 0x1000;
		if (flash && (machine->primary_screen->frame_number() & 1))
			continue;

		x = spriteram[offs + 2];
		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		if (x > 320)
			continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

/* Video on the orginal */

VIDEO_UPDATE( pktgaldx )
{
	pktgaldx_state *state = screen->machine->driver_data<pktgaldx_state>();
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);

	bitmap_fill(bitmap, cliprect, 0); /* not Confirmed */
	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);

	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}

/* Video for the bootleg */

VIDEO_UPDATE( pktgaldb )
{
	pktgaldx_state *state = screen->machine->driver_data<pktgaldx_state>();
	int x, y;
	int offset = 0;
	int tileno;
	int colour;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* the bootleg seems to treat the tilemaps as sprites */
	for (offset = 0; offset < 0x1600 / 2; offset += 8)
	{
		tileno = state->pktgaldb_sprites[offset + 3] | (state->pktgaldb_sprites[offset + 2] << 16);
		colour = state->pktgaldb_sprites[offset + 1] >> 1;
		x = state->pktgaldb_sprites[offset + 0];
		y = state->pktgaldb_sprites[offset + 4];

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0], tileno ^ 0x1000, colour, 0, 0, x, y, 0);
	}

	for (offset = 0x1600/2; offset < 0x2000 / 2; offset += 8)
	{
		tileno = state->pktgaldb_sprites[offset + 3] | (state->pktgaldb_sprites[offset + 2] << 16);
		colour = state->pktgaldb_sprites[offset + 1] >> 1;
		x = state->pktgaldb_sprites[offset + 0] & 0x1ff;
		y = state->pktgaldb_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0], tileno ^ 0x4000, colour, 0, 0, x, y, 0);
	}

	for (offset = 0x2000/2; offset < 0x4000 / 2; offset += 8)
	{
		tileno = state->pktgaldb_sprites[offset + 3] | (state->pktgaldb_sprites[offset + 2] << 16);
		colour = state->pktgaldb_sprites[offset + 1] >> 1;
		x = state->pktgaldb_sprites[offset + 0] & 0x1ff;
		y = state->pktgaldb_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0], tileno ^ 0x3000, colour, 0, 0, x, y, 0);
	}

	return 0;
}
