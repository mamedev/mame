/***************************************************************************

    Sega 16-bit common hardware

****************************************************************************

    Hang On
    -------
        Control Board (834-5668):
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5049 (x2)  -- tilemaps
            315-5107 (PAL x2) -- horizontal timing control
            315-5108       -- vertical timing control
            315-5122 (PAL) -- timing

    Enduro Racer
    ------------
        CPU Side (171-5319):
            315-5164 (PAL)
            315-5165 (PAL)
            315-5166 (PAL)
            315-5167 (PAL)

        Video Side (171-5320):
            315-5049 (x2)  -- tilemaps
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5106 (PAL)
            315-5107 (PAL)
            315-5108 (PAL)
            315-5168 (PAL)
            315-5170 (PAL)
            315-5171 (PAL)
            315-5172 (PAL)

    Pre-System 16
    -------------
        Main Board (171-5335):
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5049 (x2)  -- tilemaps
            315-5107 (PAL) -- display timing
            315-5108 (PAL) -- display timing
            315-5141 (PAL) -- Z80 address decoding
            315-5143 (PAL) -- sprite-related?
            315-5144 (PAL) -- sprite-related?
            315-5147 (PAL) -- unknown, DTACK-related
            315-5149 (PAL) -- video mixing
            315-5193 (PAL) -- 68000/MCU interface & address decoding
            315-5202 (PAL) -- 68000/MCU interface & address decoding

    Sega System 16A
    ---------------
        Bottom Board (171-5307):
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5049 (x2)  -- tilemaps
            315-5107 (PAL) -- display timing
            315-5108 (PAL) -- display timing
            315-5143 (PAL) -- sprite-related?
            315-5144 (PAL) -- sprite-related?
            315-5145 (PAL)

        Top Board (171-5306):
            315-5141 (PAL) -- Z80 address decoding
            315-5142 (PAL)
            315-5149 (PAL) -- video mixing
            315-5150 (PAL)

    Sega System 16B
    ---------------
        Main Board (171-5357):
            315-5195       -- memory mapper
            315-5196       -- sprite generator
            315-5197       -- tilemap generator
            315-5213 (PAL) -- sprite-related
            315-5214 (PAL) -- unknown

        ROM Board (171-5521):
            315-5298 (PAL)

        ROM Board (171-5704):
            315-5298 (PAL)

        ROM Board (171-5797):
            315-5248       -- hardware multiplier
            315-5250       -- compare/timer
            315-5298 (PAL)

    Sega System 18
    --------------
        Main Board (171-5873B):
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5313       -- VDP
            315-5360       -- memory mapper?
            315-5361       -- sprite generator
            315-5362       -- tilemap generator
            315-5373 (PAL) -- video mixing
            315-5374 (PAL) -- sprite timing
            315-5375 (PAL) -- system timing
            315-5389 (PAL) -- VDP sync
            315-5390 (PAL)
            315-5391 (PAL) -- Z80 address decoding

        Main Board (171-5873-02B):
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5313       -- VDP
            315-5360       -- memory mapper?
            315-5361       -- sprite generator
            315-5362       -- tilemap generator
            315-5374 (PAL) -- sprite timing
            315-5375 (PAL) -- system timing
            315-5389 (PAL) -- VDP sync
            315-5391 (PAL) -- Z80 address decoding
            315-5430 (PAL) -- video mixing

        ROM Board (171-5987A):
            315-5436       -- tile/sprite banking

    Sega System C
    -------------
        Main Board:
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5313       -- VDP
            315-5393 (PAL)
            315-5394 (PAL)
            315-5395 (PAL)

    Super Hang On
    -------------
        CPU Board 171-5376-01:
            315-5195       -- memory mapper
            315-5218       -- PCM sound controller
            315-5155 (PAL x2) -- road bit extraction
            315-5222 (PAL) -- road mixing
            315-5223a (PAL)
            315-5224 (PAL)
            315-5225 (PAL)
            315-5226 (PAL)

        VIDEO Board: (not the same as out run !) 171-5480
            315-5196       -- sprite generator
            315-5197       -- tilemap generator
            315-5213 (PAL) -- sprite-related
            315-5242       -- color encoder
            315-5251 (PAL)

    Out Run
    -------
        CPU Board 837-6063-01:
            315-5195       -- memory mapper
            315-5218       -- PCM sound controller
            315-5155 (PAL x2) -- road bit extraction
            315-5222 (PAL) -- road mixing
            315-5223a (PAL)
            315-5224 (PAL)
            315-5225 (PAL)
            315-5226 (PAL)

        VIDEO Board: 837-6064, 171-5377-01
            315-5197       -- tilemap generator
            315-5211       -- sprite generator
            315-5227a (PAL)
            315-5228 (PAL)
            315-5242       -- color encoder

    Sega System 32
    --------------
        Main Board (317-5964):
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5385
            315-5386       -- tilemap generator
            315-5387       -- sprite generator
            315-5388       -- video mixing
            315-5441 (PAL)
            315-5476

    X-Board
    -------
        Main Board:
            315-5197       -- tilemap generator
            315-5211A      -- sprite generator
            315-5218       -- PCM sound controller
            315-5242       -- color encoder
            315-5248 (x2)  -- hardware multiplier
            315-5249 (x2)  -- hardware divider
            315-5250 (x2)  -- compare/timer
            315-5275       -- road generator
            315-5278 (PAL) -- sprite ROM bank control
            315-5279 (PAL) -- video mixing (Afterburner)
            315-5280 (PAL) -- Z80 address decoding
            315-5290 (PAL) -- main CPU address decoding
            315-5291 (PAL) -- main CPU address decoding
            315-5304 (PAL) -- video mixing (Line of Fire)

    Y-Board
    -------
        Main Board (837-6565):
            315-5218       -- PCM sound controller
            315-5248 (x3)  -- hardware multiplier
            315-5249 (x3)  -- hardware divider
            315-5280 (PAL) -- Z80 address decoding
            315-5296       -- I/O chip
            315-5314 (PAL)
            315-5315 (PAL)
            315-5316 (PAL)
            315-5317 (PAL)
            315-5318 (PAL)
            315-5328 (PAL)

        Video Board (837-6566):
            315-5196       -- sprite generator
            315-5213 (PAL) -- sprite-related
            315-5242       -- color encoder
            315-5305       -- sprite generator
            315-5306 (x2)  -- video sync and rotation
            315-5312       -- video mixing
            315-5319 (PAL)
            315-5325 (PAL)


    Custom parts
    ------------
                   SYS1  SYS2  HANG  ENDU  PR16  S16A  S16B  SY18  SHNG  ORUN  XBRD  YBRD  SYSC  SY24  SY32
        315-5011:   xx    xx    xx    xx    xx    xx                                                         -- sprite line comparitor
        315-5012:   xx    xx    xx    xx    xx    xx                                                         -- sprite generator control
        315-5049:         xx    x2    x2    x2    x2                                                         -- tilemap generator
        315-5195:                                       xx          xx    xx                                 -- memory mapper
        315-5196:                                       xx          xx                xx                     -- sprite genereator
        315-5197:                                       xx          xx    xx    xx                           -- tilemap generator
        315-5211:                                                         xx                                 -- sprite generator
        315-5211A:                                                              xx                           -- sprite generator
        315-5218:                                                   xx    xx    xx    xx                     -- PCM sound controller
        315-5242:                                             xx    xx    xx    xx    xx    xx    xx    xx   -- color encoder
        315-5248:                                       xx                      x2    x3                     -- hardware multiplier
        315-5249:                                                               x2    x3                     -- hardware divider
        315-5250:                                       xx                      x2                           -- compare/timer
        315-5275:                                                               xx                           -- road generator
        315-5296:                                             xx                      xx    xx          xx   -- I/O chip
        315-5305:                                                                     xx                     --
        315-5312:                                                                     xx                     -- video mixing
        315-5313:                                             xx                            xx               -- VDP
        315-5360:                                             xx                                             -- memory mapper
        315-5361:                                             xx                                             -- sprite generator
        315-5362:                                             xx                                             -- tilemap generator
        315-5385:                                                                                       xx   -- ???
        315-5386:                                                                                       xx   -- tilemap generator
        315-5387:                                                                                       xx   -- sprite generator
        315-5388:                                                                                       xx   -- video mixing
        315-5436:                                             xx                                             -- sprite/tile banking
        315-5476:                                                                                       xx   -- ????

****************************************************************************

  Sega system16 and friends hardware

               CPU      Tiles      Sprites   Priority  Color     SCPU  Sound                Other
System C       68000    315-5313                       315-5242  z80   ym3438               315-5296(IO)
Space Harrier  68000x2                                 (c)       z80   ym2203 pcm(b)
System 16B     68000    315-5197   315-5196  GAL       (c)       z80   ym2151 upd7759       315-5195
After Burner   68000x2  315-5197   315-5211A GAL       315-5242  z80   ym2151 315-5218      315-5250(a) 315-5248(x2) 315-5249(x2) 315-5275(road)
System 18      68000    315-536x   315-536x            315-5242  z80   ym3834(x2) RF5c68(d) 315-3296(IO) 315-5313(vdp)
System 24      68000x2  315-5292   315-5293  315-5294  315-5242        ym2151 dac           315-5195(x3) 315-5296(IO)
Galaxy Force   68000x3             315-5296+ 315-5312  315-5242  z80   ym2151 315-5218      315-5296(IO)
System 32      V60      315-5386A  315-5387  315-5388  315-5242  z80   ym3834(x2) RF5c68(d) 315-5296(IO)

a) 315-5250: 68000 glue and address decoding

b) 8x8-bit voices entirely in TTL.  The 315-5218 is believed to be the
   integrated version of that

c) Resistor network and latches believed to be equivalent to the 315-5242

d) Also seen as 315-5476A and ASSP 5c105 and ASSP 5c68a

Quick review of the system16 hardware:

  Hang-on hardware:
    The first one.  Two tilemap planes, one sprite plane, one road
    plane.  The shadow capability doesn't seem to be used, the
    highlight/shadow switch in the 5242-equivalent is global for all
    colors.

  Space harrier hardware:
    Similar to hang-on, with per-color highlight/shadow selection, and
    the shadows are used.

  System16a / Pre-system16:
    Space harrier without the road generator.

  System16b:
    4-layer tilemap hardware in two pairs, with selection between each
    members on the pairs on a 8-lines basis.  Slightly better sprites.

  System18
    System 16b plus a genesis vdp.

  Outrun:
    System 16b tilemaps, frame buffered sprites with better zooming
    capabilities, and a road generator able to handle two roads
    simultaneously.

  Super hang-on:
    Outrun lite, with System 16b sprites instead of the frame buffered
    sprites, and only one of the two roads is actually used.

  X-Board:
    Outrun with a better fillrate and an even more flexible road
    generator.

  Y-Board:
    New design, with two sprite planes and no tilemaps.  The back
    sprite plane has a huge fillrate and the capability to have its
    frame buffer completely rotated.  Also, it has a palette
    indirection capability to allows for easier palette rotations.
    The front sprite plane is System 16b.

  System24:
    The odd one out.  Medium resolution.  Entirely ram-based, no
    graphics roms.  4-layer tilemap hardware in two pairs, selection
    on a 8-pixels basis.  Tile-based sprites(!) organised as a linked
    list.  The tilemap chip has been reused for model1 and model2,
    probably because they had it handy and it handles medium res.

  System32:
    5-layer tilemap hardware consisting of 4 independent rom-based
    layers with linescroll, lineselection, linezoom and window
    clipping capability and one simpler ram-based text plane.  Mixed
    ram/rom sprite engine with palette indirection, per-color priority
    (thankfully not actually used).  The sprite list includes jumping
    and clipping capabilities, and advanced hot-spot positioning.  The
    mixer chip adds totally dynamic priorities, alpha-blending of the
    tilemaps, per-component color control, and some other funnies we
    have not been able to decypher.

  ST-V (also know as Titan or the Saturn console):
    The ultimate 2D system.  Even more advanced tilemaps, with 6-dof
    roz support, alpha up to the wazoo and other niceties, known as
    the vdp2.  Ths sprite engine, vdp1, allows for any 4-point
    streching of the sprites, actually giving polygonal 3D
    capabilities.  Interestingly, the mixer capabilities took a hit,
    with no real per-sprite mixer priority, which could be considered
    annoying for a 2D system.  It still allowed some beauties like
    Radiant Silvergun.

***************************************************************************/

#include "emu.h"
#include "segaic16.h"
#include "video/resnet.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define PRINT_UNUSUAL_MODES		(0)






/*************************************
 *
 *  Globals
 *
 *************************************/

UINT8 segaic16_display_enable;
UINT16 *segaic16_tileram_0;
UINT16 *segaic16_textram_0;
UINT16 *segaic16_roadram_0;
UINT16 *segaic16_rotateram_0;

struct rotate_info segaic16_rotate[SEGAIC16_MAX_ROTATE];
struct road_info segaic16_road[SEGAIC16_MAX_ROADS];



/*************************************
 *
 *  Statics
 *
 *************************************/

static struct tilemap_info bg_tilemap[SEGAIC16_MAX_TILEMAPS];



/*************************************
 *
 *  Misc functions
 *
 *************************************/

void segaic16_set_display_enable(running_machine &machine, int enable)
{
	enable = (enable != 0);
	if (segaic16_display_enable != enable)
	{
		machine.primary_screen->update_partial(machine.primary_screen->vpos());
		segaic16_display_enable = enable;
	}
}



/*************************************
 *
 *  Draw a split tilemap in up to
 *  four pieces
 *
 *************************************/

static void segaic16_draw_virtual_tilemap(running_machine &machine, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 pages, UINT16 xscroll, UINT16 yscroll, UINT32 flags, UINT32 priority)
{
	int leftmin = -1, leftmax = -1, rightmin = -1, rightmax = -1;
	int topmin = -1, topmax = -1, bottommin = -1, bottommax = -1;
	rectangle pageclip;
	int page;

	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	/* which half/halves of the virtual tilemap do we intersect in the X direction? */
	if (xscroll < 64*8 - width)
	{
		leftmin = 0;
		leftmax = width - 1;
		rightmin = -1;
	}
	else if (xscroll < 64*8)
	{
		leftmin = 0;
		leftmax = 64*8 - xscroll - 1;
		rightmin = leftmax + 1;
		rightmax = width - 1;
	}
	else if (xscroll < 128*8 - width)
	{
		rightmin = 0;
		rightmax = width - 1;
		leftmin = -1;
	}
	else
	{
		rightmin = 0;
		rightmax = 128*8 - xscroll - 1;
		leftmin = rightmax + 1;
		leftmax = width - 1;
	}

	/* which half/halves of the virtual tilemap do we intersect in the Y direction? */
	if (yscroll < 32*8 - height)
	{
		topmin = 0;
		topmax = height - 1;
		bottommin = -1;
	}
	else if (yscroll < 32*8)
	{
		topmin = 0;
		topmax = 32*8 - yscroll - 1;
		bottommin = topmax + 1;
		bottommax = height - 1;
	}
	else if (yscroll < 64*8 - height)
	{
		bottommin = 0;
		bottommax = height - 1;
		topmin = -1;
	}
	else
	{
		bottommin = 0;
		bottommax = 64*8 - yscroll - 1;
		topmin = bottommax + 1;
		topmax = height - 1;
	}

	/* if the tilemap is flipped, we need to flip our sense within each quadrant */
	if (info->flip)
	{
		if (leftmin != -1)
		{
			int temp = leftmin;
			leftmin = width - 1 - leftmax;
			leftmax = width - 1 - temp;
		}
		if (rightmin != -1)
		{
			int temp = rightmin;
			rightmin = width - 1 - rightmax;
			rightmax = width - 1 - temp;
		}
		if (topmin != -1)
		{
			int temp = topmin;
			topmin = height - 1 - topmax;
			topmax = height - 1 - temp;
		}
		if (bottommin != -1)
		{
			int temp = bottommin;
			bottommin = height - 1 - bottommax;
			bottommax = height - 1 - temp;
		}
	}

	/* draw the upper-left chunk */
	if (leftmin != -1 && topmin != -1)
	{
		pageclip.min_x = (leftmin < cliprect.min_x) ? cliprect.min_x : leftmin;
		pageclip.max_x = (leftmax > cliprect.max_x) ? cliprect.max_x : leftmax;
		pageclip.min_y = (topmin < cliprect.min_y) ? cliprect.min_y : topmin;
		pageclip.max_y = (topmax > cliprect.max_y) ? cliprect.max_y : topmax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 0) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(bitmap, pageclip, flags, priority);
		}
	}

	/* draw the upper-right chunk */
	if (rightmin != -1 && topmin != -1)
	{
		pageclip.min_x = (rightmin < cliprect.min_x) ? cliprect.min_x : rightmin;
		pageclip.max_x = (rightmax > cliprect.max_x) ? cliprect.max_x : rightmax;
		pageclip.min_y = (topmin < cliprect.min_y) ? cliprect.min_y : topmin;
		pageclip.max_y = (topmax > cliprect.max_y) ? cliprect.max_y : topmax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 4) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(bitmap, pageclip, flags, priority);
		}
	}

	/* draw the lower-left chunk */
	if (leftmin != -1 && bottommin != -1)
	{
		pageclip.min_x = (leftmin < cliprect.min_x) ? cliprect.min_x : leftmin;
		pageclip.max_x = (leftmax > cliprect.max_x) ? cliprect.max_x : leftmax;
		pageclip.min_y = (bottommin < cliprect.min_y) ? cliprect.min_y : bottommin;
		pageclip.max_y = (bottommax > cliprect.max_y) ? cliprect.max_y : bottommax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 8) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(bitmap, pageclip, flags, priority);
		}
	}

	/* draw the lower-right chunk */
	if (rightmin != -1 && bottommin != -1)
	{
		pageclip.min_x = (rightmin < cliprect.min_x) ? cliprect.min_x : rightmin;
		pageclip.max_x = (rightmax > cliprect.max_x) ? cliprect.max_x : rightmax;
		pageclip.min_y = (bottommin < cliprect.min_y) ? cliprect.min_y : bottommin;
		pageclip.max_y = (bottommax > cliprect.max_y) ? cliprect.max_y : bottommax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 12) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(bitmap, pageclip, flags, priority);
		}
	}
}



/*******************************************************************************************
 *
 *  Hang On/System 16A-style tilemaps
 *
 *  4 total pages (Hang On)
 *  8 total pages (System 16A)
 *  Column/rowscroll enabled via external signals
 *
 *  Tile format:
 *      Bits               Usage
 *      ??------ --------  Unknown
 *      --b----- --------  Tile bank select
 *      ---p---- --------  Tile priority versus sprites
 *      ----cccc ccc-----  Tile color palette
 *      ----nnnn nnnnnnnn  Tile index
 *
 *  Text format:
 *      Bits               Usage
 *      ????---- --------  Unknown
 *      ----p--- --------  Priority
 *      -----ccc --------  Tile color palette
 *      -------- nnnnnnnn  Tile index
 *
 *  Text RAM:
 *      Offset   Bits               Usage
 *      E8C      -aaa-bbb -ccc-ddd  Background tilemap page select (screen flipped)
 *      E8E      -aaa-bbb -ccc-ddd  Foreground tilemap page select (screen flipped)
 *      E9C      -aaa-bbb -ccc-ddd  Background tilemap page select
 *      E9E      -aaa-bbb -ccc-ddd  Foreground tilemap page select
 *      F24      -------- vvvvvvvv  Foreground tilemap vertical scroll
 *      F26      -------- vvvvvvvv  Background tilemap vertical scroll
 *      F30-F7D  -------- vvvvvvvv  Foreground tilemap per-16-pixel-column vertical scroll (every 2 words)
 *      F32-F7F  -------- vvvvvvvv  Background tilemap per-16-pixel-column vertical scroll (every 2 words)
 *      F80-FED  -------h hhhhhhhh  Foreground tilemap per-8-pixel-row horizontal scroll (every 2 words)
 *      F82-FEF  -------h hhhhhhhh  Background tilemap per-8-pixel-row horizontal scroll (every 2 words)
 *      FF8      -------h hhhhhhhh  Foreground tilemap horizontal scroll
 *      FFA      -------h hhhhhhhh  Background tilemap horizontal scroll
 *
 *******************************************************************************************/

static TILE_GET_INFO( segaic16_tilemap_16a_tile_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)param;
	UINT16 data = info->rambase[tile_index];
	int code = ((data >> 1) & 0x1000) | (data & 0xfff);
	int color = (data >> 5) & 0x7f;

	SET_TILE_INFO(0, code, color, 0);
	tileinfo.category = (data >> 12) & 1;
}


static TILE_GET_INFO( segaic16_tilemap_16a_text_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)param;
	UINT16 data = info->rambase[tile_index];
	int color = (data >> 8) & 0x07;
	int code = data & 0xff;

	SET_TILE_INFO(0, code, color, 0);
	tileinfo.category = (data >> 11) & 1;
}


static void segaic16_tilemap_16a_draw_layer(running_machine &machine, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority)
{
	UINT16 *textram = info->textram;

	/* note that the scrolling for these games can only scroll as much as the top-left */
	/* page; in order to scroll beyond that they swap pages and reset the scroll value */
	UINT16 xscroll = textram[0xff8/2 + which] & 0x1ff;
	UINT16 yscroll = textram[0xf24/2 + which] & 0x0ff;
	UINT16 pages = textram[(info->flip ? 0xe8e/2 : 0xe9e/2) - which];
	int x, y;

	/* pages are swapped along the X direction, and there are only 8 of them */
	pages = ((pages >> 4) & 0x0707) | ((pages << 4) & 0x7070);
	if (info->numpages == 4)
		pages &= 0x3333;

	/* column AND row scroll */
	if (info->colscroll && info->rowscroll)
	{
		if (PRINT_UNUSUAL_MODES) mame_printf_debug("Column AND row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowcolclip;

			/* adjust to clip this row only */
			rowcolclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowcolclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* loop over column chunks */
			for (x = cliprect.min_x & ~15; x <= cliprect.max_x; x += 16)
			{
				UINT16 effxscroll, effyscroll;

				/* adjust to clip this column only */
				rowcolclip.min_x = (x < cliprect.min_x) ? cliprect.min_x : x;
				rowcolclip.max_x = (x + 15 > cliprect.max_x) ? cliprect.max_x : x + 15;

				/* get the effective scroll values */
				effxscroll = textram[0xf80/2 + rowscrollindex * 2 + which] & 0x1ff;
				effyscroll = textram[0xf30/2 + (x/16) * 2 + which] & 0x0ff;

				/* adjust the xscroll for flipped screen */
				if (info->flip)
					effxscroll += 17;

				/* draw the chunk */
				effxscroll = (0xc8 - effxscroll + info->xoffs) & 0x3ff;
				effyscroll = effyscroll & 0x1ff;
				segaic16_draw_virtual_tilemap(machine, info, bitmap, rowcolclip, pages, effxscroll, effyscroll, flags, priority);
			}
		}
	}
	else if (info->colscroll)
	{
		if (PRINT_UNUSUAL_MODES) mame_printf_debug("Column scroll\n");

		/* loop over column chunks */
		for (x = cliprect.min_x & ~15; x <= cliprect.max_x; x += 16)
		{
			rectangle colclip = cliprect;
			UINT16 effxscroll, effyscroll;

			/* adjust to clip this row only */
			colclip.min_x = (x < cliprect.min_x) ? cliprect.min_x : x;
			colclip.max_x = (x + 15 > cliprect.max_x) ? cliprect.max_x : x + 15;

			/* get the effective scroll values */
			effxscroll = xscroll;
			effyscroll = textram[0xf30/2 + (x/16) * 2 + which] & 0x0ff;

			/* adjust the xscroll for flipped screen */
			if (info->flip)
				effxscroll += 17;

			/* draw the chunk */
			effxscroll = (0xc8 - effxscroll + info->xoffs) & 0x3ff;
			effyscroll = effyscroll & 0x1ff;
			segaic16_draw_virtual_tilemap(machine, info, bitmap, colclip, pages, effxscroll, effyscroll, flags, priority);
		}
	}
	else if (info->rowscroll)
	{
		if (PRINT_UNUSUAL_MODES) mame_printf_debug("Row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowclip = cliprect;
			UINT16 effxscroll, effyscroll;

			/* adjust to clip this row only */
			rowclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* get the effective scroll values */
			effxscroll = textram[0xf80/2 + rowscrollindex * 2 + which] & 0x1ff;
			effyscroll = yscroll;

			/* adjust the xscroll for flipped screen */
			if (info->flip)
				effxscroll += 17;

			/* draw the chunk */
			effxscroll = (0xc8 - effxscroll + info->xoffs) & 0x3ff;
			effyscroll = effyscroll & 0x1ff;
			segaic16_draw_virtual_tilemap(machine, info, bitmap, rowclip, pages, effxscroll, effyscroll, flags, priority);
		}
	}
	else
	{
		/* adjust the xscroll for flipped screen */
		if (info->flip)
			xscroll += 17;
		xscroll = (0xc8 - xscroll + info->xoffs) & 0x3ff;
		yscroll = yscroll & 0x1ff;
		segaic16_draw_virtual_tilemap(machine, info, bitmap, cliprect, pages, xscroll, yscroll, flags, priority);
	}
}



/*******************************************************************************************
 *
 *  System 16B-style tilemaps
 *
 *  16 total pages
 *  Column/rowscroll enabled via bits in text layer
 *  Alternate tilemap support
 *
 *  Tile format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -??----- --------  Unknown
 *      ---ccccc cc------  Tile color palette
 *      ---nnnnn nnnnnnnn  Tile index
 *
 *  Text format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -???---- --------  Unknown
 *      ----ccc- --------  Tile color palette
 *      -------n nnnnnnnn  Tile index
 *
 *  Alternate tile format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -??----- --------  Unknown
 *      ----cccc ccc-----  Tile color palette
 *      ---nnnnn nnnnnnnn  Tile index
 *
 *  Alternate text format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -???---- --------  Unknown
 *      -----ccc --------  Tile color palette
 *      -------- nnnnnnnn  Tile index
 *
 *  Text RAM:
 *      Offset   Bits               Usage
 *      E80      aaaabbbb ccccdddd  Foreground tilemap page select
 *      E82      aaaabbbb ccccdddd  Background tilemap page select
 *      E84      aaaabbbb ccccdddd  Alternate foreground tilemap page select
 *      E86      aaaabbbb ccccdddd  Alternate background tilemap page select
 *      E90      c------- --------  Foreground tilemap column scroll enable
 *               -------v vvvvvvvv  Foreground tilemap vertical scroll
 *      E92      c------- --------  Background tilemap column scroll enable
 *               -------v vvvvvvvv  Background tilemap vertical scroll
 *      E94      -------v vvvvvvvv  Alternate foreground tilemap vertical scroll
 *      E96      -------v vvvvvvvv  Alternate background tilemap vertical scroll
 *      E98      r------- --------  Foreground tilemap row scroll enable
 *               ------hh hhhhhhhh  Foreground tilemap horizontal scroll
 *      E9A      r------- --------  Background tilemap row scroll enable
 *               ------hh hhhhhhhh  Background tilemap horizontal scroll
 *      E9C      ------hh hhhhhhhh  Alternate foreground tilemap horizontal scroll
 *      E9E      ------hh hhhhhhhh  Alternate background tilemap horizontal scroll
 *      F16-F3F  -------- vvvvvvvv  Foreground tilemap per-16-pixel-column vertical scroll
 *      F56-F7F  -------- vvvvvvvv  Background tilemap per-16-pixel-column vertical scroll
 *      F80-FB7  a------- --------  Foreground tilemap per-8-pixel-row alternate tilemap enable
 *               -------h hhhhhhhh  Foreground tilemap per-8-pixel-row horizontal scroll
 *      FC0-FF7  a------- --------  Background tilemap per-8-pixel-row alternate tilemap enable
 *               -------h hhhhhhhh  Background tilemap per-8-pixel-row horizontal scroll
 *
 *******************************************************************************************/

static TILE_GET_INFO( segaic16_tilemap_16b_tile_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)param;
	UINT16 data = info->rambase[tile_index];
	int color = (data >> 6) & 0x7f;
	int code = data & 0x1fff;

	code = info->bank[code / info->banksize] * info->banksize + code % info->banksize;

	SET_TILE_INFO(0, code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


static TILE_GET_INFO( segaic16_tilemap_16b_text_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)param;
	UINT16 data = info->rambase[tile_index];
	int bank = info->bank[0];
	int color = (data >> 9) & 0x07;
	int code = data & 0x1ff;

	SET_TILE_INFO(0, bank * info->banksize + code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


static TILE_GET_INFO( segaic16_tilemap_16b_alt_tile_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)param;
	UINT16 data = info->rambase[tile_index];
	int color = (data >> 5) & 0x7f;
	int code = data & 0x1fff;

	code = info->bank[code / info->banksize] * info->banksize + code % info->banksize;

	SET_TILE_INFO(0, code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


static TILE_GET_INFO( segaic16_tilemap_16b_alt_text_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)param;
	UINT16 data = info->rambase[tile_index];
	int bank = info->bank[0];
	int color = (data >> 8) & 0x07;
	int code = data & 0xff;

	SET_TILE_INFO(0, bank * info->banksize + code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


static void segaic16_tilemap_16b_draw_layer(running_machine &machine, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority)
{
	UINT16 *textram = info->textram;
	UINT16 xscroll, yscroll, pages;
	int x, y;

	/* get global values */
	xscroll = info->latched_xscroll[which];
	yscroll = info->latched_yscroll[which];
	pages = info->latched_pageselect[which];

	/* column scroll? */
	if (yscroll & 0x8000)
	{
		if (PRINT_UNUSUAL_MODES) mame_printf_debug("Column AND row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowcolclip;

			/* adjust to clip this row only */
			rowcolclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowcolclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* loop over column chunks */
			for (x = ((cliprect.min_x + 8) & ~15) - 8; x <= cliprect.max_x; x += 16)
			{
				UINT16 effxscroll, effyscroll, rowscroll;
				UINT16 effpages = pages;

				/* adjust to clip this column only */
				rowcolclip.min_x = (x < cliprect.min_x) ? cliprect.min_x : x;
				rowcolclip.max_x = (x + 15 > cliprect.max_x) ? cliprect.max_x : x + 15;

				/* get the effective scroll values */
				rowscroll = textram[0xf80/2 + 0x40/2 * which + rowscrollindex];
				effxscroll = (xscroll & 0x8000) ? rowscroll : xscroll;
				effyscroll = textram[0xf16/2 + 0x40/2 * which + (x+8)/16];

				/* are we using an alternate? */
				if (rowscroll & 0x8000)
				{
					effxscroll = info->latched_xscroll[which + 2];
					effyscroll = info->latched_yscroll[which + 2];
					effpages = info->latched_pageselect[which + 2];
				}

				/* draw the chunk */
				effxscroll = (0xc0 - effxscroll + info->xoffs) & 0x3ff;
				effyscroll = effyscroll & 0x1ff;
				segaic16_draw_virtual_tilemap(machine, info, bitmap, rowcolclip, effpages, effxscroll, effyscroll, flags, priority);
			}
		}
	}
	else
	{
		if (PRINT_UNUSUAL_MODES) mame_printf_debug("Row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowclip = cliprect;
			UINT16 effxscroll, effyscroll, rowscroll;
			UINT16 effpages = pages;

			/* adjust to clip this row only */
			rowclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* get the effective scroll values */
			rowscroll = textram[0xf80/2 + 0x40/2 * which + rowscrollindex];
			effxscroll = (xscroll & 0x8000) ? rowscroll : xscroll;
			effyscroll = yscroll;

			/* are we using an alternate? */
			if (rowscroll & 0x8000)
			{
				effxscroll = info->latched_xscroll[which + 2];
				effyscroll = info->latched_yscroll[which + 2];
				effpages = info->latched_pageselect[which + 2];
			}

			/* draw the chunk */
			effxscroll = (0xc0 - effxscroll + info->xoffs) & 0x3ff;
			effyscroll = effyscroll & 0x1ff;
			segaic16_draw_virtual_tilemap(machine, info, bitmap, rowclip, effpages, effxscroll, effyscroll, flags, priority);
		}
	}
}


static TIMER_CALLBACK( segaic16_tilemap_16b_latch_values )
{
	struct tilemap_info *info = &bg_tilemap[param];
	UINT16 *textram = info->textram;
	int i;

	/* latch the scroll and page select values */
	for (i = 0; i < 4; i++)
	{
		info->latched_pageselect[i] = textram[0xe80/2 + i];
		info->latched_yscroll[i] = textram[0xe90/2 + i];
		info->latched_xscroll[i] = textram[0xe98/2 + i];
	}

	/* set a timer to do this again next frame */
	info->latch_timer->adjust(machine.primary_screen->time_until_pos(261), param);
}


static void segaic16_tilemap_16b_reset(running_machine &machine, struct tilemap_info *info)
{
	/* set a timer to latch values on scanline 261 */
	info->latch_timer->adjust(machine.primary_screen->time_until_pos(261), info->index);
}



/*************************************
 *
 *  General tilemap initialization
 *
 *************************************/

void segaic16_tilemap_init(running_machine &machine, int which, int type, int colorbase, int xoffs, int numbanks)
{
	struct tilemap_info *info = &bg_tilemap[which];
	tile_get_info_func get_text_info;
	tile_get_info_func get_tile_info;
	int pagenum;
	int i;

	/* reset the tilemap info */
	memset(info, 0, sizeof(*info));
	info->index = which;
	info->type = type;
	for (i = 0; i < numbanks; i++)
		info->bank[i] = i;
	info->banksize = 0x2000 / numbanks;
	info->xoffs = xoffs;

	/* set up based on which tilemap */
	switch (which)
	{
		case 0:
			info->textram = segaic16_textram_0;
			info->tileram = segaic16_tileram_0;
			break;

		default:
			fatalerror("Invalid tilemap index specified in segaic16_tilemap_init\n");
	}

	/* determine the parameters of the tilemaps */
	switch (type)
	{
		case SEGAIC16_TILEMAP_HANGON:
			get_text_info = segaic16_tilemap_16a_text_info;
			get_tile_info = segaic16_tilemap_16a_tile_info;
			info->numpages = 4;
			info->draw_layer = segaic16_tilemap_16a_draw_layer;
			info->reset = NULL;
			info->latch_timer = NULL;
			break;

		case SEGAIC16_TILEMAP_16A:
			get_text_info = segaic16_tilemap_16a_text_info;
			get_tile_info = segaic16_tilemap_16a_tile_info;
			info->numpages = 8;
			info->draw_layer = segaic16_tilemap_16a_draw_layer;
			info->reset = NULL;
			info->latch_timer = NULL;
			break;

		case SEGAIC16_TILEMAP_16B:
			get_text_info = segaic16_tilemap_16b_text_info;
			get_tile_info = segaic16_tilemap_16b_tile_info;
			info->numpages = 16;
			info->draw_layer = segaic16_tilemap_16b_draw_layer;
			info->reset = segaic16_tilemap_16b_reset;
			info->latch_timer = machine.scheduler().timer_alloc(FUNC(segaic16_tilemap_16b_latch_values));
			break;

		case SEGAIC16_TILEMAP_16B_ALT:
			get_text_info = segaic16_tilemap_16b_alt_text_info;
			get_tile_info = segaic16_tilemap_16b_alt_tile_info;
			info->numpages = 16;
			info->draw_layer = segaic16_tilemap_16b_draw_layer;
			info->reset = segaic16_tilemap_16b_reset;
			info->latch_timer = machine.scheduler().timer_alloc(FUNC(segaic16_tilemap_16b_latch_values));
			break;

		default:
			fatalerror("Invalid tilemap type specified in segaic16_tilemap_init\n");
	}

	/* create the tilemap for the text layer */
	info->textmap = tilemap_create(machine, get_text_info, TILEMAP_SCAN_ROWS,  8,8, 64,28);

	/* configure it */
	info->textmap_info.rambase = info->textram;
	info->textmap_info.bank = info->bank;
	info->textmap_info.banksize = info->banksize;
	info->textmap->set_user_data(&info->textmap_info);
	info->textmap->set_palette_offset(colorbase);
	info->textmap->set_transparent_pen(0);
	info->textmap->set_scrolldx(-192 + xoffs, -170 + xoffs);
	info->textmap->set_scrolldy(0, 38);

	/* create the tilemaps for the tile pages */
	for (pagenum = 0; pagenum < info->numpages; pagenum++)
	{
		/* each page is 64x32 */
		info->tilemaps[pagenum] = tilemap_create(machine, get_tile_info, TILEMAP_SCAN_ROWS,  8,8, 64,32);

		/* configure the tilemap */
		info->tmap_info[pagenum].rambase = info->tileram + pagenum * 64*32;
		info->tmap_info[pagenum].bank = info->bank;
		info->tmap_info[pagenum].banksize = info->banksize;
		info->tilemaps[pagenum]->set_user_data(&info->tmap_info[pagenum]);
		info->tilemaps[pagenum]->set_palette_offset(colorbase);
		info->tilemaps[pagenum]->set_transparent_pen(0);
		info->tilemaps[pagenum]->set_scrolldx(0, 22);
		info->tilemaps[pagenum]->set_scrolldy(0, 38);
	}
}



/*************************************
 *
 *  General tilemap rendering
 *
 *************************************/

void segaic16_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int map, int priority, int priority_mark)
{
	running_machine &machine = screen.machine();
	struct tilemap_info *info = &bg_tilemap[which];

	/* text layer is a special common case */
	if (map == SEGAIC16_TILEMAP_TEXT)
		info->textmap->draw(bitmap, cliprect, priority, priority_mark);

	/* other layers are handled differently per-system */
	else
		(*info->draw_layer)(machine, info, bitmap, cliprect, map, priority, priority_mark);
}



/*************************************
 *
 *  General tilemap reset
 *
 *************************************/

void segaic16_tilemap_reset(running_machine &machine, int which)
{
	struct tilemap_info *info = &bg_tilemap[which];

	if (info->reset)
		(*info->reset)(machine, info);
}



/*************************************
 *
 *  General tilemap banking
 *
 *************************************/

void segaic16_tilemap_set_bank(running_machine &machine, int which, int banknum, int offset)
{
	struct tilemap_info *info = &bg_tilemap[which];

	if (info->bank[banknum] != offset)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		info->bank[banknum] = offset;
		machine.tilemap().mark_all_dirty();
	}
}



/*************************************
 *
 *  General tilemap screen flipping
 *
 *************************************/

void segaic16_tilemap_set_flip(running_machine &machine, int which, int flip)
{
	struct tilemap_info *info = &bg_tilemap[which];
	int pagenum;

	flip = (flip != 0);
	if (info->flip != flip)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		info->flip = flip;
		info->textmap->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
		for (pagenum = 0; pagenum < info->numpages; pagenum++)
			info->tilemaps[pagenum]->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}
}



/*************************************
 *
 *  General tilemap row scroll enable
 *
 *************************************/

void segaic16_tilemap_set_rowscroll(running_machine &machine, int which, int enable)
{
	struct tilemap_info *info = &bg_tilemap[which];

	enable = (enable != 0);
	if (info->rowscroll != enable)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		info->rowscroll = enable;
	}
}



/*************************************
 *
 *  General tilemap column scroll enable
 *
 *************************************/

void segaic16_tilemap_set_colscroll(running_machine &machine, int which, int enable)
{
	struct tilemap_info *info = &bg_tilemap[which];

	enable = (enable != 0);
	if (info->colscroll != enable)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		info->colscroll = enable;
	}
}



/*************************************
 *
 *  General tilemap write handlers
 *
 *************************************/

WRITE16_HANDLER( segaic16_tileram_0_w )
{
	COMBINE_DATA(&segaic16_tileram_0[offset]);
	bg_tilemap[0].tilemaps[offset / (64*32)]->mark_tile_dirty(offset % (64*32));
}


WRITE16_HANDLER( segaic16_textram_0_w )
{
	/* certain ranges need immediate updates */
	if (offset >= 0xe80/2)
		space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

	COMBINE_DATA(&segaic16_textram_0[offset]);
	bg_tilemap[0].textmap->mark_tile_dirty(offset);
}





/*******************************************************************************************
 *
 *  Hang On/Space Harrier-style road chip
 *
 *  Road RAM:
 *      Offset   Bits               Usage
 *      000-1FF  ----pp-- --------  road priority versus tilemaps and sprites
 *               ------s- --------  (Hang On only) Stripe coloring enable (1=enable)
 *               ------s- --------  (Space Harrier only) Solid color fill (1=solid, 0=from ROM)
 *               -------m --------  mirror enable (1=enable)
 *               -------- iiiiiiii  index for other tables
 *               -------- rrrrrrrr  road ROM line select
 *      200-3FF  ----hhhh hhhhhhhh  horizontal scroll
 *      400-5FF  --bbbbbb --------  background color (colorset 0)
 *               -------- --bbbbbb  background color (colorset 1)
 *      600-7FF  -------- s-------  stripe color index (colorset 1)
 *               -------- -s------  stripe color index (colorset 0)
 *               -------- --a-----  pixel value 2 color index (colorset 1)
 *               -------- ---a----  pixel value 2 color index (colorset 0)
 *               -------- ----b---  pixel value 1 color index (colorset 1)
 *               -------- -----b--  pixel value 1 color index (colorset 0)
 *               -------- ------c-  pixel value 0 color index (colorset 1)
 *               -------- -------c  pixel value 0 color index (colorset 0)
 *
 *  Logic:
 *      First, the scanline is used to index into the table at 000-1FF
 *
 *      The index is taken from the low 8 bits of the table value from 000-1FF
 *
 *      The horizontal scroll value is looked up using the index in the table at
 *          200-3FF
 *
 *      The background color information is looked up using the index in the table at 400-5FF.
 *
 *      The pixel color information is looked up using the index in the table at 600-7FF.
 *
 *******************************************************************************************/

static void segaic16_road_hangon_decode(running_machine &machine, struct road_info *info)
{
	int x, y;
	const UINT8 *gfx = machine.root_device().memregion("gfx3")->base();
	int len = machine.root_device().memregion("gfx3")->bytes();

	/* allocate memory for the unpacked road data */
	info->gfx = auto_alloc_array(machine, UINT8, 256 * 512);

	/* loop over rows */
	for (y = 0; y < 256; y++)
	{
		const UINT8 *src = gfx + ((y & 0xff) * 0x40) % len;
		UINT8 *dst = info->gfx + y * 512;

		/* loop over columns */
		for (x = 0; x < 512; x++)
			dst[x] = (((src[x/8] >> (~x & 7)) & 1) << 0) | (((src[x/8 + 0x4000] >> (~x & 7)) & 1) << 1);
	}
}


static void segaic16_road_hangon_draw(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	UINT16 *roadram = info->roadram;
	int x, y;

	/* loop over scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dest = &bitmap.pix16(y);
		int control = roadram[0x000 + y];
		int hpos = roadram[0x100 + (control & 0xff)];
		int color0 = roadram[0x200 + (control & 0xff)];
		int color1 = roadram[0x300 + (control & 0xff)];
		int ff9j1, ff9j2, ctr9m, ctr9n9p, ctr9n9p_ena, ss8j, plycont;
		UINT8 *src;

		/* the PLYCONT signal controls the road layering */
		plycont = (control >> 10) & 3;

		/* skip layers we aren't supposed to be drawing */
		if ((plycont == 0 && priority != SEGAIC16_ROAD_BACKGROUND) ||
			(plycont != 0 && priority != SEGAIC16_ROAD_FOREGROUND))
			continue;

		/* compute the offset of the road graphics for this line */
		src = info->gfx + (0x000 + (control & 0xff)) * 512;

		/* initialize the 4-bit counter at 9M, which counts bits within each road byte */
		ctr9m = hpos & 7;

		/* initialize the two 4-bit counters at 9P (low) and 9N (high), which count road data bytes */
		ctr9n9p = (hpos >> 3) & 0xff;

		/* initialize the flip-flop at 9J (lower half), which controls the counting direction */
		ff9j1 = (hpos >> 11) & 1;

		/* initialize the flip-flop at 9J (upper half), which controls the background color */
		ff9j2 = 1;

		/* initialize the serial shifter at 8S, which delays several signals after we flip */
		ss8j = 0;

		/* draw this scanline from the beginning */
		for (x = -24; x <= cliprect.max_x; x++)
		{
			int md, color, select;

			/* ---- the following logic all happens constantly ---- */

			/* the enable is controlled by the value in the counter at 9M */
			ctr9n9p_ena = (ctr9m == 7);

			/* if we carried out of the 9P/9N counters, we will forcibly clear the flip-flop at 9J (lower half) */
			if ((ctr9n9p & 0xff) == 0xff)
				ff9j1 = 0;

			/* if the control word bit 8 is clear, we will forcibly set the flip-flop at 9J (lower half) */
			if (!(control & 0x100))
				ff9j1 = 1;

			/* for the Hang On/Super Hang On case only: if the control word bit 9 is clear, we will forcibly */
			/* set the flip-flip at 9J (upper half) */
			if (info->type == SEGAIC16_ROAD_HANGON && !(control & 0x200))
				ff9j2 = 1;

			/* ---- now process the pixel ---- */
			md = 3;

			/* the Space Harrier/Enduro Racer hardware has a tweak that maps the control word bit 9 to the */
			/* /CE line on the road ROM; use this to effectively disable the road data */
			if (info->type != SEGAIC16_ROAD_SHARRIER || !(control & 0x200))

				/* the /OE line on the road ROM is linked to the AND of bits 2 & 3 of the counter at 9N */
				if ((ctr9n9p & 0xc0) == 0xc0)
				{
					/* note that the pixel logic is hidden in a custom at 9S; this is just a guess */
					if (ss8j & 1)
						md = src[((ctr9n9p & 0x3f) << 3) | ctr9m];
					else
						md = src[((ctr9n9p & 0x3f) << 3) | (ctr9m ^ 7)];
				}

			/* "select" is a made-up signal that comes from bit 3 of the serial shifter and is */
			/* used in several places for color selection */
			select = (ss8j >> 3) & 1;

			/* check the flip-flop at 9J (upper half) to determine if we should use the background color; */
			/* the output of this is ANDed with M0 and M1 so it only affects pixels with a value of 3; */
			/* this is done by the AND gates at 9L and 7K */
			if (ff9j2 && md == 3)
			{
				/* in this case, the "select" signal is used to select which background color to use */
				/* since the color0 control word contains two selections */
				color = (color0 >> (select ? 0 : 8)) & 0x3f;
				color |= info->colorbase2;
			}

			/* if we're not using the background color, we select pixel data from an alternate path */
			else
			{
				/* the AND gates at 7L, 9K, and 7K clamp the pixel value to 0 if bit 7 of the color 1 */
				/* signal is 1 and if the pixel value is 3 (both M0 and M1 == 1) */
				if ((color1 & 0x80) && md == 3)
					md = 0;

				/* the pixel value plus the "select" line combine to form a mux into the low 8 bits of color1 */
				color = (color1 >> ((md << 1) | select)) & 1;

				/* this value becomes the low bit of the final color; the "select" line itself and the pixel */
				/* value form the other bits */
				color |= select << 3;
				color |= md << 1;
				color |= info->colorbase1;
			}

			/* write the pixel if we're past the minimum clip */
			if (x >= cliprect.min_x)
				dest[x] = color;

			/* ---- the following logic all happens on the 6M clock ---- */

			/* clock the counter at 9M */
			ctr9m = (ctr9m + 1) & 7;

			/* if enabled, clock on the two cascaded 4-bit counters at 9P and 9N */
			if (ctr9n9p_ena)
			{
				if (ff9j1)
					ctr9n9p++;
				else
					ctr9n9p--;
			}

			/* clock the flip-flop at 9J (upper half) */
			ff9j2 = !(!ff9j1 && (ss8j & 0x80));

			/* clock the serial shift register at 8J */
			ss8j = (ss8j << 1) | ff9j1;
		}
	}
}



/*******************************************************************************************
 *
 *  Out Run/X-Board-style road chip
 *
 *  Road control register:
 *      Bits               Usage
 *      -------- -----d--  (X-board only) Direct scanline mode (1) or indirect mode (0)
 *      -------- ------pp  Road enable/priorities:
 *                            0 = road 0 only visible
 *                            1 = both roads visible, road 0 has priority
 *                            2 = both roads visible, road 1 has priority
 *                            3 = road 1 only visible
 *
 *  Road RAM:
 *      Offset   Bits               Usage
 *      000-1FF  ----s--- --------  Road 0: Solid fill (1) or ROM fill
 *               -------- -ccccccc  Road 0: Solid color (if solid fill)
 *               -------i iiiiiiii  Road 0: Index for other tables (if in indirect mode)
 *               -------r rrrrrrr-  Road 0: Road ROM line select
 *      200-3FF  ----s--- --------  Road 1: Solid fill (1) or ROM fill
 *               -------- -ccccccc  Road 1: Solid color (if solid fill)
 *               -------i iiiiiiii  Road 1: Index for other tables (if in indirect mode)
 *               -------r rrrrrrr-  Road 1: Road ROM line select
 *      400-7FF  ----hhhh hhhhhhhh  Road 0: horizontal scroll
 *      800-BFF  ----hhhh hhhhhhhh  Road 1: horizontal scroll
 *      C00-FFF  ----bbbb --------  Background color index
 *               -------- s-------  Road 1: stripe color index
 *               -------- -a------  Road 1: pixel value 2 color index
 *               -------- --b-----  Road 1: pixel value 1 color index
 *               -------- ---c----  Road 1: pixel value 0 color index
 *               -------- ----s---  Road 0: stripe color index
 *               -------- -----a--  Road 0: pixel value 2 color index
 *               -------- ------b-  Road 0: pixel value 1 color index
 *               -------- -------c  Road 0: pixel value 0 color index
 *
 *  Logic:
 *      First, the scanline is used to index into the tables at 000-1FF/200-3FF
 *          - if solid fill, the background is filled with the specified color index
 *          - otherwise, the remaining tables are used
 *
 *      If indirect mode is selected, the index is taken from the low 9 bits of the
 *          table value from 000-1FF/200-3FF
 *      If direct scanline mode is selected, the index is set equal to the scanline
 *          for road 0, or the scanline + 256 for road 1
 *
 *      The horizontal scroll value is looked up using the index in the tables at
 *          400-7FF/800-BFF
 *
 *      The color information is looked up using the index in the table at C00-FFF. Note
 *          that the same table is used for both roads.
 *
 *
 *  Out Run road priorities are controlled by a PAL that maps as indicated below.
 *  This was used to generate the priority_map. It is assumed that X-board is the
 *  same, though this logic is locked inside a Sega custom.
 *
 *  RRC0 =  CENTA & (RDA == 3) & !RRC2
 *      | CENTB & (RDB == 3) & RRC2
 *      | (RDA == 1) & !RRC2
 *      | (RDB == 1) & RRC2
 *
 *  RRC1 =  CENTA & (RDA == 3) & !RRC2
 *      | CENTB & (RDB == 3) & RRC2
 *      | (RDA == 2) & !RRC2
 *      | (RDB == 2) & RRC2
 *
 *  RRC2 = !/HSYNC & IIQ
 *      | (CTRL == 3)
 *      | !CENTA & (RDA == 3) & !CENTB & (RDB == 3) & (CTRL == 2)
 *      | CENTB & (RDB == 3) & (CTRL == 2)
 *      | !CENTA & (RDA == 3) & !M2 & (CTRL == 2)
 *      | !CENTA & (RDA == 3) & !M3 & (CTRL == 2)
 *      | !M0 & (RDB == 0) & (CTRL == 2)
 *      | !M1 & (RDB == 0) & (CTRL == 2)
 *      | !CENTA & (RDA == 3) & CENTB & (RDB == 3) & (CTRL == 1)
 *      | !M0 & CENTB & (RDB == 3) & (CTRL == 1)
 *      | !M1 & CENTB & (RDB == 3) & (CTRL == 1)
 *      | !CENTA & M0 & (RDB == 0) & (CTRL == 1)
 *      | !CENTA & M1 & (RDB == 0) & (CTRL == 1)
 *      | !CENTA & (RDA == 3) & (RDB == 1) & (CTRL == 1)
 *      | !CENTA & (RDA == 3) & (RDB == 2) & (CTRL == 1)
 *
 *  RRC3 =  VA11 & VB11
 *      | VA11 & (CTRL == 0)
 *      | (CTRL == 3) & VB11
 *
 *  RRC4 =  !CENTA & (RDA == 3) & !CENTB & (RDB == 3)
 *      | VA11 & VB11
 *      | VA11 & (CTRL == 0)
 *      | (CTRL == 3) & VB11
 *      | !CENTB & (RDB == 3) & (CTRL == 3)
 *      | !CENTA & (RDA == 3) & (CTRL == 0)
 *
 *******************************************************************************************/

static void segaic16_road_outrun_decode(running_machine &machine, struct road_info *info)
{
	int x, y;
	const UINT8 *gfx = machine.root_device().memregion("gfx3")->base();
	int len = machine.root_device().memregion("gfx3")->bytes();

	/* allocate memory for the unpacked road data */
	info->gfx = auto_alloc_array(machine, UINT8, (256 * 2 + 1) * 512);

	/* loop over rows */
	for (y = 0; y < 256 * 2; y++)
	{
		const UINT8 *src = gfx + ((y & 0xff) * 0x40 + (y >> 8) * 0x8000) % len;
		UINT8 *dst = info->gfx + y * 512;

		/* loop over columns */
		for (x = 0; x < 512; x++)
		{
			dst[x] = (((src[x/8] >> (~x & 7)) & 1) << 0) | (((src[x/8 + 0x4000] >> (~x & 7)) & 1) << 1);

			/* pre-mark road data in the "stripe" area with a high bit */
			if (x >= 256-8 && x < 256 && dst[x] == 3)
				dst[x] |= 4;
		}
	}

	/* set up a dummy road in the last entry */
	memset(info->gfx + 256 * 2 * 512, 3, 512);
}


static void segaic16_road_outrun_draw(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	UINT16 *roadram = info->buffer;
	int x, y;

	/* loop over scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		static const UINT8 priority_map[2][8] =
		{
			{ 0x80,0x81,0x81,0x87,0,0,0,0x00 },
			{ 0x81,0x81,0x81,0x8f,0,0,0,0x80 }
//
// Original guesses from X-board priorities:
//          { 0x80,0x81,0x81,0x83,0,0,0,0x00 },
//          { 0x81,0x87,0x87,0x8f,0,0,0,0x00 }
		};
		UINT16 *dest = &bitmap.pix16(y);
		int data0 = roadram[0x000 + y];
		int data1 = roadram[0x100 + y];

		/* background case: look for solid fill scanlines */
		if (priority == SEGAIC16_ROAD_BACKGROUND)
		{
			int color = -1;

			/* based on the info->control, we can figure out which sky to draw */
			switch (info->control & 3)
			{
				case 0:
					if (data0 & 0x800)
						color = data0 & 0x7f;
					break;

				case 1:
					if (data0 & 0x800)
						color = data0 & 0x7f;
					else if (data1 & 0x800)
						color = data1 & 0x7f;
					break;

				case 2:
					if (data1 & 0x800)
						color = data1 & 0x7f;
					else if (data0 & 0x800)
						color = data0 & 0x7f;
					break;

				case 3:
					if (data1 & 0x800)
						color = data1 & 0x7f;
					break;
			}

			/* fill the scanline with color */
			if (color != -1)
			{
				color |= info->colorbase3;
				for (x = cliprect.min_x; x <= cliprect.max_x; x++)
					dest[x] = color;
			}
		}

		/* foreground case: render from ROM */
		else
		{
			int hpos0, hpos1, color0, color1;
			int control = info->control & 3;
			UINT16 color_table[32];
			UINT8 *src0, *src1;
			UINT8 bgcolor;

			/* if both roads are low priority, skip */
			if ((data0 & 0x800) && (data1 & 0x800))
				continue;

			/* get road 0 data */
			src0 = (data0 & 0x800) ? info->gfx + 256 * 2 * 512 : (info->gfx + (0x000 + ((data0 >> 1) & 0xff)) * 512);
			hpos0 = (roadram[0x200 + ((info->control & 4) ? y : (data0 & 0x1ff))]) & 0xfff;
			color0 = roadram[0x600 + ((info->control & 4) ? y : (data0 & 0x1ff))];

			/* get road 1 data */
			src1 = (data1 & 0x800) ? info->gfx + 256 * 2 * 512 : (info->gfx + (0x100 + ((data1 >> 1) & 0xff)) * 512);
			hpos1 = (roadram[0x400 + ((info->control & 4) ? (0x100 + y) : (data1 & 0x1ff))]) & 0xfff;
			color1 = roadram[0x600 + ((info->control & 4) ? (0x100 + y) : (data1 & 0x1ff))];

			/* determine the 5 colors for road 0 */
			color_table[0x00] = info->colorbase1 ^ 0x00 ^ ((color0 >> 0) & 1);
			color_table[0x01] = info->colorbase1 ^ 0x02 ^ ((color0 >> 1) & 1);
			color_table[0x02] = info->colorbase1 ^ 0x04 ^ ((color0 >> 2) & 1);
			bgcolor = (color0 >> 8) & 0xf;
			color_table[0x03] = (data0 & 0x200) ? color_table[0x00] : (info->colorbase2 ^ 0x00 ^ bgcolor);
			color_table[0x07] = info->colorbase1 ^ 0x06 ^ ((color0 >> 3) & 1);

			/* determine the 5 colors for road 1 */
			color_table[0x10] = info->colorbase1 ^ 0x08 ^ ((color1 >> 4) & 1);
			color_table[0x11] = info->colorbase1 ^ 0x0a ^ ((color1 >> 5) & 1);
			color_table[0x12] = info->colorbase1 ^ 0x0c ^ ((color1 >> 6) & 1);
			bgcolor = (color1 >> 8) & 0xf;
			color_table[0x13] = (data1 & 0x200) ? color_table[0x10] : (info->colorbase2 ^ 0x10 ^ bgcolor);
			color_table[0x17] = info->colorbase1 ^ 0x0e ^ ((color1 >> 7) & 1);

			/* draw the road */
			switch (control)
			{
				case 0:
					if (data0 & 0x800)
						continue;
					hpos0 = (hpos0 - (0x5f8 + info->xoffs)) & 0xfff;
					for (x = cliprect.min_x; x <= cliprect.max_x; x++)
					{
						int pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
						dest[x] = color_table[0x00 + pix0];
						hpos0 = (hpos0 + 1) & 0xfff;
					}
					break;

				case 1:
					hpos0 = (hpos0 - (0x5f8 + info->xoffs)) & 0xfff;
					hpos1 = (hpos1 - (0x5f8 + info->xoffs)) & 0xfff;
					for (x = cliprect.min_x; x <= cliprect.max_x; x++)
					{
						int pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
						int pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
						if ((priority_map[0][pix0] >> pix1) & 1)
							dest[x] = color_table[0x10 + pix1];
						else
							dest[x] = color_table[0x00 + pix0];
						hpos0 = (hpos0 + 1) & 0xfff;
						hpos1 = (hpos1 + 1) & 0xfff;
					}
					break;

				case 2:
					hpos0 = (hpos0 - (0x5f8 + info->xoffs)) & 0xfff;
					hpos1 = (hpos1 - (0x5f8 + info->xoffs)) & 0xfff;
					for (x = cliprect.min_x; x <= cliprect.max_x; x++)
					{
						int pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
						int pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
						if ((priority_map[1][pix0] >> pix1) & 1)
							dest[x] = color_table[0x10 + pix1];
						else
							dest[x] = color_table[0x00 + pix0];
						hpos0 = (hpos0 + 1) & 0xfff;
						hpos1 = (hpos1 + 1) & 0xfff;
					}
					break;

				case 3:
					if (data1 & 0x800)
						continue;
					hpos1 = (hpos1 - (0x5f8 + info->xoffs)) & 0xfff;
					for (x = cliprect.min_x; x <= cliprect.max_x; x++)
					{
						int pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
						dest[x] = color_table[0x10 + pix1];
						hpos1 = (hpos1 + 1) & 0xfff;
					}
					break;
			}
		}
	}
}



/*************************************
 *
 *  General road initialization
 *
 *************************************/

void segaic16_road_init(running_machine &machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs)
{
	struct road_info *info = &segaic16_road[which];

	/* reset the tilemap info */
	memset(info, 0, sizeof(*info));
	info->index = which;
	info->type = type;
	info->colorbase1 = colorbase1;
	info->colorbase2 = colorbase2;
	info->colorbase3 = colorbase3;
	info->xoffs = xoffs;

	/* set up based on which road generator */
	switch (which)
	{
		case 0:
			info->roadram = segaic16_roadram_0;
			break;

		default:
			fatalerror("Invalid road index specified in segaic16_road_init\n");
	}

	/* determine the parameters of the road */
	switch (type)
	{
		case SEGAIC16_ROAD_HANGON:
		case SEGAIC16_ROAD_SHARRIER:
			info->draw = segaic16_road_hangon_draw;
			segaic16_road_hangon_decode(machine, info);
			break;

		case SEGAIC16_ROAD_OUTRUN:
		case SEGAIC16_ROAD_XBOARD:
			info->buffer = auto_alloc_array(machine, UINT16, 0x1000/2);
			info->draw = segaic16_road_outrun_draw;
			segaic16_road_outrun_decode(machine, info);
			break;

		default:
			fatalerror("Invalid road system specified in segaic16_road_init\n");
	}
}



/*************************************
 *
 *  General road drawing
 *
 *************************************/

void segaic16_road_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	struct road_info *info = &segaic16_road[which];
	(*info->draw)(info, bitmap, cliprect, priority);
}



/*************************************
 *
 *  General road control read/write
 *
 *************************************/

READ16_HANDLER( segaic16_road_control_0_r )
{
	struct road_info *info = &segaic16_road[0];

	if (info->buffer)
	{
		UINT32 *src = (UINT32 *)info->roadram;
		UINT32 *dst = (UINT32 *)info->buffer;
		int i;

		/* swap the halves of the road RAM */
		for (i = 0; i < 0x1000/4; i++)
		{
			UINT32 temp = *src;
			*src++ = *dst;
			*dst++ = temp;
		}
	}

	return 0xffff;
}


WRITE16_HANDLER( segaic16_road_control_0_w )
{
	struct road_info *info = &segaic16_road[0];

	if (ACCESSING_BITS_0_7)
	{
		info->control = data & ((info->type == SEGAIC16_ROAD_OUTRUN) ? 3 : 7);
	}
}



/*************************************
 *
 *  General rotation initialization
 *
 *************************************/

void segaic16_rotate_init(running_machine &machine, int which, int type, int colorbase)
{
	struct rotate_info *info = &segaic16_rotate[which];

	/* reset the tilemap info */
	memset(info, 0, sizeof(*info));
	info->index = which;
	info->type = type;
	info->colorbase = colorbase;

	/* set up based on which road generator */
	switch (which)
	{
		case 0:
			info->rotateram = segaic16_rotateram_0;
			break;

		default:
			fatalerror("Invalid rotate index specified in segaic16_rotate_init\n");
	}

	/* determine the parameters of the rotate */
	switch (type)
	{
		case SEGAIC16_ROTATE_YBOARD:
			info->ramsize = 0x800;
			break;

		default:
			fatalerror("Invalid rotate system specified in segaic16_rotate_init\n");
	}

	/* allocate a buffer for swapping */
	info->buffer = auto_alloc_array(machine, UINT16, info->ramsize/2);

	state_save_register_item(machine, "segaic16_rot", NULL, which, info->colorbase);
	state_save_register_item_pointer(machine, "segaic16_rot", NULL, which, ((UINT8 *) info->buffer), info->ramsize);
}



/*************************************
 *
 *  General rotation drawing
 *
 *************************************/

void segaic16_rotate_draw(running_machine &machine, int which, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind16 &srcbitmap)
{
	struct rotate_info *info = &segaic16_rotate[which];
	INT32 currx = (info->buffer[0x3f0] << 16) | info->buffer[0x3f1];
	INT32 curry = (info->buffer[0x3f2] << 16) | info->buffer[0x3f3];
	INT32 dyy = (info->buffer[0x3f4] << 16) | info->buffer[0x3f5];
	INT32 dxx = (info->buffer[0x3f6] << 16) | info->buffer[0x3f7];
	INT32 dxy = (info->buffer[0x3f8] << 16) | info->buffer[0x3f9];
	INT32 dyx = (info->buffer[0x3fa] << 16) | info->buffer[0x3fb];
	int x, y;

	/* advance forward based on the clip rect */
	currx += dxx * (cliprect.min_x + 27) + dxy * cliprect.min_y;
	curry += dyx * (cliprect.min_x + 27) + dyy * cliprect.min_y;

	/* loop over screen Y coordinates */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dest = &bitmap.pix16(y);
		UINT16 *src = &srcbitmap.pix16(0);
		UINT8 *pri = &machine.priority_bitmap.pix8(y);
		INT32 tx = currx;
		INT32 ty = curry;

		/* loop over screen X coordinates */
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			/* fetch the pixel from the source bitmap */
			int sx = (tx >> 14) & 0x1ff;
			int sy = (ty >> 14) & 0x1ff;
			int pix = src[sy * srcbitmap.rowpixels() + sx];

			/* non-zero pixels get written; everything else is the scanline color */
			if (pix != 0xffff)
			{
				*dest++ = (pix & 0x1ff) | ((pix >> 6) & 0x200) | ((pix >> 3) & 0xc00) | 0x1000;
				*pri++ = (pix >> 8) | 1;
			}
			else
			{
				*dest++ = info->colorbase + sy;
				*pri++ = 0xff;
			}

			/* advance the source X/Y pointers */
			tx += dxx;
			ty += dyx;
		}

		/* advance the source X/Y pointers */
		currx += dxy;
		curry += dyy;
	}
}



/*************************************
 *
 *  General road control read/write
 *
 *************************************/

READ16_HANDLER( segaic16_rotate_control_0_r )
{
	struct rotate_info *info = &segaic16_rotate[0];

	if (info->buffer)
	{
		UINT32 *src = (UINT32 *)info->rotateram;
		UINT32 *dst = (UINT32 *)info->buffer;
		int i;

		/* swap the halves of the rotation RAM */
		for (i = 0; i < info->ramsize/4; i++)
		{
			UINT32 temp = *src;
			*src++ = *dst;
			*dst++ = temp;
		}
	}

	return 0xffff;
}
