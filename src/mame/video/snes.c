/***************************************************************************

  snes.c

  Video file to handle emulation of the Nintendo Super NES.

  Anthony Kruize
  Based on the original code by Lee Hammerton (aka Savoury Snax)

  Some notes on the snes video hardware:

  Object Attribute Memory(OAM) is made up of 128 blocks of 32 bits, followed
  by 128 blocks of 2 bits. The format for each block is:
  -First Block----------------------------------------------------------------
  | x pos  | y pos  |char no.| v flip | h flip |priority|palette |char no msb|
  +--------+--------+--------+--------+--------+--------+--------+-----------+
  | 8 bits | 8 bits | 8 bits | 1 bit  | 1 bit  | 2 bits | 3 bits | 1 bit     |
  -Second Block---------------------------------------------------------------
  | size  | x pos msb |
  +-------+-----------+
  | 1 bit | 1 bit     |
  ---------------------

  Video RAM contains information for character data and screen maps.
  Screen maps are made up of 32 x 32 blocks of 16 bits each.
  The format for each block is:
  ----------------------------------------------
  | v flip | x flip |priority|palette |char no.|
  +--------+--------+--------+--------+--------+
  | 1 bit  | 1 bit  | 1 bit  | 3 bits |10 bits |
  ----------------------------------------------
  Mode 7 is stored differently. Character data and screen map are interleaved.
  There are two formats:
  -Normal-----------------  -EXTBG-----------------------------
  | char data | char no. |  | priority | char data | char no. |
  +-----------+----------+  +----------+-----------+----------+
  | 8 bits    | 8 bits   |  | 1 bit    | 7 bits    | 8 bits   |
  ------------------------  -----------------------------------

  The screen layers are drawn with the following priorities (updated info courtesy of byuu):

  |           |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |
  -------------------------------------------------------------------------------------------------------------
  | Mode 0    |  BG4B |  BG3B |  OAM0 |  BG4A |  BG3A |  OAM1 |  BG2B |  BG1B |  OAM2 |  BG2A |  BG1A |  OAM3 |
  -------------------------------------------------------------------------------------------------------------
  | Mode 1 (*)|  BG3B |  OAM0 |  OAM1 |  BG2B |  BG1B |  OAM2 |  BG2A |  BG1A |  OAM3 |  BG3A |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 1 (!)|  BG3B |  OAM0 |  BG3A |  OAM1 |  BG2B |  BG1B |  OAM2 |  BG2A |  BG1A |  OAM3 |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 2    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 3    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 4    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 5    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 6    |  OAM0 |  BG1B |  OAM1 |  OAM2 |  BG1A |  OAM3 |       |       |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 7 (+)|  OAM0 |  BG1n |  OAM1 |  OAM2 |  OAM3 |       |       |       |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 7 (-)|  BG2B |  OAM0 |  BG1n |  OAM1 |  BG2A |  OAM2 |  OAM3 |       |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------

  Where:
   - Mode 1 (*) is Mode 1 with bg3_pty = 1
   - Mode 1 (!) is Mode 1 with bg3_pty = 0
   - Mode 7 (+) is base Mode 7
   - Mode 7 (-) is Mode 7 EXTBG

***************************************************************************/

#include "emu.h"
#include "profiler.h"
#include "includes/snes.h"

#define SNES_MAINSCREEN    0
#define SNES_SUBSCREEN     1
#define SNES_CLIP_NEVER    0
#define SNES_CLIP_IN       1
#define SNES_CLIP_OUT      2
#define SNES_CLIP_ALWAYS   3

#ifdef SNES_LAYER_DEBUG
struct DEBUGOPTS
{
	UINT8 bg_disabled[5];
	UINT8 mode_disabled[8];
	UINT8 draw_subscreen;
	UINT8 windows_disabled;
	UINT8 mosaic_disabled;
	UINT8 colormath_disabled;
	UINT8 sprite_reversed;
	UINT8 select_pri[5];
};
static struct DEBUGOPTS debug_options;
/*                                    red   green  blue    purple  yellow cyan    grey    white */
static const UINT16 dbg_mode_colours[8] = { 0x1f, 0x3e0, 0x7c00, 0x7c1f, 0x3ff, 0x7fe0, 0x4210, 0x7fff };
static UINT8 snes_dbg_video(running_machine *machine, UINT16 curline);
#endif /* SNES_LAYER_DEBUG */

static const UINT16 table_obj_offset[8][8] =
{
	{ (0*32),   (0*32)+32,   (0*32)+64,   (0*32)+96,   (0*32)+128,   (0*32)+160,   (0*32)+192,   (0*32)+224 },
	{ (16*32),  (16*32)+32,  (16*32)+64,  (16*32)+96,  (16*32)+128,  (16*32)+160,  (16*32)+192,  (16*32)+224 },
	{ (32*32),  (32*32)+32,  (32*32)+64,  (32*32)+96,  (32*32)+128,  (32*32)+160,  (32*32)+192,  (32*32)+224 },
	{ (48*32),  (48*32)+32,  (48*32)+64,  (48*32)+96,  (48*32)+128,  (48*32)+160,  (48*32)+192,  (48*32)+224 },
	{ (64*32),  (64*32)+32,  (64*32)+64,  (64*32)+96,  (64*32)+128,  (64*32)+160,  (64*32)+192,  (64*32)+224 },
	{ (80*32),  (80*32)+32,  (80*32)+64,  (80*32)+96,  (80*32)+128,  (80*32)+160,  (80*32)+192,  (80*32)+224 },
	{ (96*32),  (96*32)+32,  (96*32)+64,  (96*32)+96,  (96*32)+128,  (96*32)+160,  (96*32)+192,  (96*32)+224 },
	{ (112*32), (112*32)+32, (112*32)+64, (112*32)+96, (112*32)+128, (112*32)+160, (112*32)+192, (112*32)+224 }
};

struct SCANLINE
{
	int enable, clip;

	UINT16 buffer[SNES_SCR_WIDTH];
	UINT8  priority[SNES_SCR_WIDTH];
	UINT8  layer[SNES_SCR_WIDTH];
	UINT8  blend_exception[SNES_SCR_WIDTH];
};

static struct SCANLINE scanlines[2];
struct SNES_PPU_STRUCT snes_ppu;

enum
{
	SNES_COLOR_DEPTH_2BPP = 0,
	SNES_COLOR_DEPTH_4BPP,
	SNES_COLOR_DEPTH_8BPP
};

/*****************************************
 * snes_get_bgcolor()
 *
 * Get the proper color (direct or from cgram)
 *****************************************/

INLINE UINT16 snes_get_bgcolor( UINT8 direct_colors, UINT16 palette, UINT8 color )
{
	UINT16 c = 0;

	if (direct_colors)
	{
		/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
		c = ((color & 0x07) << 2) | ((color & 0x38) << 4) | ((color & 0xc0) << 7);
		c |= ((palette & 0x04) >> 1) | ((palette & 0x08) << 3) | ((palette & 0x10) << 8);
	}
	else
		c = snes_cgram[(palette + color) % FIXED_COLOUR];

	return c;
}

/*****************************************
 * snes_set_scanline_pixel()
 *
 * Store pixel color, priority, layer and
 * color math exception (for OAM) in the 
 * proper scanline
 *****************************************/

INLINE void snes_set_scanline_pixel( int screen, INT16 x, UINT16 color, UINT8 priority, UINT8 layer, int blend )
{
	scanlines[screen].buffer[x] = color;
	scanlines[screen].priority[x] = priority;
	scanlines[screen].layer[x] = layer;
	scanlines[screen].blend_exception[x] = blend;
}

/*************************************************************************************************
 * SNES tiles
 *
 * The way vram is accessed to draw tiles is basically the same for both BG and OAM tiles. Main
 * differences are bit planes (variable for BG and fixed for OAM) and a few details of the scanline
 * output (since OAM has neither mosaic, nor hires, nor direct colors).
 * Hence, we use a common function to take data from VRAM and then we call specific routines for
 * OAM vs BG vs Hi-Res BG tiles.
 *************************************************************************************************/

/*****************************************
 * snes_draw_bgtile_lores()
 * snes_draw_bgtile_hires()
 * snes_draw_oamtile_()
 *
 * Check if a pixel is clipped or not, and
 * copy it to the scanline buffer when
 * appropriate. The actual way to perform
 * such operations depends on the source
 * (BG or OAM) and on the resolution (hires 
 * or lores)
 *****************************************/

INLINE void snes_draw_bgtile_lores( UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority )
{
	int screen;
	UINT16 c;

	for (screen = SNES_MAINSCREEN; screen <= SNES_SUBSCREEN; screen++)
	{
		if (ii >= 0 && ii < SNES_SCR_WIDTH && scanlines[screen].enable)
		{
			if (scanlines[screen].priority[ii] <= priority)
			{
				UINT8 clr = colour;
				UINT8 clipmask = snes_ppu.clipmasks[layer][ii];

#ifdef SNES_LAYER_DEBUG
				if (debug_options.windows_disabled)
					clipmask = 0xff;
#endif /* SNES_LAYER_DEBUG */

				/* Clip to windows */
				if (scanlines[screen].clip)
					clr &= clipmask;

				/* Only draw if we have a colour (0 == transparent) */
				if (clr)
				{
					c = snes_get_bgcolor(direct_colors, pal, clr);
					snes_set_scanline_pixel(screen, ii, c, priority, layer, 0);
				}
			}
		}
	}
}

INLINE void snes_draw_bgtile_hires( UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority )
{
	int screen;
	UINT16 c;

	for (screen = SNES_MAINSCREEN; screen <= SNES_SUBSCREEN; screen++)
	{
		// odd pixels to main screen, even pixels to sub screen
		if (ii >= 0 && ii < (SNES_SCR_WIDTH << 1) && ((ii & 1) ^ screen) && scanlines[screen].enable)
		{
			if (scanlines[screen].priority[ii >> 1] <= priority)
			{
				UINT8 clr = colour;
				UINT8 clipmask = snes_ppu.clipmasks[layer][ii];

#ifdef SNES_LAYER_DEBUG
				if (debug_options.windows_disabled)
					clipmask = 0xff;
#endif /* SNES_LAYER_DEBUG */

				/* Clip to windows */
				if (scanlines[screen].clip)
					clr &= clipmask;

				/* Only draw if we have a colour (0 == transparent) */
				if (clr)
				{
					c = snes_get_bgcolor(direct_colors, pal, clr);
					snes_set_scanline_pixel(screen, ii >> 1, c, priority, layer, 0);
				}
			}
		}
	}
}

INLINE void snes_draw_oamtile( INT16 ii, UINT8 colour, UINT16 pal, UINT8 priority )
{
	int screen;
	int blend;
	UINT16 c;
	INT16 pos = ii & 0x1ff;

	for (screen = SNES_MAINSCREEN; screen <= SNES_SUBSCREEN; screen++)
	{
		if (pos >= 0 && pos < SNES_SCR_WIDTH && scanlines[screen].enable)
		{
			UINT8 clr = colour;
			UINT8 clipmask = snes_ppu.clipmasks[SNES_OAM][pos];

#ifdef SNES_LAYER_DEBUG
			if (debug_options.windows_disabled)
				clipmask = 0xff;
#endif /* SNES_LAYER_DEBUG */

			/* Clip to windows */
			if (scanlines[screen].clip)
				clr &= clipmask;

			/* Only draw if we have a colour (0 == transparent) */
			if (clr)
			{
				c = snes_cgram[(pal + clr) % FIXED_COLOUR];
				blend = (pal + clr < 192) ? 1 : 0;
				snes_set_scanline_pixel(screen, pos, c, priority, SNES_OAM, blend);
			}
		}
	}
}

/*****************************************
 * snes_draw_tile()
 *
 * Get pixel color with variable bit planes 
 * from snes_vram and draw the tiles by
 * calling the appropriate routine
 *****************************************/

INLINE void snes_draw_tile( UINT8 planes, UINT8 layer, UINT16 tileaddr, INT16 x, int tile_size, UINT8 priority, UINT8 flip, UINT8 direct_colors, UINT16 pal, UINT8 hires )
{
	UINT8 plane[8];
	INT16 ii, jj, start_pixel;
	int tile, x_mos;

	start_pixel = x;

	/* if 8x8 tile we draw once; if 16x16 we draw twice */
	for (tile = 0; tile < (1 << tile_size); tile++)
	{
		for (ii = 0; ii < planes / 2; ii++)
		{
			plane[2 * ii] = snes_vram[tileaddr + 16 * ii];
			plane[2 * ii + 1] = snes_vram[tileaddr + 16 * ii + 1];
		}

		for (ii = x; ii < (x + 8); ii++)
		{
			UINT8 colour = 0;
			UINT8 mosaic = snes_ppu.layer[layer].mosaic_enabled;

#ifdef SNES_LAYER_DEBUG
			if (debug_options.mosaic_disabled)
				mosaic = 0;
#endif /* SNES_LAYER_DEBUG */

			if (flip)
			{
				for (jj = 0; jj < planes; jj++)
					colour |= BIT(plane[jj], ii - x) ? (1 << jj) : 0;
			}
			else
			{
				for (jj = 0; jj < planes; jj++)
					colour |= BIT(plane[jj], 7 - (ii - x)) ? (1 << jj) : 0;
			}

			if (layer == SNES_OAM)
				snes_draw_oamtile(ii, colour, pal, priority);
			else if (!hires)
			{
				if (mosaic)	
				{
					for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1); x_mos++)
						snes_draw_bgtile_lores(layer, ii + x_mos, colour, pal, direct_colors, priority);
					ii += x_mos - 1;
				}
				else
					snes_draw_bgtile_lores(layer, ii, colour, pal, direct_colors, priority);
			}
			else /* hires */
			{
				if (mosaic)	
				{
					for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1); x_mos++)
						snes_draw_bgtile_hires(layer, ii + x_mos, colour, pal, direct_colors, priority);
					ii += x_mos - 1;
				}
				else
					snes_draw_bgtile_hires(layer, ii, colour, pal, direct_colors, priority);
			}
		}

		/* adjust tileaddr & x in case we have to draw again (for 16x16 tiles) */
		if (!flip)
			tileaddr += (8 * planes);
		else
			tileaddr -= (8 * planes);

		x += 8;
	}
}

/*************************************************************************************************
 * SNES BG layers
 *
 * BG drawing theory of each scanline is quite easy: depending on the graphics Mode (0-7), there
 * are up to 4 background layers. Pixels for each BG layer can have two different priorities.
 * Depending on the line and on the BGHOFS and BGVOFS PPU registers, we first determine the tile
 * address in snes_vram (by determining x,y coord and tile size and by calling snes_get_tmap_addr).
 * Then, we load the correspondent data and we determine the tile properties: which priority to
 * use, which palette etc. Finally, for each pixel of the tile appearing on screen, we check if
 * the tile priority is higher than the BG/OAM already stored in that pixel for that line. If so
 * we store the pixel in the buffer, otherwise we discard it.
 *
 * Of course, depending on the graphics Mode, it might be easier or harder to determine the proper
 * tile address in vram (Mode 7 uses different registers, Mode 2, 4 and 6 uses OPT effect, etc.),
 * but in general it works as described.
 *************************************************************************************************/

/*********************************************
 * snes_get_tmap_addr()
 *
 * Find the address in VRAM of the tile (x,y)
 *********************************************/

INLINE UINT32 snes_get_tmap_addr( UINT8 layer, UINT8 tile_size, UINT32 base, UINT32 x, UINT32 y )
{
	UINT32 res = base;
	x  >>= (3 + tile_size);
	y  >>= (3 + tile_size);

	res += (snes_ppu.layer[layer].tilemap_size & 2) ? ((y & 0x20) << ((snes_ppu.layer[layer].tilemap_size & 1) ? 7 : 6)) : 0;
	/* Scroll vertically */
	res += (y & 0x1f) << 6;
	/* Offset horizontally */
	res += (snes_ppu.layer[layer].tilemap_size & 1) ? ((x & 0x20) << 6) : 0;
	/* Scroll horizontally */
	res += (x & 0x1f) << 1;

	return res;
}

/*********************************************
 * snes_update_line()
 *
 * Update an entire line of tiles.
 *********************************************/

INLINE void snes_update_line( UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a, UINT8 color_depth, UINT8 hires, UINT8 offset_per_tile, UINT8 direct_colors )
{
	UINT32 tmap, tile, xoff, yoff;
	UINT32 charaddr;
	UINT16 ii = 0, vflip, hflip, pal, pal_direct;
	INT8 yscroll;
	UINT8 xscroll;
	UINT8 priority;
	UINT32 addr;
	UINT16 tilemap;
	/* scrolling */
	UINT16 opt_bit = (layer == SNES_BG1) ? 13 : (layer == SNES_BG2) ? 14 : 0;
	UINT8 tile_size;
	/* variables depending on color_depth */
	UINT8 color_planes = 2 << color_depth;
	/* below we cheat to simplify the code: 8BPP should have 0 pal offset, not 0x100 (but we take care of this by later using pal % FIXED_COLOUR) */
	UINT8 color_shift = 2 << color_depth;

#ifdef SNES_LAYER_DEBUG
	if (debug_options.bg_disabled[layer])
		return;
#endif /* SNES_LAYER_DEBUG */

	scanlines[SNES_MAINSCREEN].enable = snes_ppu.layer[layer].main_bg_enabled;
	scanlines[SNES_SUBSCREEN].enable = snes_ppu.layer[layer].sub_bg_enabled;
	scanlines[SNES_MAINSCREEN].clip = snes_ppu.layer[layer].main_window_enabled;
	scanlines[SNES_SUBSCREEN].clip = snes_ppu.layer[layer].sub_window_enabled;

	if (!scanlines[SNES_MAINSCREEN].enable && !scanlines[SNES_SUBSCREEN].enable)
		return;

	/* Handle Mosaic effects */
	if (snes_ppu.layer[layer].mosaic_enabled)
		curline -= (curline % (snes_ppu.mosaic_size + 1));

	if ((snes_ppu.interlace == 2) && !hires && !snes_ppu.pseudo_hires)
		curline /= 2;

	/* Find the size of the tiles (8x8 or 16x16) */
	tile_size = snes_ppu.layer[layer].tile_size;

	/* Find scroll info */
	xoff = snes_ppu.layer[layer].hoffs;
	yoff = snes_ppu.layer[layer].voffs;

	xscroll = xoff & ((1 << (3 + tile_size)) - 1);

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].tilemap << 9;
	charaddr = snes_ppu.layer[layer].charmap << 13;

	while (ii < 256 + (8 << tile_size))
	{
		// determine the horizontal position (Bishojo Janshi Suchi Pai & Desert Figther have tile_size & hires == 1)
		UINT32 xpos = xoff + (ii << (tile_size * hires));
		UINT32 ypos = yoff + curline;

		if (offset_per_tile != SNES_OPT_NONE)
		{
			int opt_x = ii + (xoff & 7);
			UINT32 haddr = 0, vaddr = 0;
			UINT16 hval = 0, vval = 0;

			if (opt_x >= 8)
			{
				switch (offset_per_tile)
				{
				case SNES_OPT_MODE2:
				case SNES_OPT_MODE6:
					haddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff));
					vaddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff) + 8);
					hval = snes_vram[haddr] | (snes_vram[haddr + 1] << 8);
					vval = snes_vram[vaddr] | (snes_vram[vaddr + 1] << 8);
					if (BIT(hval, opt_bit))
						xpos = opt_x + (hval & ~7);
					if (BIT(vval, opt_bit))
						ypos = curline + vval;
					break;
				case SNES_OPT_MODE4:
					haddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff));
					hval = snes_vram[haddr] | (snes_vram[haddr + 1] << 8);
					if (BIT(hval, opt_bit))
					{
						if (!BIT(hval, 15))
							xpos = opt_x + (hval & ~7);
						else
							ypos = curline + hval;
					}
					break;
				}
			}
		}

		addr = snes_get_tmap_addr(layer, tile_size, tmap, xpos, ypos);

		/*
        Tilemap format
          vhopppcc cccccccc

          v/h  = Vertical/Horizontal flip this tile.
          o    = Tile priority.
          ppp  = Tile palette. The number of entries in the palette depends on the Mode and the BG.
          cccccccccc = Tile number.
        */
		tilemap = snes_vram[addr] | (snes_vram[addr + 1] << 8);
		vflip = BIT(tilemap, 15);
		hflip = BIT(tilemap, 14);
		priority = BIT(tilemap, 13) ? priority_a : priority_b;
		pal_direct = ((tilemap & 0x1c00) >> 8);
		tile = tilemap & 0x03ff;

		pal = ((pal_direct >> 2) << color_shift);

		/* Mode 0 palettes are layer specific */
		if (snes_ppu.mode == 0)
		{
			pal += (layer << 5);
		}

#ifdef SNES_LAYER_DEBUG
		/* if we want to draw only one of the priorities of this layer */
		if (((debug_options.select_pri[layer] & 0x01) && (priority == priority_a)) ||
			((debug_options.select_pri[layer] & 0x02) && (priority == priority_b)))
		{
			if (!hires && tile_size)
				ii += 16;
			else
				ii += 8;
			continue;
		}
#endif /* SNES_LAYER_DEBUG */

		/* figure out which line to draw */
		yscroll = ypos & ((8 << tile_size) - 1);

		if (tile_size)
			if ((yscroll & 8) != vflip)
				tile += 16;

		if (yscroll > 7)
			yscroll &= 7;

		if (vflip)
			yscroll = -yscroll + 7;

		yscroll <<= 1;

		if (hires)
		{
			/* adjust tile for horizontal flip */
			if (hflip)
				tile += 1;

			/* draw 16 pixels (the routine will automatically send half of them to the mainscreen scanline and half to the subscreen one) */
			snes_draw_tile(color_planes, layer, charaddr + (tile  * 8 * color_planes) + yscroll, (ii - xscroll) * 2, 1, priority, hflip, direct_colors, direct_colors ? pal_direct : pal, hires);
			ii += 8;
		}
		else
		{
			/* adjust tile for horizontal flip */
			if (tile_size && hflip)
				tile += 1;

			snes_draw_tile(color_planes, layer, charaddr + (tile  * 8 * color_planes) + yscroll, ii - xscroll, tile_size, priority, hflip, direct_colors, direct_colors ? pal_direct : pal, hires);
			ii += (8 << tile_size);
		}
	}
}


/*********************************************
 * snes_update_line_mode7()
 *
 * Update an entire line of mode7 tiles.
 *********************************************/

#define MODE7_CLIP(x) (((x) & 0x2000) ? ((x) | ~0x03ff) : ((x) & 0x03ff))

static void snes_update_line_mode7( UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a )
{
	UINT32 tiled;
	INT16 ma, mb, mc, md;
	INT32 xc, yc, tx, ty, sx, sy, hs, vs, xpos, xdir, x0, y0;
	UINT8 priority = priority_b;
	UINT8 colour = 0;
	UINT16 *mosaic_x, *mosaic_y;
	UINT16 c;
	int screen;

#ifdef SNES_LAYER_DEBUG
	if (debug_options.bg_disabled[layer])
		return;
#endif /* SNES_LAYER_DEBUG */

	scanlines[SNES_MAINSCREEN].enable = snes_ppu.layer[layer].main_bg_enabled;
	scanlines[SNES_SUBSCREEN].enable = snes_ppu.layer[layer].sub_bg_enabled;
	scanlines[SNES_MAINSCREEN].clip = snes_ppu.layer[layer].main_window_enabled;
	scanlines[SNES_SUBSCREEN].clip = snes_ppu.layer[layer].sub_window_enabled;

	if (!scanlines[SNES_MAINSCREEN].enable && !scanlines[SNES_SUBSCREEN].enable)
		return;

	ma = snes_ppu.mode7.matrix_a;
	mb = snes_ppu.mode7.matrix_b;
	mc = snes_ppu.mode7.matrix_c;
	md = snes_ppu.mode7.matrix_d;
	xc = snes_ppu.mode7.origin_x;
	yc = snes_ppu.mode7.origin_y;
	hs = snes_ppu.mode7.hor_offset;
	vs = snes_ppu.mode7.ver_offset;

	/* Sign extend */
	xc <<= 19;
	xc >>= 19;
	yc <<= 19;
	yc >>= 19;
	hs <<= 19;
	hs >>= 19;
	vs <<= 19;
	vs >>= 19;

	/* Vertical flip */
	if (snes_ppu.mode7.vflip)
		sy = 255 - curline;
	else
		sy = curline;

	/* Horizontal flip */
	if (snes_ppu.mode7.hflip)
	{
		xpos = 255;
		xdir = -1;
	}
	else
	{
		xpos = 0;
		xdir = 1;
	}

	/* MOSAIC - to be verified */
	if (layer == SNES_BG2)	// BG2 use two different bits for horizontal and vertical mosaic
	{
		mosaic_x = snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG2].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y = snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}
	else	// BG1 works as usual
	{
		mosaic_x =  snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y =  snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}

#ifdef SNES_LAYER_DEBUG
	if (debug_options.mosaic_disabled)
	{
		mosaic_x =  snes_ppu.mosaic_table[0];
		mosaic_y =  snes_ppu.mosaic_table[0];
	}
#endif /* SNES_LAYER_DEBUG */

	/* Let's do some mode7 drawing huh? */
	/* These can be computed only once, since they do not depend on sx */
	x0 = ((ma * MODE7_CLIP(hs - xc)) & ~0x3f) + ((mb * mosaic_y[sy]) & ~0x3f) + ((mb * MODE7_CLIP(vs - yc)) & ~0x3f) + (xc << 8);
	y0 = ((mc * MODE7_CLIP(hs - xc)) & ~0x3f) + ((md * mosaic_y[sy]) & ~0x3f) + ((md * MODE7_CLIP(vs - yc)) & ~0x3f) + (yc << 8);

	for (sx = 0; sx < 256; sx++, xpos += xdir)
	{
		tx = (x0 + (ma * mosaic_x[sx])) >> 8;
		ty = (y0 + (mc * mosaic_x[sx])) >> 8;

		switch (snes_ppu.mode7.repeat)
		{
			case 0x00:	/* Repeat if outside screen area */
			case 0x01:	/* Repeat if outside screen area */
				tx &= 0x3ff;
				ty &= 0x3ff;
				tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
				colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
				break;
			case 0x02:	/* Single colour backdrop screen if outside screen area */
				if ((tx >= 0) && (tx < 1024) && (ty >= 0) && (ty < 1024))
				{
					tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
					colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
				}
				else
					colour = 0;
				break;
			case 0x03:	/* Character 0x00 repeat if outside screen area */
				if ((tx >= 0) && (tx < 1024) && (ty >= 0) && (ty < 1024))
					tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
				else
					tiled = 0;

				colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
				break;
		}

		/* The last bit is for priority in EXTBG mode (used only for BG2) */
		if (layer == SNES_BG2)
		{
			priority = ((colour & 0x80) >> 7) ? priority_a : priority_b;
			colour &= 0x7f;

#ifdef SNES_LAYER_DEBUG
		/* if we want to draw only one of the priorities of this layer */
		if (((debug_options.select_pri[layer] & 0x01) && (priority == priority_a)) ||
			((debug_options.select_pri[layer] & 0x02) && (priority == priority_b)))
			continue;
#endif /* SNES_LAYER_DEBUG */
		}

		for (screen = SNES_MAINSCREEN; screen <= SNES_SUBSCREEN; screen++)
		{
			if (scanlines[screen].enable)
			{
				UINT8 clr = colour;
				UINT8 clipmask = snes_ppu.clipmasks[layer][xpos];

#ifdef SNES_LAYER_DEBUG
				if (debug_options.windows_disabled)
					clipmask = 0xff;
#endif /* SNES_LAYER_DEBUG */

				/* Clip to windows */
				if (scanlines[screen].clip)
					clr &= clipmask;

				/* Draw pixel if appropriate */
				if (scanlines[screen].priority[xpos] <= priority && clr > 0)
				{
					/* Direct select, but only outside EXTBG! */
					// Direct color format is: 0 | BB000 | GGG00 | RRR00, HW confirms that the data is zero padded. 
					// In other words, like normal direct color, with pal = 0
					c = snes_get_bgcolor(snes_ppu.direct_color && layer == SNES_BG1, 0, clr);
					snes_set_scanline_pixel(screen, xpos, c, priority, layer, 0);
				}
			}
		}
	}
}

/*************************************************************************************************
 * SNES Sprites
 *
 * 1. First of all: sprites are drawn one line in advance. We emulate this by caching the
 * starting vram address, the sprite size and the "name select" at each line, and by using
 * them the next line to output the proper sprites - see snes_update_obsel.
 *
 * 2. Each line can select its sprites among 128 available ones in oam_ram, hence we start
 * by creating a list of the available objects (each one with its x,y coordinate, its size,
 * its tile address, etc.) - see snes_oam_list_build.
 *
 * 3. Next, we start finding out which sprites will appear in the line: starting from
 * FirstSprite, we count 32 OBJs which intersect our line and we store their indexes in the
 * oam_itemlist array (if more than 32 sprites intersect our line, we set the Range Over
 * flag); then, selecting among these sprites, we count 34 8x8 tiles which are visible
 * in our line (i.e. whose x coord is between -size and 256) and we store the corresponding
 * coordinates/priorities/palettes/etc. in the oam_tilelist array (if more than 34 tiles would
 * appear on screen, we set the Time Over flag).
 * Notice that when we populate oam_tilelist, we proceed from oam_itemlist[31] (or from the last
 * item which intersects the scanline), towards oam_itemlist[0], i.e. the higher tiles (say
 * oam_tilelist[34], or the last tile which appear on screen) will contain FirstSprite object,
 * or the sprites with closer index to FirstSprite which get displayed. This will play an
 * important role for sprite priority - see snes_update_objects_rto.
 *
 * 4. All the above happens at the beginning of each VIDEO_UPDATE. When we finally draw the
 * scanline, we pass through the oam_tilelist and we store the displayed pixels in our scanline
 * buffer. Notice that, for each pixel of a SNES sprite, only the priority of the topmost sprite
 * is tested against the priority of the BG pixel (because FirstSprite is on top of FirstSprite+1,
 * which is on top of FirstSprite+2, etc., and therefore other sprites are already covered by the
 * topmost one). To emulate this, we draw each tile over the previous ones no matter what
 * priorities are (differently from what we did with BGs): in the end, we will have in each pixel z
 * its topmost sprite and scanline.priority[z] will be the topmost sprite priority as expected.
 * Of course, sprite drawing must happen before BG drawing, so that afterwords BG pixels properly
 * test their priority with the one of the correct sprite - see snes_update_objects.
 *************************************************************************************************/

struct OAM
{
	UINT16 tile;
	INT16 x, y;
	UINT8 size, vflip, hflip, priority_bits, pal;
	int height, width;
};

static struct OAM oam_spritelist[SNES_SCR_WIDTH / 2];

static UINT8 oam_itemlist[32];

struct TILELIST {
	INT16 x;
	UINT16 priority, pal, tileaddr;
	int hflip;
};

struct TILELIST oam_tilelist[34];

/*********************************************
 * snes_update_obsel()
 *
 * Update sprite settings for next line.
 *********************************************/

static void snes_update_obsel( void )
{
	snes_ppu.layer[SNES_OAM].charmap = snes_ppu.oam.next_charmap;
	snes_ppu.oam.name_select = snes_ppu.oam.next_name_select;

	if (snes_ppu.oam.size != snes_ppu.oam.next_size)
	{
		snes_ppu.oam.size = snes_ppu.oam.next_size;
		snes_ppu.update_oam_list = 1;
	}
}

/*********************************************
 * snes_oam_list_build()
 *
 * Build a list of the available obj in OAM ram.
 *********************************************/

static void snes_oam_list_build( void )
{
	UINT8 *oamram = (UINT8 *)snes_oam;
	INT16 oam = 0x1ff;
	UINT16 oam_extra = oam + 0x20;
	UINT16 extra = 0;
	int ii;

	snes_ppu.update_oam_list = 0;		// eventually, we can optimize the code by only calling this function when there is a change in size

	for (ii = 127; ii >= 0; ii--)
	{
		if (((ii + 1) % 4) == 0)
			extra = oamram[oam_extra--];

		oam_spritelist[ii].vflip = (oamram[oam] & 0x80) >> 7;
		oam_spritelist[ii].hflip = (oamram[oam] & 0x40) >> 6;
		oam_spritelist[ii].priority_bits = (oamram[oam] & 0x30) >> 4;
		oam_spritelist[ii].pal = 128 + ((oamram[oam] & 0x0e) << 3);
		oam_spritelist[ii].tile = (oamram[oam--] & 0x1) << 8;
		oam_spritelist[ii].tile |= oamram[oam--];
		oam_spritelist[ii].y = oamram[oam--] + 1;	/* We seem to need to add one here.... */
		oam_spritelist[ii].x = oamram[oam--];
		oam_spritelist[ii].size = (extra & 0x80) >> 7;
		extra <<= 1;
		oam_spritelist[ii].x |= ((extra & 0x80) << 1);
		extra <<= 1;

		oam_spritelist[ii].y *= snes_ppu.obj_interlace;

		/* Adjust if past maximum position */
		if (oam_spritelist[ii].y >= snes_ppu.beam.last_visible_line * snes_ppu.interlace)
			oam_spritelist[ii].y -= 256 * snes_ppu.interlace;

		oam_spritelist[ii].x &= 0x1ff;

		/* Determine object size */
		switch (snes_ppu.oam.size)
		{
		case 0:			/* 8x8 or 16x16 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 2 : 1;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 2 : 1;
			break;
		case 1:			/* 8x8 or 32x32 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 1;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 4 : 1;
			break;
		case 2:			/* 8x8 or 64x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 8 : 1;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 1;
			break;
		case 3:			/* 16x16 or 32x32 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 4 : 2;
			break;
		case 4:			/* 16x16 or 64x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 8 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 2;
			break;
		case 5:			/* 32x32 or 64x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 8 : 4;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 4;
			break;
		case 6:			/* undocumented: 16x32 or 32x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 4;
			if (snes_ppu.obj_interlace && !oam_spritelist[ii].size)
				oam_spritelist[ii].height = 2;
			break;
		case 7:			/* undocumented: 16x32 or 32x32 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 4 : 4;
			if (snes_ppu.obj_interlace && !oam_spritelist[ii].size)
				oam_spritelist[ii].height = 2;
			break;
		default:
			/* we should never enter here... */
			logerror("Object size unsupported: %d\n", snes_ppu.oam.size);
			break;
		}
	}
}

/*********************************************
 * is_sprite_on_scanline()
 *
 * Check if a given sprites intersect current
 * scanline
 *********************************************/

static int is_sprite_on_scanline( UINT16 curline, UINT8 sprite )
{
	//if sprite is entirely offscreen and doesn't wrap around to the left side of the screen,
	//then it is not counted. this *should* be 256, and not 255, even though dot 256 is offscreen.
	int spr_height = (oam_spritelist[sprite].height << 3);

	if (oam_spritelist[sprite].x > 256 && (oam_spritelist[sprite].x + (oam_spritelist[sprite].width << 3) - 1) < 512)
		return 0;

	if (curline >= oam_spritelist[sprite].y && curline < (oam_spritelist[sprite].y + spr_height))
		return 1;

	if ((oam_spritelist[sprite].y + spr_height) >= 256 && curline < ((oam_spritelist[sprite].y + spr_height) & 255))
		return 1;

	return 0;
}

/*********************************************
 * snes_update_objects_rto()
 *
 * Determine which OBJs will be drawn on this
 * scanline.
 *********************************************/

static void snes_update_objects_rto( UINT16 curline )
{
	int ii, jj, active_sprite;
	UINT8 range_over, time_over;
	INT8 xs, ys;
	UINT8 line;
	UINT8 height, width, vflip, hflip, priority, pal;
	UINT16 tile;
	INT16 x, y;
	UINT32 name_sel = 0;

	snes_oam_list_build();

	/* initialize counters */
	range_over = 0;
	time_over = 0;

	/* setup the proper line */
	curline /= snes_ppu.interlace;
	curline *= snes_ppu.obj_interlace;

	/* reset the list of first 32 objects which intersect current scanline */
	memset(oam_itemlist, 0xff, 32);

	/* populate the list of 32 objects */
	for (ii = 0; ii < 128; ii++)
	{
		active_sprite = (ii + snes_ppu.oam.first_sprite) & 0x7f;

		if (!is_sprite_on_scanline(curline, active_sprite))
			continue;

		if (range_over++ >= 32)
			break;

		oam_itemlist[range_over - 1] = active_sprite;
	}

	/* reset the list of first 34 tiles to be drawn */
	for (ii = 0; ii < 34; ii++)
		oam_tilelist[ii].tileaddr = 0xffff;

	/* populate the list of 34 tiles */
	for (ii = 31; ii >= 0; ii--)
	{
		if (oam_itemlist[ii] == 0xff)
			continue;

		active_sprite = oam_itemlist[ii];

		tile = oam_spritelist[active_sprite].tile;
		x = oam_spritelist[active_sprite].x;
		y = oam_spritelist[active_sprite].y;
		height = oam_spritelist[active_sprite].height;
		width = oam_spritelist[active_sprite].width;
		vflip = oam_spritelist[active_sprite].vflip;
		hflip = oam_spritelist[active_sprite].hflip;
		priority = oam_spritelist[active_sprite].priority_bits;
		pal = oam_spritelist[active_sprite].pal;

		/* Only objects using tiles over 255 use name select */
		name_sel = (tile < 256) ? 0 : snes_ppu.oam.name_select;

		ys = (curline - y) >> 3;
		line = (curline - y) % 8;
		if (vflip)
		{
			ys = height - ys - 1;
			line = (-1 * line) + 7;
		}
		line <<= 1;
		tile <<= 5;

		for (jj = 0; jj < width; jj++)
		{
			INT16 xx = (x + (jj << 3)) & 0x1ff;

			if (x != 256 && xx >= 256 && (xx + 7) < 512)
				continue;

			if (time_over++ >= 34)
				break;

			xs = (hflip) ? (width - 1 - jj) : jj;
			oam_tilelist[time_over - 1].tileaddr = name_sel + tile + table_obj_offset[ys][xs] + line;
			oam_tilelist[time_over - 1].hflip = hflip;
			oam_tilelist[time_over - 1].x = xx;
			oam_tilelist[time_over - 1].pal = pal;
			oam_tilelist[time_over - 1].priority = priority;
		}
	}

	/* set Range Over flag if necessary */
	if (range_over > 32)
		snes_ppu.stat77_flags |= 0x40;

	/* set Time Over flag if necessary */
	if (time_over > 34)
		snes_ppu.stat77_flags |= 0x80;
}

/*********************************************
 * snes_update_objects()
 *
 * Update an entire line of sprites.
 *********************************************/

static void snes_update_objects( UINT8 priority_oam0, UINT8 priority_oam1, UINT8 priority_oam2, UINT8 priority_oam3 )
{
	UINT8 pri, priority[4];
	UINT32 charaddr;
	int ii;

#ifdef SNES_LAYER_DEBUG
	if (debug_options.bg_disabled[SNES_OAM])
		return;
#endif /* SNES_LAYER_DEBUG */

	scanlines[SNES_MAINSCREEN].enable = snes_ppu.layer[SNES_OAM].main_bg_enabled;
	scanlines[SNES_SUBSCREEN].enable = snes_ppu.layer[SNES_OAM].sub_bg_enabled;
	scanlines[SNES_MAINSCREEN].clip = snes_ppu.layer[SNES_OAM].main_window_enabled;
	scanlines[SNES_SUBSCREEN].clip = snes_ppu.layer[SNES_OAM].sub_window_enabled;

	if (!scanlines[SNES_MAINSCREEN].enable && !scanlines[SNES_SUBSCREEN].enable)
		return;

	charaddr = snes_ppu.layer[SNES_OAM].charmap << 13;

	priority[0] = priority_oam0;
	priority[1] = priority_oam1;
	priority[2] = priority_oam2;
	priority[3] = priority_oam3;

	/* finally draw the tiles from the tilelist */
	for (ii = 0; ii < 34; ii++)
	{
		int tile = ii;
#ifdef SNES_LAYER_DEBUG
		if (debug_options.sprite_reversed)
			tile = 33 - ii;
#endif /* SNES_LAYER_DEBUG */

		if (oam_tilelist[tile].tileaddr == 0xffff)
			continue;

		pri = priority[oam_tilelist[tile].priority];

#ifdef SNES_LAYER_DEBUG
		if (debug_options.select_pri[SNES_OAM])
		{
			int oam_draw = debug_options.select_pri[SNES_OAM] - 1;
			if (oam_draw != oam_tilelist[tile].priority)
				continue;
		}
#endif /* SNES_LAYER_DEBUG */

		/* OAM tiles have fixed planes (4), fixed tile_size (8x8), no direct color and no hires, but otherwise work the same as BG ones */
		snes_draw_tile(4, SNES_OAM, charaddr + oam_tilelist[tile].tileaddr, oam_tilelist[tile].x, 0, pri, oam_tilelist[tile].hflip, 0, oam_tilelist[tile].pal, 0);
	}
}


/*********************************************
 * snes_update_mode_X()
 *
 * Update Mode X line.
 *********************************************/

static void snes_update_mode_0( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[0])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(3, 6, 9, 12);
	snes_update_line(curline, SNES_BG1, 8, 11, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG2, 7, 10, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG3, 2, 5,  SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG4, 1, 4,  SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
}

static void snes_update_mode_1( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[1])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!snes_ppu.bg3_priority_bit)
	{
		snes_update_objects(2, 4, 7, 10);
		snes_update_line(curline, SNES_BG1, 6, 9, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_NONE, 0);
		snes_update_line(curline, SNES_BG2, 5, 8, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_NONE, 0);
		snes_update_line(curline, SNES_BG3, 1, 3, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	}
	else
	{
		snes_update_objects(2, 3, 6, 9);
		snes_update_line(curline, SNES_BG1, 5, 8,  SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_NONE, 0);
		snes_update_line(curline, SNES_BG2, 4, 7,  SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_NONE, 0);
		snes_update_line(curline, SNES_BG3, 1, 10, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	}
}

static void snes_update_mode_2( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[2])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_MODE2, 0);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_MODE2, 0);
}

static void snes_update_mode_3( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[3])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_8BPP, 0, SNES_OPT_NONE, snes_ppu.direct_color);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_NONE, 0);
}

static void snes_update_mode_4( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[4])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_8BPP, 0, SNES_OPT_MODE4, snes_ppu.direct_color);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_MODE4, 0);
}

static void snes_update_mode_5( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[5])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_4BPP, 1, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_2BPP, 1, SNES_OPT_NONE, 0);
}

static void snes_update_mode_6( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[6])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(1, 3, 4, 6);
	snes_update_line(curline, SNES_BG1, 2, 5, SNES_COLOR_DEPTH_4BPP, 1, SNES_OPT_MODE6, 0);
}

static void snes_update_mode_7( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[7])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!snes_ppu.mode7.extbg)
	{
		snes_update_objects(1, 3, 4, 5);
		snes_update_line_mode7(curline, SNES_BG1, 2, 2);
	}
	else
	{
		snes_update_objects(2, 4, 6, 7);
		snes_update_line_mode7(curline, SNES_BG1, 3, 3);
		snes_update_line_mode7(curline, SNES_BG2, 1, 5);
	}
}

/*********************************************
 * snes_draw_screens()
 *
 * Draw the whole screen (Mode 0 -> 7).
 *********************************************/

static void snes_draw_screens( UINT16 curline )
{
	switch (snes_ppu.mode)
	{
		case 0: snes_update_mode_0(curline); break;		/* Mode 0 */
		case 1: snes_update_mode_1(curline); break;		/* Mode 1 */
		case 2: snes_update_mode_2(curline); break;		/* Mode 2 - Supports offset per tile */
		case 3: snes_update_mode_3(curline); break;		/* Mode 3 - Supports direct colour */
		case 4: snes_update_mode_4(curline); break;		/* Mode 4 - Supports offset per tile and direct colour */
		case 5: snes_update_mode_5(curline); break;		/* Mode 5 - Supports hires */
		case 6: snes_update_mode_6(curline); break;		/* Mode 6 - Supports offset per tile and hires */
		case 7: snes_update_mode_7(curline); break;		/* Mode 7 - Supports direct colour */
	}
}

/*********************************************
 * snes_update_windowmasks()
 *
 * An example of how windows work:
 * Win1: ...#####......
 * Win2: ......#####...
 *             IN                 OUT
 * OR:   ...########...     ###........###
 * AND:  ......##......     ######..######
 * XOR:  ...###..###...     ###...##...###
 * XNOR: ###...##...###     ...###..###...
 *********************************************/

static void snes_update_windowmasks( void )
{
	UINT16 ii, jj;
	INT8 w1, w2;

	snes_ppu.update_windows = 0;		/* reset the flag */

	for (ii = 0; ii < SNES_SCR_WIDTH; ii++)
	{
		/* update bg 1, 2, 3, 4, obj & color windows */
		/* jj = layer */
		for (jj = 0; jj < 6; jj++)
		{
			snes_ppu.clipmasks[jj][ii] = 0xff;	/* let's start from un-masked */
			w1 = w2 = -1;

			if (snes_ppu.layer[jj].window1_enabled)
			{
				/* Default to mask area inside */
				if ((ii < snes_ppu.window1_left) || (ii > snes_ppu.window1_right))
					w1 = 0;
				else
					w1 = 1;

				/* If mask area is outside then swap */
				if (snes_ppu.layer[jj].window1_invert)
					w1 = !w1;
			}

			if (snes_ppu.layer[jj].window2_enabled)
			{
				if ((ii < snes_ppu.window2_left) || (ii > snes_ppu.window2_right))
					w2 = 0;
				else
					w2 = 1;
				if (snes_ppu.layer[jj].window2_invert)
					w2 = !w2;
			}

			/* mask if the appropriate expression is true */
			if (w1 >= 0 && w2 >= 0)
			{
				switch (snes_ppu.layer[jj].wlog_mask)
				{
					case 0x00:	/* OR */
						snes_ppu.clipmasks[jj][ii] = w1 | w2 ? 0x00 : 0xff;
						break;
					case 0x01:	/* AND */
						snes_ppu.clipmasks[jj][ii] = w1 & w2 ? 0x00 : 0xff;
						break;
					case 0x02:	/* XOR */
						snes_ppu.clipmasks[jj][ii] = w1 ^ w2 ? 0x00 : 0xff;
						break;
					case 0x03:	/* XNOR */
						snes_ppu.clipmasks[jj][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
						break;
				}
			}
			else if (w1 >= 0)
				snes_ppu.clipmasks[jj][ii] = w1 ? 0x00 : 0xff;
			else if (w2 >= 0)
				snes_ppu.clipmasks[jj][ii] = w2 ? 0x00 : 0xff;
		}
	}
}

/*********************************************
 * snes_update_offsets()
 *
 * Update the offsets with the latest changes.
 * This is currently unused, but it could
 * possibly be handy for some minor optimization
 *********************************************/

static void snes_update_offsets( void )
{
	int ii;
	for (ii = 0; ii < 4; ii++)
	{
	}
	snes_ppu.update_offsets = 0;
}

/*****************************************
 * snes_draw_blend()
 *
 * Routine for additive/subtractive blending
 * between the main and sub screens, i.e.
 * color math.
 *****************************************/

INLINE void snes_draw_blend( UINT16 offset, UINT16 *colour, UINT8 prevent_color_math, UINT8 black_pen_clip, int switch_screens )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.colormath_disabled)
		return;
#endif /* SNES_LAYER_DEBUG */

	/* when color math is applied to subscreen pixels, the blending depends on the blending used by the previous mainscreen
    pixel, except for subscreen pixel 0 which has no previous mainscreen pixel, see comments in snes_refresh_scanline */
	if (switch_screens && offset > 0)
		offset -= 1;

	if ((black_pen_clip == SNES_CLIP_ALWAYS) ||
		(black_pen_clip == SNES_CLIP_IN && snes_ppu.clipmasks[SNES_COLOR][offset]) ||
		(black_pen_clip == SNES_CLIP_OUT && !snes_ppu.clipmasks[SNES_COLOR][offset]))
		*colour = 0; //clip to black before color math

	if (prevent_color_math == SNES_CLIP_ALWAYS) // blending mode 3 == always OFF
		return;

	if ((prevent_color_math == SNES_CLIP_NEVER) ||
		(prevent_color_math == SNES_CLIP_IN  && !snes_ppu.clipmasks[SNES_COLOR][offset]) ||
		(prevent_color_math == SNES_CLIP_OUT && snes_ppu.clipmasks[SNES_COLOR][offset]))
	{
		UINT16 r, g, b;
		struct SCANLINE *subscreen;
		int clip_max = 0;	// if add then clip to 0x1f, if sub then clip to 0

#ifdef SNES_LAYER_DEBUG
		/* Toggle drawing of SNES_SUBSCREEN or SNES_MAINSCREEN */
		if (debug_options.draw_subscreen)
		{
			subscreen = switch_screens ? &scanlines[SNES_SUBSCREEN] : &scanlines[SNES_MAINSCREEN];
		}
		else
#endif /* SNES_LAYER_DEBUG */
		{
			subscreen = switch_screens ? &scanlines[SNES_MAINSCREEN] : &scanlines[SNES_SUBSCREEN];
		}

		if (snes_ppu.sub_add_mode) /* SNES_SUBSCREEN*/
		{
			if (!BIT(snes_ppu.color_modes, 7))
			{
				/* 0x00 add */
				r = (*colour & 0x1f) + (subscreen->buffer[offset] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) + ((subscreen->buffer[offset] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) + ((subscreen->buffer[offset] & 0x7c00) >> 10);
				clip_max = 1;
			}
			else
			{
				/* 0x80 sub */
				r = (*colour & 0x1f) - (subscreen->buffer[offset] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) - ((subscreen->buffer[offset] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) - ((subscreen->buffer[offset] & 0x7c00) >> 10);
				if (r > 0x1f) r = 0;
				if (g > 0x1f) g = 0;
				if (b > 0x1f) b = 0;
			}
			/* only halve if the color is not the back colour */
			if (BIT(snes_ppu.color_modes, 6) && (subscreen->buffer[offset] != snes_cgram[FIXED_COLOUR]))
			{
				r >>= 1;
				g >>= 1;
				b >>= 1;
			}
		}
		else /* Fixed colour */
		{
			if (!BIT(snes_ppu.color_modes, 7))
			{
				/* 0x00 add */
				r = (*colour & 0x1f) + (snes_cgram[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) + ((snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) + ((snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
				clip_max = 1;
			}
			else
			{
				/* 0x80: sub */
				r = (*colour & 0x1f) - (snes_cgram[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) - ((snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) - ((snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
				if (r > 0x1f) r = 0;
				if (g > 0x1f) g = 0;
				if (b > 0x1f) b = 0;
			}
			/* halve if necessary */
			if (BIT(snes_ppu.color_modes, 6))
			{
				r >>= 1;
				g >>= 1;
				b >>= 1;
			}
		}

		/* according to anomie's docs, after addition has been performed, division by 2 happens *before* clipping to max, hence we clip now */
		if (clip_max)
		{
			if (r > 0x1f) r = 0x1f;
			if (g > 0x1f) g = 0x1f;
			if (b > 0x1f) b = 0x1f;
		}

		*colour = ((r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10));
	}
}

/*********************************************
 * snes_refresh_scanline()
 *
 * Redraw the current line.
 *********************************************/
/*********************************************
 * Notice that in hires and pseudo hires modes,
 * i.e. when 512 different pixels are present
 * in a scanline, a crt TV monitor would end
 * up blending adjacent pixels. To mimic this,
 * we add a small (optional) hack which enters
 * only in the very last stage of the scanline
 * drawing and which simulates the TV by
 * replacing the exact pixel color with an
 * average of the current and next pixel colors.
 * Credits (and thanks) to Blargg and Byuu for
 * the optimized averaging algorithm.
 *********************************************/

static void snes_refresh_scanline( running_machine *machine, bitmap_t *bitmap, UINT16 curline )
{
	UINT16 ii;
	int x;
	int fade;
	struct SCANLINE *scanline1, *scanline2;
	UINT16 c;
	UINT16 prev_colour = 0;
	int blurring = input_port_read_safe(machine, "OPTIONS", 0) & 0x01;

	profiler_mark_start(PROFILER_VIDEO);

	if (snes_ppu.screen_disabled) /* screen is forced blank */
		for (x = 0; x < SNES_SCR_WIDTH * 2; x++)
			*BITMAP_ADDR32(bitmap, curline, x) = RGB_BLACK;
	else
	{
		/* Update clip window masks if necessary */
		if (snes_ppu.update_windows)
			snes_update_windowmasks();
		/* Update the offsets if necessary */
		if (snes_ppu.update_offsets)
			snes_update_offsets();

		/* Clear priority */
		memset(scanlines[SNES_MAINSCREEN].priority, 0, SNES_SCR_WIDTH);
		memset(scanlines[SNES_SUBSCREEN].priority, 0, SNES_SCR_WIDTH);

		/* Clear layers */
		memset(scanlines[SNES_MAINSCREEN].layer, SNES_COLOR, SNES_SCR_WIDTH);
		memset(scanlines[SNES_SUBSCREEN].layer, SNES_COLOR, SNES_SCR_WIDTH);

		/* Clear blend_exception (only used for OAM) */
		memset(scanlines[SNES_MAINSCREEN].blend_exception, 0, SNES_SCR_WIDTH);
		memset(scanlines[SNES_SUBSCREEN].blend_exception, 0, SNES_SCR_WIDTH);

		/* Draw back colour */
		for (ii = 0; ii < SNES_SCR_WIDTH; ii++)
		{
			if (snes_ppu.mode == 5 || snes_ppu.mode == 6 || snes_ppu.pseudo_hires)
				scanlines[SNES_SUBSCREEN].buffer[ii] = snes_cgram[0];
			else
				scanlines[SNES_SUBSCREEN].buffer[ii] = snes_cgram[FIXED_COLOUR];

			scanlines[SNES_MAINSCREEN].buffer[ii] = snes_cgram[0];
		}

		/* Prepare OAM for this scanline */
		snes_update_objects_rto(curline);

		/* Draw scanline */
		snes_draw_screens(curline);

		snes_update_obsel();

#ifdef SNES_LAYER_DEBUG
		if (snes_dbg_video(machine, curline))
		{
			profiler_mark_end();
			return;
		}

		/* Toggle drawing of SNES_SUBSCREEN or SNES_MAINSCREEN */
		if (debug_options.draw_subscreen)
		{
			scanline1 = &scanlines[SNES_SUBSCREEN];
			scanline2 = &scanlines[SNES_MAINSCREEN];
		}
		else
#endif /* SNES_LAYER_DEBUG */
		{
			scanline1 = &scanlines[SNES_MAINSCREEN];
			scanline2 = &scanlines[SNES_SUBSCREEN];
		}

		/* Draw the scanline to screen */

		fade = snes_ppu.screen_brightness;

		for (x = 0; x < SNES_SCR_WIDTH; x++)
		{
			int r, g, b, hires;
			UINT16 tmp_col[2];
			hires = (snes_ppu.mode != 5 && snes_ppu.mode != 6 && !snes_ppu.pseudo_hires) ? 0 : 1;

			/* in hires, the first pixel (of 512) is subscreen pixel, then the first mainscreen pixel follows, and so on... */
			if (!hires)
			{
				c = scanline1->buffer[x];

				/* perform color math if the layer wants it (except if it's an object > 192) */
				if (!scanline1->blend_exception[x] && snes_ppu.layer[scanline1->layer[x]].color_math)
					snes_draw_blend(x, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 0);

				r = ((c & 0x1f) * fade) >> 4;
				g = (((c & 0x3e0) >> 5) * fade) >> 4;
				b = (((c & 0x7c00) >> 10) * fade) >> 4;

				*BITMAP_ADDR32(bitmap, curline, x * 2 + 0) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
				*BITMAP_ADDR32(bitmap, curline, x * 2 + 1) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
			}
			else
			{
				/* prepare the pixel from main screen */
				c = scanline1->buffer[x];

				/* perform color math if the layer wants it (except if it's an object > 192) */
				if (!scanline1->blend_exception[x] && snes_ppu.layer[scanline1->layer[x]].color_math)
					snes_draw_blend(x, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 0);

				tmp_col[1] = c;

				/* prepare the pixel from sub screen */
				c = scanline2->buffer[x];

				/* in hires/pseudo-hires, subscreen pixels are blended as well: for each subscreen pixel, color math
                is applied if it had been applied to the previous mainscreen pixel. What happens at subscreen pixel 0
                (which has no previous mainscreen pixel) is undocumented. Until more info are discovered, we (arbitrarily)
                apply to it the same color math as the *next* mainscreen pixel (i.e. mainscreen pixel 0), which seems as good as
                any other choice */
				if (x == 0 && !scanline1->blend_exception[0] && snes_ppu.layer[scanline1->layer[0]].color_math)
					snes_draw_blend(0, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 1);
				else if (x > 0  && !scanline1->blend_exception[x - 1] && snes_ppu.layer[scanline1->layer[x - 1]].color_math)
					snes_draw_blend(x, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 1);

				tmp_col[0] = c;

				/* average the first pixel if required, or draw it directly*/
				if (blurring)
					c = (prev_colour + tmp_col[0] - ((prev_colour ^ tmp_col[0]) & 0x0421)) >> 1;	// Hack code to mimic TV pixel blurring
				else
					c = tmp_col[0];

				r = ((c & 0x1f) * fade) >> 4;
				g = (((c & 0x3e0) >> 5) * fade) >> 4;
				b = (((c & 0x7c00) >> 10) * fade) >> 4;

				*BITMAP_ADDR32(bitmap, curline, x * 2 + 0) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
				prev_colour = tmp_col[0];

				/* average the second pixel if required, or draw it directly*/
				if (blurring)
					c = (prev_colour + tmp_col[1] - ((prev_colour ^ tmp_col[1]) & 0x0421)) >> 1;	// Hack code to mimic TV pixel blurring
				else
					c = tmp_col[1];

				r = ((c & 0x1f) * fade) >> 4;
				g = (((c & 0x3e0) >> 5) * fade) >> 4;
				b = (((c & 0x7c00) >> 10) * fade) >> 4;

				*BITMAP_ADDR32(bitmap, curline, x * 2 + 1) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
				prev_colour = tmp_col[1];
			}
		}
	}

	profiler_mark_end();
}

VIDEO_START( snes )
{
#ifdef SNES_LAYER_DEBUG
	memset(&debug_options, 0, sizeof(debug_options));
#endif
}

VIDEO_UPDATE( snes )
{
	int y;

	/*NTSC SNES draw range is 1-225. */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		snes_refresh_scanline(screen->machine, bitmap, y + 1);
	}
	return 0;
}


/***** Debug Functions *****/

#ifdef SNES_LAYER_DEBUG

#define DEBUG_TOGGLE(bit, debug_settings, MSG1, MSG2) \
	if (BIT(toggles, bit) && !debug_settings)       \
	{                                               \
		debug_settings = 1;                       \
		popmessage MSG1;                          \
	}                                               \
	else if (!BIT(toggles, bit) && debug_settings)  \
	{                                               \
		debug_settings = 0;                       \
		popmessage MSG2;                          \
	}                                               \


static UINT8 snes_dbg_video( running_machine *machine, UINT16 curline )
{
	int i;
	UINT8 toggles = input_port_read_safe(machine, "DEBUG1", 0);
	debug_options.select_pri[SNES_BG1] = (toggles & 0x03);
	debug_options.select_pri[SNES_BG2] = (toggles & 0x0c) >> 2;
	debug_options.select_pri[SNES_BG3] = (toggles & 0x30) >> 4;
	debug_options.select_pri[SNES_BG4] = (toggles & 0xc0) >> 6;

	toggles = input_port_read_safe(machine, "DEBUG2", 0);
	for (i = 0; i < 4; i++)
		DEBUG_TOGGLE(i, debug_options.bg_disabled[i], ("Debug: Disabled BG%d.\n", i + 1), ("Debug: Enabled BG%d.\n", i + 1))
	DEBUG_TOGGLE(4, debug_options.bg_disabled[SNES_OAM], ("Debug: Disabled OAM.\n"), ("Debug: Enabled OAM.\n"))
	DEBUG_TOGGLE(5, debug_options.draw_subscreen, ("Debug: Switched screens.\n"), ("Debug: Switched screens.\n"))
	DEBUG_TOGGLE(6, debug_options.colormath_disabled, ("Debug: Disabled Color Math.\n"), ("Debug: Enabled Color Math.\n"))
	DEBUG_TOGGLE(7, debug_options.windows_disabled, ("Debug: Disabled Window Masks.\n"), ("Debug: Enabled Window Masks.\n"))

	toggles = input_port_read_safe(machine, "DEBUG4", 0);
	for (i = 0; i < 8; i++)
		DEBUG_TOGGLE(i, debug_options.mode_disabled[i], ("Debug: Disabled Mode %d drawing.\n", i), ("Debug: Enabled Mode %d drawing.\n", i))

	toggles = input_port_read_safe(machine, "DEBUG3", 0);
	DEBUG_TOGGLE(2, debug_options.mosaic_disabled, ("Debug: Disabled Mosaic.\n"), ("Debug: Enabled Mosaic.\n"))
	debug_options.sprite_reversed = BIT(toggles, 7);
	debug_options.select_pri[SNES_OAM] = (toggles & 0x70) >> 4;

#ifdef MAME_DEBUG
	/* Once per frame, log video properties */
	if (curline == 1)
	{
		static const char WINLOGIC[4] = { '|', '&', '^', '!' };

		logerror("%s", debug_options.windows_disabled?" ":"W");
		logerror("%s1 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
				debug_options.bg_disabled[0]?" ":"*",
				(snes_ram[TM] & 0x1)?"M":" ",
				(snes_ram[TS] & 0x1)?"S":" ",
				(snes_ram[CGADSUB] & 0x1)?"B":" ",
				(snes_ram[TMW] & 0x1)?"m":" ",
				(snes_ram[TSW] & 0x1)?"s":" ",
				WINLOGIC[(snes_ram[WBGLOG] & 0x3)],
				(snes_ram[W12SEL] & 0x2)?((snes_ram[W12SEL] & 0x1)?"o":"i"):" ",
				(snes_ram[W12SEL] & 0x8)?((snes_ram[W12SEL] & 0x4)?"o":"i"):" ",
				snes_ppu.layer[SNES_BG1].tile_size + 1,
				(snes_ram[MOSAIC] & 0x1)?"m":" ",
				snes_ram[BG1SC] & 0x3,
				(snes_ram[BG1SC] & 0xfc) << 9,
				snes_ppu.layer[SNES_BG1].charmap << 13);
		logerror("%s2 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
				debug_options.bg_disabled[1]?" ":"*",
				(snes_ram[TM] & 0x2)?"M":" ",
				(snes_ram[TS] & 0x2)?"S":" ",
				(snes_ram[CGADSUB] & 0x2)?"B":" ",
				(snes_ram[TMW] & 0x2)?"m":" ",
				(snes_ram[TSW] & 0x2)?"s":" ",
				WINLOGIC[(snes_ram[WBGLOG] & 0xc) >> 2],
				(snes_ram[W12SEL] & 0x20)?((snes_ram[W12SEL] & 0x10)?"o":"i"):" ",
				(snes_ram[W12SEL] & 0x80)?((snes_ram[W12SEL] & 0x40)?"o":"i"):" ",
				snes_ppu.layer[SNES_BG2].tile_size + 1,
				(snes_ram[MOSAIC] & 0x2)?"m":" ",
				snes_ram[BG2SC] & 0x3,
				(snes_ram[BG2SC] & 0xfc) << 9,
				snes_ppu.layer[SNES_BG2].charmap << 13);
		logerror("%s3 %s%s%s%s%s%c%s%s%d%s%s%d %4X %4X",
				debug_options.bg_disabled[2]?" ":"*",
				(snes_ram[TM] & 0x4)?"M":" ",
				(snes_ram[TS] & 0x4)?"S":" ",
				(snes_ram[CGADSUB] & 0x4)?"B":" ",
				(snes_ram[TMW] & 0x4)?"m":" ",
				(snes_ram[TSW] & 0x4)?"s":" ",
				WINLOGIC[(snes_ram[WBGLOG] & 0x30)>>4],
				(snes_ram[W34SEL] & 0x2)?((snes_ram[W34SEL] & 0x1)?"o":"i"):" ",
				(snes_ram[W34SEL] & 0x8)?((snes_ram[W34SEL] & 0x4)?"o":"i"):" ",
				snes_ppu.layer[SNES_BG3].tile_size + 1,
				(snes_ram[MOSAIC] & 0x4)?"m":" ",
				(snes_ram[BGMODE] & 0x8)?"P":" ",
				snes_ram[BG3SC] & 0x3,
				(snes_ram[BG3SC] & 0xfc) << 9,
				snes_ppu.layer[SNES_BG3].charmap << 13);
		logerror("%s4 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
				debug_options.bg_disabled[3]?" ":"*",
				(snes_ram[TM] & 0x8)?"M":" ",
				(snes_ram[TS] & 0x8)?"S":" ",
				(snes_ram[CGADSUB] & 0x8)?"B":" ",
				(snes_ram[TMW] & 0x8)?"m":" ",
				(snes_ram[TSW] & 0x8)?"s":" ",
				WINLOGIC[(snes_ram[WBGLOG] & 0xc0)>>6],
				(snes_ram[W34SEL] & 0x20)?((snes_ram[W34SEL] & 0x10)?"o":"i"):" ",
				(snes_ram[W34SEL] & 0x80)?((snes_ram[W34SEL] & 0x40)?"o":"i"):" ",
				snes_ppu.layer[SNES_BG4].tile_size + 1,
				(snes_ram[MOSAIC] & 0x8)?"m":" ",
				snes_ram[BG4SC] & 0x3,
				(snes_ram[BG4SC] & 0xfc) << 9,
				snes_ppu.layer[SNES_BG4].charmap << 13 );
		logerror("%sO %s%s%s%s%s%c%s%s       %4X",
				debug_options.bg_disabled[4]?" ":"*",
				(snes_ram[TM] & 0x10)?"M":" ",
				(snes_ram[TS] & 0x10)?"S":" ",
				(snes_ram[CGADSUB] & 0x10)?"B":" ",
				(snes_ram[TMW] & 0x10)?"m":" ",
				(snes_ram[TSW] & 0x10)?"s":" ",
				WINLOGIC[(snes_ram[WOBJLOG] & 0x3)],
				(snes_ram[WOBJSEL] & 0x2)?((snes_ram[WOBJSEL] & 0x1)?"o":"i"):" ",
				(snes_ram[WOBJSEL] & 0x8)?((snes_ram[WOBJSEL] & 0x4)?"o":"i"):" ",
				snes_ppu.layer[SNES_OAM].charmap << 13 );
		logerror("%sB   %s  %c%s%s",
				debug_options.colormath_disabled?" ":"*",
				(snes_ram[CGADSUB] & 0x20)?"B":" ",
				WINLOGIC[(snes_ram[WOBJLOG] & 0xc)>>2],
				(snes_ram[WOBJSEL] & 0x20)?((snes_ram[WOBJSEL] & 0x10)?"o":"i"):" ",
				(snes_ram[WOBJSEL] & 0x80)?((snes_ram[WOBJSEL] & 0x40)?"o":"i"):" " );
		logerror("1) %3d %3d   2) %3d %3d", (snes_ppu.bgd_offset.horizontal[0] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[0] & 0x3ff) >> 3, (snes_ppu.bgd_offset.horizontal[1] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[1] & 0x3ff) >> 3 );
		logerror("3) %3d %3d   4) %3d %3d", (snes_ppu.bgd_offset.horizontal[2] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[2] & 0x3ff) >> 3, (snes_ppu.bgd_offset.horizontal[3] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[3] & 0x3ff) >> 3 );
		logerror("Flags: %s%s%s %s %2d", (snes_ram[CGWSEL] & 0x2)?"S":"F", (snes_ram[CGADSUB] & 0x80)?"-":"+", (snes_ram[CGADSUB] & 0x40)?" 50%":"100%",(snes_ram[CGWSEL] & 0x1)?"D":"P", (snes_ram[MOSAIC] & 0xf0) >> 4 );
		logerror("SetINI: %s %s %s %s %s %s", (snes_ram[SETINI] & 0x1)?" I":"NI", (snes_ram[SETINI] & 0x2)?"P":"R", (snes_ram[SETINI] & 0x4)?"240":"225",(snes_ram[SETINI] & 0x8)?"512":"256",(snes_ram[SETINI] & 0x40)?"E":"N",(snes_ram[SETINI] & 0x80)?"ES":"NS" );
		logerror("Mode7: A %5d B %5d", snes_ppu.mode7.matrix_a, snes_ppu.mode7.matrix_b );
		logerror(" %s%s%s   C %5d D %5d", (snes_ram[M7SEL] & 0xc0)?((snes_ram[M7SEL] & 0x40)?"0":"C"):"R", (snes_ram[M7SEL] & 0x1)?"H":" ", (snes_ram[M7SEL] & 0x2)?"V":" ", snes_ppu.mode7.matrix_c, snes_ppu.mode7.matrix_d );
		logerror("       X %5d Y %5d", snes_ppu.mode7.origin_x, snes_ppu.mode7.origin_y );
	}
#endif

	return 0;
}
#endif /* SNES_LAYER_DEBUG */
