/***************************************************************************

    P&P Marketing Police Trainer hardware

***************************************************************************/

#include "driver.h"
#include "cpu/mips/r3000.h"
#include "policetr.h"


/* constants */
#define SRCBITMAP_WIDTH		4096

#define DSTBITMAP_WIDTH		512
#define DSTBITMAP_HEIGHT	256


/* local variables */
static UINT32 palette_offset;
static UINT8 palette_index;
static UINT8 palette_data[3];

static rectangle render_clip;
static UINT8 *srcbitmap;
static UINT8 *dstbitmap;

static UINT16 src_xoffs, src_yoffs;
static UINT16 dst_xoffs, dst_yoffs;
static UINT8 video_latch;

static UINT32 srcbitmap_height_mask;



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( policetr )
{
	/* the source bitmap is in ROM */
	srcbitmap = memory_region(REGION_GFX1);

	/* compute the height */
	srcbitmap_height_mask = (memory_region_length(REGION_GFX1) / SRCBITMAP_WIDTH) - 1;

	/* the destination bitmap is not directly accessible to the CPU */
	dstbitmap = auto_malloc(DSTBITMAP_WIDTH * DSTBITMAP_HEIGHT);
}



/*************************************
 *
 *  Display list processor
 *
 *************************************/

static void render_display_list(offs_t offset)
{
	/* mask against the R3000 address space */
	offset &= 0x1fffffff;

	/* loop over all items */
	while (offset != 0x1fffffff)
	{
		UINT32 *entry = &policetr_rambase[offset / 4];
		UINT32 srcx = entry[0] & 0xfffffff;
		UINT32 srcy = entry[1] & ((srcbitmap_height_mask << 16) | 0xffff);
		UINT32 srcxstep = entry[2];
		UINT32 srcystep = entry[3];
		int dstw = (entry[4] & 0x1ff) + 1;
		int dsth = ((entry[4] >> 12) & 0x1ff) + 1;
		int dstx = entry[5] & 0x1ff;
		int dsty = (entry[5] >> 12) & 0x1ff;
		UINT8 mask = ~entry[6] >> 16;
		UINT8 color = (entry[6] >> 24) & ~mask;
		UINT32 curx, cury;
		int x, y;

		if (dstx > render_clip.max_x)
		{
			dstw -= (512 - dstx);
			dstx = 0;
		}
		/* apply X clipping */
		if (dstx < render_clip.min_x)
		{
			srcx += srcxstep * (render_clip.min_x - dstx);
			dstw -= render_clip.min_x - dstx;
			dstx = render_clip.min_x;
		}
		if (dstx + dstw > render_clip.max_x)
			dstw = render_clip.max_x - dstx + 1;

		/* apply Y clipping */
		if (dsty < render_clip.min_y)
		{
			srcy += srcystep * (render_clip.min_y - dsty);
			dsth -= render_clip.min_y - dsty;
			dsty = render_clip.min_y;
		}
		if (dsty + dsth > render_clip.max_y)
			dsth = render_clip.max_y - dsty + 1;

		/* special case for fills */
		if (srcxstep == 0 && srcystep == 0)
		{
			/* prefetch the pixel */
			UINT8 pixel = srcbitmap[((srcy >> 16) * srcbitmap_height_mask) * SRCBITMAP_WIDTH + (srcx >> 16) % SRCBITMAP_WIDTH];
			pixel = color | (pixel & mask);

			/* loop over rows and columns */
			if (dstw > 0)
				for (y = 0; y < dsth; y++)
				{
					UINT8 *dst = &dstbitmap[(dsty + y) * DSTBITMAP_WIDTH + dstx];
					memset(dst, pixel, dstw);
				}
		}

		/* otherwise, standard render */
		else
		{
			/* loop over rows */
			for (y = 0, cury = srcy; y < dsth; y++, cury += srcystep)
			{
				UINT8 *src = &srcbitmap[((cury >> 16) & srcbitmap_height_mask) * SRCBITMAP_WIDTH];
				UINT8 *dst = &dstbitmap[(dsty + y) * DSTBITMAP_WIDTH + dstx];

				/* loop over columns */
				for (x = 0, curx = srcx; x < dstw; x++, curx += srcxstep)
				{
					UINT8 pixel = src[(curx >> 16) % SRCBITMAP_WIDTH];
					if (pixel)
						dst[x] = color | (pixel & mask);
				}
			}
		}

		/* advance to the next link */
		offset = entry[7] & 0x1fffffff;
	}
}



/*************************************
 *
 *  Video controller writes
 *
 *************************************/

WRITE32_HANDLER( policetr_video_w )
{
	/* we assume 4-byte accesses */
	if (mem_mask)
		logerror("%08X: policetr_video_w access with mask %08X\n", activecpu_get_previouspc(), ~mem_mask);

	/* 4 offsets */
	switch (offset)
	{
		/* offset 0 specifies the start address of a display list */
		case 0:
			render_display_list(data);
			break;

		/* offset 1 specifies a latch value in the upper 8 bits */
		case 1:
			video_latch = data >> 24;
			break;

		/* offset 2 has various meanings based on the latch */
		case 2:
		{
			switch (video_latch)
			{
				/* latch 0x04 specifies the source X offset for a source bitmap pixel read */
				case 0x04:
					src_xoffs = data >> 16;
					break;

				/* latch 0x14 specifies the source Y offset for a source bitmap pixel read */
				case 0x14:
					src_yoffs = data >> 16;
					break;

				/* latch 0x20 specifies the top/left corners of the render cliprect */
				case 0x20:
					render_clip.min_y = (data >> 12) & 0xfff;
					render_clip.min_x = data & 0xfff;
					break;

				/* latch 0x30 specifies the bottom/right corners of the render cliprect */
				case 0x30:
					render_clip.max_y = (data >> 12) & 0xfff;
					render_clip.max_x = data & 0xfff;
					break;

				/* latch 0x50 allows a direct write to the destination bitmap */
				case 0x50:
					if (ACCESSING_MSB32 && dst_xoffs < DSTBITMAP_WIDTH && dst_yoffs < DSTBITMAP_HEIGHT)
						dstbitmap[dst_yoffs * DSTBITMAP_WIDTH + dst_xoffs] = data >> 24;
					break;

				/* log anything else */
				default:
					logerror("%08X: policetr_video_w(2) = %08X & %08X with latch %02X\n", activecpu_get_previouspc(), data, ~mem_mask, video_latch);
					break;
			}
			break;
		}

		/* offset 3 has various meanings based on the latch */
		case 3:
		{
			switch (video_latch)
			{
				/* latch 0x00 is unknown; 0, 1, and 2 get written into the upper 12 bits before rendering */
				case 0x00:
					if (data != (0 << 20) && data != (1 << 20) && data != (2 << 20))
						logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", activecpu_get_previouspc(), data, ~mem_mask, video_latch);
					break;

				/* latch 0x10 specifies destination bitmap X and Y offsets */
				case 0x10:
					dst_yoffs = (data >> 12) & 0xfff;
					dst_xoffs = data & 0xfff;
					break;

				/* latch 0x20 is unknown; either 0xef or 0x100 is written every IRQ4 */
				case 0x20:
					if (data != (0x100 << 12) && data != (0xef << 12))
						logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", activecpu_get_previouspc(), data, ~mem_mask, video_latch);
					break;

				/* latch 0x40 is unknown; a 0 is written every IRQ4 */
				case 0x40:
					if (data != 0)
						logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", activecpu_get_previouspc(), data, ~mem_mask, video_latch);
					break;

				/* latch 0x50 clears IRQ4 */
				case 0x50:
					cpunum_set_input_line(0, R3000_IRQ4, CLEAR_LINE);
					break;

				/* latch 0x60 clears IRQ5 */
				case 0x60:
					cpunum_set_input_line(0, R3000_IRQ5, CLEAR_LINE);
					break;

				/* log anything else */
				default:
					logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", activecpu_get_previouspc(), data, ~mem_mask, video_latch);
					break;
			}
			break;
		}
	}
}



/*************************************
 *
 *  Video controller reads
 *
 *************************************/

READ32_HANDLER( policetr_video_r )
{
	int inputval;

	/* the value read is based on the latch */
	switch (video_latch)
	{
		/* latch 0x00 is player 1's gun X coordinate */
		case 0x00:
			inputval = ((readinputport(3) & 0xff) * Machine->screen[0].width) >> 8;
			inputval += 0x50;
			return (inputval << 20) | 0x20000000;

		/* latch 0x01 is player 1's gun Y coordinate */
		case 0x01:
			inputval = ((readinputport(4) & 0xff) * Machine->screen[0].height) >> 8;
			inputval += 0x17;
			return (inputval << 20);

		/* latch 0x02 is player 2's gun X coordinate */
		case 0x02:
			inputval = ((readinputport(5) & 0xff) * Machine->screen[0].width) >> 8;
			inputval += 0x50;
			return (inputval << 20) | 0x20000000;

		/* latch 0x03 is player 2's gun Y coordinate */
		case 0x03:
			inputval = ((readinputport(6) & 0xff) * Machine->screen[0].height) >> 8;
			inputval += 0x17;
			return (inputval << 20);

		/* latch 0x04 is the pixel value in the ROM at the specified address */
		case 0x04:
			return srcbitmap[(src_yoffs & srcbitmap_height_mask) * SRCBITMAP_WIDTH + src_xoffs % SRCBITMAP_WIDTH] << 24;

		/* latch 0x50 is read at IRQ 4; the top 2 bits are checked. If they're not 0,
            they skip the rest of the interrupt processing */
		case 0x50:
			return 0;
	}

	/* log anything else */
	logerror("%08X: policetr_video_r with latch %02X\n", activecpu_get_previouspc(), video_latch);
	return 0;
}




/*************************************
 *
 *  Palette access
 *
 *************************************/

WRITE32_HANDLER( policetr_palette_offset_w )
{
	if (!(mem_mask & 0x00ff0000))
	{
		palette_offset = (data >> 16) & 0xff;
		palette_index = 0;
	}
}


WRITE32_HANDLER( policetr_palette_data_w )
{
	if (!(mem_mask & 0x00ff0000))
	{
		palette_data[palette_index] = (data >> 16) & 0xff;
		if (++palette_index == 3)
		{
			palette_set_color(Machine, palette_offset, MAKE_RGB(palette_data[0], palette_data[1], palette_data[2]));
			palette_index = 0;
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( policetr )
{
	int width = cliprect->max_x - cliprect->min_x + 1;
	int y;

	/* render all the scanlines from the dstbitmap to MAME's bitmap */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		draw_scanline8(bitmap, cliprect->min_x, y, width, &dstbitmap[DSTBITMAP_WIDTH * y + cliprect->min_x], NULL, -1);

	return 0;
}
