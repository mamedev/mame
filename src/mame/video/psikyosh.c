/*

Psikyo PS6406B (PS3v1/PS5/PS5v2):
See src/drivers/psikyosh.c for more info

Hardware is extremely flexible (and luckily underused :)

3 Types of tilemaps:
1. Normal
Used everywhere (Types 0x0a and 0x0b where 0x0b uses alternate registers to control scroll/bank/alpha/columnzoom)

2. Seems to be two interleaved layers in each
Used for text layers in daraku (Type 0x0c and 0x0d)

3. Has 224 values for rowscroll (And implicit zoom?)/columnscroll/rowzoom/per-row priority/per-row alpha/per-row bank
Used in S1945II test+level 7 and S1945III levels 7+8
(i.e instead of using 0x13f0+0x17f0 registers it uses a block as indicated by the type)

There are 32 data banks of 0x800 bytes starting at 0x3000000
The first 8 are continuous and used for sprites.

Bank Offset Purpose
0   0000    Sprites
1   8000    "
2   1000    "
3   1800    "
4   2000    "
5   2800    "
6   3000    "
7   3800    Sprite List
---
8   4000    Pre Lineblend (0x000, 224 values) and Post Lineblend (0x400, 224 values)
9   4800    Unknown
a   5000    Tilemap XScroll/YScroll
b   5800    Tilemap Priority/Zoom/AlphaBlending/Bank

c   6000    General Purpose banks for either tilemaps (can optionally use two consecutive banks)
            Or To contain RowScroll/ColumnScroll (0x000, 224 values) followed by Priority/Zoom/AlphaBlending/Bank (0x400, 224 values)
...
1f  f800    "
*/

/*
BG Scroll/Priority/Zoom/Alpha/Tilebank:
Either at 0x30053f0 / 4/8 (For 0a/0c/0d), 0x3005bf0 / 4/8 (For 0b) Or per-line in a bank (For 0e-1f)
   0x?vvv?xxx - v = vertical scroll - x = x scroll
Either at 0x30057f0 / 4/8 (For 0a/0c/0d), 0x3005ff0 / 4/8 (For 0b) Or per-line in a bank (For 0e-1f)
   0xppzzaabb - p = priority, z = zoom/expand(00 is none), a = alpha value/effect, b = tilebank (used when register below = 0x0a)


Vid Regs:

0x00 -- alpha values for sprites. sbomberb = 0000 3830
0x04 --   "     "     "     "     sbomberb = 2820 1810.

0x08 -- 0xff00 priority values for sprites, 4-bits per value, c0 is vert game (Controls whether row/line effects?), 0x000f is priority for per-line post-blending

0x0c -- ????c0?? -c0 is flip screen
0x10 -- 00aa2000? always? -gb2 tested- 00000fff Controls gfx data available to be read by SH-2 for verification
0x14 -- 83ff000e? always? -gb2 tested-
0x18 -- double buffer/mode for tilemaps. As follows for the different tilemaps: 112233--
        0a = normal 0b = alt buffer
        0c/0d are used by daraku for text layers. same as above except bank is still controlled by registers and seems to contain two 16x16 timemaps with alternate columns from each.
        0e-1f indicates layer uses row and/or line scroll. values come from associated bank, tiles from 2 below i.e bank c-1d
        Bit 0x80 indicates use of line effects.
0x1c -- ????123- enable bits  8 is enable. 4 indicates 8bpp tiles. 1 is size select for tilemap
*/

/*
TODO:

pre and post line-blending hooked-up, is there a toggle for row/column? Does the pre actually have a configurable priority like the post?

row scroll+zoom / column scroll+zoom (s1945ii test, level 7 and s1945iii use a whole block of ram for scrolling/zooming,  -dh
(224 values for both x and y scroll + zoom) Also used for daraku text layers, again only yscroll differ
Also, xscroll values are always the same, maybe the hw can't do simultaneous line/columnscroll. -pjp

figure out how the daraku text layers work correctly, dimensions are different (even more tilemaps needed)
daraku seems to use tilemaps only for text layer (hi-scores, insert coin, warning message, test mode, psikyo (c)) how this is used is uncertain,

flip screen, located but not implemented. wait until tilemaps.

the stuff might be converted to use the tilemaps once all the features is worked out ...
complicated by the fact that the whole tilemap will have to be marked dirty each time the bank changes (this can happen once per frame, unless a tilemap is allocated for each bank.
18 + 9 = 27 tilemaps (including both sizes, possibly another 8 if the large tilemaps can start on odd banks).
Would also need to support TRANSPARENCY_ALPHARANGE

sol divide doesn't seem to make much use of tilemaps at all, it uses them to fade between scenes in the intro

*/

#include "driver.h"
#include "profiler.h"
#include "drawgfxm.h"
#include "includes/psikyosh.h"

/* Psikyo PS6406B */
/* --- BACKGROUNDS --- */

static UINT8 alphatable[256];	// this might be moved to psikyosh_state, if we ever add a *machine parameter to drawgfxm.h macros

#define PIXEL_OP_REMAP_TRANS0_ALPHATABLE32(DEST, PRIORITY, SOURCE)				\
do																			\
{																			\
	UINT32 srcdata = (SOURCE);														\
	if (srcdata != 0)																\
		(DEST) = alpha_blend_r32((DEST), paldata[srcdata], alphatable[srcdata]);	\
}																			\
while (0)																		\

static void drawgfx_alphatable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		int fixedalpha)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	const pen_t *paldata;

	/* if we have a fixed alpha, call the standard drawgfx_alpha */
	if (fixedalpha >= 0)
	{
		drawgfx_alpha(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, 0, fixedalpha);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 32);
	assert(gfx != NULL);
	assert(alphatable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && (gfx->pen_usage[code] & ~(1 << 0)) == 0)
		return;

	/* render based on dest bitmap depth */
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHATABLE32, NO_PRIORITY);
}


/* 'Normal' layers, no line/columnscroll. No per-line effects */
static void draw_bglayer( running_machine *machine, int layer, bitmap_t *bitmap, const rectangle *cliprect )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	gfx_element *gfx;
	int offs = 0, sx, sy;
	int scrollx, scrolly, bank, alpha, alphamap, size, width;

	if (BG_TYPE(layer) == BG_NORMAL_ALT)
	{
		bank     = (state->bgram[0x1ff0 / 4 + (layer * 0x04) / 4] & 0x000000ff) >> 0;
		alpha    = (state->bgram[0x1ff0 / 4 + (layer * 0x04) / 4] & 0x00003f00) >> 8;
		alphamap = (state->bgram[0x1ff0 / 4 + (layer * 0x04) / 4] & 0x00008000) >> 15;
		scrollx  = (state->bgram[0x1bf0 / 4 + (layer * 0x04) / 4] & 0x000001ff) >> 0;
		scrolly  = (state->bgram[0x1bf0 / 4 + (layer * 0x04) / 4] & 0x03ff0000) >> 16;
	}
	else /* BG_NORMAL */
	{
		bank     = (state->bgram[0x17f0 / 4 + (layer * 0x04) / 4] & 0x000000ff) >> 0;
		alpha    = (state->bgram[0x17f0 / 4 + (layer * 0x04) / 4] & 0x00003f00) >> 8;
		alphamap = (state->bgram[0x17f0 / 4 + (layer * 0x04) / 4] & 0x00008000) >> 15;
		scrollx  = (state->bgram[0x13f0 / 4 + (layer * 0x04) / 4] & 0x000001ff) >> 0;
		scrolly  = (state->bgram[0x13f0 / 4 + (layer * 0x04) / 4] & 0x03ff0000) >> 16;
	}

	if (BG_TYPE(layer) == BG_SCROLL_0D)
		scrollx += 0x08; /* quick kludge until using rowscroll */

	gfx = BG_DEPTH_8BPP(layer) ? machine->gfx[1] : machine->gfx[0];
	size = BG_LARGE(layer) ? 32 : 16;
	width = BG_LARGE(layer) ? 0x200 : 0x100;

	if (alphamap) /* alpha values are per-pen */
		alpha = -1;
	else
		alpha = ((0x3f - alpha) * 0xff) / 0x3f;	/* 0x3f-0x00 maps to 0x00-0xff */


	if ((bank >= 0x0c) && (bank <= 0x1f)) /* shouldn't happen, 20 banks of 0x800 bytes */
	{
		for (sy = 0; sy < size; sy++)
		{
			for (sx = 0; sx < 32; sx++)
			{
				int tileno, colour;

				tileno = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0x0007ffff); /* seems to take into account spriteram, hence -0x4000 */
				colour = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0xff000000) >> 24;

				drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)), alpha); /* normal */

				if (scrollx)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)), alpha); /* wrap x */
				if (scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap y */
				if (scrollx && scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap xy */

				offs++;
			}
		}
	}
}

/* This is a complete bodge for the daraku text layers. There is not enough info to be sure how it is supposed to work */
/* It appears that there are row/column scroll values for 2 seperate layers, just drawing it twice using one of each of the sets of values for now */
static void draw_bglayertext( running_machine *machine, int layer, bitmap_t *bitmap, const rectangle *cliprect )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	gfx_element *gfx;
	int offs, sx, sy;
	int scrollx, scrolly, bank, size, width, scrollbank, alpha, alphamap;

	scrollbank = BG_TYPE(layer); /* Scroll bank appears to be same as layer type */

	gfx = BG_DEPTH_8BPP(layer) ? machine->gfx[1] : machine->gfx[0];
	size = BG_LARGE(layer) ? 32 : 16;
	width = BG_LARGE(layer) ? 0x200 : 0x100;

	/* Use first values from the first set of scroll values */
	bank     = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0x000000ff) >> 0;
	alpha    = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0x00003f00) >> 8;
	alphamap = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0x00008000) >> 15;
	scrollx  = (state->bgram[(scrollbank * 0x800) / 4 - 0x4000 / 4] & 0x000001ff) >> 0;
	scrolly  = (state->bgram[(scrollbank * 0x800) / 4 - 0x4000 / 4] & 0x03ff0000) >> 16;

	if (alphamap)	/* alpha values are per-pen */
		alpha = -1;
	else
		alpha = ((0x3f - alpha) * 0xff) / 0x3f; /* 0x3f-0x00 maps to 0x00-0xff */

	if ((bank >= 0x0c) && (bank <= 0x1f)) /* shouldn't happen, 20 banks of 0x800 bytes */
	{
		offs = 0;
		for (sy = 0; sy < size; sy++)
		{
			for (sx = 0; sx < 32; sx++)
			{
				int tileno, colour;

				tileno = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0x0007ffff); /* seems to take into account spriteram, hence -0x4000 */
				colour = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0xff000000) >> 24;

				drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)), alpha); /* normal */

				if (scrollx)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)), alpha); /* wrap x */
				if (scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap y */
				if (scrollx && scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap xy */

				offs++;
			}
		}
	}

	/* Use first values from the second set of scroll values */
	bank     = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 + 0x20 / 4 - 0x4000 / 4] & 0x000000ff) >> 0;
	alpha    = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 + 0x20 / 4 - 0x4000 / 4] & 0x00003f00) >> 8;
	alphamap = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 + 0x20 / 4 - 0x4000 / 4] & 0x00008000) >> 15;
	scrollx  = (state->bgram[(scrollbank * 0x800) / 4 - 0x4000 / 4 + 0x20 / 4] & 0x000001ff) >> 0;
	scrolly  = (state->bgram[(scrollbank * 0x800) / 4 - 0x4000 / 4 + 0x20 / 4] & 0x03ff0000) >> 16;

	if (alphamap) /* alpha values are per-pen */
		alpha = -1;
	else
		alpha = ((0x3f - alpha) * 0xff) / 0x3f; /* 0x3f-0x00 maps to 0x00-0xff */

	if ((bank >= 0x0c) && (bank <= 0x1f)) /* shouldn't happen, 20 banks of 0x800 bytes */
	{
		offs = 0;
		for (sy = 0; sy < size; sy++)
		{
			for (sx = 0; sx < 32; sx++)
			{
				int tileno, colour;

				tileno = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0x0007ffff); /* seems to take into account spriteram, hence -0x4000 */
				colour = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0xff000000) >> 24;

				drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)), alpha); /* normal */

				if (scrollx)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)), alpha); /* wrap x */
				if (scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap y */
				if (scrollx && scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap xy */

				offs++;
			}
		}
	}
}

/* Row Scroll and/or Column Scroll/Zoom, has per-column Alpha/Bank/Priority. This isn't correct, just testing */
/* For now I'm just using the first alpha/bank/priority values and sodding the rest of it */
static void draw_bglayerscroll( running_machine *machine, int layer, bitmap_t *bitmap, const rectangle *cliprect )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	gfx_element *gfx;
	int offs, sx, sy;
	int scrollx, scrolly, bank, alpha, alphamap, size, width, scrollbank;

	scrollbank = BG_TYPE(layer); /* Scroll bank appears to be same as layer type */

//  bank = BG_TYPE(layer) - 0x02; /* This is an assumption which seems to hold true so far, although the bank seems to be selectable per-line */

	/* Take the following details from the info for the first row, the same for every row in all cases so far */
	bank    = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0x000000ff) >> 0;
	alpha   = (state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0x00003f00) >> 8;
	alphamap =(state->bgram[(scrollbank * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0x00008000) >> 15;

	/* Just to get things moving :) */
	scrollx =(state->bgram[(scrollbank * 0x800) / 4 - 0x4000 / 4] & 0x000001ff) >> 0;
	scrolly = 0; // ColumnZoom is combined with ColumnScroll values :(

	gfx = BG_DEPTH_8BPP(layer) ? machine->gfx[1] : machine->gfx[0];
	size = BG_LARGE(layer) ? 32 : 16;
	width = BG_LARGE(layer) ? 0x200 : 0x100;

	if (alphamap) /* alpha values are per-pen */
		alpha = -1;
	else
		alpha = ((0x3f - alpha) * 0xff) / 0x3f; /* 0x3f-0x00 maps to 0x00-0xff */

	if ((bank >= 0x0c) && (bank <= 0x1f)) /* shouldn't happen, 20 banks of 0x800 bytes */
	{
/* Looks better with blending and one scroll value than with 1D linescroll and no zoom */
#if 0
		int bg_scrollx[256], bg_scrolly[512];
		bitmap_fill(state->zoom_bitmap, NULL, get_black_pen(screen->machine));
		for (offs = 0; offs < (0x400 / 4); offs++) /* 224 values for each */
		{
			bg_scrollx[offs]         = (state->bgram[(scrollbank * 0x800) / 4 + offs - 0x4000 / 4] & 0x000001ff) >> 0;
			bg_scrolly[2 * offs]     = (state->bgram[(scrollbank * 0x800) / 4 + offs - 0x4000 / 4] & 0x03ff0000) >> 16;
			bg_scrolly[2 * offs + 1] = (state->bgram[(scrollbank * 0x800) / 4 + offs - 0x4000 / 4] & 0x03ff0000) >> 16;
		}
#endif

		offs = 0;
		for (sy = 0; sy < size; sy++)
		{
			for (sx = 0; sx < 32; sx++)
			{
				int tileno, colour;

				tileno = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0x0007ffff); /* seems to take into account spriteram, hence -0x4000 */
				colour = (state->bgram[(bank * 0x800) / 4 + offs - 0x4000 / 4] & 0xff000000) >> 24;

//              drawgfx_transpen(state->zoom_bitmap, NULL, gfx, tileno, colour, 0, 0, (16 * sx) & 0x1ff, ((16 * sy) & (width - 1)), 0);

				drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)), alpha); /* normal */

				if (scrollx)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)), alpha); /* wrap x */
				if (scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap y */
				if (scrollx && scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap xy */

				offs++;
			}
		}
		/* Only ever seems to use one linescroll value, ok for now */
		/* Disabled for now, as they doesn't even support alpha :( */
//      copyscrollbitmap_trans(bitmap, state->zoom_bitmap, 1, bg_scrollx, 512, bg_scrolly, cliprect, 0);
//      copyscrollbitmap_trans(bitmap, state->zoom_bitmap, 256, bg_scrollx, 0, bg_scrolly, cliprect, 0);
	}
}

/* 3 BG layers, with priority */
static void draw_background( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	int i;

#if 0
#ifdef MAME_DEBUG
	popmessage	("Pri %d=%02x-%s %d=%02x-%s %d=%02x-%s",
		0, BG_TYPE(0), BG_LAYER_ENABLE(0)?"y":"n",
		1, BG_TYPE(1), BG_LAYER_ENABLE(1)?"y":"n",
		2, BG_TYPE(2), BG_LAYER_ENABLE(2)?"y":"n");
#endif
#endif

	/* 1st-3rd layers */
	for (i = 0; i < 3; i++)
	{
		if (!BG_LAYER_ENABLE(i))
			continue;

		switch (BG_TYPE(i))
		{
			case BG_NORMAL:
				if (((state->bgram[0x17f0 / 4 + (i * 0x04) / 4] & 0xff000000) >> 24) == req_pri)
					draw_bglayer(machine, i, bitmap, cliprect);
				break;
			case BG_NORMAL_ALT:
				if (((state->bgram[0x1ff0 / 4 + (i * 0x04) / 4] & 0xff000000) >> 24) == req_pri)
					draw_bglayer(machine, i, bitmap, cliprect);
				break;
			case BG_SCROLL_0C: // Using normal for now
			case BG_SCROLL_0D: // Using normal for now
				if (((state->bgram[(BG_TYPE(i) * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0xff000000) >> 24) == req_pri)
					draw_bglayertext(machine, i, bitmap, cliprect);
				break;
			case BG_SCROLL_ZOOM:
			/* 0x10 - 0x1f */
			case 0x10: case 0x11: case 0x12: case 0x13:
			case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1a: case 0x1b:
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				if (((state->bgram[(BG_TYPE(i) * 0x800) / 4 + 0x400 / 4 - 0x4000 / 4] & 0xff000000) >> 24) == req_pri)
					draw_bglayerscroll(machine, i, bitmap, cliprect);
				break;
			default:
				popmessage	("Unknown layer type %02x", BG_TYPE(i));
		}
	}
}

/* --- SPRITES --- */

/* 32-bit ONLY */
/* zoomx/y are pixel slopes in 6.10 floating point, not scale. 0x400 is 1:1 */
/* high/wide are number of tiles wide/high up to max size of zoom_bitmap in either direction */
/* code is index of first tile and incremented across rows then down columns (adjusting for flip obviously) */
/* sx and sy is top-left of entire sprite regardless of flip */
/* Note that Level 5-4 of sbomberb boss is perfect! (Alpha blended zoomed) as well as S1945II logo */
/* pixel is only plotted if z is >= priority_buffer[y][x] */
static void psikyosh_drawgfxzoom( running_machine *machine,
		bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
		int alpha,
		int zoomx, int zoomy, int wide, int high, UINT32 z)
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	rectangle myclip; /* Clip to screen boundaries */
	int code_offset = 0;
	int xtile, ytile, xpixel, ypixel;

	if (!zoomx || !zoomy)
		return;

	profiler_mark_start(PROFILER_DRAWGFX);

	assert(dest_bmp->bpp == 32);

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	if (clip)
	{
		myclip.min_x = clip->min_x;
		myclip.max_x = clip->max_x;
		myclip.min_y = clip->min_y;
		myclip.max_y = clip->max_y;

		if (myclip.min_x < 0) myclip.min_x = 0;
		if (myclip.max_x >= dest_bmp->width) myclip.max_x = dest_bmp->width - 1;
		if (myclip.min_y < 0) myclip.min_y = 0;
		if (myclip.max_y >= dest_bmp->height) myclip.max_y = dest_bmp->height-1;

		clip = &myclip;
	}

	/* Temporary fallback for non-zoomed, needs z-buffer. Note that this is probably a lot slower than drawgfx.c, especially if there was seperate code for flipped cases */
	if (zoomx == 0x400 && zoomy == 0x400)
	{
		int xstart, ystart, xend, yend, xinc, yinc;

		if (flipx)	{ xstart = wide - 1; xend = -1;   xinc = -1; }
		else		{ xstart = 0;        xend = wide; xinc = +1; }

		if (flipy)	{ ystart = high - 1; yend = -1;   yinc = -1; }
		else		{ ystart = 0;        yend = high; yinc = +1; }

		/* Start drawing */
		if (gfx)
		{
			for (ytile = ystart; ytile != yend; ytile += yinc)
			{
				for (xtile = xstart; xtile != xend; xtile += xinc)
				{
					const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
					const UINT8 *code_base = gfx_element_get_data(gfx, (code + code_offset++) % gfx->total_elements);

					int x_index_base, y_index, sx, sy, ex, ey;

					if (flipx)	{ x_index_base = gfx->width - 1; }
					else		{ x_index_base = 0; }

					if (flipy)	{ y_index = gfx->height-1; }
					else		{ y_index = 0; }

					/* start coordinates */
					sx = offsx + xtile * gfx->width;
					sy = offsy + ytile * gfx->height;

					/* end coordinates */
					ex = sx + gfx->width;
					ey = sy + gfx->height;

					if (clip)
					{
						if (sx < clip->min_x)
						{ /* clip left */
							int pixels = clip->min_x - sx;
							sx += pixels;
							x_index_base += xinc * pixels;
						}
						if (sy < clip->min_y)
						{ /* clip top */
							int pixels = clip->min_y - sy;
							sy += pixels;
							y_index += yinc * pixels;
						}
						/* NS 980211 - fixed incorrect clipping */
						if (ex > clip->max_x + 1)
						{ /* clip right */
							int pixels = ex - clip->max_x - 1;
							ex -= pixels;
						}
						if (ey > clip->max_y + 1)
						{ /* clip bottom */
							int pixels = ey - clip->max_y - 1;
							ey -= pixels;
						}
					}

					if (ex > sx)
					{ /* skip if inner loop doesn't draw anything */
						int y;

						/* case 1: no alpha */
						if (alpha == 0xff)
						{
							if (z > 0)
							{
								const UINT8 *source = code_base + (y_index) * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								UINT16 *pri = (UINT16 *)state->z_bitmap->base + sy * state->z_bitmap->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										if (z >= *pri)
										{
											int c = *source;
											if (c != 0)
											{
												*dest = pal[c];
												*pri = z;
											}
										}
										dest++;
										pri++;
										source += xinc;
									}
									dest += dst_modulo;
									pri += dst_modulo;
									source += src_modulo;
								}
							}
							else
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										int c = *source;
										if (c != 0)
											*dest = pal[c];

										dest++;
										source += xinc;
									}
									dest += dst_modulo;
									source += src_modulo;
								}
							}
						}

						/* case 6: alpha-blended */
						else if (alpha >= 0)
						{
							if (z > 0)
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								UINT16 *pri = (UINT16 *)state->z_bitmap->base + sy * state->z_bitmap->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										if (z >= *pri)
										{
											int c = *source;
											if (c != 0)
											{
												*dest = alpha_blend_r32(*dest, pal[c], alpha);
												*pri = z;
											}
										}
										dest++;
										pri++;
										source += xinc;
									}
									dest += dst_modulo;
									pri += dst_modulo;
									source += src_modulo;
								}
							}
							else
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										int c = *source;
										if (c != 0)
											*dest = alpha_blend_r32(*dest, pal[c], alpha);

										dest++;
										source += xinc;
									}
									dest += dst_modulo;
									source += src_modulo;
								}
							}
						}

						/* pjp 31/5/02 */
						/* case 7: TRANSPARENCY_ALPHARANGE */
						else
						{
							if (z > 0)
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								UINT16 *pri = (UINT16 *)state->z_bitmap->base + sy * state->z_bitmap->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										if (z >= *pri)
										{
											int c = *source;
											if (c != 0)
											{
												if (alphatable[c] == 0xff)
													*dest = pal[c];
												else
													*dest = alpha_blend_r32(*dest, pal[c], alphatable[c]);

												*pri = z;
											}
										}
										dest++;
										pri++;
										source += xinc;
									}
									dest += dst_modulo;
									pri += dst_modulo;
									source += src_modulo;
								}
							}
							else
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										int c = *source;
										if (c != 0)
										{
											if (alphatable[c] == 0xff)
												*dest = pal[c];
											else
												*dest = alpha_blend_r32(*dest, pal[c], alphatable[c]);
										}
										dest++;
										source += xinc;
									}
									dest += dst_modulo;
									source += src_modulo;
								}
							}
						}

					}
				}
			}
		}
	}
	else /* Zoomed */
	{
		/* Make a copy of complete sprite at top-left of zoom_bitmap */
		/* Because I'm too slow to get it to work on the fly */
		for (ytile = 0; ytile < high; ytile++)
		{
			for (xtile = 0; xtile < wide; xtile++)
			{
				const UINT8 *code_base = gfx_element_get_data(gfx, (code + code_offset++) % gfx->total_elements);
				for (ypixel = 0; ypixel < gfx->height; ypixel++)
				{
					const UINT8 *source = code_base + ypixel * gfx->line_modulo;
					UINT8 *dest = BITMAP_ADDR8(state->zoom_bitmap, ypixel + ytile*gfx->height, 0);

					for (xpixel = 0; xpixel < gfx->width; xpixel++)
					{
						dest[xpixel + xtile*gfx->width] = source[xpixel];
					}
				}
			}
		}

		/* Start drawing */
		if (gfx)
		{
			const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];

			int sprite_screen_height = ((high * gfx->height * (0x400 * 0x400)) / zoomy + 0x200) >> 10; /* Round up to nearest pixel */
			int sprite_screen_width = ((wide * gfx->width * (0x400 * 0x400)) / zoomx + 0x200) >> 10;

			if (sprite_screen_width && sprite_screen_height)
			{
				/* start coordinates */
				int sx = offsx;
				int sy = offsy;

				/* end coordinates */
				int ex = sx + sprite_screen_width;
				int ey = sy + sprite_screen_height;

				int x_index_base;
				int y_index;

				int dx, dy;

				if (flipx)	{ x_index_base = (sprite_screen_width - 1) * zoomx; dx = -zoomx; }
				else		{ x_index_base = 0; dx = zoomx; }

				if (flipy)	{ y_index = (sprite_screen_height - 1) * zoomy; dy = -zoomy; }
				else		{ y_index = 0; dy = zoomy; }

				if (clip)
				{
					if (sx < clip->min_x)
					{ /* clip left */
						int pixels = clip->min_x - sx;
						sx += pixels;
						x_index_base += pixels * dx;
					}
					if (sy < clip->min_y)
					{ /* clip top */
						int pixels = clip->min_y - sy;
						sy += pixels;
						y_index += pixels * dy;
					}
					/* NS 980211 - fixed incorrect clipping */
					if (ex > clip->max_x + 1)
					{ /* clip right */
						int pixels = ex-clip->max_x - 1;
						ex -= pixels;
					}
					if (ey > clip->max_y + 1)
					{ /* clip bottom */
						int pixels = ey-clip->max_y - 1;
						ey -= pixels;
					}
				}

				if (ex > sx)
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 1: no alpha */
					/* Note: adjusted to >>10 and draws from zoom_bitmap not gfx */
					if (alpha == 0xff)
					{
						if (z > 0)
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT16 *pri = BITMAP_ADDR16(state->z_bitmap, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									if (z >= pri[x])
									{
										int c = source[x_index >> 10];
										if (c != 0)
										{
											dest[x] = pal[c];
											pri[x] = z;
										}
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index >> 10];
									if (c != 0)
										dest[x] = pal[c];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* case 6: alpha-blended */
					else if (alpha >= 0)
					{
						if (z > 0)
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT16 *pri = BITMAP_ADDR16(state->z_bitmap, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									if (z >= pri[x])
									{
										int c = source[x_index >> 10];
										if (c != 0)
										{
											dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
											pri[x] = z;
										}
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index >> 10];
									if (c != 0) dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* case 7: TRANSPARENCY_ALPHARANGE */
					else
					{
						if (z > 0)
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT16 *pri = BITMAP_ADDR16(state->z_bitmap, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									if (z >= pri[x])
									{
										int c = source[x_index >> 10];
										if (c != 0)
										{
											if (alphatable[c] == 0xff)
												dest[x] = pal[c];
											else
												dest[x] = alpha_blend_r32(dest[x], pal[c], alphatable[c]);

											pri[x] = z;
										}
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index >> 10];
									if (c != 0)
									{
										if (alphatable[c] == 0xff)
											dest[x] = pal[c];
										else
											dest[x] = alpha_blend_r32(dest[x], pal[c], alphatable[c]);
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
	profiler_mark_end();
}

#define SPRITE_PRI(n) (((state->vidregs[2] << (4*n)) & 0xf0000000 ) >> 28)

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri )
{
	/*- Sprite Format 0x0000 - 0x37ff -**

    0 ---- --yy yyyy yyyy | ---- --xx xxxx xxxx  1  F--- hhhh ZZZZ ZZZZ | fPPP wwww zzzz zzzz
    2 pppp pppp -aaa -nnn | nnnn nnnn nnnn nnnn  3  ---- ---- ---- ---- | ---- ---- ---- ----

    y = ypos
    x = xpos

    h = height
    w = width

    F = flip (y)
    f = flip (x)

    Z = zoom (y)
    z = zoom (x)

    n = tile number

    p = palette

    a = alpha blending, selects which of the 8 alpha values in vid_regs[0-1] to use

    P = priority
    Points to a 4-bit entry in vid_regs[2] which provides a priority comparable with the bg layer's priorities.
    However, sprite-sprite priority needs to be preserved.
    daraku and soldivid only use the lsb

    **- End Sprite Format -*/

	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	const gfx_element *gfx;
	UINT32 *src = machine->generic.buffered_spriteram.u32; /* Use buffered spriteram */
	UINT16 *list = (UINT16 *)src + 0x3800 / 2;
	UINT16 listlen = 0x800/2, listcntr = 0;
	UINT16 *zoom_table = (UINT16 *)state->zoomram;
	UINT8  *alpha_table = (UINT8 *)state->vidregs;

	while (listcntr < listlen)
	{
		UINT32 listdat, sprnum, xpos, ypos, high, wide, flpx, flpy, zoomx, zoomy, tnum, colr, dpth;
		UINT32 pri, alphamap;
		int alpha;

		listdat = list[BYTE_XOR_BE(listcntr)];
		sprnum = (listdat & 0x03ff) * 4;

		pri  = (src[sprnum + 1] & 0x00003000) >> 12; // & 0x00007000/0x00003000 ?
		pri = SPRITE_PRI(pri);

		if (pri == req_pri)
		{
			ypos = (src[sprnum + 0] & 0x03ff0000) >> 16;
			xpos = (src[sprnum + 0] & 0x000003ff) >> 00;

			if (ypos & 0x200) ypos -= 0x400;
			if (xpos & 0x200) xpos -= 0x400;

			high  = ((src[sprnum + 1] & 0x0f000000) >> 24) + 1;
			wide  = ((src[sprnum + 1] & 0x00000f00) >> 8) + 1;

			flpy  = (src[sprnum + 1] & 0x80000000) >> 31;
			flpx  = (src[sprnum + 1] & 0x00008000) >> 15;

			zoomy = (src[sprnum + 1] & 0x00ff0000) >> 16;
			zoomx = (src[sprnum + 1] & 0x000000ff) >> 00;

			tnum  = (src[sprnum + 2] & 0x0007ffff) >> 00;
			dpth  = (src[sprnum + 2] & 0x00800000) >> 23;
			colr  = (src[sprnum + 2] & 0xff000000) >> 24;

			alpha = (src[sprnum + 2] & 0x00700000) >> 20;

			alphamap = (alpha_table[BYTE4_XOR_BE(alpha)] & 0x80)? 1:0;
			alpha = alpha_table[BYTE4_XOR_BE(alpha)] & 0x3f;

			gfx = dpth ? machine->gfx[1] : machine->gfx[0];

			if (alphamap) /* alpha values are per-pen */
				alpha = -1;
			else
				alpha = ((0x3f - alpha) * 0xff) / 0x3f; /* 0x3f-0x00 maps to 0x00-0xff */

			/* start drawing */
			if (zoom_table[BYTE_XOR_BE(zoomy)] && zoom_table[BYTE_XOR_BE(zoomx)]) /* Avoid division-by-zero when table contains 0 (Uninitialised/Bug) */
			{
				psikyosh_drawgfxzoom(machine, bitmap, cliprect, gfx, tnum, colr, flpx, flpy, xpos, ypos, alpha,
									(UINT32)zoom_table[BYTE_XOR_BE(zoomx)], (UINT32)zoom_table[BYTE_XOR_BE(zoomy)], wide, high, listcntr);

#if 0
#ifdef MAME_DEBUG
				if (input_code_pressed(machine, KEYCODE_Z))	/* Display some info on each sprite */
				{
					char buf[10];
					int x, y;

					sprintf(buf, "%X",xdim/16); /* Display Zoom in 16.16 */
					if (machine->gamedrv->flags & ORIENTATION_SWAP_XY)
					{
						x = ypos;
						y = video_screen_get_visible_area(machine->primary_screen)->max_x - xpos; /* ORIENTATION_FLIP_Y */
					}
					else
					{
						x = xpos;
						y = ypos;
					}
					ui_draw_text(buf, x, y);
				}
#endif
#endif
			}
			/* end drawing */
		}
		listcntr++;
		if (listdat & 0x4000) break;
	}
}

VIDEO_START( psikyosh )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);
	state->z_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);

	/* Need 16-bit z-buffer */
	state->zoom_bitmap = auto_bitmap_alloc(machine, 16*16, 16*16, BITMAP_FORMAT_INDEXED8);

	machine->gfx[1]->color_granularity = 16; /* 256 colour sprites with palette selectable on 16 colour boundaries */

	{ /* Pens 0xc0-0xff have a gradient of alpha values associated with them */
		int i;
		for (i = 0; i < 0xc0; i++)
			alphatable[i] = 0xff;
		for (i = 0; i < 0x40; i++)
		{
			int alpha = ((0x3f - i) * 0xff) / 0x3f;
			alphatable[i + 0xc0] = alpha;
		}
	}

	state_save_register_global_bitmap(machine, state->z_bitmap);
	state_save_register_global_bitmap(machine, state->zoom_bitmap);
}

static void psikyosh_prelineblend( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	/* There are 224 values for pre-lineblending. Using one for every row currently */
	/* I suspect that it should be blended against black by the amount specified as
       gnbarich sets the 0x000000ff to 0x7f in test mode whilst the others use 0x80.
       As it's only used in testmode I'll just leave it as a toggle for now */
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	UINT32 *dstline;
	UINT32 *linefill = state->bgram; /* Per row */
	int x, y;

	assert(bitmap->bpp == 32);

	profiler_mark_start(PROFILER_USER1);
	for (y = cliprect->min_y; y <= cliprect->max_y; y += 1) {

		dstline = BITMAP_ADDR32(bitmap, y, 0);

		if (linefill[y] & 0xff) /* Row */
			for (x = cliprect->min_x; x <= cliprect->max_x; x += 1)
					dstline[x] = linefill[y] >> 8;
	}
	profiler_mark_end();
}

static void psikyosh_postlineblend( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	/* There are 224 values for post-lineblending. Using one for every row currently */
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	UINT32 *dstline;
	UINT32 *lineblend = state->bgram + 0x400 / 4; /* Per row */
	int x, y;

	assert(bitmap->bpp == 32);

	profiler_mark_start(PROFILER_USER2);
	for (y = cliprect->min_y; y <= cliprect->max_y; y += 1) {

		dstline = BITMAP_ADDR32(bitmap, y, 0);

		if (lineblend[y] & 0x80) /* Row */
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x += 1)
				dstline[x] = lineblend[y] >> 8;
		}
		else if (lineblend[y] & 0x7f) /* Row */
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x += 1)
				dstline[x] = alpha_blend_r32(dstline[x], lineblend[y] >> 8, 2 * (lineblend[y] & 0x7f));
		}
	}
	profiler_mark_end();
}

VIDEO_UPDATE( psikyosh ) /* Note the z-buffer on each sprite to get correct priority */
{
	psikyosh_state *state = (psikyosh_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(state->z_bitmap, cliprect, 0); /* z-buffer */

	psikyosh_prelineblend(screen->machine, bitmap, cliprect);

	for (i = 0; i <= 7; i++)
	{
		draw_sprites(screen->machine, bitmap, cliprect, i); // When same priority bg's have higher pri
		draw_background(screen->machine, bitmap, cliprect, i);
		if ((state->vidregs[2] & 0xf) == i)
			psikyosh_postlineblend(screen->machine, bitmap, cliprect);
	}
	return 0;
}

VIDEO_EOF( psikyosh )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram32_w(space, 0, 0, 0xffffffff);
}

/*popmessage   ("Regs %08x %08x %08x\n     %08x %08x %08x",
    state->bgram[0x17f0 / 4], state->bgram[0x17f4 / 4], state->bgram[0x17f8 / 4],
    state->bgram[0x1ff0 / 4], state->bgram[0x1ff4 / 4], state->bgram[0x1ff8 / 4]);*/
/*popmessage   ("Regs %08x %08x %08x\n     %08x %08x %08x",
    state->bgram[0x13f0 / 4], state->bgram[0x13f4 / 4], state->bgram[0x13f8 / 4],
    state->bgram[0x1bf0 / 4], state->bgram[0x1bf4 / 4], state->bgram[0x1bf8 / 4]);*/
/*popmessage   ("Regs %08x %08x %08x %08x %08x %08x %08x %08x",
    state->vidregs[0], state->vidregs[1],
    state->vidregs[2], state->vidregs[3],
    state->vidregs[4], state->vidregs[5],
    state->vidregs[6], state->vidregs[7]);*/
