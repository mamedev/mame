// license:BSD-3-Clause
// copyright-holders:Aaron Giles


#include "emu.h"
#include "segaic16_road.h"
#include "video/resnet.h"

const device_type SEGAIC16_ROAD = &device_creator<segaic16_road_device>;

segaic16_road_device::segaic16_road_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGAIC16_ROAD, "Sega 16-bit Road Generator", tag, owner, clock, "segaic16_road", __FILE__)
{
}


void segaic16_road_device::device_config_complete()
{
}

void segaic16_road_device::device_start()
{
}

void segaic16_road_device::device_reset()
{
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



void segaic16_road_device::segaic16_road_hangon_decode(running_machine &machine, struct road_info *info)
{
	int x, y;
	const UINT8 *gfx = memregion("^gfx3")->base();
	int len = memregion("^gfx3")->bytes();

	/* allocate memory for the unpacked road data */
	info->gfx = std::make_unique<UINT8[]>(256 * 512);

	/* loop over rows */
	for (y = 0; y < 256; y++)
	{
		const UINT8 *src = gfx + ((y & 0xff) * 0x40) % len;
		UINT8 *dst = info->gfx.get() + y * 512;

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
		src = info->gfx.get() + (0x000 + (control & 0xff)) * 512;

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

void segaic16_road_device::segaic16_road_outrun_decode(running_machine &machine, struct road_info *info)
{
	int x, y;
	const UINT8 *gfx = memregion("^gfx3")->base();
	int len = memregion("^gfx3")->bytes();

	/* allocate memory for the unpacked road data */
	info->gfx = std::make_unique<UINT8[]>((256 * 2 + 1) * 512);

	/* loop over rows */
	for (y = 0; y < 256 * 2; y++)
	{
		const UINT8 *src = gfx + ((y & 0xff) * 0x40 + (y >> 8) * 0x8000) % len;
		UINT8 *dst = info->gfx.get() + y * 512;

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
	memset(info->gfx.get() + 256 * 2 * 512, 3, 512);
}


static void segaic16_road_outrun_draw(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	UINT16 *roadram = info->buffer.get();
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
			src0 = (data0 & 0x800) ? info->gfx.get() + 256 * 2 * 512 : (info->gfx.get() + (0x000 + ((data0 >> 1) & 0xff)) * 512);
			hpos0 = (roadram[0x200 + ((info->control & 4) ? y : (data0 & 0x1ff))]) & 0xfff;
			color0 = roadram[0x600 + ((info->control & 4) ? y : (data0 & 0x1ff))];

			/* get road 1 data */
			src1 = (data1 & 0x800) ? info->gfx.get() + 256 * 2 * 512 : (info->gfx.get() + (0x100 + ((data1 >> 1) & 0xff)) * 512);
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

void segaic16_road_device::segaic16_road_init(running_machine &machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs)
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
			info->buffer = std::make_unique<UINT16[]>(0x1000/2);
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

void segaic16_road_device::segaic16_road_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	struct road_info *info = &segaic16_road[which];
	(*info->draw)(info, bitmap, cliprect, priority);
}



/*************************************
 *
 *  General road control read/write
 *
 *************************************/

READ16_MEMBER( segaic16_road_device::segaic16_road_control_0_r )
{
	struct road_info *info = &segaic16_road[0];

	if (info->buffer)
	{
		UINT32 *src = (UINT32 *)info->roadram;
		UINT32 *dst = (UINT32 *)info->buffer.get();
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


WRITE16_MEMBER( segaic16_road_device::segaic16_road_control_0_w )
{
	struct road_info *info = &segaic16_road[0];

	if (ACCESSING_BITS_0_7)
	{
		info->control = data & ((info->type == SEGAIC16_ROAD_OUTRUN) ? 3 : 7);
	}
}
