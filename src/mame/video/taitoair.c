/***************************************************************************

Functions to emulate the video hardware of the machine.

TODO
====

Harmonise draw_sprites code and offload it into taitoic.c, leaving this
as short as possible (TC0080VCO has sprites as well as tilemaps in its
address space).

Maybe wait until the Taito Air system is done - also uses TC0080VCO.

Why does syvalion draw_sprites ignore the zoomy value
(using zoomx instead) ???


Sprite ram notes
----------------

BG / chain RAM
-----------------------------------------
[0]  +0         +1
     -xxx xxxx  xxxx xxxx = tile number

[1]  +0         +1
     ---- ----  x--- ---- = flip Y
     ---- ----  -x-- ---- = flip X
     ---- ----  --xx xxxx = color


sprite RAM
--------------------------------------------------------------
 +0         +1         +2         +3
 ---x ----  ---- ----  ---- ----  ---- ---- = unknown
 ---- xx--  ---- ----  ---- ----  ---- ---- = chain y size
 ---- --xx  xxxx xxxx  ---- ----  ---- ---- = sprite x coords
 ---- ----  ---- ----  ---- --xx  xxxx xxxx = sprite y coords

 +4         +5         +6         +7
 --xx xxxx  ---- ----  ---- ----  ---- ---- = zoom x
 ---- ----  --xx xxxx  ---- ----  ---- ---- = zoom y
 ---- ----  ---- ----  ---x xxxx  xxxx xxxx = tile information offset


***************************************************************************/

#include "emu.h"
#include "video/taitoic.h"
#include "includes/taitoair.h"


/* These are hand-tuned values */
static const int zoomy_conv_table[] =
{
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,	/* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,	/* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};

/***************************************************************************
  Screen refresh
***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
       is always 64 pixels. */

	taitoair_state *state = (taitoair_state *)machine->driver_data;
	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, dy, ex, ey, zx, zy;
	int ysize;
	int j, k;
	int offs;					/* sprite RAM offset */
	int tile_offs;				/* sprite chain offset */
	int zoomx, zoomy;			/* zoom value */

	for (offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		if (offs <  0x01b0 && priority == 0)	continue;
		if (offs >= 0x01b0 && priority == 1)	continue;

		x0        =  tc0080vco_sprram_r(state->tc0080vco, offs + 1, 0xffff) & 0x3ff;
		y0        =  tc0080vco_sprram_r(state->tc0080vco, offs + 0, 0xffff) & 0x3ff;
		zoomx     = (tc0080vco_sprram_r(state->tc0080vco, offs + 2, 0xffff) & 0x7f00) >> 8;
		zoomy     = (tc0080vco_sprram_r(state->tc0080vco, offs + 2, 0xffff) & 0x007f);
		tile_offs = (tc0080vco_sprram_r(state->tc0080vco, offs + 3, 0xffff) & 0x1fff) << 2;
		ysize     = size[(tc0080vco_sprram_r(state->tc0080vco, offs, 0xffff) & 0x0c00) >> 10];

		if (tile_offs)
		{
			/* Convert zoomy value to real value as zoomx */
			zoomy = zoomy_conv_table[zoomy];

			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (zoomy < 63)
			{
				dy = 8 + (zoomy + 2) / 8;
				ey = (zoomy + 2) % 8;
				zy = ((dy << 1) + ey) << 11;
			}
			else
			{
				dy = 16 + (zoomy - 63) / 4;
				ey = (zoomy - 63) % 4;
				zy = (dy + ey) << 12;
			}

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (tc0080vco_flipscreen_r(state->tc0080vco))
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
				dy = -dy;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			y = y0;
			for (j = 0; j < ysize; j ++)
			{
				x = x0;
				for (k = 0; k < 4; k ++)
				{
					if (tile_offs >= 0x1000)
					{
						int tile, color, flipx, flipy;

						tile  = tc0080vco_cram_0_r(state->tc0080vco, tile_offs, 0xffff) & 0x7fff;
						color = tc0080vco_cram_1_r(state->tc0080vco, tile_offs, 0xffff) & 0x001f;
						flipx = tc0080vco_cram_1_r(state->tc0080vco, tile_offs, 0xffff) & 0x0040;
						flipy = tc0080vco_cram_1_r(state->tc0080vco, tile_offs, 0xffff) & 0x0080;

						if (tc0080vco_flipscreen_r(state->tc0080vco))
						{
							flipx ^= 0x0040;
							flipy ^= 0x0080;
						}

						drawgfxzoom_transpen( bitmap, cliprect,
								 machine -> gfx[0],
								 tile,
								 color,
								 flipx, flipy,
								 x, y,
								 zx, zy, 0
						);
					}
					tile_offs ++;
					x += dx;
				}
				y += dy;
			}
		}
	}
}

static void fill_slope( bitmap_t *bitmap, const rectangle *cliprect, int color, INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 y1, INT32 y2, INT32 *nx1, INT32 *nx2 )
{
	if (y1 > cliprect->max_y)
		return;

	if (y2 <= cliprect->min_y)
	{
		int delta = y2 - y1;
		*nx1 = x1 + delta * sl1;
		*nx2 = x2 + delta * sl2;
		return;
	}

	if (y1 < -1000000 || y1 > 1000000)
		return;

	if (y2 > cliprect->max_y)
		y2 = cliprect->max_y + 1;

	if (y1 < cliprect->min_y)
	{
		int delta = cliprect->min_y - y1;
		x1 += delta * sl1;
		x2 += delta * sl2;
		y1 = cliprect->min_y;
	}

	if (x1 > x2 || (x1==x2 && sl1 > sl2))
	{
		INT32 t, *tp;
		t = x1;
		x1 = x2;
		x2 = t;
		t = sl1;
		sl1 = sl2;
		sl2 = t;
		tp = nx1;
		nx1 = nx2;
		nx2 = tp;
	}

	while (y1 < y2)
	{
		if (y1 >= cliprect->min_y)
		{
			int xx1 = x1 >> TAITOAIR_FRAC_SHIFT;
			int xx2 = x2 >> TAITOAIR_FRAC_SHIFT;
			if (xx1 <= cliprect->max_x || xx2 >= cliprect->min_x)
			{
				if (xx1 < cliprect->min_x)
					xx1 = cliprect->min_x;
				if (xx2 > cliprect->max_x)
					xx2 = cliprect->max_x;

				while (xx1 <= xx2)
				{
					*BITMAP_ADDR16(bitmap, y1, xx1) = color;
					xx1++;
				}
			}
		}

		x1 += sl1;
		x2 += sl2;
		y1++;
	}
	*nx1 = x1;
	*nx2 = x2;
}

static void fill_poly( bitmap_t *bitmap, const rectangle *cliprect, const struct taitoair_poly *q )
{
	INT32 sl1, sl2, cury, limy, x1, x2;
	int pmin, pmax, i, ps1, ps2;
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT * 2];
	int color = q->col;
	int pcount = q->pcount;

	for (i = 0; i < pcount; i++)
	{
		p[i].x = p[i + pcount].x = q->p[i].x << TAITOAIR_FRAC_SHIFT;
		p[i].y = p[i + pcount].y = q->p[i].y;
	}

	pmin = pmax = 0;
	for (i = 1; i < pcount; i++)
	{
		if (p[i].y < p[pmin].y)
			pmin = i;
		if (p[i].y > p[pmax].y)
			pmax = i;
	}

	cury = p[pmin].y;
	limy = p[pmax].y;

	if (cury == limy)
		return;

	if (cury > cliprect->max_y)
		return;
	if (limy <= cliprect->min_y)
		return;

	if (limy > cliprect->max_y)
		limy = cliprect->max_y;

	ps1 = pmin + pcount;
	ps2 = pmin;

	goto startup;

	for (;;)
	{
		if (p[ps1 - 1].y == p[ps2 + 1].y)
		{
			fill_slope(bitmap, cliprect, color, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if (cury >= limy)
				break;
			ps1--;
			ps2++;

		startup:
			while (p[ps1 - 1].y == cury)
				ps1--;
			while (p[ps2 + 1].y == cury)
				ps2++;
			x1 = p[ps1].x;
			x2 = p[ps2].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
		else if (p[ps1 - 1].y < p[ps2 + 1].y)
		{
			fill_slope(bitmap, cliprect, color, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if (cury >= limy)
				break;
			ps1--;
			while (p[ps1 - 1].y == cury)
				ps1--;
			x1 = p[ps1].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
		}
		else
		{
			fill_slope(bitmap, cliprect, color, x1, x2, sl1, sl2, cury, p[ps2 + 1].y, &x1, &x2);
			cury = p[ps2 + 1].y;
			if (cury >= limy)
				break;
			ps2++;
			while (p[ps2 + 1].y == cury)
				ps2++;
			x2 = p[ps2].x;
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
	}
}

VIDEO_UPDATE( taitoair )
{
	taitoair_state *state = (taitoair_state *)screen->machine->driver_data;

	tc0080vco_tilemap_update(state->tc0080vco);

	bitmap_fill(bitmap, cliprect, 0x41);

#ifdef MAME_DEBUG
	if (!input_code_pressed(screen->machine, KEYCODE_A))
		tc0080vco_tilemap_draw(state->tc0080vco, bitmap, cliprect, 0, 0, 0);
	if (!input_code_pressed(screen->machine, KEYCODE_S))
		draw_sprites(screen->machine, bitmap, cliprect, 0);
	if (!input_code_pressed(screen->machine, KEYCODE_D))
		tc0080vco_tilemap_draw(state->tc0080vco, bitmap, cliprect, 1, 0, 0);
	if (!input_code_pressed(screen->machine, KEYCODE_F))
		draw_sprites(screen->machine, bitmap, cliprect, 1);
#else
	tc0080vco_tilemap_draw(state->tc0080vco, bitmap, cliprect, 0, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 0);
	tc0080vco_tilemap_draw(state->tc0080vco, bitmap, cliprect, 1, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 1);
#endif

	tc0080vco_tilemap_draw(state->tc0080vco, bitmap, cliprect, 2, 0, 0);

	if (state->line_ram[0x3fff])
	{
		int adr = 0x3fff;
//      struct taitoair_poly q;

		while (adr >= 0 && state->line_ram[adr] && state->line_ram[adr] != 0x4000)
		{
			int pcount;
			if (!(state->line_ram[adr] & 0x8000) || adr < 10)
			{
				logerror("quad: unknown value %04x at %04x\n", state->line_ram[adr], adr);
				break;
			}
			state->q.col = (state->line_ram[adr] & 0x7fff) + 0x300;
			adr--;
			pcount = 0;
			while (pcount < TAITOAIR_POLY_MAX_PT && adr >= 1 && !(state->line_ram[adr] & 0xc000))
			{
				state->q.p[pcount].y = state->line_ram[adr] + 3 * 16;
				state->q.p[pcount].x = state->line_ram[adr - 1];
				pcount++;
				adr -= 2;
			}
			adr--;
			state->q.pcount = pcount;
			fill_poly(bitmap, cliprect, &state->q);
		}
	}
	return 0;
}
