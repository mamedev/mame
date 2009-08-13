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

#include "driver.h"
#include "profiler.h"
#include "includes/snes.h"

#define MAINSCREEN      0
#define SUBSCREEN       1
#define SNES_BLEND_NONE 0
#define SNES_BLEND_ADD  1
#define SNES_BLEND_SUB  2
#define SNES_CLIP_ALL   0
#define SNES_CLIP_IN    1
#define SNES_CLIP_OUT   2
#define SNES_CLIP_ALL2  3

#ifdef MAME_DEBUG
struct DEBUGOPTS
{
	UINT8 input_count;
	UINT8 bg_disabled[6];
	UINT8 mode_disabled[8];
	UINT8 draw_subscreen;
	UINT8 windows_disabled;
	UINT8 transparency_disabled;
};
static struct DEBUGOPTS debug_options  = {5, {0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0};
/*                                    red   green  blue    purple  yellow cyan    grey    white */
static const UINT16 dbg_mode_colours[8] = { 0x1f, 0x3e0, 0x7c00, 0x7c1f, 0x3ff, 0x7fe0, 0x4210, 0x7fff };
static UINT8 snes_dbg_video(running_machine *machine, bitmap_t *bitmap, UINT16 curline);
#endif /* MAME_DEBUG */

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
/* Scroll tables                             32x32      64x32              32x64              64x64 */
static const UINT16 table_hscroll[4][4] = { {0,0,0,0}, {0,0x800,0,0x800}, {0,0,0,0}, {0,0x800,0,0x800} };
static const UINT16 table_vscroll[4][4] = { {0,0,0,0}, {0,0,0,0}, {0,0x800,0,0x800}, {0,0x1000,0,0x1000} };

struct SCANLINE
{
	UINT16 buffer[(SNES_SCR_WIDTH * 2) + 16];
	UINT8  zbuf[(SNES_SCR_WIDTH * 2) + 16];
};

static struct SCANLINE scanlines[2];
struct SNES_PPU_STRUCT snes_ppu;

extern UINT16 snes_htmult;

enum
{
	SNES_COLOR_DEPTH_2BPP = 0,
	SNES_COLOR_DEPTH_4BPP,
	SNES_COLOR_DEPTH_8BPP
};

/*****************************************
 * snes_draw_blend()
 *
 * Routine for additive/subtractive blending
 * between the main and sub screens.
 *****************************************/
INLINE void snes_draw_blend(UINT16 offset, UINT16 *colour, UINT8 mode, UINT8 clip )
{
	if (clip == SNES_CLIP_ALL2) // blending mode 3 == always OFF
		return;

#ifdef MAME_DEBUG
	if( !debug_options.transparency_disabled )
#endif /* MAME_DEBUG */
	if( (clip == SNES_CLIP_ALL) ||
		(clip == SNES_CLIP_IN  && !snes_ppu.clipmasks[5][offset]) ||
		(clip == SNES_CLIP_OUT && snes_ppu.clipmasks[5][offset]) )
	{
		UINT16 r, g, b;
		if( mode == SNES_BLEND_ADD )
		{
			if( snes_ppu.sub_add_mode ) /* Subscreen*/
			{
				r = (*colour & 0x1f) + (scanlines[SUBSCREEN].buffer[offset] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) + ((scanlines[SUBSCREEN].buffer[offset] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) + ((scanlines[SUBSCREEN].buffer[offset] & 0x7c00) >> 10);
				if( (snes_ram[CGADSUB] & 0x40) && (scanlines[SUBSCREEN].zbuf[offset]) ) /* FIXME: We shouldn't halve for the back colour */
				{
					r >>= 1;
					g >>= 1;
					b >>= 1;
				}
			}
			else /* Fixed colour */
			{
				r = (*colour & 0x1f) + (snes_cgram[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) + ((snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) + ((snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
				if( snes_ram[CGADSUB] & 0x40 ) /* FIXME: We shouldn't halve for the back colour */
				{
					r >>= 1;
					g >>= 1;
					b >>= 1;
				}
			}
			if( r > 0x1f ) r = 0x1f;
			if( g > 0x1f ) g = 0x1f;
			if( b > 0x1f ) b = 0x1f;
			*colour = ((r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10));
		}
		else if( mode == SNES_BLEND_SUB )
		{
			if( snes_ppu.sub_add_mode ) /* Subscreen */
			{
				r = (*colour & 0x1f) - (scanlines[SUBSCREEN].buffer[offset] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) - ((scanlines[SUBSCREEN].buffer[offset] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) - ((scanlines[SUBSCREEN].buffer[offset] & 0x7c00) >> 10);
				if( r > 0x1f ) r = 0;
				if( g > 0x1f ) g = 0;
				if( b > 0x1f ) b = 0;
				if( (snes_ram[CGADSUB] & 0x40) && (scanlines[SUBSCREEN].zbuf[offset]) ) /* FIXME: We shouldn't halve for the back colour */
				{
					r >>= 1;
					g >>= 1;
					b >>= 1;
				}
			}
			else /* Fixed colour */
			{
				r = (*colour & 0x1f) - (snes_cgram[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) - ((snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) - ((snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
				if( r > 0x1f ) r = 0;
				if( g > 0x1f ) g = 0;
				if( b > 0x1f ) b = 0;
				if( snes_ram[CGADSUB] & 0x40 ) /* FIXME: We shouldn't halve for the back colour */
				{
					r >>= 1;
					g >>= 1;
					b >>= 1;
				}
			}
			*colour = ((r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10));
		}
	}
}


/*****************************************
 * snes_draw_tile()
 *
 * Draw tiles with variable bit planes
 *****************************************/
INLINE void snes_draw_tile(UINT8 screen, UINT8 planes, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal )
{
	UINT8 mask, plane[8], window_enabled = 0;
	UINT16 c;
	INT16 ii, jj;

	for (ii = 0; ii < planes / 2; ii++)
	{
		plane[2 * ii] = snes_vram[tileaddr + 16 * ii];
		plane[2 * ii + 1] = snes_vram[tileaddr + 16 * ii + 1];
	}

	if (flip)
		mask = 0x1;
	else
		mask = 0x80;

	for (ii = x; ii < (x + 8); ii++)
	{
		UINT8 colour = 0;
		if (flip)
		{
			for (jj = 0; jj < planes; jj++)
				colour |= plane[jj] & mask ? (1 << jj) : 0;

			mask <<= 1;
		}
		else
		{
			for (jj = 0; jj < planes; jj++)
				colour |= plane[jj] & mask ? (1 << jj) : 0;

			mask >>= 1;
		}

#ifdef MAME_DEBUG
		if(!debug_options.windows_disabled)
#endif /* MAME_DEBUG */
		/* Clip to windows */
		window_enabled = (screen == MAINSCREEN) ? snes_ppu.layer[layer].main_window_enabled : snes_ppu.layer[layer].sub_window_enabled;
		if (window_enabled)
			colour &= snes_ppu.clipmasks[layer][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if (colour)
		{
			if ((scanlines[screen].zbuf[ii] <= priority) && (ii >= 0))
			{
				c = snes_cgram[pal + colour];
				if (screen == MAINSCREEN)	/* Only blend main screens */
					snes_draw_blend(ii/snes_htmult, &c, snes_ppu.layer[layer].blend, (snes_ram[CGWSEL] & 0x30) >> 4);
				if (snes_ppu.layer[layer].mosaic_enabled) // handle horizontal mosaic
				{
					int x_mos;

					//TODO: 512 modes has the h values doubled.
					for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1) ; x_mos++)
					{
						scanlines[screen].buffer[ii + x_mos] = c;
						scanlines[screen].zbuf[ii + x_mos] = priority;
					}
					ii += x_mos - 1;
				}
				else
				{
					scanlines[screen].buffer[ii] = c;
					scanlines[screen].zbuf[ii] = priority;
				}
			}
		}
	}
}

/*****************************************
 * snes_draw_tile_x2()
 *
 * Draw 2 tiles with variable bit planes
 *****************************************/
INLINE void snes_draw_tile_x2(UINT8 screen, UINT8 planes, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal )
{
	if( flip )
	{
		snes_draw_tile(screen, planes, layer, tileaddr + (8 * planes), x, priority, flip, pal);
		snes_draw_tile(screen, planes, layer, tileaddr, x + 8, priority, flip, pal);
	}
	else
	{
		snes_draw_tile(screen, planes, layer, tileaddr, x, priority, flip, pal);
		snes_draw_tile(screen, planes, layer, tileaddr + (8 * planes), x + 8, priority, flip, pal);
	}
}

/*****************************************
 * snes_draw_tile_object()
 *
 * Draw tiles with 4 bit planes(16 colors)
 * The same as snes_draw_tile_4() except
 * that it takes a blend parameter.
 *****************************************/
INLINE void snes_draw_tile_object(UINT8 screen, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal, UINT8 blend, UINT8 wide )
{
	UINT8 mask, plane[4], window_enabled = 0;
	UINT16 c;
	INT16 ii;
	UINT8 x_shift = wide ? 1 : 0;
	UINT8 size = wide ? 16 : 8;
	UINT8 step = wide ? 1 : 0;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];
	plane[2] = snes_vram[tileaddr + 16];
	plane[3] = snes_vram[tileaddr + 17];

	if (flip)
		mask = 0x01;
	else
		mask = 0x80;

	x <<= x_shift;
	for (ii = x; ii < (x + size); ii += 1 + step)
	{
		UINT8 colour;
		if (flip)
		{
			colour = (plane[0] & mask ? 1 : 0) | (plane[1] & mask ? 2 : 0) |
					 (plane[2] & mask ? 4 : 0) | (plane[3] & mask ? 8 : 0);
			mask <<= 1;
		}
		else
		{
			colour = (plane[0] & mask ? 1 : 0) | (plane[1] & mask ? 2 : 0) |
					 (plane[2] & mask ? 4 : 0) | (plane[3] & mask ? 8 : 0);
			mask >>= 1;
		}

#ifdef MAME_DEBUG
		if (!debug_options.windows_disabled)
#endif /* MAME_DEBUG */
		/* Clip to windows */
		window_enabled = (screen == MAINSCREEN) ? snes_ppu.layer[4].main_window_enabled : snes_ppu.layer[4].sub_window_enabled;
		if (window_enabled)
			colour &= snes_ppu.clipmasks[4][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if (colour)
		{
			if (ii >= 0)
			{
				c = snes_cgram[pal + colour];
				if (blend && screen == MAINSCREEN)	/* Only blend main screens */
					snes_draw_blend(ii/snes_htmult, &c, snes_ppu.layer[4].blend, (snes_ram[CGWSEL] & 0x30) >> 4);

				scanlines[screen].buffer[ii] = c;
				scanlines[screen].zbuf[ii] = priority;
				if (wide)
				{
					scanlines[screen].buffer[ii + 1] = c;
					scanlines[screen].zbuf[ii + 1] = priority;
				}
			}
		}
	}
}

/*********************************************
 * snes_update_line()
 *
 * Update an entire line of tiles.
 *********************************************/
INLINE void snes_update_line( UINT8 screen, UINT8 color_depth, UINT8 hires, UINT8 priority_a, UINT8 priority_b, UINT8 layer, UINT16 curline )
{
	UINT32 tmap, tile;
	UINT16 ii, vflip, hflip, pal;
	INT8 line, tile_line;
	UINT8 priority;
	/* scrolling */
	UINT32 basevmap;
	UINT16 vscroll, hscroll, vtilescroll;
//	UINT16 offset_per_tile_valid;
//	UINT8 offset_per_tile_mode;
	UINT8 vshift, hshift, tile_size;
	/* variables depending on color_depth */
	UINT8 color_shift = 0;
	UINT8 color_planes = 2;
	UINT8 tile_divider = 1;
	UINT8 wrap_around_x; //helper for wrap-around

#ifdef MAME_DEBUG
	if (debug_options.bg_disabled[layer])
		return;

	if (debug_options.mode_disabled[snes_ppu.mode])
		return;
#endif /* MAME_DEBUG */

	/* Handle Mosaic effects */
	if (snes_ppu.layer[layer].mosaic_enabled)
		curline -= (curline % (snes_ppu.mosaic_size + 1));

	/* Find the size of the tiles (8x8 or 16x16) */
	tile_size = snes_ppu.layer[layer].tile_size;
	/* Find scroll info */
	vscroll = snes_ppu.layer[layer].offset.tile_vert;
	vshift = snes_ppu.layer[layer].offset.shift_vert;
	hscroll = snes_ppu.layer[layer].offset.tile_horz;
	hshift = snes_ppu.layer[layer].offset.shift_horz;

	/* Find vertical scroll amount */
	vtilescroll = vscroll + (curline >> (3 + tile_size));
	/* figure out which line to draw */
	line = (curline % (8 << tile_size)) + vshift;
	if (line > ((8 << tile_size) - 1))	/* scrolled into the next tile */
	{
		vtilescroll++;	/* pretend we scrolled by 1 tile line */
		line -= (8 << tile_size);
	}
	if( vtilescroll >= 128 )
		vtilescroll -= 128;

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].map;

	/* Offset vertically */
	tmap += table_vscroll[snes_ppu.layer[layer].map_size & 3][(vtilescroll >> 5) & 3];
	/* Scroll vertically */
	tmap += (vtilescroll & 0x1f) << 6;
	/* Remember this position */
	basevmap = tmap;
	/* Offset horizontally */
	tmap += table_hscroll[snes_ppu.layer[layer].map_size & 3][(hscroll >> 5) & 3];
	/* Scroll horizontally */
	tmap += (hscroll & 0x1f) << 1;

	/* set a couple of variables depending on color_dept */
	switch (color_depth)
	{
		case SNES_COLOR_DEPTH_2BPP:
			color_shift = 0;
			color_planes = 2;
			tile_divider = 2;
			break;
		case SNES_COLOR_DEPTH_4BPP:
			color_shift = 2;
			color_planes = 4;
			tile_divider = 2;
			break;
		case SNES_COLOR_DEPTH_8BPP:
			color_shift = 0;	//n/a, pal offset is always zero
			color_planes = 8;
			tile_divider = 4;
			if(snes_ppu.direct_color) //we don't know what games trigger this one...
				fatalerror("8bpp graphics with direct color, gfx mode used = %02x",snes_ppu.mode);
			break;
	}

	wrap_around_x = 1;

	for (ii = 0; ii < (66*(hires+1) >> tile_size); ii += 2)
	{
		/* Have we scrolled into the next map? */
		if (wrap_around_x && ((ii >> 1) >= 32 - (hscroll & 0x1f)))
		{
			tmap = basevmap + table_hscroll[snes_ppu.layer[layer].map_size & 3][((hscroll >> 5) + 1) & 3];
			tmap -= ii;
			wrap_around_x = 0;	/* Make sure we don't do this again */
		}
		//if (tmap > 0x10000) //<- causes corrupt tiles in places, needed?
		//	tmap %= 0x10000;

		vflip = snes_vram[tmap + ii + 1] & 0x80;
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		priority = ((snes_vram[tmap + ii + 1] & 0x20) >> 5) ? priority_b : priority_a;
		pal = (color_depth == SNES_COLOR_DEPTH_8BPP) ? 0 : (snes_vram[tmap + ii + 1] & 0x1c) << color_shift;		/* 8 palettes of (4 * color_shift) colours */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii] & 0xff;

		/* Mode 0 palettes are layer specific */
		if (snes_ppu.mode == 0)
		{
			pal += (layer << 5);
		}

		tile_line = line;
		if (vflip)
		{
			if (tile_size)
			{
				if (line > 7)
				{
					tile_line -= 8;
				}
				else
				{
					tile += 32 / tile_divider;
				}
			}
			tile_line = -tile_line + 7;
		}
		else
		{
			if (line > 7)
			{
				tile += 32 / tile_divider;
				tile_line -= 8;
			}
		}
		tile_line <<= 1;

		/* below, only color_planes depends on color_depth */
		if (tile_size)
		{
			#if 0
			/* Bishoujo Janshi SuchiiPai and Desert Fighter sets this in hires, no noticeable difference apart that x must be doubled somehow... */
			if (hires)	/* Hi-Res: 2bpp & 4bpp */
			{
				if (hflip)
				{
					snes_draw_tile_x2(screen, color_planes, layer, snes_ppu.layer[layer].data + (tile * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1) + 16, priority, hflip, pal);
					snes_draw_tile_x2(screen, color_planes, layer, snes_ppu.layer[layer].data + ((tile + 2)  * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal);
				}
				else
				{
					snes_draw_tile_x2(screen, color_planes, layer, snes_ppu.layer[layer].data + (tile  * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal);
					snes_draw_tile_x2(screen, color_planes, layer, snes_ppu.layer[layer].data + ((tile + 2)  * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1) + 16, priority, hflip, pal);
				}
			}
			else	/* No Hi-Res: 2bpp, 4bpp & 8bpp */
			#endif
			{
				snes_draw_tile_x2(screen, color_planes, layer, snes_ppu.layer[layer].data + (tile  * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip, pal);
			}
		}
		else	/* tile_size = 0 */
		{
			if (hires)	/* Hi-Res: 2bpp & 4bpp */
				snes_draw_tile_x2(screen, color_planes, layer, snes_ppu.layer[layer].data + (tile  * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal);
			else	/* No Hi-Res: 2bpp, 4bpp & 8bpp */
				snes_draw_tile(screen, color_planes, layer, snes_ppu.layer[layer].data + (tile  * 8 * color_planes) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip, pal);
		}
	}
}


/*********************************************
 * snes_update_line_mode7()
 *
 * Update an entire line of mode7 tiles.
 *********************************************/
#define MODE7_CLIP(x) (((x) & 0x2000) ? ((x) | ~0x03ff) : ((x) & 0x03ff))

static void snes_update_line_mode7(UINT8 screen, UINT8 priority_a, UINT8 priority_b, UINT8 layer, UINT16 curline )
{
	UINT32 tiled;
	INT16 ma, mb, mc, md;
	INT32 xc, yc, tx, ty, sx, sy, hs, vs, xpos, xdir, x0, y0;
	UINT8 priority = priority_a;
	UINT8 colour = 0, window_enabled = 0;
	UINT16 *mosaic_x, *mosaic_y;

#ifdef MAME_DEBUG
	if (debug_options.bg_disabled[layer])
		return;

	if (debug_options.mode_disabled[snes_ppu.mode])
		return;
#endif /* MAME_DEBUG */

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
	if (layer == 1)	// BG2 use two different bits for horizontal and vertical mosaic
	{
		mosaic_x = snes_ppu.mosaic_table[snes_ppu.layer[1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y = snes_ppu.mosaic_table[snes_ppu.layer[0].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}
	else	// BG1 works as usual
	{
		mosaic_x =  snes_ppu.mosaic_table[snes_ppu.layer[0].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y =  snes_ppu.mosaic_table[snes_ppu.layer[0].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}

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
				if ((tx > 0) && (tx < 1024) && (ty > 0) && (ty < 1024))
				{
					tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
					colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
				}
				else
					colour = 0;
				break;
			case 0x03:	/* Character 0x00 repeat if outside screen area */
				if ((tx > 0) && (tx < 1024) && (ty > 0) && (ty < 1024))
					tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
				else
					tiled = 0;

				colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
				break;
		}

		/* The last bit is for priority in EXTBG mode (used only for BG2) */
		if (layer == 1)
		{
			priority = ((colour & 0x80) >> 7) ? priority_b : priority_a;
			colour &= 0x7f;
		}

		window_enabled = (screen == MAINSCREEN) ? snes_ppu.layer[layer].main_window_enabled : snes_ppu.layer[layer].sub_window_enabled;
		if (window_enabled)
			colour &= snes_ppu.clipmasks[layer][xpos];

		/* Draw pixel if appropriate */
		if (scanlines[screen].zbuf[xpos] <= priority && colour > 0)
		{
			UINT16 clr;
			/* Direct select, but only outside EXTBG! */
			if (snes_ppu.direct_color && layer == 0)
				clr = ((colour & 0x07) << 2) | ((colour & 0x38) << 4) | ((colour & 0xc0) << 7);
			else
				clr = snes_cgram[colour];
			/* Only blend main screens */
			if (screen == MAINSCREEN)
				snes_draw_blend(xpos, &clr, snes_ppu.layer[layer].blend, (snes_ram[CGWSEL] & 0x30) >> 4);		/* FIXME: Need to support clip mode */

			scanlines[screen].buffer[xpos] = clr;
			scanlines[screen].zbuf[xpos] = priority;
		}
	}
}

/*********************************************
 * snes_update_objects()
 *
 * Update an entire line of sprites.
 * FIXME: We need to support high priority bit
 *********************************************/
static void snes_update_objects( UINT8 screen, UINT8 priority_tbl, UINT16 curline )
{
	INT8 xs, ys;
	UINT8 line, widemode = 0;
	UINT16 oam_extra, extra;
	INT16 oam;
	UINT8 range_over = 0, time_over = 0;
	UINT8 size, vflip, hflip, priority, pal, blend;
	UINT16 tile;
	INT16 i, x, y;
	UINT8 *oamram = (UINT8 *)snes_oam;
	UINT32 name_sel = 0;
	static const UINT8 table_obj_priority[10][4] = {
						{2, 5, 8, 11},	// mode 0
						{1, 3, 6, 9},	// mode 1
						{1, 3, 5, 7},	// mode 2
						{1, 3, 5, 7},	// mode 3
						{1, 3, 5, 7},	// mode 4
						{1, 3, 5, 7},	// mode 5
						{0, 2, 3, 5},	// mode 6
						{0, 2, 3, 4},	// mode 7
						{1, 3, 5, 6},	// mode 7 EXTBG
						{1, 2, 5, 8}	// mode 1 + BG3 priority bit
					};

#ifdef MAME_DEBUG
	if( debug_options.bg_disabled[4] )
		return;
#endif /* MAME_DEBUG */

	if( snes_ppu.mode == 5 || snes_ppu.mode == 6 )
		widemode = 1;

	curline/=snes_ppu.interlace;

	oam = 0x1ff;
	oam_extra = oam + 0x20;
	extra = 0;
	for( i = 128; i > 0; i-- )
	{
		if( (i % 4) == 0 )
			extra = oamram[oam_extra--];

		vflip = (oamram[oam] & 0x80) >> 7;
		hflip = (oamram[oam] & 0x40) >> 6;
		priority = table_obj_priority[priority_tbl][(oamram[oam] & 0x30) >> 4];
		pal = 128 + ((oamram[oam] & 0x0e) << 3);
		tile = (oamram[oam--] & 0x1) << 8;
		tile |= oamram[oam--];
		y = oamram[oam--] + 1;	/* We seem to need to add one here.... */
		x = oamram[oam--];
		size = (extra & 0x80) >> 7;
		extra <<= 1;
		x |= ((extra & 0x80) << 1);
		extra <<= 1;

		/* Adjust if past maximum position */
		if( y >= snes_ppu.beam.last_visible_line*snes_ppu.interlace )
			y -= 256*snes_ppu.interlace;
		if( x > 255 )
			x -= 512;

		/* Draw sprite if it intersects the current line */
		if( curline >= y && curline < (y + (snes_ppu.oam.size[size] << 3)) )
		{
			/* Only objects using palettes 4-7 can be transparent */
			blend = (pal < 192) ? 0 : 1;

			/* Only objects using tiles over 255 use name select */
			name_sel = (tile < 256) ? 0 : snes_ppu.oam.name_select;

			ys = (curline - y) >> 3;
			line = (curline - y) % 8;
			if( vflip )
			{
				ys = snes_ppu.oam.size[size] - ys - 1;
				line = (-1 * line) + 7;
			}
			line <<= 1;
			tile <<= 5;
			if( hflip )
			{
				UINT8 count = 0;
				for( xs = (snes_ppu.oam.size[size] - 1); xs >= 0; xs-- )
				{
					if( (x + (count << 3) < SNES_SCR_WIDTH + 8) )
					{
						if( widemode )
							snes_draw_tile_object(screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (count++ << 3), priority, hflip, pal, blend, 1);
						else
							snes_draw_tile_object(screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (count++ << 3), priority, hflip, pal, blend, 0);
					}
					time_over++;	/* Increase time_over. Should we stop drawing if exceeded 34 tiles? */
				}
			}
			else
			{
				for( xs = 0; xs < snes_ppu.oam.size[size]; xs++ )
				{
					if( (x + (xs << 3) < SNES_SCR_WIDTH + 8) )
					{
						if( widemode )
							snes_draw_tile_object(screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (xs << 3), priority, hflip, pal, blend, 1);
						else
							snes_draw_tile_object(screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (xs << 3), priority, hflip, pal, blend, 0);
					}
					time_over++;	/* Increase time_over. Should we stop drawing if exceeded 34 tiles? */
				}
			}

			/* Increase range_over.
             * Stop drawing if exceeded 32 objects and
             * enforcing that limit is enabled */
			range_over++;
			if( range_over == 32 ) //&& (input_port_read(machine, "INTERNAL") & 0x01) )
			{
				/* Set the flag in STAT77 register */
				snes_ram[STAT77] |= 0x40;
				/* FIXME: This stops the SNESTest rom from drawing the object
                 *        test properly.  Maybe we shouldn't stop drawing? */
				/* return; */
			}
		}
	}

	if( time_over >= 34 )
	{
		/* Set the flag in STAT77 register */
		snes_ram[STAT77] |= 0x80;
	}
}


/*********************************************
 * snes_update_mode_X()
 *
 * Update Mode X line.
 *********************************************/
static void snes_update_mode_0( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (bg_enabled[4]) snes_update_objects(screen, 0, curline);
	if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 7, 10, 0, curline);
	if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 6, 9, 1, curline);
	if (bg_enabled[2]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 1, 4, 2, curline);
	if (bg_enabled[3]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 0, 3, 3, curline);
}

static void snes_update_mode_1( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (!snes_ppu.bg3_priority_bit)
	{
		if (bg_enabled[4]) snes_update_objects(screen, 1, curline);
		if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 5, 8, 0, curline);
		if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 4, 7, 1, curline);
		if (bg_enabled[2]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 0, 2, 2, curline);
	}
	else
	{
		if (bg_enabled[4]) snes_update_objects(screen, 9, curline);
		if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 4, 7, 0, curline);
		if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 3, 6, 1, curline);
		if (bg_enabled[2]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 0, 9, 2, curline);
	}
}

static void snes_update_mode_2( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (bg_enabled[4]) snes_update_objects(screen, 2, curline);
	if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 2, 6, 0, curline);
	if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 0, 4, 1, curline);
}

static void snes_update_mode_3( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (bg_enabled[4]) snes_update_objects(screen, 3, curline);
	if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_8BPP, 0, 2, 6, 0, curline);
	if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 0, 0, 4, 1, curline);
}

static void snes_update_mode_4( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (bg_enabled[4]) snes_update_objects(screen, 4, curline);
	if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_8BPP, 0, 2, 6, 0, curline);
	if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 0, 0, 4, 1, curline);
}

static void snes_update_mode_5( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (bg_enabled[4]) snes_update_objects(screen, 5, curline);
	if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 1, 2, 6, 0, curline);
	if (bg_enabled[1]) snes_update_line(screen, SNES_COLOR_DEPTH_2BPP, 1, 0, 4, 1, curline);
}

static void snes_update_mode_6( UINT8 screen, UINT16 curline )
{
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (bg_enabled[4]) snes_update_objects(screen, 6, curline);
	if (bg_enabled[0]) snes_update_line(screen, SNES_COLOR_DEPTH_4BPP, 1, 1, 4, 0, curline);
}

static void snes_update_mode_7( UINT8 screen, UINT16 curline )
{
	UINT8 extbg_mode = snes_ram[SETINI] & 0x40;
	UINT8 *bg_enabled;
	bg_enabled = (screen == MAINSCREEN) ? snes_ppu.main_bg_enabled : snes_ppu.sub_bg_enabled;

	if (!extbg_mode)
	{
		if (bg_enabled[4]) snes_update_objects(screen, 7, curline);
		if (bg_enabled[0]) snes_update_line_mode7(screen, 1, 1, 0, curline);
	}
	else
	{
		if (bg_enabled[4]) snes_update_objects(screen, 8, curline);
		if (bg_enabled[0]) snes_update_line_mode7(screen, 2, 2, 0, curline);
		if (bg_enabled[1]) snes_update_line_mode7(screen, 0, 4, 1, curline);
	}
}

/*********************************************
 * snes_draw_screen()
 *
 * Draw the whole screen (Mode 0 -> 7).
 *********************************************/
static void snes_draw_screen( UINT8 screen, UINT16 curline )
{
	switch (snes_ppu.mode)
	{
		case 0: snes_update_mode_0(screen, curline); break;		/* Mode 0 */
		case 1: snes_update_mode_1(screen, curline); break;		/* Mode 1 */
		case 2: snes_update_mode_2(screen, curline); break;		/* Mode 2 - Supports offset per tile */
		case 3: snes_update_mode_3(screen, curline); break;		/* Mode 3 - Supports direct colour */
		case 4: snes_update_mode_4(screen, curline); break;		/* Mode 4 - Supports offset per tile and direct colour */
		case 5: snes_update_mode_5(screen, curline); break;		/* Mode 5 - Supports hires */
		case 6: snes_update_mode_6(screen, curline); break;		/* Mode 6 - Supports offset per tile and hires */
		case 7: snes_update_mode_7(screen, curline); break;		/* Mode 7 - Supports direct colour */
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
static void snes_update_windowmasks(void)
{
	UINT16 ii, jj;
	INT8 w1, w2;

	snes_ppu.update_windows = 0;		/* reset the flag */

	for (ii = 0; ii < SNES_SCR_WIDTH + 8; ii++)
	{
		/* update bg 1, 2, 3, 4 & obj */
		/* jj = layer */
		for (jj = 0; jj < 5; jj++)
		{
			snes_ppu.clipmasks[jj][ii] = 0xff;
			w1 = w2 = -1;
			if (snes_ppu.layer[jj].window1_enabled)
			{
				if ((ii < snes_ppu.window1_left) || (ii > snes_ppu.window1_right))
					w1 = 0;
				else
					w1 = 1;
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
			else if( w1 >= 0 )
				snes_ppu.clipmasks[jj][ii] = w1 ? 0x00 : 0xff;
			else if( w2 >= 0 )
				snes_ppu.clipmasks[jj][ii] = w2 ? 0x00 : 0xff;
		}

		/* update colour window */
		snes_ppu.clipmasks[5][ii] = 0xff;
		w1 = w2 = -1;
		if (snes_ppu.colour.window1_enabled)
		{
			/* Default to mask area inside */
			if ((ii < snes_ppu.window1_left) || (ii > snes_ppu.window1_right))
				w1 = 0;
			else
				w1 = 1;
			/* If mask area is outside then swap */
			if (snes_ppu.colour.window1_invert)
				w1 = !w1;
		}
		if (snes_ppu.colour.window2_enabled)
		{
			if ((ii < snes_ppu.window2_left) || (ii > snes_ppu.window2_right))
				w2 = 0;
			else
				w2 = 1;
			if (snes_ppu.colour.window2_invert)
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch (snes_ppu.colour.wlog_mask)
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[5][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x4:	/* AND */
					snes_ppu.clipmasks[5][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x8:	/* XOR */
					snes_ppu.clipmasks[5][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0xc:	/* XNOR */
					snes_ppu.clipmasks[5][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[5][ii] = w1 ? 0x00 : 0xff;
		else if( w2 >= 0 )
			snes_ppu.clipmasks[5][ii] = w2 ? 0x00 : 0xff;
	}
}

/*********************************************
 * snes_update_offsets()
 *
 * Update the offsets with the latest changes.
 *********************************************/
static void snes_update_offsets(void)
{
	int ii;
	for( ii = 0; ii < 4; ii++ )
	{
		snes_ppu.layer[ii].offset.tile_horz = (snes_ppu.layer[ii].offset.horizontal & 0x3ff) >> (3 + snes_ppu.layer[ii].tile_size);
		snes_ppu.layer[ii].offset.shift_horz = snes_ppu.layer[ii].offset.horizontal & ((8 << snes_ppu.layer[ii].tile_size) - 1);
		snes_ppu.layer[ii].offset.tile_vert = (snes_ppu.layer[ii].offset.vertical & 0x3ff) >> (3 + snes_ppu.layer[ii].tile_size);
		snes_ppu.layer[ii].offset.shift_vert = snes_ppu.layer[ii].offset.vertical & ((8 << snes_ppu.layer[ii].tile_size) - 1);
	}
	#if 0
	popmessage("%04x %04x|%04x %04x|%04x %04x|%04x %04x",
	snes_ppu.layer[0].offset.tile_horz,
	snes_ppu.layer[0].offset.tile_vert,
	snes_ppu.layer[1].offset.tile_horz,
	snes_ppu.layer[1].offset.tile_vert,
	snes_ppu.layer[2].offset.tile_horz,
	snes_ppu.layer[2].offset.tile_vert,
	snes_ppu.layer[3].offset.tile_horz,
	snes_ppu.layer[3].offset.tile_vert
	);
	#endif
	snes_ppu.update_offsets = 0;
}

/*********************************************
 * snes_refresh_scanline()
 *
 * Redraw the current line.
 *********************************************/
static void snes_refresh_scanline( running_machine *machine, bitmap_t *bitmap, UINT16 curline )
{
	UINT16 ii;
	int x;
	int fade;
	struct SCANLINE *scanline;

	profiler_mark(PROFILER_VIDEO);

	if (snes_ram[INIDISP] & 0x80) /* screen is forced blank */
		for (x = 0; x < SNES_SCR_WIDTH * snes_htmult; x++)
			*BITMAP_ADDR32(bitmap, curline, x) = RGB_BLACK;
	else
	{
		/* Update clip window masks if necessary */
		if (snes_ppu.update_windows)
			snes_update_windowmasks();
		/* Update the offsets if necessary */
		if (snes_ppu.update_offsets)
			snes_update_offsets();

		/* Clear zbuffers */
		memset(scanlines[MAINSCREEN].zbuf, 0, SNES_SCR_WIDTH * 2);
		memset(scanlines[SUBSCREEN].zbuf, 0, SNES_SCR_WIDTH * 2);

		/* Clear subscreen and draw back colour */
		for (ii = 0; ii < SNES_SCR_WIDTH * 2; ii++)
		{
			/* Not sure if this is correct behaviour, but a few games seem to
             * require it. (SMW, Zelda etc) */
			scanlines[SUBSCREEN].buffer[ii] = snes_cgram[FIXED_COLOUR];
			/* Draw back colour */
			scanlines[MAINSCREEN].buffer[ii] = snes_cgram[0];
		}

		/* Draw subscreen */
		snes_draw_screen(SUBSCREEN, curline);

		/* Draw the back plane */
#ifdef MAME_DEBUG
		if (!debug_options.bg_disabled[5])
#endif /* MAME_DEBUG */
		if (snes_ram[CGADSUB] & 0x20)
		{
			for(ii = 0; ii < SNES_SCR_WIDTH * snes_htmult; ii++)
			{
				snes_draw_blend(ii/snes_htmult, &scanlines[MAINSCREEN].buffer[ii], (snes_ram[CGADSUB] & 0x80) ? SNES_BLEND_SUB : SNES_BLEND_ADD, (snes_ram[CGWSEL] & 0x30) >> 4);
			}
		}

		/* Draw mainscreen */
		snes_draw_screen(MAINSCREEN, curline);

#ifdef MAME_DEBUG
		if (snes_dbg_video(machine, bitmap, curline))
		{
			profiler_mark(PROFILER_END);
			return;
		}

		/* Toggle drawing of subscreen or mainscreen */
		if (debug_options.draw_subscreen)
			scanline = &scanlines[SUBSCREEN];
		else
#endif /* MAME_DEBUG */
			scanline = &scanlines[MAINSCREEN];

		/* Phew! Draw the line to screen */
		fade = (snes_ram[INIDISP] & 0xf) + 1;

		for (x = 0; x < SNES_SCR_WIDTH * snes_htmult; x++)
		{
			int r = ((scanline->buffer[x] & 0x1f) * fade) >> 4;
			int g = (((scanline->buffer[x] & 0x3e0) >> 5) * fade) >> 4;
			int b = (((scanline->buffer[x] & 0x7c00) >> 10) * fade) >> 4;
			*BITMAP_ADDR32(bitmap, curline, x) = MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b));
		}
	}

	profiler_mark(PROFILER_END);
}

VIDEO_UPDATE( snes )
{
	int y;

	/*NTSC SNES draw range is 1-225. */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		snes_refresh_scanline(screen->machine, bitmap, y+1);
	return 0;
}


/***** Debug Functions *****/

#ifdef MAME_DEBUG

static void snes_dbg_draw_maps( bitmap_t *bitmap, UINT32 tmap, UINT8 bpl, UINT16 curline, UINT8 layer )
{
	UINT32 tile, addr = tmap;
	UINT16 ii, vflip, hflip, pal;
	INT8 line;

	tmap += (curline >> 3) * 64;
	for( ii = 0; ii < 64; ii += 2 )
	{
		vflip = (snes_vram[tmap + ii + 1] & 0x80);
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		pal = (snes_vram[tmap + ii + 1] & 0x1c);		/* 8 palettes of 4 colours */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii];
		line = curline % 8;
		if( vflip )
			line = -line + 7;

		if( tile != 0 )
		{
			switch( bpl )
			{
				case 1:
					snes_draw_tile(MAINSCREEN, 2, layer, snes_ppu.layer[layer].data + (tile << 4) + ((curline % 8) * 2), (ii >> 1) * 8, 255, hflip, pal);
					break;
				case 2:
					pal <<= 2;
					snes_draw_tile(MAINSCREEN, 4, layer, snes_ppu.layer[layer].data + (tile << 5) + ((curline % 8) * 2), (ii >> 1) * 8, 255, hflip, pal);
					break;
				case 4:
					pal <<= 6;	// 2009-08 FP: it might be 8... still to investigate
					snes_draw_tile(MAINSCREEN, 8, layer, snes_ppu.layer[layer].data + (tile << 6) + ((curline % 8) * 2), (ii >> 1) * 8, 255, hflip, pal);
					break;
			}
		}
	}

	logerror("%d : %8X  ", layer, addr );
	//ui_draw_text( str, 0, 227 );
}

static void snes_dbg_draw_all_tiles( running_machine *machine, bitmap_t *bitmap, UINT32 tileaddr, UINT8 bpl, UINT16 pal )
{
	UINT16 ii, jj, kk;
	UINT32 addr = tileaddr;

	for( jj = 0; jj < 32; jj++ )
	{
		addr = tileaddr + (jj * bpl * 16 * 32);
		for( kk = 0; kk < 8; kk++ )
		{
			UINT32 *destline = BITMAP_ADDR32(bitmap, jj * 8 + kk, 0);

			/* Clear buffers */
			memset( scanlines[MAINSCREEN].buffer, 0, SNES_SCR_WIDTH * 2 );
			memset( scanlines[MAINSCREEN].zbuf, 0, SNES_SCR_WIDTH * 2 );
			for( ii = 0; ii < 32; ii++ )
			{
				switch( bpl )
				{
					case 1:
						snes_draw_tile(MAINSCREEN, 2, 0, addr, ii * 8, 255, 0, pal);
						break;
					case 2:
						snes_draw_tile(MAINSCREEN, 4, 0, addr, ii * 8, 255, 0, pal);
						break;
					case 4:
						snes_draw_tile(MAINSCREEN, 8, 0, addr, ii * 8, 255, 0, pal);
						break;
				}
				addr += (bpl * 16);
			}
			for (ii = 0; ii < SNES_SCR_WIDTH * 2; ii++)
			{
				int pixdata = scanlines[MAINSCREEN].buffer[ii];
				if (pixdata != 200)
					destline[ii] = machine->pens[pixdata];
			}
			addr -= (32 * (bpl * 16)) - 2;
		}
	}

	logerror("  %8X  ", tileaddr );
	//ui_draw_text( str, 0, 227 );
}

static UINT8 snes_dbg_video( running_machine *machine, bitmap_t *bitmap, UINT16 curline )
{
	UINT16 ii;

#define SNES_DBG_HORZ_POS 545

	/* Check if the user has enabled or disabled stuff */
	if( curline == 0 )
	{
		//UINT16 y = 1;
		static const char WINLOGIC[4] = { '|', '&', '^', '!' };

		if( !debug_options.input_count-- )
		{
			UINT8 toggles = input_port_read_safe(machine, "DEBUG2", 0);
			if( toggles & 0x1 )
				debug_options.bg_disabled[0] = !debug_options.bg_disabled[0];
			if( toggles & 0x2 )
				debug_options.bg_disabled[1] = !debug_options.bg_disabled[1];
			if( toggles & 0x4 )
				debug_options.bg_disabled[2] = !debug_options.bg_disabled[2];
			if( toggles & 0x8 )
				debug_options.bg_disabled[3] = !debug_options.bg_disabled[3];
			if( toggles & 0x10 )
				debug_options.bg_disabled[4] = !debug_options.bg_disabled[4];
			if( toggles & 0x20 )
				debug_options.draw_subscreen = !debug_options.draw_subscreen;
			if( toggles & 0x40 )
				debug_options.bg_disabled[5] = !debug_options.bg_disabled[5];
			if( toggles & 0x80 )
				debug_options.windows_disabled = !debug_options.windows_disabled;
			toggles = input_port_read_safe(machine, "DEBUG4", 0);
			if( toggles & 0x01 )
				debug_options.mode_disabled[0] = !debug_options.mode_disabled[0];
			if( toggles & 0x02 )
				debug_options.mode_disabled[1] = !debug_options.mode_disabled[1];
			if( toggles & 0x04 )
				debug_options.mode_disabled[2] = !debug_options.mode_disabled[2];
			if( toggles & 0x08 )
				debug_options.mode_disabled[3] = !debug_options.mode_disabled[3];
			if( toggles & 0x10 )
				debug_options.mode_disabled[4] = !debug_options.mode_disabled[4];
			if( toggles & 0x20 )
				debug_options.mode_disabled[5] = !debug_options.mode_disabled[5];
			if( toggles & 0x40 )
				debug_options.mode_disabled[6] = !debug_options.mode_disabled[6];
			if( toggles & 0x80 )
				debug_options.mode_disabled[7] = !debug_options.mode_disabled[7];
			toggles = input_port_read_safe(machine, "DEBUG3", 0);
			if( toggles & 0x4 )
				debug_options.transparency_disabled = !debug_options.transparency_disabled;
			debug_options.input_count = 5;
		}
		/* Display some debug info on the screen */
		logerror("%s%s", debug_options.windows_disabled?" ":"W", debug_options.transparency_disabled?" ":"T" );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
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
				snes_ppu.layer[0].tile_size + 1,
				(snes_ram[MOSAIC] & 0x1)?"m":" ",
				snes_ram[BG1SC] & 0x3,
				(snes_ram[BG1SC] & 0xfc) << 9,
				snes_ppu.layer[0].data );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
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
				snes_ppu.layer[1].tile_size + 1,
				(snes_ram[MOSAIC] & 0x2)?"m":" ",
				snes_ram[BG2SC] & 0x3,
				(snes_ram[BG2SC] & 0xfc) << 9,
				snes_ppu.layer[1].data );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
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
				snes_ppu.layer[2].tile_size + 1,
				(snes_ram[MOSAIC] & 0x4)?"m":" ",
				(snes_ram[BGMODE] & 0x8)?"P":" ",
				snes_ram[BG3SC] & 0x3,
				(snes_ram[BG3SC] & 0xfc) << 9,
				snes_ppu.layer[2].data );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
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
				snes_ppu.layer[3].tile_size + 1,
				(snes_ram[MOSAIC] & 0x8)?"m":" ",
				snes_ram[BG4SC] & 0x3,
				(snes_ram[BG4SC] & 0xfc) << 9,
				snes_ppu.layer[3].data );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("%sO %s%s%s%s%s%c%s%s%d%d       %4X",
				debug_options.bg_disabled[4]?" ":"*",
				(snes_ram[TM] & 0x10)?"M":" ",
				(snes_ram[TS] & 0x10)?"S":" ",
				(snes_ram[CGADSUB] & 0x10)?"B":" ",
				(snes_ram[TMW] & 0x10)?"m":" ",
				(snes_ram[TSW] & 0x10)?"s":" ",
				WINLOGIC[(snes_ram[WOBJLOG] & 0x3)],
				(snes_ram[WOBJSEL] & 0x2)?((snes_ram[WOBJSEL] & 0x1)?"o":"i"):" ",
				(snes_ram[WOBJSEL] & 0x8)?((snes_ram[WOBJSEL] & 0x4)?"o":"i"):" ",
				snes_ppu.oam.size[0], snes_ppu.oam.size[1],
				snes_ppu.layer[4].data );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("%sB   %s  %c%s%s",
				debug_options.bg_disabled[5]?" ":"*",
				(snes_ram[CGADSUB] & 0x20)?"B":" ",
				WINLOGIC[(snes_ram[WOBJLOG] & 0xc)>>2],
				(snes_ram[WOBJSEL] & 0x20)?((snes_ram[WOBJSEL] & 0x10)?"o":"i"):" ",
				(snes_ram[WOBJSEL] & 0x80)?((snes_ram[WOBJSEL] & 0x40)?"o":"i"):" " );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("1) %3d %3d   2) %3d %3d", (snes_ppu.bgd_offset.horizontal[0] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[0] & 0x3ff) >> 3, (snes_ppu.bgd_offset.horizontal[1] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[1] & 0x3ff) >> 3 );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("3) %3d %3d   4) %3d %3d", (snes_ppu.bgd_offset.horizontal[2] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[2] & 0x3ff) >> 3, (snes_ppu.bgd_offset.horizontal[3] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[3] & 0x3ff) >> 3 );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("Flags: %s%s%s %s %2d", (snes_ram[CGWSEL] & 0x2)?"S":"F", (snes_ram[CGADSUB] & 0x80)?"-":"+", (snes_ram[CGADSUB] & 0x40)?" 50%":"100%",(snes_ram[CGWSEL] & 0x1)?"D":"P", (snes_ram[MOSAIC] & 0xf0) >> 4 );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("SetINI: %s %s %s %s %s %s", (snes_ram[SETINI] & 0x1)?" I":"NI", (snes_ram[SETINI] & 0x2)?"P":"R", (snes_ram[SETINI] & 0x4)?"240":"225",(snes_ram[SETINI] & 0x8)?"512":"256",(snes_ram[SETINI] & 0x40)?"E":"N",(snes_ram[SETINI] & 0x80)?"ES":"NS" );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("Mode7: A %5d B %5d", snes_ppu.mode7.matrix_a, snes_ppu.mode7.matrix_b );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror(" %s%s%s   C %5d D %5d", (snes_ram[M7SEL] & 0xc0)?((snes_ram[M7SEL] & 0x40)?"0":"C"):"R", (snes_ram[M7SEL] & 0x1)?"H":" ", (snes_ram[M7SEL] & 0x2)?"V":" ", snes_ppu.mode7.matrix_c, snes_ppu.mode7.matrix_d );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		logerror("       X %5d Y %5d", snes_ppu.mode7.origin_x, snes_ppu.mode7.origin_y );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
	}
	/* Just for testing, draw as many tiles as possible */
	{
		UINT8 adjust = input_port_read(machine, "PAD1H");
		UINT8 dip = input_port_read_safe(machine, "DEBUG1", 0);
		UINT8 inp = input_port_read_safe(machine, "DEBUG3", 0);
		UINT8 dt = 1 << ((dip & 0x3) - 1);
		UINT8 dm = 1 << (((dip & 0xc) >> 2) - 1);
		if( dt )
		{
			static INT16 pal = 0;
			static UINT32 addr = 0;
			if( curline == 0 )
			{
				if( adjust & 0x1 ) addr += (dt * 16);
				if( adjust & 0x2 ) addr -= (dt * 16);
				if( adjust & 0x4 ) addr += (dt * 16 * 32);
				if( adjust & 0x8 ) addr -= (dt * 16 * 32);
				if( inp & 0x1 ) pal -= 1;
				if( inp & 0x2 ) pal += 1;
				if( pal < 0 ) pal = 0;
				if( pal > 8 ) pal = 8;
				for( ii = 0; ii < SNES_SCR_WIDTH; ii++ )
				{
					scanlines[MAINSCREEN].buffer[ii] = 0;
				}
				snes_dbg_draw_all_tiles(machine, bitmap, addr, dt, pal * 16 );
			}
			return 1;
		}
		if( dm )
		{
			static UINT32 tmaddr = 0;
			static INT8 tmbg = 0;
			UINT32 *destline = BITMAP_ADDR32(bitmap, curline, 0);
			if( curline == 0 )
			{
				if( adjust & 0x1 ) tmaddr += 2;
				if( adjust & 0x2 ) tmaddr -= 2;
				if( adjust & 0x4 ) tmaddr += 64;
				if( adjust & 0x8 ) tmaddr -= 64;
				if( inp & 0x1 ) tmbg -= 1;
				if( inp & 0x2 ) tmbg += 1;
				if( tmbg < 0 ) tmbg = 0;
				if( tmbg > 3 ) tmbg = 3;
			}
			/* Clear zbuffer */
			memset( scanlines[MAINSCREEN].zbuf, 0, SNES_SCR_WIDTH );
			/* Draw back colour */
			for( ii = 0; ii < SNES_SCR_WIDTH; ii++ )
				scanlines[MAINSCREEN].buffer[ii] = machine->pens[0];
			snes_dbg_draw_maps(bitmap, tmaddr, dm, curline, tmbg );
			for (ii = 0; ii < SNES_SCR_WIDTH; ii++)
			{
				int pixdata = scanlines[MAINSCREEN].buffer[ii];
				if (pixdata != 200)
					destline[ii] = machine->pens[pixdata];
			}
			return 1;
				}
		}
#if 0	// 2009-08 FP: what was the purpose of these lines?
			/* Draw some useful information about the back/fixed colours and current bg mode etc. */
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 26) = machine->pens[dbg_mode_colours[(snes_ram[CGWSEL] & 0xc0) >> 6]];
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 24) = machine->pens[dbg_mode_colours[(snes_ram[CGWSEL] & 0x30) >> 4]];
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 22) = machine->pens[dbg_mode_colours[snes_ram[BGMODE] & 0x7]];
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 12) = machine->pens[32767];
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 2 ) = machine->pens[32767];
			for( ii = 0; ii < 5; ii++ )
			{
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 19 + ii) = snes_cgram[0];
			*BITMAP_ADDR32(bitmap, curline, SNES_DBG_HORZ_POS - 9  + ii) = snes_cgram[FIXED_COLOUR];
			}
			/* Draw window positions */
			scanlines[MAINSCREEN].buffer[snes_ram[WH0]] = machine->pens[dbg_mode_colours[0]];
			scanlines[MAINSCREEN].buffer[snes_ram[WH1]] = machine->pens[dbg_mode_colours[0]];
			scanlines[MAINSCREEN].buffer[snes_ram[WH2]] = machine->pens[dbg_mode_colours[2]];
			scanlines[MAINSCREEN].buffer[snes_ram[WH3]] = machine->pens[dbg_mode_colours[2]];
#endif
	return 0;
	}

#endif /* MAME_DEBUG */
