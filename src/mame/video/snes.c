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
#include "includes/snes.h"

#define SNES_MAINSCREEN    0
#define SNES_SUBSCREEN     1
#define SNES_CLIP_NEVER    0
#define SNES_CLIP_IN       1
#define SNES_CLIP_OUT      2
#define SNES_CLIP_ALWAYS   3

#if SNES_LAYER_DEBUG
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
static UINT8 snes_dbg_video(running_machine &machine, UINT16 curline);
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

inline UINT16 snes_state::snes_get_bgcolor( UINT8 direct_colors, UINT16 palette, UINT8 color )
{
	UINT16 c = 0;

	if (direct_colors)
	{
		/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
		c = ((color & 0x07) << 2) | ((color & 0x38) << 4) | ((color & 0xc0) << 7);
		c |= ((palette & 0x04) >> 1) | ((palette & 0x08) << 3) | ((palette & 0x10) << 8);
	}
	else
		c = m_snes_cgram[(palette + color) % FIXED_COLOUR];

	return c;
}

/*****************************************
 * snes_set_scanline_pixel()
 *
 * Store pixel color, priority, layer and
 * color math exception (for OAM) in the
 * proper scanline
 *****************************************/

inline void snes_state::snes_set_scanline_pixel( int screen, INT16 x, UINT16 color, UINT8 priority, UINT8 layer, int blend )
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

inline void snes_state::snes_draw_bgtile_lores( UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority )
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

#if SNES_LAYER_DEBUG
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

inline void snes_state::snes_draw_bgtile_hires( UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority )
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
				UINT8 clipmask = snes_ppu.clipmasks[layer][ii >> 1];

#if SNES_LAYER_DEBUG
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

inline void snes_state::snes_draw_oamtile( INT16 ii, UINT8 colour, UINT16 pal, UINT8 priority )
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

#if SNES_LAYER_DEBUG
			if (debug_options.windows_disabled)
				clipmask = 0xff;
#endif /* SNES_LAYER_DEBUG */

			/* Clip to windows */
			if (scanlines[screen].clip)
				clr &= clipmask;

			/* Only draw if we have a colour (0 == transparent) */
			if (clr)
			{
				c = m_snes_cgram[(pal + clr) % FIXED_COLOUR];
				blend = (pal + clr < 192) ? 1 : 0;
				snes_set_scanline_pixel(screen, pos, c, priority, SNES_OAM, blend);
			}
		}
	}
}

/*****************************************
 * snes_draw_tile()
 *
 * Draw 8 pixels from the expected tile
 * by reading the color planes from vram
 * and by calling the appropriate routine
 * (depending on layer and resolution)
 *****************************************/

inline void snes_state::snes_draw_tile( UINT8 planes, UINT8 layer, UINT32 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT8 direct_colors, UINT16 pal, UINT8 hires )
{
	UINT8 plane[8];
	INT16 ii, jj;
	int x_mos;

	for (ii = 0; ii < planes / 2; ii++)
	{
		plane[2 * ii + 0] = m_snes_vram[(tileaddr + 16 * ii + 0) % SNES_VRAM_SIZE];
		plane[2 * ii + 1] = m_snes_vram[(tileaddr + 16 * ii + 1) % SNES_VRAM_SIZE];
	}

	for (ii = x; ii < (x + 8); ii++)
	{
		UINT8 colour = 0;
		UINT8 mosaic = snes_ppu.layer[layer].mosaic_enabled;

#if SNES_LAYER_DEBUG
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
}

/*************************************************************************************************
 * SNES BG layers
 *
 * BG drawing theory of each scanline is quite easy: depending on the graphics Mode (0-7), there
 * are up to 4 background layers. Pixels for each BG layer can have two different priorities.
 * Depending on the line and on the BGHOFS and BGVOFS PPU registers, we first determine the tile
 * address in m_snes_vram (by determining x,y coord and tile size and by calling snes_get_tmap_addr).
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

inline UINT32 snes_state::snes_get_tmap_addr( UINT8 layer, UINT8 tile_size, UINT32 base, UINT32 x, UINT32 y )
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

inline void snes_state::snes_update_line( UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a, UINT8 color_depth, UINT8 hires, UINT8 offset_per_tile, UINT8 direct_colors )
{
	UINT32 tmap, tile, xoff, yoff, charaddr, addr;
	UINT16 ii = 0, vflip, hflip, pal, pal_direct, tilemap;
	UINT8 xscroll, priority;
	INT8 yscroll;
	int tile_incr = 0;
	UINT16 opt_bit = (layer == SNES_BG1) ? 13 : (layer == SNES_BG2) ? 14 : 0;
	UINT8 tile_size;
	/* variables depending on color_depth */
	UINT8 color_planes = 2 << color_depth;
	/* below we cheat to simplify the code: 8BPP should have 0 pal offset, not 0x100 (but we take care of this by later using pal % FIXED_COLOUR) */
	UINT8 color_shift = 2 << color_depth;

#if SNES_LAYER_DEBUG
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
					hval = m_snes_vram[haddr % SNES_VRAM_SIZE] | (m_snes_vram[(haddr + 1) % SNES_VRAM_SIZE] << 8);
					vval = m_snes_vram[vaddr % SNES_VRAM_SIZE] | (m_snes_vram[(vaddr + 1) % SNES_VRAM_SIZE] << 8);
					if (BIT(hval, opt_bit))
						xpos = opt_x + (hval & ~7);
					if (BIT(vval, opt_bit))
						ypos = curline + vval;
					break;
				case SNES_OPT_MODE4:
					haddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff));
					hval = m_snes_vram[haddr % SNES_VRAM_SIZE] | (m_snes_vram[(haddr + 1) % SNES_VRAM_SIZE] << 8);
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
		tilemap = m_snes_vram[addr % SNES_VRAM_SIZE] | (m_snes_vram[(addr + 1) % SNES_VRAM_SIZE] << 8);
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

#if SNES_LAYER_DEBUG
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
			if (BIT(yscroll, 3) != vflip)
				tile += 16;

		if (yscroll > 7)
			yscroll &= 7;

		if (vflip)
			yscroll = 7 - yscroll;

		yscroll <<= 1;

		/* if we have to draw 16 pixels, set tile_incr and adjust tile for horizontal flip */
		if (tile_size || hires)
		{
			if (hflip)
			{
				tile += 1;
				tile_incr = -1; // next 8 pixels from previous tile (because of hflip)
			}
			else
				tile_incr = 1;  // next 8 pixels from next tile
		}

		if (hires)
		{
			/* draw 16 pixels (the routine will automatically send half of them to the mainscreen scanline and half to the subscreen one) */
			snes_draw_tile(color_planes, layer, charaddr + (((tile + 0)         & 0x3ff) * 8 * color_planes) + yscroll, (ii - xscroll) * 2,     priority, hflip, direct_colors, direct_colors ? pal_direct : pal, hires);
			snes_draw_tile(color_planes, layer, charaddr + (((tile + tile_incr) & 0x3ff) * 8 * color_planes) + yscroll, (ii - xscroll) * 2 + 8, priority, hflip, direct_colors, direct_colors ? pal_direct : pal, hires);
			ii += 8;
		}
		else
		{
			snes_draw_tile(color_planes, layer, charaddr + ((tile & 0x3ff) * 8 * color_planes) + yscroll, ii - xscroll, priority, hflip, direct_colors, direct_colors ? pal_direct : pal, hires);
			ii += 8;

			if (tile_size)
			{
				snes_draw_tile(color_planes, layer, charaddr + (((tile + tile_incr) & 0x3ff) * 8 * color_planes) + yscroll, ii - xscroll, priority, hflip, direct_colors, direct_colors ? pal_direct : pal, hires);
				ii += 8;
			}
		}
	}
}


/*********************************************
 * snes_update_line_mode7()
 *
 * Update an entire line of mode7 tiles.
 *********************************************/

#define MODE7_CLIP(x) (((x) & 0x2000) ? ((x) | ~0x03ff) : ((x) & 0x03ff))

void snes_state::snes_update_line_mode7( UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a )
{
	UINT32 tiled;
	INT16 ma, mb, mc, md;
	INT32 xc, yc, tx, ty, sx, sy, hs, vs, xpos, xdir, x0, y0;
	UINT8 priority = priority_b;
	UINT8 colour = 0;
	UINT16 *mosaic_x, *mosaic_y;
	UINT16 c;
	int screen;

#if SNES_LAYER_DEBUG
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
	if (layer == SNES_BG2)  // BG2 use two different bits for horizontal and vertical mosaic
	{
		mosaic_x = snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG2].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y = snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}
	else    // BG1 works as usual
	{
		mosaic_x =  snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y =  snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}

#if SNES_LAYER_DEBUG
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
			case 0x00:  /* Repeat if outside screen area */
			case 0x01:  /* Repeat if outside screen area */
				tx &= 0x3ff;
				ty &= 0x3ff;
				tiled = m_snes_vram[((((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2) % SNES_VRAM_SIZE] << 7;
				colour = m_snes_vram[(tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1) % SNES_VRAM_SIZE];
				break;
			case 0x02:  /* Single colour backdrop screen if outside screen area */
				if ((tx >= 0) && (tx < 1024) && (ty >= 0) && (ty < 1024))
				{
					tiled = m_snes_vram[((((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2) % SNES_VRAM_SIZE] << 7;
					colour = m_snes_vram[(tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1) % SNES_VRAM_SIZE];
				}
				else
					colour = 0;
				break;
			case 0x03:  /* Character 0x00 repeat if outside screen area */
				if ((tx >= 0) && (tx < 1024) && (ty >= 0) && (ty < 1024))
					tiled = m_snes_vram[((((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2) % SNES_VRAM_SIZE] << 7;
				else
					tiled = 0;

				colour = m_snes_vram[(tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1) % SNES_VRAM_SIZE];
				break;
		}

		/* The last bit is for priority in EXTBG mode (used only for BG2) */
		if (layer == SNES_BG2)
		{
			priority = ((colour & 0x80) >> 7) ? priority_a : priority_b;
			colour &= 0x7f;

#if SNES_LAYER_DEBUG
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

#if SNES_LAYER_DEBUG
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

static struct TILELIST oam_tilelist[34];

/*********************************************
 * snes_update_obsel()
 *
 * Update sprite settings for next line.
 *********************************************/

void snes_state::snes_update_obsel( void )
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

void snes_state::snes_oam_list_build( void )
{
	UINT8 *oamram = (UINT8 *)m_snes_oam;
	INT16 oam = 0x1ff;
	UINT16 oam_extra = oam + 0x20;
	UINT16 extra = 0;
	int ii;

	snes_ppu.update_oam_list = 0;       // eventually, we can optimize the code by only calling this function when there is a change in size

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
		oam_spritelist[ii].y = oamram[oam--] + 1;
		oam_spritelist[ii].x = oamram[oam--];
		oam_spritelist[ii].size = (extra & 0x80) >> 7;
		extra <<= 1;
		oam_spritelist[ii].x |= ((extra & 0x80) << 1);
		extra <<= 1;

		oam_spritelist[ii].y *= snes_ppu.obj_interlace;
		oam_spritelist[ii].y &= 0x1ff;

		oam_spritelist[ii].x &= 0x1ff;

		/* Determine object size */
		switch (snes_ppu.oam.size)
		{
		case 0:         /* 8x8 or 16x16 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 2 : 1;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 2 : 1;
			break;
		case 1:         /* 8x8 or 32x32 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 1;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 4 : 1;
			break;
		case 2:         /* 8x8 or 64x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 8 : 1;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 1;
			break;
		case 3:         /* 16x16 or 32x32 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 4 : 2;
			break;
		case 4:         /* 16x16 or 64x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 8 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 2;
			break;
		case 5:         /* 32x32 or 64x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 8 : 4;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 4;
			break;
		case 6:         /* undocumented: 16x32 or 32x64 */
			oam_spritelist[ii].width  = oam_spritelist[ii].size ? 4 : 2;
			oam_spritelist[ii].height = oam_spritelist[ii].size ? 8 : 4;
			if (snes_ppu.obj_interlace && !oam_spritelist[ii].size)
				oam_spritelist[ii].height = 2;
			break;
		case 7:         /* undocumented: 16x32 or 32x32 */
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

int snes_state::is_sprite_on_scanline( UINT16 curline, UINT8 sprite )
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

void snes_state::snes_update_objects_rto( UINT16 curline )
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

		/* Adjust y, if past maximum position (for sprites which overlap between top & bottom) */
		if (y >= (0x100 - 16) * snes_ppu.interlace)
			y -= (0x100) * snes_ppu.interlace;

		if (curline >= y && curline < (y + (height << 3)))
		{
			/* Only objects using tiles over 255 use name select */
			name_sel = (tile < 256) ? 0 : snes_ppu.oam.name_select;

			ys = (curline - y) >> 3;
			line = (curline - y) % 8;
			if (vflip)
			{
				ys = height - ys - 1;
				line = 7 - line;
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

void snes_state::snes_update_objects( UINT8 priority_oam0, UINT8 priority_oam1, UINT8 priority_oam2, UINT8 priority_oam3 )
{
	UINT8 pri, priority[4];
	UINT32 charaddr;
	int ii;

#if SNES_LAYER_DEBUG
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
#if SNES_LAYER_DEBUG
		if (debug_options.sprite_reversed)
			tile = 33 - ii;
#endif /* SNES_LAYER_DEBUG */

		if (oam_tilelist[tile].tileaddr == 0xffff)
			continue;

		pri = priority[oam_tilelist[tile].priority];

#if SNES_LAYER_DEBUG
		if (debug_options.select_pri[SNES_OAM])
		{
			int oam_draw = debug_options.select_pri[SNES_OAM] - 1;
			if (oam_draw != oam_tilelist[tile].priority)
				continue;
		}
#endif /* SNES_LAYER_DEBUG */

		/* OAM tiles have fixed planes (4), no direct color and no hires, but otherwise work the same as BG ones */
		snes_draw_tile(4, SNES_OAM, charaddr + oam_tilelist[tile].tileaddr, oam_tilelist[tile].x, pri, oam_tilelist[tile].hflip, 0, oam_tilelist[tile].pal, 0);
	}
}


/*********************************************
 * snes_update_mode_X()
 *
 * Update Mode X line.
 *********************************************/

void snes_state::snes_update_mode_0( UINT16 curline )
{
#if SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[0])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(3, 6, 9, 12);
	snes_update_line(curline, SNES_BG1, 8, 11, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG2, 7, 10, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG3, 2, 5,  SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG4, 1, 4,  SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_NONE, 0);
}

void snes_state::snes_update_mode_1( UINT16 curline )
{
#if SNES_LAYER_DEBUG
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

void snes_state::snes_update_mode_2( UINT16 curline )
{
#if SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[2])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_MODE2, 0);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_MODE2, 0);
}

void snes_state::snes_update_mode_3( UINT16 curline )
{
#if SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[3])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_8BPP, 0, SNES_OPT_NONE, snes_ppu.direct_color);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_4BPP, 0, SNES_OPT_NONE, 0);
}

void snes_state::snes_update_mode_4( UINT16 curline )
{
#if SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[4])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_8BPP, 0, SNES_OPT_MODE4, snes_ppu.direct_color);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_2BPP, 0, SNES_OPT_MODE4, 0);
}

void snes_state::snes_update_mode_5( UINT16 curline )
{
#if SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[5])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(2, 4, 6, 8);
	snes_update_line(curline, SNES_BG1, 3, 7, SNES_COLOR_DEPTH_4BPP, 1, SNES_OPT_NONE, 0);
	snes_update_line(curline, SNES_BG2, 1, 5, SNES_COLOR_DEPTH_2BPP, 1, SNES_OPT_NONE, 0);
}

void snes_state::snes_update_mode_6( UINT16 curline )
{
#if SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[6])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_objects(1, 3, 4, 6);
	snes_update_line(curline, SNES_BG1, 2, 5, SNES_COLOR_DEPTH_4BPP, 1, SNES_OPT_MODE6, 0);
}

void snes_state::snes_update_mode_7( UINT16 curline )
{
#if SNES_LAYER_DEBUG
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

void snes_state::snes_draw_screens( UINT16 curline )
{
	switch (snes_ppu.mode)
	{
		case 0: snes_update_mode_0(curline); break;     /* Mode 0 */
		case 1: snes_update_mode_1(curline); break;     /* Mode 1 */
		case 2: snes_update_mode_2(curline); break;     /* Mode 2 - Supports offset per tile */
		case 3: snes_update_mode_3(curline); break;     /* Mode 3 - Supports direct colour */
		case 4: snes_update_mode_4(curline); break;     /* Mode 4 - Supports offset per tile and direct colour */
		case 5: snes_update_mode_5(curline); break;     /* Mode 5 - Supports hires */
		case 6: snes_update_mode_6(curline); break;     /* Mode 6 - Supports offset per tile and hires */
		case 7: snes_update_mode_7(curline); break;     /* Mode 7 - Supports direct colour */
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

void snes_state::snes_update_windowmasks( void )
{
	UINT16 ii, jj;
	INT8 w1, w2;

	snes_ppu.update_windows = 0;        /* reset the flag */

	for (ii = 0; ii < SNES_SCR_WIDTH; ii++)
	{
		/* update bg 1, 2, 3, 4, obj & color windows */
		/* jj = layer */
		for (jj = 0; jj < 6; jj++)
		{
			snes_ppu.clipmasks[jj][ii] = 0xff;  /* let's start from un-masked */
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
					case 0x00:  /* OR */
						snes_ppu.clipmasks[jj][ii] = (w1 | w2) ? 0x00 : 0xff;
						break;
					case 0x01:  /* AND */
						snes_ppu.clipmasks[jj][ii] = (w1 & w2) ? 0x00 : 0xff;
						break;
					case 0x02:  /* XOR */
						snes_ppu.clipmasks[jj][ii] = (w1 ^ w2) ? 0x00 : 0xff;
						break;
					case 0x03:  /* XNOR */
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

void snes_state::snes_update_offsets( void )
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

inline void snes_state::snes_draw_blend( UINT16 offset, UINT16 *colour, UINT8 prevent_color_math, UINT8 black_pen_clip, int switch_screens )
{
#if SNES_LAYER_DEBUG
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
		int clip_max = 0;   // if add then clip to 0x1f, if sub then clip to 0

#if SNES_LAYER_DEBUG
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
			if (BIT(snes_ppu.color_modes, 6) && (subscreen->buffer[offset] != m_snes_cgram[FIXED_COLOUR]))
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
				r = (*colour & 0x1f) + (m_snes_cgram[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) + ((m_snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) + ((m_snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
				clip_max = 1;
			}
			else
			{
				/* 0x80: sub */
				r = (*colour & 0x1f) - (m_snes_cgram[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) - ((m_snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) - ((m_snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
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

void snes_state::snes_refresh_scanline( running_machine &machine, bitmap_rgb32 &bitmap, UINT16 curline )
{
	UINT16 ii;
	int x;
	int fade;
	struct SCANLINE *scanline1, *scanline2;
	UINT16 c;
	UINT16 prev_colour = 0;
	int blurring = machine.root_device().ioport("OPTIONS")->read_safe(0) & 0x01;

	g_profiler.start(PROFILER_VIDEO);

	if (snes_ppu.screen_disabled) /* screen is forced blank */
		for (x = 0; x < SNES_SCR_WIDTH * 2; x++)
			bitmap.pix32(curline, x) = RGB_BLACK;
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
				scanlines[SNES_SUBSCREEN].buffer[ii] = m_snes_cgram[0];
			else
				scanlines[SNES_SUBSCREEN].buffer[ii] = m_snes_cgram[FIXED_COLOUR];

			scanlines[SNES_MAINSCREEN].buffer[ii] = m_snes_cgram[0];
		}

		/* Prepare OAM for this scanline */
		snes_update_objects_rto(curline);

		/* Draw scanline */
		snes_draw_screens(curline);

		snes_update_obsel();

#if SNES_LAYER_DEBUG
		if (snes_dbg_video(machine, curline))
		{
			g_profiler.stop();
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

				bitmap.pix32(curline, x * 2 + 0) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
				bitmap.pix32(curline, x * 2 + 1) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
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
					c = (prev_colour + tmp_col[0] - ((prev_colour ^ tmp_col[0]) & 0x0421)) >> 1;    // Hack code to mimic TV pixel blurring
				else
					c = tmp_col[0];

				r = ((c & 0x1f) * fade) >> 4;
				g = (((c & 0x3e0) >> 5) * fade) >> 4;
				b = (((c & 0x7c00) >> 10) * fade) >> 4;

				bitmap.pix32(curline, x * 2 + 0) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
				prev_colour = tmp_col[0];

				/* average the second pixel if required, or draw it directly*/
				if (blurring)
					c = (prev_colour + tmp_col[1] - ((prev_colour ^ tmp_col[1]) & 0x0421)) >> 1;    // Hack code to mimic TV pixel blurring
				else
					c = tmp_col[1];

				r = ((c & 0x1f) * fade) >> 4;
				g = (((c & 0x3e0) >> 5) * fade) >> 4;
				b = (((c & 0x7c00) >> 10) * fade) >> 4;

				bitmap.pix32(curline, x * 2 + 1) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
				prev_colour = tmp_col[1];
			}
		}
	}

	g_profiler.stop();
}

VIDEO_START( snes )
{
	snes_state *state = machine.driver_data<snes_state>();
	int i,j;

#if SNES_LAYER_DEBUG
	memset(&debug_options, 0, sizeof(debug_options));
#endif

	state->m_snes_vram = auto_alloc_array(machine, UINT8, SNES_VRAM_SIZE);
	state->m_snes_cgram = auto_alloc_array(machine, UINT16, SNES_CGRAM_SIZE/2);
	state->m_snes_oam = auto_alloc_array(machine, UINT16, SNES_OAM_SIZE/2);

	/* Inititialize registers/variables */
	snes_ppu.update_windows = 1;
	snes_ppu.beam.latch_vert = 0;
	snes_ppu.beam.latch_horz = 0;
	snes_ppu.beam.current_vert = 0;
	snes_ppu.beam.current_horz = 0;
	snes_ppu.beam.last_visible_line = 225; /* TODO: PAL setting */
	snes_ppu.mode = 0;
	snes_ppu.ppu1_version = 1;  // 5C77 chip version number, read by STAT77, only '1' is known
	snes_ppu.ppu2_version = 3;  // 5C78 chip version number, read by STAT78, only '2' & '3' encountered so far.

	/* Inititialize mosaic table */
	for (j = 0; j < 16; j++)
	{
		for (i = 0; i < 4096; i++)
			snes_ppu.mosaic_table[j][i] = (i / (j + 1)) * (j + 1);
	}

	/* Init VRAM */
	memset(state->m_snes_vram, 0, SNES_VRAM_SIZE);

	/* Init Palette RAM */
	memset((UINT8 *)state->m_snes_cgram, 0, SNES_CGRAM_SIZE);

	/* Init oam RAM */
	memset((UINT8 *)state->m_snes_oam, 0xff, SNES_OAM_SIZE);

	for (i = 0; i < 6; i++)
	{
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].window1_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].window1_invert);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].window2_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].window2_invert);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].wlog_mask);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].color_math);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].charmap);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].tilemap);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].tilemap_size);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].tile_size);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].mosaic_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].main_window_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].sub_window_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].main_bg_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].sub_bg_enabled);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].hoffs);
		state_save_register_item(machine, "snes_ppu", NULL, i, snes_ppu.layer[i].voffs);

		state_save_register_item_array(machine, "snes_ppu", NULL, i, snes_ppu.clipmasks[i]);
	}

	state_save_register_global(machine, snes_ppu.oam.address_low);
	state_save_register_global(machine, snes_ppu.oam.address_high);
	state_save_register_global(machine, snes_ppu.oam.saved_address_low);
	state_save_register_global(machine, snes_ppu.oam.saved_address_high);
	state_save_register_global(machine, snes_ppu.oam.address);
	state_save_register_global(machine, snes_ppu.oam.priority_rotation);
	state_save_register_global(machine, snes_ppu.oam.next_charmap);
	state_save_register_global(machine, snes_ppu.oam.next_size);
	state_save_register_global(machine, snes_ppu.oam.size);
	state_save_register_global(machine, snes_ppu.oam.next_name_select);
	state_save_register_global(machine, snes_ppu.oam.name_select);
	state_save_register_global(machine, snes_ppu.oam.first_sprite);
	state_save_register_global(machine, snes_ppu.oam.flip);
	state_save_register_global(machine, snes_ppu.oam.write_latch);

	state_save_register_global(machine, snes_ppu.beam.latch_horz);
	state_save_register_global(machine, snes_ppu.beam.latch_vert);
	state_save_register_global(machine, snes_ppu.beam.current_horz);
	state_save_register_global(machine, snes_ppu.beam.current_vert);
	state_save_register_global(machine, snes_ppu.beam.last_visible_line);
	state_save_register_global(machine, snes_ppu.beam.interlace_count);

	state_save_register_global(machine, snes_ppu.mode7.repeat);
	state_save_register_global(machine, snes_ppu.mode7.hflip);
	state_save_register_global(machine, snes_ppu.mode7.vflip);
	state_save_register_global(machine, snes_ppu.mode7.matrix_a);
	state_save_register_global(machine, snes_ppu.mode7.matrix_b);
	state_save_register_global(machine, snes_ppu.mode7.matrix_c);
	state_save_register_global(machine, snes_ppu.mode7.matrix_d);
	state_save_register_global(machine, snes_ppu.mode7.origin_x);
	state_save_register_global(machine, snes_ppu.mode7.origin_y);
	state_save_register_global(machine, snes_ppu.mode7.hor_offset);
	state_save_register_global(machine, snes_ppu.mode7.ver_offset);
	state_save_register_global(machine, snes_ppu.mode7.extbg);

	state_save_register_global(machine, snes_ppu.mosaic_size);
	state_save_register_global(machine, snes_ppu.clip_to_black);
	state_save_register_global(machine, snes_ppu.prevent_color_math);
	state_save_register_global(machine, snes_ppu.sub_add_mode);
	state_save_register_global(machine, snes_ppu.bg3_priority_bit);
	state_save_register_global(machine, snes_ppu.direct_color);
	state_save_register_global(machine, snes_ppu.ppu_last_scroll);
	state_save_register_global(machine, snes_ppu.mode7_last_scroll);

	state_save_register_global(machine, snes_ppu.ppu1_open_bus);
	state_save_register_global(machine, snes_ppu.ppu2_open_bus);
	state_save_register_global(machine, snes_ppu.ppu1_version);
	state_save_register_global(machine, snes_ppu.ppu2_version);
	state_save_register_global(machine, snes_ppu.window1_left);
	state_save_register_global(machine, snes_ppu.window1_right);
	state_save_register_global(machine, snes_ppu.window2_left);
	state_save_register_global(machine, snes_ppu.window2_right);

	state_save_register_global(machine, snes_ppu.update_windows);
	state_save_register_global(machine, snes_ppu.update_offsets);
	state_save_register_global(machine, snes_ppu.update_oam_list);
	state_save_register_global(machine, snes_ppu.mode);
	state_save_register_global(machine, snes_ppu.interlace);
	state_save_register_global(machine, snes_ppu.obj_interlace);
	state_save_register_global(machine, snes_ppu.screen_brightness);
	state_save_register_global(machine, snes_ppu.screen_disabled);
	state_save_register_global(machine, snes_ppu.pseudo_hires);
	state_save_register_global(machine, snes_ppu.color_modes);
	state_save_register_global(machine, snes_ppu.stat77_flags);

	state_save_register_global_pointer(machine, state->m_snes_vram, SNES_VRAM_SIZE);
	state_save_register_global_pointer(machine, state->m_snes_cgram, SNES_CGRAM_SIZE/2);
	state_save_register_global_pointer(machine, state->m_snes_oam, SNES_OAM_SIZE/2);
}

UINT32 snes_state::snes_screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y;

	/*NTSC SNES draw range is 1-225. */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		snes_refresh_scanline(machine(), bitmap, y + 1);
	}
	return 0;
}


/* CPU <-> PPU comms */

// full graphic variables
static const UINT16 vram_fgr_inctab[4] = { 1, 32, 128, 128 };
static const UINT16 vram_fgr_inccnts[4] = { 0, 32, 64, 128 };
static const UINT16 vram_fgr_shiftab[4] = { 0, 5, 6, 7 };

// utility function - latches the H/V counters.  Used by IRQ, writes to WRIO, etc.
void snes_latch_counters( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();

	snes_ppu.beam.current_horz = machine.primary_screen->hpos() / state->m_htmult;
	snes_ppu.beam.latch_vert = machine.primary_screen->vpos();
	snes_ppu.beam.latch_horz = snes_ppu.beam.current_horz;
	snes_ram[STAT78] |= 0x40;   // indicate we latched
//  state->m_read_ophct = state->m_read_opvct = 0;    // clear read flags - 2009-08: I think we must clear these when STAT78 is read...

//  printf("latched @ H %d V %d\n", snes_ppu.beam.latch_horz, snes_ppu.beam.latch_vert);
}

static void snes_dynamic_res_change( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	rectangle visarea = machine.primary_screen->visible_area();
	attoseconds_t refresh;

	visarea.min_x = visarea.min_y = 0;
	visarea.max_y = snes_ppu.beam.last_visible_line * snes_ppu.interlace - 1;
	visarea.max_x = (SNES_SCR_WIDTH * 2) - 1;

	// fixme: should compensate for SNES_DBG_video
	if (snes_ppu.mode == 5 || snes_ppu.mode == 6 || snes_ppu.pseudo_hires)
		state->m_htmult = 2;
	else
		state->m_htmult = 1;

	/* FIXME: does the timing changes when the gfx mode is equal to 5 or 6? */
	if ((snes_ram[STAT78] & 0x10) == SNES_NTSC)
		refresh = HZ_TO_ATTOSECONDS(DOTCLK_NTSC) * SNES_HTOTAL * SNES_VTOTAL_NTSC;
	else
		refresh = HZ_TO_ATTOSECONDS(DOTCLK_PAL) * SNES_HTOTAL * SNES_VTOTAL_PAL;

	if ((snes_ram[STAT78] & 0x10) == SNES_NTSC)
		machine.primary_screen->configure(SNES_HTOTAL * 2, SNES_VTOTAL_NTSC * snes_ppu.interlace, visarea, refresh);
	else
		machine.primary_screen->configure(SNES_HTOTAL * 2, SNES_VTOTAL_PAL * snes_ppu.interlace, visarea, refresh);
}

/*************************************************

 SNES VRAM accesses:

 VRAM accesses during active display are invalid.
 Unlike OAM and CGRAM, they will not be written
 anywhere at all. Thanks to byuu's researches,
 the ranges where writes are invalid have been
 validated on hardware, as has the edge case where
 the S-CPU open bus can be written if the write
 occurs during the very last clock cycle of
 vblank.
 Our implementation could be not 100% accurate
 when interlace is active.
*************************************************/

inline UINT32 snes_get_vram_address( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT32 addr = state->m_vmadd;

	if (state->m_vram_fgr_count)
	{
		UINT32 rem = addr & state->m_vram_fgr_mask;
		UINT32 faddr = (addr & ~state->m_vram_fgr_mask) + (rem >> state->m_vram_fgr_shift) + ((rem & (state->m_vram_fgr_count - 1)) << 3);
		return faddr << 1;
	}

	return addr << 1;
}

READ8_MEMBER( snes_state::snes_vram_read )
{
	UINT8 res = 0;
	offset &= 0xffff; // only 64KB are present on SNES

	if (snes_ppu.screen_disabled)
		res = m_snes_vram[offset];
	else
	{
		UINT16 v = machine().primary_screen->vpos();
		UINT16 h = machine().primary_screen->hpos();
		UINT16 ls = (((snes_ram[STAT78] & 0x10) == SNES_NTSC ? 525 : 625) >> 1) - 1;

		if (snes_ppu.interlace == 2)
			ls++;

		if (v == ls && h == 1362)
			res = 0;
		else if (v < snes_ppu.beam.last_visible_line - 1)
			res = 0;
		else if (v == snes_ppu.beam.last_visible_line - 1)
		{
			if (h == 1362)
				res = m_snes_vram[offset];
			else
			{
				//printf("%d %d VRAM read, CHECK!\n",h,v);
				res = 0;
			}
		}
		else
			res = m_snes_vram[offset];
	}
	return res;
}

WRITE8_MEMBER( snes_state::snes_vram_write )
{
	offset &= 0xffff; // only 64KB are present on SNES, Robocop 3 relies on this

	if (snes_ppu.screen_disabled)
		m_snes_vram[offset] = data;
	else
	{
		UINT16 v = machine().primary_screen->vpos();
		UINT16 h = machine().primary_screen->hpos();
		if (v == 0)
		{
			if (h <= 4)
				m_snes_vram[offset] = data;
			else if (h == 6)
				m_snes_vram[offset] = snes_open_bus_r(space, 0);
			else
			{
				//printf("%d %d VRAM write, CHECK!\n",h,v);
				//no write
			}
		}
		else if (v < snes_ppu.beam.last_visible_line)
		{
			//printf("%d %d VRAM write, CHECK!\n",h,v);
			//no write
		}
		else if (v == snes_ppu.beam.last_visible_line)
		{
			if (h <= 4)
			{
				//printf("%d %d VRAM write, CHECK!\n",h,v);
				//no write
			}
			else
				m_snes_vram[offset] = data;
		}
		else
			m_snes_vram[offset] = data;
	}
}

/*************************************************

 SNES OAM accesses:

 OAM accesses during active display are allowed.
 The actual address varies during rendering, as the
 PPU reads in data itself for processing.
 Unfortunately, no one has been able (yet) to
 determine how this works. The only known game to
 actually access OAM during active display is
 Uniracers and it expects accesses to map to
 offset 0x0218. Hence, following byuu's choice
 we rerouted OAM accesses during active display
 to 0x0218 (0x010c in our snes_oam).
 This is a hack, but it is more accurate than
 writing to the 'expected' address set by
 $2102,$2103.

 Notice that, since snes_ram[OAMDATA] is never
 read/written directly, we use it as an index
 to choose the high/low byte of the snes_oam word.
*************************************************/

READ8_MEMBER( snes_state::snes_oam_read )
{
	offset &= 0x1ff;

	if (offset & 0x100)
		offset &= 0x10f;

	if (!snes_ppu.screen_disabled)
	{
		UINT16 v = machine().primary_screen->vpos();

		if (v < snes_ppu.beam.last_visible_line)
			offset = 0x010c;
	}

	return (m_snes_oam[offset] >> (snes_ram[OAMDATA] << 3)) & 0xff;
}

WRITE8_MEMBER( snes_state::snes_oam_write )
{
	offset &= 0x1ff;

	if (offset & 0x100)
		offset &= 0x10f;

	if (!snes_ppu.screen_disabled)
	{
		UINT16 v = machine().primary_screen->vpos();

		if (v < snes_ppu.beam.last_visible_line)
			offset = 0x010c;
	}

	if (!(snes_ram[OAMDATA]))
		m_snes_oam[offset] = (m_snes_oam[offset] & 0xff00) | (data << 0);
	else
		m_snes_oam[offset] = (m_snes_oam[offset] & 0x00ff) | (data << 8);
}

/*************************************************

 SNES CGRAM accesses:

 CGRAM writes during hblank are valid. During
 active display, the actual address the data
 is written to varies, as the PPU itself changes
 the address. Like OAM, it is not known the exact
 algorithm used, but no commercial software seems
 to attempt this. While byuu, in his emu, maps
 those accesses to 0x01ff, because it is more
 accurate to invalidate the 'expected' address
 than not, MESS has issues if we don't write to
 the expected address (see e.g. Tokimeki Memorial).
 This is because writes should work during hblank
 (so that the game can produce color fading), but
 ends up not working with the conditions below.
 Hence, for the moment, we only document the
 solution adopted by BSNES without enabling it.
*************************************************/

READ8_MEMBER( snes_state::snes_cgram_read )
{
	UINT8 res = 0;
	offset &= 0x1ff;

#if 0
	if (!snes_ppu.screen_disabled)
	{
		UINT16 v = space.machine().primary_screen->vpos();
		UINT16 h = space.machine().primary_screen->hpos();

		if (v < snes_ppu.beam.last_visible_line && h >= 128 && h < 1096)
			offset = 0x1ff;
	}
#endif

	res = ((UINT8 *)m_snes_cgram)[offset];

	// CGRAM palette data format is 15-bits (0,bbbbb,ggggg,rrrrr).
	// Highest bit is simply ignored.
	if (offset & 0x01)
		res &= 0x7f;

	return res;
}

WRITE8_MEMBER( snes_state::snes_cgram_write )
{
	offset &= 0x1ff;

#if 0
	// FIXME: this currently breaks some games (e.g. Tokimeki Memorial),
	// even if it's expected to be more accurate than allowing for
	// writes to the cgram address
	if (!snes_ppu.screen_disabled)
	{
		UINT16 v = space.machine().primary_screen->vpos();
		UINT16 h = space.machine().primary_screen->hpos();

		if (v < snes_ppu.beam.last_visible_line && h >= 128 && h < 1096)
			offset = 0x1ff;
	}
#endif

	// CGRAM palette data format is 15-bits (0,bbbbb,ggggg,rrrrr).
	// Highest bit is simply ignored.
	if (offset & 0x01)
		data &= 0x7f;

	((UINT8 *)m_snes_cgram)[offset] = data;
}

READ8_HANDLER( snes_ppu_read )
{
	snes_state *state = space.machine().driver_data<snes_state>();
	UINT8 value;

	switch (offset)
	{
		case OAMDATA:   /* 21xy for x=0,1,2 and y=4,5,6,8,9,a returns PPU1 open bus*/
		case BGMODE:
		case MOSAIC:
		case BG2SC:
		case BG3SC:
		case BG4SC:
		case BG4VOFS:
		case VMAIN:
		case VMADDL:
		case VMDATAL:
		case VMDATAH:
		case M7SEL:
		case W34SEL:
		case WOBJSEL:
		case WH0:
		case WH2:
		case WH3:
		case WBGLOG:
			return snes_ppu.ppu1_open_bus;

		case MPYL:      /* Multiplication result (low) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = (INT16)snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = c & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYM:      /* Multiplication result (mid) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = (INT16)snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 8) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYH:      /* Multiplication result (high) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = (INT16)snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 16) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case SLHV:      /* Software latch for H/V counter */
			snes_latch_counters(space.machine());
			return snes_open_bus_r(space, 0);       /* Return value is meaningless */
		case ROAMDATA:  /* Read data from OAM (DR) */
			snes_ppu.ppu1_open_bus = state->snes_oam_read(space, snes_ppu.oam.address);
			snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
			if (!snes_ram[OAMDATA])
			{
				snes_ppu.oam.address++;
				snes_ppu.oam.address &= 0x1ff;
				snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			}
			return snes_ppu.ppu1_open_bus;
		case RVMDATAL:  /* Read data from VRAM (low) */
			{
				UINT32 addr = snes_get_vram_address(space.machine());
				snes_ppu.ppu1_open_bus = state->m_vram_read_buffer & 0xff;

				if (!state->m_vram_fgr_high)
				{
					state->m_vram_read_buffer = state->snes_vram_read(space, addr);
					state->m_vram_read_buffer |= (state->snes_vram_read(space, addr + 1) << 8);

					state->m_vmadd = (state->m_vmadd + state->m_vram_fgr_increment) & 0xffff;
				}

				return snes_ppu.ppu1_open_bus;
			}
		case RVMDATAH:  /* Read data from VRAM (high) */
			{
				UINT32 addr = snes_get_vram_address(space.machine());
				snes_ppu.ppu1_open_bus = (state->m_vram_read_buffer >> 8) & 0xff;

				if (state->m_vram_fgr_high)
				{
					state->m_vram_read_buffer = state->snes_vram_read(space, addr);
					state->m_vram_read_buffer |= (state->snes_vram_read(space, addr + 1) << 8);

					state->m_vmadd = (state->m_vmadd + state->m_vram_fgr_increment) & 0xffff;
				}

				return snes_ppu.ppu1_open_bus;
			}
		case RCGDATA:   /* Read data from CGRAM */
			if (!(state->m_cgram_address & 0x01))
				snes_ppu.ppu2_open_bus = state->snes_cgram_read(space, state->m_cgram_address);
			else
			{
				snes_ppu.ppu2_open_bus &= 0x80;
				snes_ppu.ppu2_open_bus |= state->snes_cgram_read(space, state->m_cgram_address) & 0x7f;
			}

			state->m_cgram_address = (state->m_cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
			return snes_ppu.ppu2_open_bus;
		case OPHCT:     /* Horizontal counter data by ext/soft latch */
			if (state->m_read_ophct)
			{
				snes_ppu.ppu2_open_bus &= 0xfe;
				snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_horz >> 8) & 0x01;
			}
			else
			{
				snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_horz & 0xff;
			}
			state->m_read_ophct ^= 1;
			return snes_ppu.ppu2_open_bus;
		case OPVCT:     /* Vertical counter data by ext/soft latch */
			if (state->m_read_opvct)
			{
				snes_ppu.ppu2_open_bus &= 0xfe;
				snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_vert >> 8) & 0x01;
			}
			else
			{
				snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_vert & 0xff;
			}
			state->m_read_opvct ^= 1;
			return snes_ppu.ppu2_open_bus;
		case STAT77:    /* PPU status flag and version number */
			value = snes_ppu.stat77_flags & 0xc0; // 0x80 & 0x40 are Time Over / Range Over Sprite flags, set by the video code
			// 0x20 - Master/slave mode select. Little is known about this bit. We always seem to read back 0 here.
			value |= (snes_ppu.ppu1_open_bus & 0x10);
			value |= (snes_ppu.ppu1_version & 0x0f);
			snes_ppu.stat77_flags = value;  // not sure if this is needed...
			snes_ppu.ppu1_open_bus = value;
			return snes_ppu.ppu1_open_bus;
		case STAT78:    /* PPU status flag and version number */
			state->m_read_ophct = 0;
			state->m_read_opvct = 0;
			if(snes_ram[WRIO] & 0x80)
				snes_ram[STAT78] &= ~0x40; //clear ext latch if bit 7 of WRIO is set
			snes_ram[STAT78] = (snes_ram[STAT78] & ~0x2f) | (snes_ppu.ppu2_open_bus & 0x20) | (snes_ppu.ppu2_version & 0x0f);
			snes_ppu.ppu2_open_bus = snes_ram[STAT78];
			return snes_ppu.ppu2_open_bus;
	}

	/* note: remaining registers (Namely TM in Super Kick Boxing) returns MDR open bus, not PPU Open Bus! */
	return snes_open_bus_r(space, 0);
}


WRITE8_HANDLER( snes_ppu_write )
{
	snes_state *state = space.machine().driver_data<snes_state>();

	switch (offset)
	{
		case INIDISP:   /* Initial settings for screen */
			if ((snes_ppu.screen_disabled & 0x80) && (!(data & 0x80))) //a 1->0 force blank transition causes a reset OAM address
			{
				space.write_byte(OAMADDL, snes_ppu.oam.saved_address_low);
				space.write_byte(OAMADDH, snes_ppu.oam.saved_address_high);
				snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			}
			snes_ppu.screen_disabled = data & 0x80;
			snes_ppu.screen_brightness = (data & 0x0f) + 1;
			break;
		case OBSEL:     /* Object size and data area designation */
			snes_ppu.oam.next_charmap = (data & 0x03) << 1;
			snes_ppu.oam.next_name_select = (((data & 0x18) >> 3) * 0x1000) << 1;
			snes_ppu.oam.next_size = (data & 0xe0) >> 5;
			break;
		case OAMADDL:   /* Address for accessing OAM (low) */
			snes_ppu.oam.saved_address_low = data;
			snes_ppu.oam.address = (snes_ppu.oam.address & 0xff00) + data;
			snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMADDH:   /* Address for accessing OAM (high) */
			snes_ppu.oam.saved_address_high = data;
			snes_ppu.oam.address = (snes_ppu.oam.address & 0x00ff) | ((data & 0x01) << 8);
			snes_ppu.oam.priority_rotation = BIT(data, 7);
			snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMDATA:   /* Data for OAM write (DW) */
			if (snes_ppu.oam.address >= 0x100)
				state->snes_oam_write(space, snes_ppu.oam.address, data);
			else
			{
				if (!snes_ram[OAMDATA])
					snes_ppu.oam.write_latch = data;
				else
				{
					// in this case, we not only write data to the upper byte of the word,
					// but also snes_ppu.oam.write_latch to the lower byte (recall that
					// snes_ram[OAMDATA] is used to select high/low byte)
					state->snes_oam_write(space, snes_ppu.oam.address, data);
					snes_ram[OAMDATA] = 0;
					state->snes_oam_write(space, snes_ppu.oam.address, snes_ppu.oam.write_latch);
					snes_ram[OAMDATA] = 1;
				}
			}
			snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
			if (!snes_ram[OAMDATA])
			{
				snes_ppu.oam.address++;
				snes_ppu.oam.address &= 0x1ff;
				snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			}
			return;
		case BGMODE:    /* BG mode and character size settings */
			snes_ppu.mode = data & 0x07;
			snes_dynamic_res_change(space.machine());
			snes_ppu.bg3_priority_bit = BIT(data, 3);
			snes_ppu.layer[SNES_BG1].tile_size = BIT(data, 4);
			snes_ppu.layer[SNES_BG2].tile_size = BIT(data, 5);
			snes_ppu.layer[SNES_BG3].tile_size = BIT(data, 6);
			snes_ppu.layer[SNES_BG4].tile_size = BIT(data, 7);
			snes_ppu.update_offsets = 1;
			break;
		case MOSAIC:    /* Size and screen designation for mosaic */
			snes_ppu.mosaic_size = (data & 0xf0) >> 4;
			snes_ppu.layer[SNES_BG1].mosaic_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].mosaic_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].mosaic_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].mosaic_enabled = BIT(data, 3);
			break;
		case BG1SC:     /* Address for storing SC data BG1 SC size designation */
		case BG2SC:     /* Address for storing SC data BG2 SC size designation  */
		case BG3SC:     /* Address for storing SC data BG3 SC size designation  */
		case BG4SC:     /* Address for storing SC data BG4 SC size designation  */
			snes_ppu.layer[offset - BG1SC].tilemap = data & 0xfc;
			snes_ppu.layer[offset - BG1SC].tilemap_size = data & 0x3;
			break;
		case BG12NBA:   /* Address for BG 1 and 2 character data */
			snes_ppu.layer[SNES_BG1].charmap = (data & 0x0f);
			snes_ppu.layer[SNES_BG2].charmap = (data & 0xf0) >> 4;
			break;
		case BG34NBA:   /* Address for BG 3 and 4 character data */
			snes_ppu.layer[SNES_BG3].charmap = (data & 0x0f);
			snes_ppu.layer[SNES_BG4].charmap = (data & 0xf0) >> 4;
			break;

		// Anomie says "H Current = (Byte<<8) | (Prev&~7) | ((Current>>8)&7); V Current = (Current<<8) | Prev;" and Prev is shared by all scrolls but in Mode 7!
		case BG1HOFS:   /* BG1 - horizontal scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			snes_ppu.layer[SNES_BG1].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG1].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			snes_ppu.mode7.hor_offset = (data << 8) | (snes_ppu.mode7_last_scroll & ~7) | ((snes_ppu.mode7.hor_offset >> 8) & 7);
			snes_ppu.mode7_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG1VOFS:   /* BG1 - vertical scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			snes_ppu.layer[SNES_BG1].voffs = (data << 8) | snes_ppu.ppu_last_scroll;
			snes_ppu.ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			snes_ppu.mode7.ver_offset = (data << 8) | snes_ppu.mode7_last_scroll;
			snes_ppu.mode7_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2HOFS:   /* BG2 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG2].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG2].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2VOFS:   /* BG2 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG2].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3HOFS:   /* BG3 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG3].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG3].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3VOFS:   /* BG3 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG3].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4HOFS:   /* BG4 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG4].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG4].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4VOFS:   /* BG4 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG4].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case VMAIN:     /* VRAM address increment value designation */
			state->m_vram_fgr_high = (data & 0x80);
			state->m_vram_fgr_increment = vram_fgr_inctab[data & 3];

			if (data & 0xc)
			{
				int md = (data & 0xc) >> 2;

				state->m_vram_fgr_count = vram_fgr_inccnts[md];         // 0x20, 0x40, 0x80
				state->m_vram_fgr_mask = (state->m_vram_fgr_count * 8) - 1; // 0xff, 0x1ff, 0x2ff
				state->m_vram_fgr_shift = vram_fgr_shiftab[md];         // 5, 6, 7
			}
			else
			{
				state->m_vram_fgr_count = 0;
			}
//          printf("VMAIN: high %x inc %x count %x mask %x shift %x\n", state->m_vram_fgr_high, state->m_vram_fgr_increment, state->m_vram_fgr_count, state->m_vram_fgr_mask, state->m_vram_fgr_shift);
			break;
		case VMADDL:    /* Address for VRAM read/write (low) */
			{
				UINT32 addr;
				state->m_vmadd = (state->m_vmadd & 0xff00) | (data << 0);
				addr = snes_get_vram_address(space.machine());
				state->m_vram_read_buffer = state->snes_vram_read(space, addr);
				state->m_vram_read_buffer |= (state->snes_vram_read(space, addr + 1) << 8);
			}
			break;
		case VMADDH:    /* Address for VRAM read/write (high) */
			{
				UINT32 addr;
				state->m_vmadd = (state->m_vmadd & 0x00ff) | (data << 8);
				addr = snes_get_vram_address(space.machine());
				state->m_vram_read_buffer = state->snes_vram_read(space, addr);
				state->m_vram_read_buffer |= (state->snes_vram_read(space, addr + 1) << 8);
			}
			break;
		case VMDATAL:   /* 2118: Data for VRAM write (low) */
			{
				UINT32 addr = snes_get_vram_address(space.machine());
				state->snes_vram_write(space, addr, data);

				if (!state->m_vram_fgr_high)
					state->m_vmadd = (state->m_vmadd + state->m_vram_fgr_increment) & 0xffff;
			}
			return;
		case VMDATAH:   /* 2119: Data for VRAM write (high) */
			{
				UINT32 addr = snes_get_vram_address(space.machine());
				state->snes_vram_write(space, addr + 1, data);

				if (state->m_vram_fgr_high)
					state->m_vmadd = (state->m_vmadd + state->m_vram_fgr_increment) & 0xffff;
			}
			return;
		case M7SEL:     /* Mode 7 initial settings */
			snes_ppu.mode7.repeat = (data >> 6) & 3;
			snes_ppu.mode7.vflip  = BIT(data, 1);
			snes_ppu.mode7.hflip  = BIT(data, 0);
			break;
		/* As per Anomie's doc: Reg = (Current<<8) | Prev; and there is only one Prev, shared by these matrix regs and Mode 7 scroll regs */
		case M7A:       /* Mode 7 COS angle/x expansion (DW) */
			snes_ppu.mode7.matrix_a = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7B:       /* Mode 7 SIN angle/ x expansion (DW) */
			snes_ppu.mode7.matrix_b = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7C:       /* Mode 7 SIN angle/y expansion (DW) */
			snes_ppu.mode7.matrix_c = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7D:       /* Mode 7 COS angle/y expansion (DW) */
			snes_ppu.mode7.matrix_d = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7X:       /* Mode 7 x center position (DW) */
			snes_ppu.mode7.origin_x = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7Y:       /* Mode 7 y center position (DW) */
			snes_ppu.mode7.origin_y = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case CGADD:     /* Initial address for colour RAM writing */
			/* CGRAM is 16-bit, but when reading/writing we treat it as 8-bit, so we need to double the address */
			state->m_cgram_address = data << 1;
			break;
		case CGDATA:    /* Data for colour RAM */
			state->snes_cgram_write(space, state->m_cgram_address, data);
			state->m_cgram_address = (state->m_cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
			break;
		case W12SEL:    /* Window mask settings for BG1-2 */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG1].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_BG1].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_BG1].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_BG1].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_BG2].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_BG2].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_BG2].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_BG2].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case W34SEL:    /* Window mask settings for BG3-4 */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG3].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_BG3].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_BG3].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_BG3].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_BG4].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_BG4].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_BG4].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_BG4].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case WOBJSEL:   /* Window mask settings for objects */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_OAM].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_OAM].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_OAM].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_OAM].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_COLOR].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_COLOR].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_COLOR].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_COLOR].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case WH0:       /* Window 1 left position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window1_left = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH1:       /* Window 1 right position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window1_right = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH2:       /* Window 2 left position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window2_left = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH3:       /* Window 2 right position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window2_right = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WBGLOG:    /* Window mask logic for BG's */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG1].wlog_mask = data & 0x03;
				snes_ppu.layer[SNES_BG2].wlog_mask = (data & 0x0c) >> 2;
				snes_ppu.layer[SNES_BG3].wlog_mask = (data & 0x30) >> 4;
				snes_ppu.layer[SNES_BG4].wlog_mask = (data & 0xc0) >> 6;
				snes_ppu.update_windows = 1;
			}
			break;
		case WOBJLOG:   /* Window mask logic for objects */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_OAM].wlog_mask = data & 0x03;
				snes_ppu.layer[SNES_COLOR].wlog_mask = (data & 0x0c) >> 2;
				snes_ppu.update_windows = 1;
			}
			break;
		case TM:        /* Main screen designation */
			snes_ppu.layer[SNES_BG1].main_bg_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].main_bg_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].main_bg_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].main_bg_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].main_bg_enabled = BIT(data, 4);
			break;
		case TS:        /* Subscreen designation */
			snes_ppu.layer[SNES_BG1].sub_bg_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].sub_bg_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].sub_bg_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].sub_bg_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].sub_bg_enabled = BIT(data, 4);
			break;
		case TMW:       /* Window mask for main screen designation */
			snes_ppu.layer[SNES_BG1].main_window_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].main_window_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].main_window_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].main_window_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].main_window_enabled = BIT(data, 4);
			break;
		case TSW:       /* Window mask for subscreen designation */
			snes_ppu.layer[SNES_BG1].sub_window_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].sub_window_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].sub_window_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].sub_window_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].sub_window_enabled = BIT(data, 4);
			break;
		case CGWSEL:    /* Initial settings for Fixed colour addition or screen addition */
			snes_ppu.clip_to_black = (data >> 6) & 0x03;
			snes_ppu.prevent_color_math = (data >> 4) & 0x03;
			snes_ppu.sub_add_mode = BIT(data, 1);
			snes_ppu.direct_color = BIT(data, 0);
#ifdef SNES_DBG_REG_W
			if ((data & 0x2) != (snes_ram[CGWSEL] & 0x2))
				mame_printf_debug( "Add/Sub Layer: %s\n", ((data & 0x2) >> 1) ? "Subscreen" : "Fixed colour" );
#endif
			break;
		case CGADSUB:   /* Addition/Subtraction designation for each screen */
			snes_ppu.color_modes = data & 0xc0;
			snes_ppu.layer[SNES_BG1].color_math = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].color_math = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].color_math = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].color_math = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].color_math = BIT(data, 4);
			snes_ppu.layer[SNES_COLOR].color_math = BIT(data, 5);
			break;
		case COLDATA:   /* Fixed colour data for fixed colour addition/subtraction */
			{
				/* Store it in the extra space we made in the CGRAM. It doesn't really go there, but it's as good a place as any. */
				UINT8 r, g, b;

				/* Get existing value. */
				r = state->m_snes_cgram[FIXED_COLOUR] & 0x1f;
				g = (state->m_snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5;
				b = (state->m_snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10;
				/* Set new value */
				if (data & 0x20)
					r = data & 0x1f;
				if (data & 0x40)
					g = data & 0x1f;
				if (data & 0x80)
					b = data & 0x1f;
				state->m_snes_cgram[FIXED_COLOUR] = (r | (g << 5) | (b << 10));
			} break;
		case SETINI:    /* Screen mode/video select */
			snes_ppu.interlace = (data & 0x01) ? 2 : 1;
			snes_ppu.obj_interlace = (data & 0x02) ? 2 : 1;
			snes_ppu.beam.last_visible_line = (data & 0x04) ? 240 : 225;
			snes_ppu.pseudo_hires = BIT(data, 3);
			snes_ppu.mode7.extbg = BIT(data, 6);
			snes_dynamic_res_change(space.machine());
#ifdef SNES_DBG_REG_W
			if ((data & 0x8) != (snes_ram[SETINI] & 0x8))
				mame_printf_debug( "Pseudo 512 mode: %s\n", (data & 0x8) ? "on" : "off" );
#endif
			break;
		}

	snes_ram[offset] = data;
}

/***** Debug Functions *****/

#if SNES_LAYER_DEBUG

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
	}

static UINT8 snes_dbg_video( running_machine &machine, UINT16 curline )
{
	int i;
	UINT8 toggles = machine.root_device().ioport("DEBUG1")->read_safe(0);
	debug_options.select_pri[SNES_BG1] = (toggles & 0x03);
	debug_options.select_pri[SNES_BG2] = (toggles & 0x0c) >> 2;
	debug_options.select_pri[SNES_BG3] = (toggles & 0x30) >> 4;
	debug_options.select_pri[SNES_BG4] = (toggles & 0xc0) >> 6;

	toggles = machine.root_device().ioport("DEBUG2")->read_safe(0);
	for (i = 0; i < 4; i++)
		DEBUG_TOGGLE(i, debug_options.bg_disabled[i], ("Debug: Disabled BG%d.\n", i + 1), ("Debug: Enabled BG%d.\n", i + 1))
	DEBUG_TOGGLE(4, debug_options.bg_disabled[SNES_OAM], ("Debug: Disabled OAM.\n"), ("Debug: Enabled OAM.\n"))
	DEBUG_TOGGLE(5, debug_options.draw_subscreen, ("Debug: Switched screens.\n"), ("Debug: Switched screens.\n"))
	DEBUG_TOGGLE(6, debug_options.colormath_disabled, ("Debug: Disabled Color Math.\n"), ("Debug: Enabled Color Math.\n"))
	DEBUG_TOGGLE(7, debug_options.windows_disabled, ("Debug: Disabled Window Masks.\n"), ("Debug: Enabled Window Masks.\n"))

	toggles = machine.root_device().ioport("DEBUG4")->read_safe(0);
	for (i = 0; i < 8; i++)
		DEBUG_TOGGLE(i, debug_options.mode_disabled[i], ("Debug: Disabled Mode %d drawing.\n", i), ("Debug: Enabled Mode %d drawing.\n", i))

	toggles = machine.root_device().ioport("DEBUG3")->read_safe(0);
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
