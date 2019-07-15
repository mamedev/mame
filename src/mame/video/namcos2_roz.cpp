// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino

/*
    Namco System 2 ROZ Tilemap - found on Namco System 2 video board (standard type)

    based on namcoic.txt this probably consists of the following
    C102 - Controls CPU access to ROZ Memory Area.
    (anything else?)

    used by the following drivers
    namcos2.cpp (all games EXCEPT Final Lap 1,2,3 , Lucky & Wild , Steel Gunner 1,2 , Suzuka 8 Hours 1,2 , Metal Hawk)


*/

#include "emu.h"
#include "namcos2_roz.h"

static const gfx_layout layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

GFXDECODE_START( namcos2_roz_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout, 0, 16 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(NAMCOS2_ROZ, namcos2_roz_device, "namcos2_roz", "Namco Sysem 2 ROZ (C102)")

namcos2_roz_device::namcos2_roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCOS2_ROZ, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	m_rozram(*this, finder_base::DUMMY_TAG),
	m_roz_ctrl(*this, finder_base::DUMMY_TAG)
{
}

void namcos2_roz_device::device_start()
{
	m_tilemap_roz = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(namcos2_roz_device::roz_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 256, 256);
	m_tilemap_roz->set_transparent_pen(0xff);
}


TILE_GET_INFO_MEMBER(namcos2_roz_device::roz_tile_info)
{
	int tile = m_rozram[tile_index];
	SET_TILE_INFO_MEMBER(0, tile, 0/*color*/, 0);
}

struct roz_param
{
	uint32_t size;
	uint32_t startx, starty;
	int incxx, incxy, incyx, incyy;
	int color;
	int wrap;
};

static inline void
draw_roz_helper_block(const struct roz_param *rozInfo, int destx, int desty,
	int srcx, int srcy, int width, int height,
	bitmap_ind16 &destbitmap, bitmap_ind8 &flagsbitmap,
	bitmap_ind16 &srcbitmap, uint32_t size_mask)
{
	int desty_end = desty + height;

	int end_incrx = rozInfo->incyx - (width * rozInfo->incxx);
	int end_incry = rozInfo->incyy - (width * rozInfo->incxy);

	uint16_t *dest = &destbitmap.pix16(desty, destx);
	int dest_rowinc = destbitmap.rowpixels() - width;

	while (desty < desty_end)
	{
		uint16_t *dest_end = dest + width;
		while (dest < dest_end)
		{
			uint32_t xpos = (srcx >> 16);
			uint32_t ypos = (srcy >> 16);

			if (rozInfo->wrap)
			{
				xpos &= size_mask;
				ypos &= size_mask;
			}
			else if ((xpos > rozInfo->size) || (ypos >= rozInfo->size))
			{
				goto L_SkipPixel;
			}

			if (flagsbitmap.pix8(ypos, xpos) & TILEMAP_PIXEL_LAYER0)
			{
				*dest = srcbitmap.pix16(ypos, xpos) + rozInfo->color;
			}

		L_SkipPixel:

			srcx += rozInfo->incxx;
			srcy += rozInfo->incxy;
			dest++;
		}
		srcx += end_incrx;
		srcy += end_incry;
		dest += dest_rowinc;
		desty++;
	}
}

static void
draw_roz_helper(
	screen_device &screen,
	bitmap_ind16 &bitmap,
	tilemap_t *tmap,
	const rectangle &clip,
	const struct roz_param *rozInfo)
{
	tmap->set_palette_offset(rozInfo->color);

	if (bitmap.bpp() == 16)
	{
		/* On many processors, the simple approach of an outer loop over the
		    rows of the destination bitmap with an inner loop over the columns
		    of the destination bitmap has poor performance due to the order
		    that memory in the source bitmap is referenced when rotation
		    approaches 90 or 270 degrees.  The reason is that the inner loop
		    ends up reading pixels not sequentially in the source bitmap, but
		    instead at rozInfo->incxx increments, which is at its maximum at 90
		    degrees of rotation.  This means that only a few (or as few as
		    one) source pixels are in each cache line at a time.

		    Instead of the above, this code iterates in NxN blocks through the
		    destination bitmap.  This has more overhead when there is little or
		    no rotation, but much better performance when there is closer to 90
		    degrees of rotation (as long as the chunk of the source bitmap that
		    corresponds to an NxN destination block fits in cache!).

		    N is defined by ROZ_BLOCK_SIZE below; the best N is one that is as
		    big as possible but at the same time not too big to prevent all of
		    the source bitmap pixels from fitting into cache at the same time.
		    Keep in mind that the block of source pixels used can be somewhat
		    scattered in memory.  8x8 works well on the few processors that
		    were tested; 16x16 seems to work even better for more modern
		    processors with larger caches, but since 8x8 works well enough and
		    is less likely to result in cache misses on processors with smaller
		    caches, it is used.
		*/

#define ROZ_BLOCK_SIZE 8

		uint32_t size_mask = rozInfo->size - 1;
		bitmap_ind16 &srcbitmap = tmap->pixmap();
		bitmap_ind8 &flagsbitmap = tmap->flagsmap();
		uint32_t srcx = (rozInfo->startx + (clip.min_x * rozInfo->incxx) +
			(clip.min_y * rozInfo->incyx));
		uint32_t srcy = (rozInfo->starty + (clip.min_x * rozInfo->incxy) +
			(clip.min_y * rozInfo->incyy));
		int destx = clip.min_x;
		int desty = clip.min_y;

		int row_count = (clip.max_y - desty) + 1;
		int row_block_count = row_count / ROZ_BLOCK_SIZE;
		int row_extra_count = row_count % ROZ_BLOCK_SIZE;

		int column_count = (clip.max_x - destx) + 1;
		int column_block_count = column_count / ROZ_BLOCK_SIZE;
		int column_extra_count = column_count % ROZ_BLOCK_SIZE;

		int row_block_size_incxx = ROZ_BLOCK_SIZE * rozInfo->incxx;
		int row_block_size_incxy = ROZ_BLOCK_SIZE * rozInfo->incxy;
		int row_block_size_incyx = ROZ_BLOCK_SIZE * rozInfo->incyx;
		int row_block_size_incyy = ROZ_BLOCK_SIZE * rozInfo->incyy;

		int i, j;

		// Do the block rows
		for (i = 0; i < row_block_count; i++)
		{
			int sx = srcx;
			int sy = srcy;
			int dx = destx;
			// Do the block columns
			for (j = 0; j < column_block_count; j++)
			{
				draw_roz_helper_block(rozInfo, dx, desty, sx, sy, ROZ_BLOCK_SIZE,
					ROZ_BLOCK_SIZE, bitmap, flagsbitmap, srcbitmap, size_mask);
				// Increment to the next block column
				sx += row_block_size_incxx;
				sy += row_block_size_incxy;
				dx += ROZ_BLOCK_SIZE;
			}
			// Do the extra columns
			if (column_extra_count)
			{
				draw_roz_helper_block(rozInfo, dx, desty, sx, sy, column_extra_count,
					ROZ_BLOCK_SIZE, bitmap, flagsbitmap, srcbitmap, size_mask);
			}
			// Increment to the next row block
			srcx += row_block_size_incyx;
			srcy += row_block_size_incyy;
			desty += ROZ_BLOCK_SIZE;
		}
		// Do the extra rows
		if (row_extra_count)
		{
			// Do the block columns
			for (i = 0; i < column_block_count; i++)
			{
				draw_roz_helper_block(rozInfo, destx, desty, srcx, srcy, ROZ_BLOCK_SIZE,
					row_extra_count, bitmap, flagsbitmap, srcbitmap, size_mask);
				srcx += row_block_size_incxx;
				srcy += row_block_size_incxy;
				destx += ROZ_BLOCK_SIZE;
			}
			// Do the extra columns
			if (column_extra_count)
			{
				draw_roz_helper_block(rozInfo, destx, desty, srcx, srcy, column_extra_count,
					row_extra_count, bitmap, flagsbitmap, srcbitmap, size_mask);
			}
		}
	}
	else
	{
		tmap->draw_roz(screen,
			bitmap, clip,
			rozInfo->startx, rozInfo->starty,
			rozInfo->incxx, rozInfo->incxy,
			rozInfo->incyx, rozInfo->incyy,
			rozInfo->wrap, 0, 0); // wrap, flags, pri
	}
}

void namcos2_roz_device::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t gfx_ctrl)
{
	const int xoffset = 38, yoffset = 0;
	struct roz_param rozParam;

	rozParam.color = (gfx_ctrl & 0x0f00);
	rozParam.incxx = (int16_t)m_roz_ctrl[0];
	rozParam.incxy = (int16_t)m_roz_ctrl[1];
	rozParam.incyx = (int16_t)m_roz_ctrl[2];
	rozParam.incyy = (int16_t)m_roz_ctrl[3];
	rozParam.startx = (int16_t)m_roz_ctrl[4];
	rozParam.starty = (int16_t)m_roz_ctrl[5];
	rozParam.size = 2048;
	rozParam.wrap = 1;


	switch (m_roz_ctrl[7])
	{
	case 0x4400: /* (2048x2048) */
		break;

	case 0x4488: /* attract mode */
		rozParam.wrap = 0;
		break;

	case 0x44cc: /* stage1 demo */
		rozParam.wrap = 0;
		break;

	case 0x44ee: /* (256x256) used in Dragon Saber */
		rozParam.wrap = 0;
		rozParam.size = 256;
		break;
	}

	rozParam.startx <<= 4;
	rozParam.starty <<= 4;
	rozParam.startx += xoffset * rozParam.incxx + yoffset * rozParam.incyx;
	rozParam.starty += xoffset * rozParam.incxy + yoffset * rozParam.incyy;

	rozParam.startx <<= 8;
	rozParam.starty <<= 8;
	rozParam.incxx <<= 8;
	rozParam.incxy <<= 8;
	rozParam.incyx <<= 8;
	rozParam.incyy <<= 8;

	draw_roz_helper(screen, bitmap, m_tilemap_roz, cliprect, &rozParam);
}

WRITE16_MEMBER(namcos2_roz_device::rozram_word_w)
{
	COMBINE_DATA(&m_rozram[offset]);
	m_tilemap_roz->mark_tile_dirty(offset);
}

