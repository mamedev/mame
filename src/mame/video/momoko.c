/*******************************************************************************

    Momoko 120% (c) 1986 Jaleco

    Video hardware driver by Uki

    02/Mar/2001 -

*******************************************************************************/

#include "emu.h"
#include "includes/momoko.h"


WRITE8_HANDLER ( momoko_fg_scrollx_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->fg_scrollx = data;
}

WRITE8_HANDLER ( momoko_fg_scrolly_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->fg_scrolly = data;
}

WRITE8_HANDLER ( momoko_fg_select_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->fg_select = data & 0x0f;
	state->fg_mask = data & 0x10;
}

WRITE8_HANDLER ( momoko_text_scrolly_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->text_scrolly = data;
}

WRITE8_HANDLER ( momoko_text_mode_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->text_mode = data;
}

WRITE8_HANDLER ( momoko_bg_scrollx_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->bg_scrollx[offset] = data;
}

WRITE8_HANDLER ( momoko_bg_scrolly_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->bg_scrolly[offset] = data;
}

WRITE8_HANDLER( momoko_bg_select_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->bg_select = data & 0x0f;
	state->bg_mask = data & 0x10;
}

WRITE8_HANDLER( momoko_bg_priority_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->bg_priority = data & 0x01;
}

WRITE8_HANDLER( momoko_flipscreen_w )
{
	momoko_state *state = space->machine->driver_data<momoko_state>();
	state->flipscreen = data & 0x01;
}

/****************************************************************************/

static void momoko_draw_bg_pri( running_machine *machine, bitmap_t *bitmap, int chr, int col, int flipx, int flipy, int x, int y, int pri )
{
	int xx, sx, sy, px, py, dot;
	UINT32 gfxadr;
	UINT8 d0, d1;
	UINT8 *BG_GFX = memory_region(machine, "gfx2");

	for (sy = 0; sy < 8; sy++)
	{
		gfxadr = chr * 16 + sy * 2;
		for (xx = 0; xx < 2; xx++)
		{
			d0 = BG_GFX[gfxadr + xx * 4096];
			d1 = BG_GFX[gfxadr + xx * 4096 + 1];

			for (sx = 0; sx < 4; sx++)
			{
				dot = (d0 & 0x08) | ((d0 & 0x80) >>5) | ((d1 & 0x08) >>2) | ((d1 & 0x80) >>7);
				if (flipx == 0) px = sx + xx * 4 + x;
					else      px = 7 - sx - xx * 4 + x;
				if (flipy == 0) py = sy + y;
					else      py = 7 - sy + y;

				if (dot >= pri)
					*BITMAP_ADDR16(bitmap, py, px) = col * 16 + dot + 256;

				d0 = d0 << 1;
				d1 = d1 << 1;
			}
		}
	}
}

/****************************************************************************/

VIDEO_UPDATE( momoko )
{
	momoko_state *state = screen->machine->driver_data<momoko_state>();
	int x, y, dx, dy, rx, ry, radr, chr, sy, fx, fy, px, py, offs, col, pri, flip ;
	UINT8 *spriteram = state->spriteram;

	UINT8 *BG_MAP     = memory_region(screen->machine, "user1");
	UINT8 *BG_COL_MAP = memory_region(screen->machine, "user2");
	UINT8 *FG_MAP     = memory_region(screen->machine, "user3");
	UINT8 *TEXT_COLOR = memory_region(screen->machine, "proms");


	flip = state->flipscreen ^ (input_port_read(screen->machine, "FAKE") & 0x01);

	/* draw BG layer */
	dx = (7 - state->bg_scrollx[0]) & 7;
	dy = (7 - state->bg_scrolly[0]) & 7;
	rx = (state->bg_scrollx[0] + state->bg_scrollx[1] * 256) >> 3;
	ry = (state->bg_scrolly[0] + state->bg_scrolly[1] * 256) >> 3;

	if (state->bg_mask == 0)
	{
		for (y = 0; y < 29; y++)
		{
			for (x = 0; x < 32; x++)
			{
				radr = ((ry + y + 2) & 0x3ff) * 128 + ((rx + x) & 0x7f);
				chr = BG_MAP[radr];
				col = BG_COL_MAP[chr + state->bg_select * 512 + state->bg_priority * 256] & 0x0f;
				chr = chr + state->bg_select * 512;

				if (flip == 0)
				{
					px = 8 * x + dx - 6;
					py = 8 * y + dy + 9;
				}
				else
				{
					px = 248 - (8 * x + dx - 8);
					py = 248 - (8 * y + dy + 9);
				}

				drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[1],
					chr,
					col,
					flip,flip,
					px,py);
			}
		}
	}
	else
	bitmap_fill(bitmap, cliprect, 256);


	/* draw sprites (momoko) */
	for (offs = 0; offs < 9 * 4; offs +=4)
	{
		chr = spriteram[offs + 1] | ((spriteram[offs + 2] & 0x60) << 3);
		chr = ((chr & 0x380) << 1) | (chr & 0x7f);
		col = spriteram[offs + 2] & 0x07;
		fx = ((spriteram[offs + 2] & 0x10) >> 4) ^ flip;
		fy = ((spriteram[offs + 2] & 0x08) >> 3) ^ flip; /* ??? */
		x = spriteram[offs + 3];
		y = spriteram[offs + 0];

		if (flip == 0)
		{
			px = x;
			py = 239 - y;
		}
		else
		{
			px = 248 - x;
			py = y + 1;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[3],
			chr,
			col,
			!fx,fy,
			px,py,0);
	}


	/* draw BG layer */
	if (state->bg_mask ==0)
	{
		for (y = 0; y < 29; y++)
		{
			for (x = 0; x < 32; x++)
			{
				radr = ((ry + y + 2) & 0x3ff) * 128 + ((rx + x) & 0x7f) ;
				chr = BG_MAP[radr] ;
				col = BG_COL_MAP[chr + state->bg_select * 512 + state->bg_priority * 256];
				pri = (col & 0x10) >> 1;

				if (flip == 0)
				{
					px = 8 * x + dx - 6;
					py = 8 * y + dy + 9;
				 }
				else
				{
					px = 248 - (8 * x + dx - 8);
					py = 248 - (8 * y + dy + 9);
				}
				if (pri != 0)
				{
					col = col & 0x0f;
					chr = chr + state->bg_select * 512;
					momoko_draw_bg_pri(screen->machine, bitmap, chr, col, flip, flip, px, py, pri);
				}
			}
		}
	}


	/* draw sprites (others) */
	for (offs = 9 * 4; offs < state->spriteram_size; offs += 4)
	{
		chr = spriteram[offs + 1] | ((spriteram[offs + 2] & 0x60) << 3);
		chr = ((chr & 0x380) << 1) | (chr & 0x7f);
		col = spriteram[offs + 2] & 0x07;
		fx = ((spriteram[offs + 2] & 0x10) >> 4) ^ flip;
		fy = ((spriteram[offs + 2] & 0x08) >> 3) ^ flip; /* ??? */
		x = spriteram[offs + 3];
		y = spriteram[offs + 0];

		if (flip == 0)
		{
			px = x;
			py = 239 - y;
		}
		else
		{
			px = 248 - x;
			py = y + 1;
		}
		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[3],
			chr,
			col,
			!fx,fy,
			px,py,0);
	}


	/* draw text layer */
	for (y = 16; y < 240; y++)
	{
		for (x = 0; x < 32; x++)
		{
			sy = y;
			if (state->text_mode == 0)
				col = TEXT_COLOR[(sy >> 3) + 0x100] & 0x0f;
			else
			{
				if (TEXT_COLOR[y] < 0x08)
					sy += state->text_scrolly;
				col = (TEXT_COLOR[y] & 0x07) + 0x10;
			}
			dy = sy & 7;
			if (flip == 0)
			{
				px = x * 8;
				py = y;
			}
			else
			{
				px = 248 - x * 8;
				py = 255 - y;
			}
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
				state->videoram[(sy >> 3) * 32 + x] * 8 + dy,
				col,
				flip,0,
				px,py,0);
		}
	}


	/* draw FG layer */
	if (state->fg_mask == 0)
	{
		dx = (7 - state->fg_scrollx) & 7;
		dy = (7 - state->fg_scrolly) & 7;
		rx = state->fg_scrollx >> 3;
		ry = state->fg_scrolly >> 3;

		for (y = 0; y < 29; y++)
		{
			for (x = 0; x < 32; x++)
			{
				radr = ((ry + y + 34) & 0x3f) * 0x20 + ((rx + x) & 0x1f) + (state->fg_select & 3) * 0x800;
				chr = FG_MAP[radr] ;
				if (flip == 0)
				{
					px = 8 * x + dx - 6;
					py = 8 * y + dy + 9;
				}
				else
				{
					px = 248 - (8 * x + dx - 8);
					py = 248 - (8 * y + dy + 9);
				}
				drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
					chr,
					0, /* color */
					flip,flip, /* flip */
					px,py,0);
			}
		}
	}
	return 0;
}
