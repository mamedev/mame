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

  The screen layers are drawn with the following priorities:
  (highest to lowest)

  Modes 0 and 1                         Modes 2 to 7
  -------------------------------------------------------
  Mainscreens
          (BG3:1 - BG3 priority)        OBJ:3
           OBJ:3                        BG1:1
           BG1:1                        OBJ:2
           BG2:1                        BG2:1
           OBJ:2                        OBJ:1
           BG1:0                        BG1:0
           BG2:0                        OBJ:0
           OBJ:1                        BG2:0
          (BG3:1 - BG3 not priority)    Background Colour
           BG4:1
           OBJ:0
           BG3:0
           BG4:0
           Background Colour
  Subscreens
          (BG3:1 - BG3 priority)        OBJ:3
           OBJ:3                        BG1:1
           BG1:1                        OBJ:2
           BG2:1                        BG2:1
           OBJ:2                        OBJ:1
           BG1:0                        BG1:0
           BG2:0                        OBJ:0
           OBJ:1                        BG2:0
          (BG3:1 - BG3 not priority)
           BG4:1
           OBJ:0
           BG3:0
           BG4:0

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

#ifdef SNES_DBG_video
struct DEBUGOPTS
{
	UINT8 input_count;
	UINT8 bg_disabled[6];
	UINT8 draw_subscreen;
	UINT8 windows_disabled;
	UINT8 transparency_disabled;
};
static struct DEBUGOPTS debug_options  = {5, {0,0,0,0,0,0}, 0, 0, 0};
/*                                    red   green  blue    purple  yellow cyan    grey    white */
static UINT16 dbg_mode_colours[8] = { 0x1f, 0x3e0, 0x7c00, 0x7c1f, 0x3ff, 0x7fe0, 0x4210, 0x7fff };
static UINT8 snes_dbg_video(running_machine *machine, mame_bitmap *bitmap, UINT16 curline);
#endif /* SNES_DBG_video */

/* Forward declarations */
static void snes_update_line_2(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline );
static void snes_update_line_2_hi(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline );
static void snes_update_line_4(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline );
static void snes_update_line_4_hi(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline );
static void snes_update_line_8(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline );
static void snes_update_line_mode7(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline );

/* Lookup tables */
static const UINT8  table_bgd_pty[2][4][2] = { { {7,10}, {6,9}, {1,4}, {0,3} }, { {3,10}, {1,7}, {0,0}, {0,0} } };
static const UINT8  table_obj_pty[4]       = { 2, 5, 8, 11 };
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
struct SNES_MODE_CONFIG
{
	void (*drawLayer[5])(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline);
	UINT8 count;
};

static struct SCANLINE scanlines[2];
struct SNES_PPU_STRUCT snes_ppu;
static struct SNES_MODE_CONFIG snes_modedefs[8] =
{
/*0*/	{ {snes_update_line_2, snes_update_line_2, snes_update_line_2, snes_update_line_2}, 4 },
/*1*/	{ {snes_update_line_4, snes_update_line_4, snes_update_line_2, NULL}, 3 },
/*2*/	{ {snes_update_line_4, snes_update_line_4, NULL, NULL}, 2 },		/* Supports offset per tile */
/*3*/	{ {snes_update_line_8, snes_update_line_4, NULL, NULL}, 2 },		/* Supports direct colour */
/*4*/	{ {snes_update_line_8, snes_update_line_2, NULL, NULL}, 2 },		/* Supports offset per tile and direct colour */
/*5*/	{ {snes_update_line_4_hi, snes_update_line_2_hi, NULL, NULL}, 2 },	/* Supports hires */
/*6*/	{ {snes_update_line_4_hi, NULL, NULL, NULL}, 1 },					/* Supports offset per tile and hires */
/*7*/	{ {snes_update_line_mode7, NULL, NULL, NULL}, 1 }					/* Supports direct colour */
};

/*****************************************
 * snes_draw_blend()
 *
 * Routine for additive/subtractive blending
 * between the main and sub screens.
 *****************************************/
INLINE void snes_draw_blend(running_machine *machine, UINT16 offset, UINT16 *colour, UINT8 mode, UINT8 clip )
{
#ifdef SNES_DBG_video
	if( !debug_options.transparency_disabled )
#endif /* SNES_DBG_video */
	if( (clip == SNES_CLIP_ALL || clip == SNES_CLIP_ALL2) ||
		(clip == SNES_CLIP_IN  && snes_ppu.clipmasks[5][offset]) ||
		(clip == SNES_CLIP_OUT && !snes_ppu.clipmasks[5][offset]) )
	{
		UINT16 r, g, b;
		if( mode == SNES_BLEND_ADD )
		{
			if( snes_ram[CGWSEL] & 0x2 ) /* Subscreen*/
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
				r = (*colour & 0x1f) + (machine->remapped_colortable[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) + ((machine->remapped_colortable[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) + ((machine->remapped_colortable[FIXED_COLOUR] & 0x7c00) >> 10);
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
			if( snes_ram[CGWSEL] & 0x2 ) /* Subscreen */
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
				r = (*colour & 0x1f) - (machine->remapped_colortable[FIXED_COLOUR] & 0x1f);
				g = ((*colour & 0x3e0) >> 5) - ((machine->remapped_colortable[FIXED_COLOUR] & 0x3e0) >> 5);
				b = ((*colour & 0x7c00) >> 10) - ((machine->remapped_colortable[FIXED_COLOUR] & 0x7c00) >> 10);
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
 * snes_draw_tile_2()
 *
 * Draw tiles with 2 bit planes(4 colors)
 *****************************************/
INLINE void snes_draw_tile_2(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal )
{
	UINT8 mask, plane[2];
	UINT16 c;
	INT16 ii;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];

	if( flip )
		mask = 0x1;
	else
		mask = 0x80;

	for( ii = x; ii < (x + 8); ii++ )
	{
		register UINT8 colour;
		if( flip )
		{
			colour = (plane[0] & mask ? 1 : 0) | (plane[1] & mask ? 2 : 0);
			mask <<= 1;
		}
		else
		{
			colour = (plane[0] & mask ? 1 : 0) | (plane[1] & mask ? 2 : 0);
			mask >>= 1;
		}

#ifdef SNES_DBG_video
		if( !debug_options.windows_disabled )
#endif /* SNES_DBG_video */
		/* Clip to windows */
		if( (screen == MAINSCREEN && (snes_ram[TMW] & (0x1 << layer))) || (screen == SUBSCREEN && (snes_ram[TSW] & (0x1 << layer))))
			colour &= snes_ppu.clipmasks[layer][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if( colour )
		{
			if( (scanlines[screen].zbuf[ii] <= priority) && (ii >= 0) )
			{
				c = machine->remapped_colortable[pal + colour];
				if( screen == MAINSCREEN )	/* Only blend main screens */
					snes_draw_blend(machine, ii, &c, snes_ppu.layer[layer].blend, (snes_ram[CGWSEL] & 0x30) >> 4 );
				scanlines[screen].buffer[ii] = c;
				scanlines[screen].zbuf[ii] = priority;
			}
		}
	}
}

/*****************************************
 * snes_draw_tile_2x2()
 *
 * Draw 2 tiles with 2 bit planes(4 colors)
 *****************************************/
INLINE void snes_draw_tile_2x2(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal )
{
	if( flip )
	{
		snes_draw_tile_2(machine, screen, layer, tileaddr + 16, x, priority, flip, pal );
		snes_draw_tile_2(machine, screen, layer, tileaddr, x + 8, priority, flip, pal );
	}
	else
	{
		snes_draw_tile_2(machine, screen, layer, tileaddr, x, priority, flip, pal );
		snes_draw_tile_2(machine, screen, layer, tileaddr + 16, x + 8, priority, flip, pal );
	}
}

/*****************************************
 * snes_draw_tile_4()
 *
 * Draw tiles with 4 bit planes(16 colors)
 *****************************************/
INLINE void snes_draw_tile_4(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal )
{
	UINT8 mask, plane[4];
	UINT16 c;
	INT16 ii;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];
	plane[2] = snes_vram[tileaddr + 16];
	plane[3] = snes_vram[tileaddr + 17];

	if( flip )
		mask = 0x1;
	else
		mask = 0x80;

	for( ii = x; ii < (x + 8); ii++ )
	{
		register UINT8 colour;
		if( flip )
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

#ifdef SNES_DBG_video
		if( !debug_options.windows_disabled )
#endif /* SNES_DBG_video */
		/* Clip to windows */
		if( (screen == MAINSCREEN && (snes_ram[TMW] & (0x1 << layer))) || (screen == SUBSCREEN && (snes_ram[TSW] & (0x1 << layer))))
			colour &= snes_ppu.clipmasks[layer][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if( colour )
		{
			if( (scanlines[screen].zbuf[ii] <= priority) && (ii >= 0) )
			{
				c = machine->remapped_colortable[pal + colour];
				if( screen == MAINSCREEN )	/* Only blend main screens */
					snes_draw_blend(machine, ii, &c, snes_ppu.layer[layer].blend, (snes_ram[CGWSEL] & 0x30) >> 4 );
				scanlines[screen].buffer[ii] = c;
				scanlines[screen].zbuf[ii] = priority;
			}
		}
	}
}

/*****************************************
 * snes_draw_tile_4x2()
 *
 * Draw 2 tiles with 4 bit planes(16 colors)
 *****************************************/
INLINE void snes_draw_tile_4x2(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal )
{
	if( flip )
	{
		snes_draw_tile_4(machine, screen, layer, tileaddr + 32, x, priority, flip, pal );
		snes_draw_tile_4(machine, screen, layer, tileaddr, x + 8, priority, flip, pal );
	}
	else
	{
		snes_draw_tile_4(machine, screen, layer, tileaddr, x, priority, flip, pal );
		snes_draw_tile_4(machine, screen, layer, tileaddr + 32, x + 8, priority, flip, pal );
	}
}

/*****************************************
 * snes_draw_tile_8()
 *
 * Draw tiles with 8 bit planes(256 colors)
 *****************************************/
INLINE void snes_draw_tile_8(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip )
{
	UINT8 mask, plane[8];
	UINT16 c;
	INT16 ii;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];
	plane[2] = snes_vram[tileaddr + 16];
	plane[3] = snes_vram[tileaddr + 17];
	plane[4] = snes_vram[tileaddr + 32];
	plane[5] = snes_vram[tileaddr + 33];
	plane[6] = snes_vram[tileaddr + 48];
	plane[7] = snes_vram[tileaddr + 49];

	if( flip )
		mask = 0x1;
	else
		mask = 0x80;

	for( ii = x; ii < (x + 8); ii++ )
	{
		register UINT8 colour;
		if( flip )
		{
			colour = (plane[0] & mask ? 1 : 0)  | (plane[1] & mask ? 2 : 0)  |
					 (plane[2] & mask ? 4 : 0)  | (plane[3] & mask ? 8 : 0)  |
					 (plane[4] & mask ? 16 : 0) | (plane[5] & mask ? 32 : 0) |
					 (plane[6] & mask ? 64 : 0) | (plane[7] & mask ? 128 : 0);
			mask <<= 1;
		}
		else
		{
			colour = (plane[0] & mask ? 1 : 0)  | (plane[1] & mask ? 2 : 0)  |
					 (plane[2] & mask ? 4 : 0)  | (plane[3] & mask ? 8 : 0)  |
					 (plane[4] & mask ? 16 : 0) | (plane[5] & mask ? 32 : 0) |
					 (plane[6] & mask ? 64 : 0) | (plane[7] & mask ? 128 : 0);
			mask >>= 1;
		}

#ifdef SNES_DBG_video
		if( !debug_options.windows_disabled )
#endif /* SNES_DBG_video */
		/* Clip to windows */
		if( (screen == MAINSCREEN && (snes_ram[TMW] & (0x1 << layer))) || (screen == SUBSCREEN && (snes_ram[TSW] & (0x1 << layer))))
			colour &= snes_ppu.clipmasks[layer][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if( colour )
		{
			if( (scanlines[screen].zbuf[ii] <= priority) && (ii >= 0) )
			{
				c = machine->remapped_colortable[colour];
				if( screen == MAINSCREEN )	/* Only blend main screens */
					snes_draw_blend(machine, ii, &c, snes_ppu.layer[layer].blend, (snes_ram[CGWSEL] & 0x30) >> 4 );
				scanlines[screen].buffer[ii] = c;
				scanlines[screen].zbuf[ii] = priority;
			}
		}
	}
}

/*****************************************
 * snes_draw_tile_8x2()
 *
 * Draw 2 tiles with 8 bit planes(256 colors)
 *****************************************/
INLINE void snes_draw_tile_8x2(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip )
{
	if( flip )
	{
		snes_draw_tile_8(machine, screen, layer, tileaddr + 64, x, priority, flip );
		snes_draw_tile_8(machine, screen, layer, tileaddr, x + 8, priority, flip );
	}
	else
	{
		snes_draw_tile_8(machine, screen, layer, tileaddr, x, priority, flip );
		snes_draw_tile_8(machine, screen, layer, tileaddr + 64, x + 8, priority, flip );
	}
}

/*****************************************
 * snes_draw_tile_object()
 *
 * Draw tiles with 4 bit planes(16 colors)
 * The same as snes_draw_tile_4() except
 * that it takes a blend parameter.
 *****************************************/
INLINE void snes_draw_tile_object(running_machine *machine, UINT8 screen, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal, UINT8 blend )
{
	UINT8 mask, plane[4];
	UINT16 c;
	INT16 ii;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];
	plane[2] = snes_vram[tileaddr + 16];
	plane[3] = snes_vram[tileaddr + 17];

	if( flip )
		mask = 0x1;
	else
		mask = 0x80;

	for( ii = x; ii < (x + 8); ii++ )
	{
		register UINT8 colour;
		if( flip )
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

#ifdef SNES_DBG_video
		if( !debug_options.windows_disabled )
#endif /* SNES_DBG_video */
		/* Clip to windows */
		if( (screen == MAINSCREEN && (snes_ram[TMW] & 0x10)) || (screen == SUBSCREEN && (snes_ram[TSW] & 0x10)))
			colour &= snes_ppu.clipmasks[4][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if( colour )
		{
			if( ii >= 0 )
			{
				c = machine->remapped_colortable[pal + colour];
				if( blend && screen == MAINSCREEN )	/* Only blend main screens */
					snes_draw_blend(machine, ii, &c, snes_ppu.layer[4].blend, (snes_ram[CGWSEL] & 0x30) >> 4 );

				scanlines[screen].buffer[ii] = c;
				scanlines[screen].zbuf[ii] = priority;
			}
		}
	}
}

/*****************************************
 * snes_draw_tile_object_w()
 *
 * Draw tiles with 4 bit planes(16 colors)
 * The same as snes_draw_tile_4() except
 * that it takes a blend parameter.
 * Wide version.
 *****************************************/
INLINE void snes_draw_tile_object_w(running_machine *machine, UINT8 screen, UINT16 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT16 pal, UINT8 blend )
{
	UINT8 mask, plane[4];
	UINT16 c;
	INT16 ii;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];
	plane[2] = snes_vram[tileaddr + 16];
	plane[3] = snes_vram[tileaddr + 17];

	if( flip )
		mask = 0x1;
	else
		mask = 0x80;

	x <<= 1;
	for( ii = x; ii < (x + 16); ii += 2 )
	{
		register UINT8 colour;
		if( flip )
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

#ifdef SNES_DBG_video
		if( !debug_options.windows_disabled )
#endif /* SNES_DBG_video */
		/* Clip to windows */
		if( (screen == MAINSCREEN && (snes_ram[TMW] & 0x10)) || (screen == SUBSCREEN && (snes_ram[TSW] & 0x10)))
			colour &= snes_ppu.clipmasks[4][ii];

		/* Only draw if we have a colour (0 == transparent) */
		if( colour )
		{
			if( ii >= 0 )
			{
				c = machine->remapped_colortable[pal + colour];
				if( blend && screen == MAINSCREEN )	/* Only blend main screens */
					snes_draw_blend(machine, ii, &c, snes_ppu.layer[4].blend, (snes_ram[CGWSEL] & 0x30) >> 4 );

				scanlines[screen].buffer[ii] = c;
				scanlines[screen].zbuf[ii] = priority;
				scanlines[screen].buffer[ii + 1] = c;
				scanlines[screen].zbuf[ii + 1] = priority;
			}
		}
	}
}

/*********************************************
 * snes_update_line_2()
 *
 * Update an entire line of 2 bit plane tiles.
 *********************************************/
static void snes_update_line_2(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline )
{
	UINT32 tmap, tile;
	UINT16 ii, vflip, hflip, pal;
	INT8 line, tile_line;
	UINT8 priority;
	/* scrolling */
	UINT32 basevmap;
	UINT16 vscroll, hscroll, vtilescroll;
	UINT8 vshift, hshift, tile_size;
	UINT8 bg3_pty = 0;

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[layer] )
		return;
#endif /* SNES_DBG_video */

	/* set special priority bit */
	if( snes_ppu.mode == 1 && snes_ram[BGMODE] & 0x8 )
		bg3_pty = 1;

	/* Handle Mosaic effects */
	if( snes_ram[MOSAIC] & (1 << layer) )
		curline -= (curline % ((snes_ram[MOSAIC] >> 4) + 1));

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
	if( line > ((8 << tile_size) - 1) )	/* scrolled into the next tile */
	{
		vtilescroll++;	/* pretend we scrolled by 1 tile line */
		line -= (8 << tile_size);
	}
	if( vtilescroll >= 128 )
		vtilescroll -= 128;

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].map;
	/* Offset vertically */
	tmap += table_vscroll[snes_ppu.layer[layer].map_size][vtilescroll >> 5];
	/* Scroll vertically */
	tmap += (vtilescroll & 0x1f) << 6;
	/* Remember this position */
	basevmap = tmap;
	/* Offset horizontally */
	tmap += table_hscroll[snes_ppu.layer[layer].map_size][hscroll >> 5];
	/* Scroll horizontally */
	tmap += (hscroll & 0x1f) << 1;

	for( ii = 0; ii < (66 >> tile_size); ii += 2 )
	{
		/* Have we scrolled into the next map? */
		if( hscroll && ((ii >> 1) >= 32 - (hscroll & 0x1f)) )
		{
			tmap = basevmap + table_hscroll[snes_ppu.layer[layer].map_size][(hscroll >> 5) + 1];
			tmap -= ii;
			hscroll = 0;	/* Make sure we don't do this again */
		}
		if (tmap > 0x10000) tmap %= 0x10000;
		vflip = (snes_vram[tmap + ii + 1] & 0x80);
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		priority = table_bgd_pty[snes_ppu.mode > 1][layer][(snes_vram[tmap + ii + 1] & 0x20) >> 5];
		pal = (snes_vram[tmap + ii + 1] & 0x1c);		/* 8 palettes of 4 colours */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii];

		/* Mode 0 palettes are layer specific */
		if( snes_ppu.mode == 0 )
		{
			pal += (layer << 5);
		}

		tile_line = line;
		if( vflip )
		{
			if( tile_size )
			{
				if( line > 7 )
				{
					tile_line -= 8;
				}
				else
				{
					tile += 32;
				}
			}
			tile_line = -tile_line + 7;
		}
		else
		{
			if( line > 7 )
			{
				tile += 32;
				tile_line -= 8;
			}
		}
		tile_line <<= 1;

		/* Special case for bg3 */
		if( layer == 2 && bg3_pty && (snes_vram[tmap + ii + 1] & 0x20) )
			priority = table_obj_pty[3] + 1;		/* We want to have the highest priority here */

		if( tile_size )
		{
			snes_draw_tile_2x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 4) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip, pal );
		}
		else
		{
			snes_draw_tile_2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 4) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip, pal );
		}
	}
}

/*********************************************
 * snes_update_line_2_hi()
 *
 * Update an entire line of 2 bit plane tiles.
 * This is the hires version.
 *********************************************/
static void snes_update_line_2_hi(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline )
{
	UINT32 tmap, tile;
	UINT16 ii, vflip, hflip, pal;
	INT8 line, tile_line;
	UINT8 priority;
	/* scrolling */
	UINT32 basevmap;
	UINT16 vscroll, hscroll, vtilescroll;
	UINT8 vshift, hshift, tile_size;
	UINT8 bg3_pty = 0;

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[layer] )
		return;
#endif /* SNES_DBG_video */

	/* set special priority bit */
	if( snes_ppu.mode == 1 && snes_ram[BGMODE] & 0x8 )
		bg3_pty = 1;

	/* Handle Mosaic effects */
	if( snes_ram[MOSAIC] & (1 << layer) )
		curline -= (curline % ((snes_ram[MOSAIC] >> 4) + 1));

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
	if( line > ((8 << tile_size) - 1) )	/* scrolled into the next tile */
	{
		vtilescroll++;	/* pretend we scrolled by 1 tile line */
		line -= (8 << tile_size);
	}
	if( vtilescroll >= 128 )
		vtilescroll -= 128;

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].map;
	/* Offset vertically */
	tmap += table_vscroll[snes_ppu.layer[layer].map_size][vtilescroll >> 5];
	/* Scroll vertically */
	tmap += (vtilescroll & 0x1f) << 6;
	/* Remember this position */
	basevmap = tmap;
	/* Offset horizontally */
	tmap += table_hscroll[snes_ppu.layer[layer].map_size][hscroll >> 5];
	/* Scroll horizontally */
	tmap += (hscroll & 0x1f) << 1;

	for( ii = 0; ii < (66 >> tile_size); ii += 2 )
	{
		/* Have we scrolled into the next map? */
		if( hscroll && ((ii >> 1) >= 32 - (hscroll & 0x1f)) )
		{
			tmap = basevmap + table_hscroll[snes_ppu.layer[layer].map_size][(hscroll >> 5) + 1];
			tmap -= ii;
			hscroll = 0;	/* Make sure we don't do this again */
		}
		if (tmap > 0x10000) tmap %= 0x10000;
		vflip = (snes_vram[tmap + ii + 1] & 0x80);
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		priority = table_bgd_pty[snes_ppu.mode > 1][layer][(snes_vram[tmap + ii + 1] & 0x20) >> 5];
		pal = (snes_vram[tmap + ii + 1] & 0x1c);		/* 8 palettes of 4 colours */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii];

		/* Mode 0 palettes are layer specific */
		if( snes_ppu.mode == 0 )
		{
			pal += (layer << 5);
		}

		tile_line = line;
		if( vflip )
		{
			if( tile_size )
			{
				if( line > 7 )
				{
					tile_line -= 8;
				}
				else
				{
					tile += 32;
				}
			}
			tile_line = -tile_line + 7;
		}
		else
		{
			if( line > 7 )
			{
				tile += 32;
				tile_line -= 8;
			}
		}
		tile_line <<= 1;

		/* Special case for bg3 */
		if( layer == 2 && bg3_pty && (snes_vram[tmap + ii + 1] & 0x20) )
			priority = table_obj_pty[3] + 1;		/* We want to have the highest priority here */

		if( tile_size )
		{
			if( hflip )
			{
				snes_draw_tile_2x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 4) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1) + 16, priority, hflip, pal );
				snes_draw_tile_2x2(machine, screen, layer, snes_ppu.layer[layer].data + ((tile + 2) << 4) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal );
			}
			else
			{
				snes_draw_tile_2x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 4) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal );
				snes_draw_tile_2x2(machine, screen, layer, snes_ppu.layer[layer].data + ((tile + 2) << 4) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1) + 16, priority, hflip, pal );
			}
		}
		else
		{
			snes_draw_tile_2x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 4) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal );
		}
	}
}

/*********************************************
 * snes_update_line_4()
 *
 * Update an entire line of 4 bit plane tiles.
 *********************************************/
static void snes_update_line_4(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline )
{
	UINT32 tmap, tile;
	UINT16 ii, vflip, hflip, pal;
	INT8 line, tile_line;
	UINT8 priority;
	/* scrolling */
	UINT32 basevmap;
	UINT16 vscroll, hscroll, vtilescroll;
	UINT8 vshift, hshift, tile_size;

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[layer] )
		return;
#endif /* SNES_DBG_video */

	/* Handle Mosaic effects */
	if( snes_ram[MOSAIC] & (1 << layer) )
		curline -= (curline % ((snes_ram[MOSAIC] >> 4) + 1));

	/* Find the size of the tiles (8x8 or 16x16) */
	tile_size = snes_ppu.layer[layer].tile_size;
	/* Find scroll info */
	vscroll = snes_ppu.layer[layer].offset.tile_vert;
	vshift = snes_ppu.layer[layer].offset.shift_vert;
	hscroll = snes_ppu.layer[layer].offset.tile_horz;
	hshift = snes_ppu.layer[layer].offset.shift_horz;

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].map;

	/* Find vertical scroll amount */
	vtilescroll = vscroll + (curline >> (3 + tile_size));
	/* figure out which line to draw */
	line = (curline % (8 << tile_size)) + vshift;
	if( line > ((8 << tile_size) - 1) )	/* scrolled into the next tile */
	{
		vtilescroll++;	/* pretend we scrolled by 1 tile line */
		line -= (8 << tile_size);
	}
	if( vtilescroll >= 128 )
		vtilescroll -= 128;

	/* Offset vertically */
	tmap += table_vscroll[snes_ppu.layer[layer].map_size][vtilescroll >> 5];
	/* Scroll vertically */
	tmap += (vtilescroll & 0x1f) << 6;
	/* Remember this position */
	basevmap = tmap;
	/* Offset horizontally */
	tmap += table_hscroll[snes_ppu.layer[layer].map_size][hscroll >> 5];
	/* Scroll horizontally */
	tmap += (hscroll & 0x1f) << 1;

	for( ii = 0; ii < (66 >> tile_size); ii += 2 )
	{
		/* Have we scrolled into the next map? */
		if( hscroll && ((ii >> 1) >= 32 - (hscroll & 0x1f)) )
		{
			tmap = basevmap + table_hscroll[snes_ppu.layer[layer].map_size][(hscroll >> 5) + 1];
			tmap -= ii;
			hscroll = 0;	/* Make sure we don't do this again */
		}
		if (tmap > 0x10000) tmap %= 0x10000;
		vflip = snes_vram[tmap + ii + 1] & 0x80;
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		priority = table_bgd_pty[snes_ppu.mode > 1][layer][(snes_vram[tmap + ii + 1] & 0x20) >> 5];		/* is this even right??? */
		pal = (snes_vram[tmap + ii + 1] & 0x1c) << 2;	/* 8 palettes of 16 colours */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii];

		tile_line = line;
		if( vflip )
		{
			if( tile_size )
			{
				if( line > 7 )
				{
					tile_line -= 8;
				}
				else
				{
					tile += 16;
				}
			}
			tile_line = -tile_line + 7;
		}
		else
		{
			if( line > 7 )
			{
				tile += 16;
				tile_line -= 8;
			}
		}
		tile_line <<= 1;

		if( tile_size )
		{
			snes_draw_tile_4x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 5) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip, pal );
		}
		else
		{
			snes_draw_tile_4(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 5) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip, pal );
		}
	}
}

/*********************************************
 * snes_update_line_4_hi()
 *
 * Update an entire line of 4 bit plane tiles.
 * This is the hires version
 *********************************************/
static void snes_update_line_4_hi(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline )
{
	UINT32 tmap, tile;
	UINT16 ii, vflip, hflip, pal;
	INT8 line, tile_line;
	UINT8 priority;
	/* scrolling */
	UINT32 basevmap;
	UINT16 vscroll, hscroll, vtilescroll;
	UINT8 vshift, hshift, tile_size;

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[layer] )
		return;
#endif /* SNES_DBG_video */

	/* Handle Mosaic effects */
	if( snes_ram[MOSAIC] & (1 << layer) )
		curline -= (curline % ((snes_ram[MOSAIC] >> 4) + 1));

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
	if( line > ((8 << tile_size) - 1) )	/* scrolled into the next tile */
	{
		vtilescroll++;	/* pretend we scrolled by 1 tile line */
		line -= (8 << tile_size);
	}
	if( vtilescroll >= 128 )
		vtilescroll -= 128;

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].map;
	/* Offset vertically */
	tmap += table_vscroll[snes_ppu.layer[layer].map_size][vtilescroll >> 5];
	/* Scroll vertically */
	tmap += (vtilescroll & 0x1f) << 6;
	/* Remember this position */
	basevmap = tmap;
	/* Offset horizontally */
	tmap += table_hscroll[snes_ppu.layer[layer].map_size][hscroll >> 5];
	/* Scroll horizontally */
	tmap += (hscroll & 0x1f) << 1;

	for( ii = 0; ii < (66 >> tile_size); ii += 2 )
	{
		/* Have we scrolled into the next map? */
		if( hscroll && ((ii >> 1) >= 32 - (hscroll & 0x1f)) )
		{
			tmap = basevmap + table_hscroll[snes_ppu.layer[layer].map_size][(hscroll >> 5) + 1];
			tmap -= ii;
			hscroll = 0;	/* Make sure we don't do this again */
		}
		if (tmap > 0x10000) tmap %= 0x10000;

		vflip = snes_vram[tmap + ii + 1] & 0x80;
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		priority = table_bgd_pty[snes_ppu.mode > 1][layer][(snes_vram[tmap + ii + 1] & 0x20) >> 5];		/* is this even right??? */
		pal = (snes_vram[tmap + ii + 1] & 0x1c) << 2;	/* 8 palettes of 16 colours */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii];

		tile_line = line;
		if( vflip )
		{
			if( tile_size )
			{
				if( line > 7 )
				{
					tile_line -= 8;
				}
				else
				{
					tile += 16;
				}
			}
			tile_line = -tile_line + 7;
		}
		else
		{
			if( line > 7 )
			{
				tile += 16;
				tile_line -= 8;
			}
		}
		tile_line <<= 1;

		/* Does hi-res support the tile-size option? */
		if( tile_size )
		{
			if( hflip )
			{
				snes_draw_tile_4x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 5) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1) + 16, priority, hflip, pal );
				snes_draw_tile_4x2(machine, screen, layer, snes_ppu.layer[layer].data + ((tile + 2) << 5) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal );
			}
			else
			{
				snes_draw_tile_4x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 5) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal );
				snes_draw_tile_4x2(machine, screen, layer, snes_ppu.layer[layer].data + ((tile + 2) << 5) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1) + 16, priority, hflip, pal );
			}
		}
		else
		{
			snes_draw_tile_4x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 5) + tile_line, ((ii >> 1) * (8 << (tile_size + 1))) - (hshift << 1), priority, hflip, pal );
		}
	}
}

/*********************************************
 * snes_update_line_8()
 *
 * Update an entire line of 8 bit plane tiles.
 *********************************************/
static void snes_update_line_8(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline )
{
	UINT32 tmap, tile;
	UINT16 ii, vflip, hflip, pal;
	INT8 line, tile_line;
	UINT8 priority;
	/* scrolling */
	UINT32 basevmap;
	UINT16 vscroll, hscroll, vtilescroll;
	UINT8 vshift, hshift, tile_size;

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[layer] )
		return;
#endif /* SNES_DBG_video */

	/* Handle Mosaic effects */
	if( snes_ram[MOSAIC] & (1 << layer) )
		curline -= (curline % ((snes_ram[MOSAIC] >> 4) + 1));

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
	if( line > ((8 << tile_size) - 1) )	/* scrolled into the next tile */
	{
		vtilescroll++;	/* pretend we scrolled by 1 tile line */
		line -= (8 << tile_size);
	}
	if( vtilescroll >= 128 )
		vtilescroll -= 128;

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].map;
	/* Offset vertically */
	tmap += table_vscroll[snes_ppu.layer[layer].map_size][vtilescroll >> 5];
	/* Scroll vertically */
	tmap += (vtilescroll & 0x1f) << 6;
	/* Remember this position */
	basevmap = tmap;
	/* Offset horizontally */
	tmap += table_hscroll[snes_ppu.layer[layer].map_size][hscroll >> 5];
	/* Scroll horizontally */
	tmap += (hscroll & 0x1f) << 1;

	for( ii = 0; ii < (66 >> tile_size); ii += 2 )
	{
		/* Have we scrolled into the next map? */
		if( hscroll && ((ii >> 1) >= 32 - (hscroll & 0x1f)) )
		{
			tmap = basevmap + table_hscroll[snes_ppu.layer[layer].map_size][(hscroll >> 5) + 1];
			tmap -= ii;
			hscroll = 0;	/* Make sure we don't do this again */
		}
		if (tmap > 0x10000) tmap %= 0x10000;
		vflip = (snes_vram[tmap + ii + 1] & 0x80);
		hflip = snes_vram[tmap + ii + 1] & 0x40;
		priority = table_bgd_pty[snes_ppu.mode > 1][layer][(snes_vram[tmap + ii + 1] & 0x20) >> 5];
		pal = (snes_vram[tmap + ii + 1] & 0x1c);		/* what does this do for 8 bit screen? */
		tile = (snes_vram[tmap + ii + 1] & 0x3) << 8;
		tile |= snes_vram[tmap + ii];

		tile_line = line;
		if( vflip )
		{
			if( tile_size )
			{
				if( line > 7 )
				{
					tile_line -= 8;
				}
				else
				{
					tile += 8;
				}
			}
			tile_line = -tile_line + 7;
		}
		else
		{
			if( line > 7 )
			{
				tile += 8;
				tile_line -= 8;
			}
		}
		tile_line <<= 1;

		if( tile_size )
		{
			snes_draw_tile_8x2(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 6) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip );
		}
		else
		{
			snes_draw_tile_8(machine, screen, layer, snes_ppu.layer[layer].data + (tile << 6) + tile_line, ((ii >> 1) * (8 << tile_size)) - hshift, priority, hflip );
		}
	}
}

/*********************************************
 * snes_update_line_mode7()
 *
 * Update an entire line of mode7 tiles.
 *********************************************/
static void snes_update_line_mode7(running_machine *machine, UINT8 screen, UINT8 layer, UINT16 curline )
{
	UINT32 tiled;
	INT16 ma, mb, mc, md;
	INT16 xc, yc, tx, ty, sx, sy, hs, vs, xpos, xdir;
	UINT8 priority = 0;
	register UINT8 colour = 0;

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[0] )
		return;
#endif /* SNES_DBG_video */

	ma = snes_ppu.mode7.matrix_a;
	mb = snes_ppu.mode7.matrix_b;
	mc = snes_ppu.mode7.matrix_c;
	md = snes_ppu.mode7.matrix_d;
	xc = snes_ppu.mode7.origin_x;
	yc = snes_ppu.mode7.origin_y;
	hs = snes_ppu.layer[0].offset.horizontal;
	vs = snes_ppu.layer[0].offset.vertical;

	/* Sign extend */
	xc <<= 3;
	xc >>= 3;
	yc <<= 3;
	yc >>= 3;
	hs <<= 3;
	hs >>= 3;
	vs <<= 3;
	vs >>= 3;

	/* Vertical flip */
	if( snes_ram[M7SEL] & 0x2 )
		sy = 255 - curline;
	else
		sy = curline;

	/* Horizontal flip */
	if( snes_ram[M7SEL] & 0x1 )
	{
		xpos = 255;
		xdir = -1;
	}
	else
	{
		xpos = 0;
		xdir = 1;
	}

	/* Let's do some mode7 drawing huh? */
	for( sx = 0; sx < 256; sx++, xpos += xdir )
	{
		tx = (((ma * ((sx + hs) - xc)) + (mb * ((sy + vs) - yc))) >> 8) + xc;
		ty = (((mc * ((sx + hs) - xc)) + (md * ((sy + vs) - yc))) >> 8) + yc;
		switch( snes_ram[M7SEL] & 0xc0 )
		{
			case 0x00:	/* Repeat if outside screen area */
				tx &= 0x3ff;
				ty &= 0x3ff;
				tiled = snes_vram[((tx >> 3) * 2) + ((ty >> 3) * 128 * 2)] << 7;
				colour = snes_vram[tiled + ((tx & 0x7) * 2) + ((ty & 0x7) * 16) + 1];
				break;
			case 0x80:	/* Single colour backdrop screen if outside screen area */
				if( (tx & 0x7fff) < 1024 && (ty & 0x7fff) < 1024 )
				{
					tiled = snes_vram[((tx >> 3) * 2) + ((ty >> 3) * 128 * 2)] << 7;
					colour = snes_vram[tiled + ((tx & 0x7) * 2) + ((ty & 0x7) * 16) + 1];
				}
				else
				{
					colour = 0;
				}
				break;
			case 0xC0:	/* Character 0x00 repeat if outside screen area */
				if( (tx & 0x7fff) < 1024 && (ty & 0x7fff) < 1024 )
				{
					tiled = snes_vram[(((tx & 0x3ff) >> 3) * 2) + (((ty & 0x3ff) >> 3) * 128 * 2)] << 7;
					colour = snes_vram[tiled + ((tx & 0x7) * 2) + ((ty & 0x7) * 16) + 1];
				}
				else
				{
					colour = snes_vram[((sx & 0x7) * 2) + ((sy & 0x7) * 16) + 1];
				}
				break;
		}

		/* The last bit is for priority in EXTBG mode */
		if( snes_ram[SETINI] & 0x40 )
		{
			priority = (colour & 0x80) >> 7;
			colour &= 0x7f;
		}

		colour &= snes_ppu.clipmasks[0][xpos];

		/* Draw pixel if appropriate */
		if( scanlines[screen].zbuf[xpos] < table_bgd_pty[1][0][priority] && colour > 0 )
		{
			UINT16 clr;
			/* Direct select */
			if( snes_ram[CGWSEL] & 0x1 )
				clr = ((colour & 0x7) << 2) | ((colour & 0x38) << 4) | ((colour & 0xc0) << 7);
			else
				clr = machine->remapped_colortable[colour];
			/* Only blend main screens */
			if( screen == MAINSCREEN )
				snes_draw_blend(machine, xpos, &clr, snes_ppu.layer[0].blend, (snes_ram[CGWSEL] & 0x30) >> 4 );		/* FIXME: Need to support clip mode */

			scanlines[screen].buffer[xpos] = clr;
			scanlines[screen].zbuf[xpos] = table_bgd_pty[1][0][priority];
		}
	}
}

/*********************************************
 * snes_update_objects()
 *
 * Update an entire line of sprites.
 * FIXME: We need to support high priority bit
 *********************************************/
static void snes_update_objects(running_machine *machine, UINT8 screen, UINT16 curline )
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

#ifdef SNES_DBG_video
	if( debug_options.bg_disabled[4] )
		return;
#endif /* SNES_DBG_video */

	if( snes_ppu.mode == 5 || snes_ppu.mode == 6 )
		widemode = 1;

	oam = 0x1ff;
	oam_extra = oam + 0x20;
	extra = 0;
	for( i = 128; i > 0; i-- )
	{
		if( (i % 4) == 0 )
			extra = oamram[oam_extra--];

		vflip = (oamram[oam] & 0x80) >> 7;
		hflip = (oamram[oam] & 0x40) >> 6;
		priority = table_obj_pty[(oamram[oam] & 0x30) >> 4];
		pal = 128 + ((oamram[oam] & 0xE) << 3);
		tile = (oamram[oam--] & 0x1) << 8;
		tile |= oamram[oam--];
		y = oamram[oam--] + 1;	/* We seem to need to add one here.... */
		x = oamram[oam--];
		size = (extra & 0x80) >> 7;
		extra <<= 1;
		x |= ((extra & 0x80) << 1);
		extra <<= 1;

		/* Adjust if past maximum position */
		if( y >= snes_ppu.beam.last_visible_line )
			y -= 256;
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
							snes_draw_tile_object_w(machine, screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (count++ << 3), priority, hflip, pal, blend );
						else
							snes_draw_tile_object(machine, screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (count++ << 3), priority, hflip, pal, blend );
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
							snes_draw_tile_object_w(machine, screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (xs << 3), priority, hflip, pal, blend );
						else
							snes_draw_tile_object(machine, screen, snes_ppu.layer[4].data + name_sel + tile + table_obj_offset[ys][xs] + line, x + (xs << 3), priority, hflip, pal, blend );
					}
					time_over++;	/* Increase time_over. Should we stop drawing if exceeded 34 tiles? */
				}
			}

			/* Increase range_over.
             * Stop drawing if exceeded 32 objects and
             * enforcing that limit is enabled */
			range_over++;
			if( range_over == 32 && (readinputport( 16 ) & 0x10) )
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
	UINT16 ii;
	INT8 w1, w2;

	snes_ppu.update_windows = 0;		/* reset the flag */

	for( ii = 0; ii < SNES_SCR_WIDTH; ii++ )
	{
		/* update bg 1 */
		snes_ppu.clipmasks[0][ii] = 0xff;
		w1 = w2 = -1;
		if( snes_ram[W12SEL] & 0x2 )
		{
			if( (ii < snes_ram[WH0]) || (ii > snes_ram[WH1]) )
				w1 = 0;
			else
				w1 = 1;
			if( snes_ram[W12SEL] & 0x1 )
				w1 = !w1;
		}
		if( snes_ram[W12SEL] & 0x8 )
		{
			if( (ii < snes_ram[WH2]) || (ii > snes_ram[WH3]) )
				w2 = 0;
			else
				w2 = 1;
			if( snes_ram[W12SEL] & 0x4 )
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch( snes_ram[WBGLOG] & 0x3 )
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[0][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x1:	/* AND */
					snes_ppu.clipmasks[0][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x2:	/* XOR */
					snes_ppu.clipmasks[0][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0x3:	/* XNOR */
					snes_ppu.clipmasks[0][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[0][ii] = w1 ? 0x00 : 0xff;
		else if( w2 >= 0 )
			snes_ppu.clipmasks[0][ii] = w2 ? 0x00 : 0xff;

		/* update bg 2 */
		snes_ppu.clipmasks[1][ii] = 0xff;
		w1 = w2 = -1;
		if( snes_ram[W12SEL] & 0x20 )
		{
			if( (ii < snes_ram[WH0]) || (ii > snes_ram[WH1]) )
				w1 = 0;
			else
				w1 = 1;
			if( snes_ram[W12SEL] & 0x10 )
				w1 = !w1;
		}
		if( snes_ram[W12SEL] & 0x80 )
		{
			if( (ii < snes_ram[WH2]) || (ii > snes_ram[WH3]) )
				w2 = 0;
			else
				w2 = 1;
			if( snes_ram[W12SEL] & 0x40 )
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch( snes_ram[WBGLOG] & 0xc )
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[1][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x4:	/* AND */
					snes_ppu.clipmasks[1][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x8:	/* XOR */
					snes_ppu.clipmasks[1][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0xc:	/* XNOR */
					snes_ppu.clipmasks[1][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[1][ii] = w1 ? 0x00 : 0xff;
		else if( w2 >= 0 )
			snes_ppu.clipmasks[1][ii] = w2 ? 0x00 : 0xff;

		/* update bg 3 */
		snes_ppu.clipmasks[2][ii] = 0xff;
		w1 = w2 = -1;
		if( snes_ram[W34SEL] & 0x2 )
		{
			if( (ii < snes_ram[WH0]) || (ii > snes_ram[WH1]) )
				w1 = 0;
			else
				w1 = 1;
			if( snes_ram[W34SEL] & 0x1 )
				w1 = !w1;
		}
		if( snes_ram[W34SEL] & 0x8 )
		{
			if( (ii < snes_ram[WH2]) || (ii > snes_ram[WH3]) )
				w2 = 0;
			else
				w2 = 1;
			if( snes_ram[W34SEL] & 0x4 )
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch( snes_ram[WBGLOG] & 0x30 )
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[2][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x10:	/* AND */
					snes_ppu.clipmasks[2][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x20:	/* XOR */
					snes_ppu.clipmasks[2][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0x30:	/* XNOR */
					snes_ppu.clipmasks[2][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[2][ii] = w1 ? 0x00 : 0xff;
		else if( w2 >= 0 )
			snes_ppu.clipmasks[2][ii] = w2 ? 0x00 : 0xff;

		/* update bg 4 */
		snes_ppu.clipmasks[3][ii] = 0xff;
		w1 = w2 = -1;
		if( snes_ram[W34SEL] & 0x20 )
		{
			if( (ii < snes_ram[WH0]) || (ii > snes_ram[WH1]) )
				w1 = 0;
			else
				w1 = 1;
			if( snes_ram[W34SEL] & 0x10 )
				w1 = !w1;
		}
		if( snes_ram[W34SEL] & 0x80 )
		{
			if( (ii < snes_ram[WH2]) || (ii > snes_ram[WH3]) )
				w2 = 0;
			else
				w2 = 1;
			if( snes_ram[W34SEL] & 0x40 )
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch( snes_ram[WBGLOG] & 0xc0 )
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[3][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x40:	/* AND */
					snes_ppu.clipmasks[3][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x80:	/* XOR */
					snes_ppu.clipmasks[3][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0xc0:	/* XNOR */
					snes_ppu.clipmasks[3][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[3][ii] = w1 ? 0x00 : 0xff;
		else if( w2 >= 0 )
			snes_ppu.clipmasks[3][ii] = w2 ? 0x00 : 0xff;

		/* update objects */
		snes_ppu.clipmasks[4][ii] = 0xff;
		w1 = w2 = -1;
		if( snes_ram[WOBJSEL] & 0x2 )
		{
			if( (ii < snes_ram[WH0]) || (ii > snes_ram[WH1]) )
				w1 = 0;
			else
				w1 = 1;
			if( snes_ram[WOBJSEL] & 0x1 )
				w1 = !w1;
		}
		if( snes_ram[WOBJSEL] & 0x8 )
		{
			if( (ii < snes_ram[WH2]) || (ii > snes_ram[WH3]) )
				w2 = 0;
			else
				w2 = 1;
			if( snes_ram[WOBJSEL] & 0x4 )
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch( snes_ram[WOBJLOG] & 0x3 )
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[4][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x1:	/* AND */
					snes_ppu.clipmasks[4][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x2:	/* XOR */
					snes_ppu.clipmasks[4][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0x3:	/* XNOR */
					snes_ppu.clipmasks[4][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[4][ii] = w1 ? 0x00 : 0xff;
		else if( w2 >= 0 )
			snes_ppu.clipmasks[4][ii] = w2 ? 0x00 : 0xff;

		/* update colour window */
		/* FIXME: Why is the colour window different to the other windows? *
         * Have I overlooked something or done something wrong? */
		snes_ppu.clipmasks[5][ii] = 0xff;
		w1 = w2 = -1;
		if( snes_ram[WOBJSEL] & 0x20 )
		{
			/* Default to mask area inside */
			if( (ii < snes_ram[WH0]) || (ii > snes_ram[WH1]) )
				w1 = 0;
			else
				w1 = 1;
			/* If mask area is outside then swap */
			if( snes_ram[WOBJSEL] & 0x10 )
				w1 = !w1;
		}
		if( snes_ram[WOBJSEL] & 0x80 )
		{
			/* Default to mask area inside */
			if( (ii < snes_ram[WH2]) || (ii > snes_ram[WH3]) )
				w2 = 0;
			else
				w2 = 1;
			/* If mask area is outside then swap */
			if( snes_ram[WOBJSEL] & 0x40 )
				w2 = !w2;
		}
		if( w1 >= 0 && w2 >= 0 )
		{
			switch( snes_ram[WOBJLOG] & 0xc )
			{
				case 0x0:	/* OR */
					snes_ppu.clipmasks[5][ii] = w1 | w2 ? 0xff : 0x00;
/*                  snes_ppu.clipmasks[5][ii] = w1 | w2 ? 0x00 : 0xff;*/
					break;
				case 0x4:	/* AND */
					snes_ppu.clipmasks[5][ii] = w1 & w2 ? 0xff : 0x00;
/*                  snes_ppu.clipmasks[5][ii] = w1 & w2 ? 0x00 : 0xff;*/
					break;
				case 0x8:	/* XOR */
					snes_ppu.clipmasks[5][ii] = w1 ^ w2 ? 0xff : 0x00;
/*                  snes_ppu.clipmasks[5][ii] = w1 ^ w2 ? 0x00 : 0xff;*/
					break;
				case 0xc:	/* XNOR */
					snes_ppu.clipmasks[5][ii] = !(w1 ^ w2) ? 0xff : 0x00;
/*                  snes_ppu.clipmasks[5][ii] = !(w1 ^ w2) ? 0x00 : 0xff;*/
					break;
			}
		}
		else if( w1 >= 0 )
			snes_ppu.clipmasks[5][ii] = w1 ? 0xff : 0x00;
/*          snes_ppu.clipmasks[5][ii] = w1 ? 0x00 : 0xff;*/
		else if( w2 >= 0 )
			snes_ppu.clipmasks[5][ii] = w2 ? 0xff : 0x00;
/*          snes_ppu.clipmasks[5][ii] = w2 ? 0x00 : 0xff;*/
	}
}

/*********************************************
 * snes_update_palette()
 *
 * Update the palette with the latest changes.
 *********************************************/
static void snes_update_palette(running_machine *machine)
{
	UINT8 r, g, b, fade;
	UINT16 ii;
	UINT32 col;

	/* Reset the flag */
	snes_ppu.update_palette = 0;

	/* Modify the palette to fade out the colours */
	fade = (snes_ram[INIDISP] & 0xf) + 1;
	for( ii = 0; ii <= 256; ii++ )
	{
		col = machine->pens[snes_cgram[ii] & 0x7fff];
		r = ((col & 0x1f) * fade) >> 4;
		g = (((col & 0x3e0) >> 5) * fade) >> 4;
		b = (((col & 0x7c00) >> 10) * fade) >> 4;
		/* palette hacks! */
		((pen_t *)machine->remapped_colortable)[ii] = ((r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10));
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
	snes_ppu.update_offsets = 0;
}

/*********************************************
 * snes_refresh_scanline()
 *
 * Redraw the current line.
 *********************************************/
static void snes_refresh_scanline(running_machine *machine, mame_bitmap *bitmap, UINT16 curline )
{
	UINT16 ii;

	profiler_mark(PROFILER_VIDEO);

	if( snes_ram[INIDISP] & 0x80 ) /* screen is forced blank */
	{
		rectangle r = machine->screen[0].visarea;
		r.min_y = r.max_y = curline;
		fillbitmap(bitmap, machine->pens[0], &r);
	}
	else
	{
		/* Update clip window masks if necessary */
		if( snes_ppu.update_windows )
			snes_update_windowmasks();
		/* Update the palette if necessary */
		if( snes_ppu.update_palette )
			snes_update_palette(machine);
		/* Update the offsets if necessary */
		if( snes_ppu.update_offsets )
			snes_update_offsets();

		/* Clear zbuffers */
		memset( scanlines[MAINSCREEN].zbuf, 0, SNES_SCR_WIDTH * 2 );
		memset( scanlines[SUBSCREEN].zbuf, 0, SNES_SCR_WIDTH * 2 );

		/* Clear subscreen and draw back colour */
		for( ii = 0; ii < SNES_SCR_WIDTH * 2; ii++ )
		{
			/* Not sure if this is correct behaviour, but a few games seem to
             * require it. (SMW, Zelda etc) */
			scanlines[SUBSCREEN].buffer[ii] = machine->remapped_colortable[FIXED_COLOUR];
			/* Draw back colour */
			scanlines[MAINSCREEN].buffer[ii] = machine->remapped_colortable[0];
		}

		/* Draw subscreen */
		if( snes_ram[TS] & 0x10 )
			snes_update_objects(machine, SUBSCREEN, curline );
		for( ii = 0; ii < snes_modedefs[snes_ppu.mode].count; ii++ )
		{
			if( snes_ram[TS] & (0x1 << ii) )
				snes_modedefs[snes_ppu.mode].drawLayer[ii](machine, SUBSCREEN, ii, curline );
		}
		/* Draw the back plane */
#ifdef SNES_DBG_video
		if( !debug_options.bg_disabled[5] )
#endif /* SNES_DBG_video */
		if( snes_ram[CGADSUB] & 0x20 )
		{
			for( ii = 0; ii < SNES_SCR_WIDTH; ii++ )
			{
				snes_draw_blend(machine, ii, &scanlines[MAINSCREEN].buffer[ii], (snes_ram[CGADSUB] & 0x80)?SNES_BLEND_SUB:SNES_BLEND_ADD, (snes_ram[CGWSEL] & 0x30) >> 4 );
			}
		}
		/* Draw mainscreen */
		if( snes_ram[TM] & 0x10 )
			snes_update_objects(machine, MAINSCREEN, curline );
		for( ii = 0; ii < snes_modedefs[snes_ppu.mode].count; ii++ )
		{
			if( snes_ram[TM] & (0x1 << ii) )
				snes_modedefs[snes_ppu.mode].drawLayer[ii](machine, MAINSCREEN, ii, curline );
		}

#ifdef SNES_DBG_video
		if( snes_dbg_video(machine, bitmap, curline ) )
		{
			profiler_mark(PROFILER_END);
			return;
		}

		/* Toggle drawing of subscreen or mainscreen */
		if( debug_options.draw_subscreen )
			memcpy( (UINT16 *)bitmap->base + bitmap->rowpixels * curline, scanlines[SUBSCREEN].buffer, (SNES_SCR_WIDTH * 2) * sizeof(UINT16) );
		else
#endif /* SNES_DBG_video */

		/* Phew! Draw the line to screen */
		memcpy( (UINT16 *)bitmap->base + bitmap->rowpixels * curline, scanlines[MAINSCREEN].buffer, (SNES_SCR_WIDTH * 2) * sizeof(UINT16) );
	}

	profiler_mark(PROFILER_END);
}

VIDEO_UPDATE( snes )
{
	int y;

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		snes_refresh_scanline(machine, bitmap, y);
	return 0;
}


/***** Debug Functions *****/

#ifdef SNES_DBG_video

static void snes_dbg_draw_maps(running_machine *machine, mame_bitmap *bitmap, UINT32 tmap, UINT8 bpl, UINT16 curline, UINT8 layer )
{
	UINT32 tile, addr = tmap;
	UINT16 ii, vflip, hflip, pal;
	INT8 line;
	char str[50];

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
					snes_draw_tile_2(machine, MAINSCREEN, layer, snes_ppu.layer[layer].data + (tile << 4) + ((curline % 8) * 2), (ii >> 1) * 8, 255, hflip, pal );
					break;
				case 2:
					pal <<= 2;
					snes_draw_tile_4(machine, MAINSCREEN, layer, snes_ppu.layer[layer].data + (tile << 5) + ((curline % 8) * 2), (ii >> 1) * 8, 255, hflip, pal );
					break;
				case 4:
					snes_draw_tile_8(machine, MAINSCREEN, layer, snes_ppu.layer[layer].data + (tile << 6) + ((curline % 8) * 2), (ii >> 1) * 8, 255, hflip );
					break;
			}
		}
	}

	sprintf( str, "%d : %8X  ", layer, addr );
	//ui_draw_text( str, 0, 227 );
}

static void snes_dbg_draw_all_tiles(running_machine *machine, mame_bitmap *bitmap, UINT32 tileaddr, UINT8 bpl, UINT16 pal )
{
	UINT16 ii, jj, kk;
	UINT32 addr = tileaddr;
	char str[50];

	for( jj = 0; jj < 32; jj++ )
	{
		addr = tileaddr + (jj * bpl * 16 * 32);
		for( kk = 0; kk < 8; kk++ )
		{
			/* Clear buffers */
			memset( scanlines[MAINSCREEN].buffer, 0, SNES_SCR_WIDTH * 2 );
			memset( scanlines[MAINSCREEN].zbuf, 0, SNES_SCR_WIDTH * 2 );
			for( ii = 0; ii < 32; ii++ )
			{
				switch( bpl )
				{
					case 1:
						snes_draw_tile_2(machine, MAINSCREEN, 0, addr, ii * 8, 255, 0, pal );
						break;
					case 2:
						snes_draw_tile_4(machine, MAINSCREEN, 0, addr, ii * 8, 255, 0, pal );
						break;
					case 4:
						snes_draw_tile_8(machine, MAINSCREEN, 0, addr, ii * 8, 255, 0 );
						break;
				}
				addr += (bpl * 16);
			}
			draw_scanline16( bitmap, 0, jj * 8 + kk, SNES_SCR_WIDTH * 2, scanlines[MAINSCREEN].buffer, machine->pens, 200 );
			addr -= (32 * (bpl * 16)) - 2;
		}
	}

	sprintf( str, "  %8X  ", tileaddr );
	//ui_draw_text( str, 0, 227 );
}

static UINT8 snes_dbg_video(running_machine *machine, mame_bitmap *bitmap, UINT16 curline )
{
	UINT16 ii;

#define SNES_DBG_HORZ_POS 545

	/* Check if the user has enabled or disabled stuff */
	if( curline == 0 )
	{
		//UINT16 y = 1;
		char t[100];
		static char WINLOGIC[4] = { '|', '&', '^', '!' };

		if( !debug_options.input_count-- )
		{
			UINT8 toggles = readinputport( 10 );
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
			toggles = readinputport( 11 );
			if( toggles & 0x4 )
				debug_options.transparency_disabled = !debug_options.transparency_disabled;
			debug_options.input_count = 5;
		}
		/* Display some debug info on the screen */
		sprintf( t, "%s%s", debug_options.windows_disabled?" ":"W", debug_options.transparency_disabled?" ":"T" );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "%s1 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
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
		sprintf( t, "%s2 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
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
		sprintf( t, "%s3 %s%s%s%s%s%c%s%s%d%s%s%d %4X %4X",
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
		sprintf( t, "%s4 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
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
		sprintf( t, "%sO %s%s%s%s%s%c%s%s%d%d       %4X",
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
		sprintf( t, "%sB   %s  %c%s%s",
				debug_options.bg_disabled[5]?" ":"*",
				(snes_ram[CGADSUB] & 0x20)?"B":" ",
				WINLOGIC[(snes_ram[WOBJLOG] & 0xc)>>2],
				(snes_ram[WOBJSEL] & 0x20)?((snes_ram[WOBJSEL] & 0x10)?"o":"i"):" ",
				(snes_ram[WOBJSEL] & 0x80)?((snes_ram[WOBJSEL] & 0x40)?"o":"i"):" " );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "1) %3d %3d   2) %3d %3d", (snes_ppu.bgd_offset.horizontal[0] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[0] & 0x3ff) >> 3, (snes_ppu.bgd_offset.horizontal[1] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[1] & 0x3ff) >> 3 );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "3) %3d %3d   4) %3d %3d", (snes_ppu.bgd_offset.horizontal[2] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[2] & 0x3ff) >> 3, (snes_ppu.bgd_offset.horizontal[3] & 0x3ff) >> 3, (snes_ppu.bgd_offset.vertical[3] & 0x3ff) >> 3 );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "Flags: %s%s%s %s %2d", (snes_ram[CGWSEL] & 0x2)?"S":"F", (snes_ram[CGADSUB] & 0x80)?"-":"+", (snes_ram[CGADSUB] & 0x40)?" 50%":"100%",(snes_ram[CGWSEL] & 0x1)?"D":"P", (snes_ram[MOSAIC] & 0xf0) >> 4 );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "SetINI: %s %s %s %s %s %s", (snes_ram[SETINI] & 0x1)?" I":"NI", (snes_ram[SETINI] & 0x2)?"P":"R", (snes_ram[SETINI] & 0x4)?"240":"225",(snes_ram[SETINI] & 0x8)?"512":"256",(snes_ram[SETINI] & 0x40)?"E":"N",(snes_ram[SETINI] & 0x80)?"ES":"NS" );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "Mode7: A %5d B %5d", snes_ppu.mode7.matrix_a, snes_ppu.mode7.matrix_b );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, " %s%s%s   C %5d D %5d", (snes_ram[M7SEL] & 0xc0)?((snes_ram[M7SEL] & 0x40)?"0":"C"):"R", (snes_ram[M7SEL] & 0x1)?"H":" ", (snes_ram[M7SEL] & 0x2)?"V":" ", snes_ppu.mode7.matrix_c, snes_ppu.mode7.matrix_d );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
		sprintf( t, "       X %5d Y %5d", snes_ppu.mode7.origin_x, snes_ppu.mode7.origin_y );
		//ui_draw_text( t, SNES_DBG_HORZ_POS, y++ * 9 );
	}
	/* Just for testing, draw as many tiles as possible */
	{
		UINT8 adjust = readinputport( 1 );
		UINT8 dip = readinputport( 9 );
		UINT8 inp = readinputport( 11 );
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
			snes_dbg_draw_maps(machine, bitmap, tmaddr, dm, curline, tmbg );
			draw_scanline16( bitmap, 0, curline, SNES_SCR_WIDTH, scanlines[MAINSCREEN].buffer, machine->pens, 200 );
			return 1;
				}
		}

			/* Draw some useful information about the back/fixed colours and current bg mode etc. */
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 26) = machine->pens[dbg_mode_colours[(snes_ram[CGWSEL] & 0xc0) >> 6]];
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 24) = machine->pens[dbg_mode_colours[(snes_ram[CGWSEL] & 0x30) >> 4]];
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 22) = machine->pens[dbg_mode_colours[snes_ram[BGMODE] & 0x7]];
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 12) = machine->pens[32767];
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 2 ) = machine->pens[32767];
			for( ii = 0; ii < 5; ii++ )
			{
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 19 + ii) = machine->remapped_colortable[0];
			*BITMAP_ADDR16(bitmap, curline, SNES_DBG_HORZ_POS - 9  + ii) = machine->remapped_colortable[FIXED_COLOUR];
			}
			/* Draw window positions */
			scanlines[MAINSCREEN].buffer[snes_ram[WH0]] = machine->pens[dbg_mode_colours[0]];
			scanlines[MAINSCREEN].buffer[snes_ram[WH1]] = machine->pens[dbg_mode_colours[0]];
			scanlines[MAINSCREEN].buffer[snes_ram[WH2]] = machine->pens[dbg_mode_colours[2]];
			scanlines[MAINSCREEN].buffer[snes_ram[WH3]] = machine->pens[dbg_mode_colours[2]];

	return 0;
	}

#endif /* SNES_DBG_video */
