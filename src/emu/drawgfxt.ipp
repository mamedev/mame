// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/*********************************************************************

    drawgfxt.ipp

    Template function implementing drawgfx core operations. Drivers
    can use these if they need custom behavior not provided by the
    existing drawgfx functions.

*********************************************************************/

#ifndef MAME_EMU_DRAWGFXT_IPP
#define MAME_EMU_DRAWGFXT_IPP

#pragma once


/***************************************************************************
    PIXEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    PIXEL_OP_COPY_OPAQUE - render all pixels
    regardless of pen, copying directly
-------------------------------------------------*/

#define PIXEL_OP_COPY_OPAQUE(DEST, SOURCE)                                          \
do                                                                                  \
{                                                                                   \
	(DEST) = SOURCE;                                                                \
}                                                                                   \
while (0)
#define PIXEL_OP_COPY_OPAQUE_PRIORITY(DEST, PRIORITY, SOURCE)                       \
do                                                                                  \
{                                                                                   \
	if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                                  \
		(DEST) = SOURCE;                                                            \
	(PRIORITY) = 31;                                                                \
}                                                                                   \
while (0)
#define PIXEL_OP_COPY_OPAQUE_PRIMASK(DEST, PRIORITY, SOURCE)                        \
do                                                                                  \
{                                                                                   \
	(DEST) = SOURCE;                                                                \
	(PRIORITY) = ((PRIORITY) & pmask) | pcode;                                      \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_COPY_TRANSPEN - render all pixels
    except those matching 'transpen', copying
    directly
-------------------------------------------------*/

#define PIXEL_OP_COPY_TRANSPEN(DEST, SOURCE)                                        \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
		(DEST) = SOURCE;                                                            \
}                                                                                   \
while (0)
#define PIXEL_OP_COPY_TRANSPEN_PRIORITY(DEST, PRIORITY, SOURCE)                     \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = SOURCE;                                                        \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)
#define PIXEL_OP_COPY_TRANSPEN_PRIMASK(DEST, PRIORITY, SOURCE)                      \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		(DEST) = SOURCE;                                                            \
		(PRIORITY) = ((PRIORITY) & pmask) | pcode;                                  \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_COPY_TRANSALPHA - render all pixels
    except those with an alpha of zero, copying
    directly
-------------------------------------------------*/

#define PIXEL_OP_COPY_TRANSALPHA(DEST, SOURCE)                                      \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if ((srcdata & 0xff000000) != 0)                                                \
		(DEST) = SOURCE;                                                            \
}                                                                                   \
while (0)
#define PIXEL_OP_COPY_TRANSALPHA_PRIORITY(DEST, PRIORITY, SOURCE)                   \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if ((srcdata & 0xff000000) != 0)                                                \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = SOURCE;                                                        \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)
#define PIXEL_OP_COPY_TRANSALPHA_PRIMASK(DEST, PRIORITY, SOURCE)                    \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if ((srcdata & 0xff000000) != 0)                                                \
	{                                                                               \
		(DEST) = SOURCE;                                                            \
		(PRIORITY) = ((PRIORITY) & pmask) | pcode;                                  \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REMAP_OPAQUE - render all pixels
    regardless of pen, mapping the pen via the
    'paldata' array
-------------------------------------------------*/

#define PIXEL_OP_REMAP_OPAQUE(DEST, SOURCE)                                         \
do                                                                                  \
{                                                                                   \
	(DEST) = paldata[SOURCE];                                                       \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_OPAQUE_PRIORITY(DEST, PRIORITY, SOURCE)                      \
do                                                                                  \
{                                                                                   \
	if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                                  \
		(DEST) = paldata[SOURCE];                                                   \
	(PRIORITY) = 31;                                                                \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_OPAQUE_PRIMASK(DEST, PRIORITY, SOURCE)                       \
do                                                                                  \
{                                                                                   \
	(DEST) = paldata[SOURCE];                                                       \
	(PRIORITY) = ((PRIORITY) & pmask) | pcode;                                      \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REBASE_OPAQUE - render all pixels
    regardless of pen, adding 'color' to the
    pen value
-------------------------------------------------*/

#define PIXEL_OP_REBASE_OPAQUE(DEST, SOURCE)                                        \
do                                                                                  \
{                                                                                   \
	(DEST) = color + (SOURCE);                                                      \
}                                                                                   \
while (0)
#define PIXEL_OP_REBASE_OPAQUE_PRIORITY(DEST, PRIORITY, SOURCE)                     \
do                                                                                  \
{                                                                                   \
	if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                                  \
		(DEST) = color + (SOURCE);                                                  \
	(PRIORITY) = 31;                                                                \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REMAP_TRANSPEN - render all pixels
    except those matching 'trans_pen', mapping the
    pen via the 'paldata' array
-------------------------------------------------*/

#define PIXEL_OP_REMAP_TRANSPEN(DEST, SOURCE)                                       \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
		(DEST) = paldata[srcdata];                                                  \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_TRANSPEN_PRIORITY(DEST, PRIORITY, SOURCE)                    \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = paldata[srcdata];                                              \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REBASE_TRANSPEN - render all pixels
    except those matching 'transpen', adding
    'color' to the pen value
-------------------------------------------------*/

#define PIXEL_OP_REBASE_TRANSPEN(DEST, SOURCE)                                      \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
		(DEST) = color + srcdata;                                                   \
}                                                                                   \
while (0)
#define PIXEL_OP_REBASE_TRANSPEN_PRIORITY(DEST, PRIORITY, SOURCE)                   \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = color + srcdata;                                               \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REMAP_TRANSMASK - render all pixels
    except those matching 'trans_mask', mapping the
    pen via the 'paldata' array
-------------------------------------------------*/

#define PIXEL_OP_REMAP_TRANSMASK(DEST, SOURCE)                                      \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (((trans_mask >> srcdata) & 1) == 0)                                         \
		(DEST) = paldata[srcdata];                                                  \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_TRANSMASK_PRIORITY(DEST, PRIORITY, SOURCE)                   \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (((trans_mask >> srcdata) & 1) == 0)                                         \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = paldata[srcdata];                                              \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REBASE_TRANSMASK - render all pixels
    except those matching 'trans_mask', adding
    'color' to the pen value
-------------------------------------------------*/

#define PIXEL_OP_REBASE_TRANSMASK(DEST, SOURCE)                                     \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (((trans_mask >> srcdata) & 1) == 0)                                         \
		(DEST) = color + srcdata;                                                   \
}                                                                                   \
while (0)
#define PIXEL_OP_REBASE_TRANSMASK_PRIORITY(DEST, PRIORITY, SOURCE)                  \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (((trans_mask >> srcdata) & 1) == 0)                                         \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = color + srcdata;                                               \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REBASE_TRANSTABLE - look up each pen in
    'pentable'; if the entry is DRAWMODE_NONE,
    don't draw it; if the entry is DRAWMODE_SOURCE,
    add 'color' to the pen value; if the entry is
    DRAWMODE_SHADOW, generate a shadow of the
    destination pixel using 'shadowtable'

    PIXEL_OP_REMAP_TRANSTABLE - look up each pen in
    'pentable'; if the entry is DRAWMODE_NONE,
    don't draw it; if the entry is DRAWMODE_SOURCE,
    look up the pen via the 'paldata' array; if the
    entry is DRAWMODE_SHADOW, generate a shadow of
    the destination pixel using 'shadowtable'

    If the entry is DRAWMODE_SHADOW_PRI and priority
    is used, shadow is reapplied to SOURCE entries
    drawn underneath it. Otherwise, shadows are
    presumed to be lowest priority.
-------------------------------------------------*/

#define PIXEL_OP_REBASE_TRANSTABLE16(DEST, SOURCE)                                  \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	u32 entry = pentable[srcdata];                                                  \
	if (entry != DRAWMODE_NONE)                                                     \
	{                                                                               \
		if (entry == DRAWMODE_SOURCE)                                               \
			(DEST) = color + srcdata;                                               \
		else                                                                        \
			(DEST) = shadowtable[DEST];                                             \
	}                                                                               \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_TRANSTABLE32(DEST, SOURCE)                                   \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	u32 entry = pentable[srcdata];                                                  \
	if (entry != DRAWMODE_NONE)                                                     \
	{                                                                               \
		if (entry == DRAWMODE_SOURCE)                                               \
			(DEST) = paldata[srcdata];                                              \
		else                                                                        \
			(DEST) = shadowtable[rgb_t(DEST).as_rgb15()];                           \
	}                                                                               \
}                                                                                   \
while (0)
#define PIXEL_OP_REBASE_TRANSTABLE16_PRIORITY(DEST, PRIORITY, SOURCE)               \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	u32 entry = pentable[srcdata];                                                  \
	if (entry != DRAWMODE_NONE)                                                     \
	{                                                                               \
		u8 pridata = (PRIORITY);                                                    \
		if (entry == DRAWMODE_SOURCE)                                               \
		{                                                                           \
			if (((1 << (pridata & 0x1f)) & pmask) == 0)                             \
			{                                                                       \
				if ((pridata & 0xc0) == (DRAWMODE_SHADOW_PRI << 6))                 \
					(DEST) = shadowtable[u16(color + srcdata)];                     \
				else                                                                \
					(DEST) = color + srcdata;                                       \
			}                                                                       \
			(PRIORITY) = 31;                                                        \
		}                                                                           \
		else if ((pridata & 0x80) == 0 && ((1 << (pridata & 0x1f)) & pmask) == 0)   \
		{                                                                           \
			(DEST) = shadowtable[DEST];                                             \
			(PRIORITY) = pridata | (entry << 6);                                    \
		}                                                                           \
	}                                                                               \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY(DEST, PRIORITY, SOURCE)                \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	u32 entry = pentable[srcdata];                                                  \
	if (entry != DRAWMODE_NONE)                                                     \
	{                                                                               \
		u8 pridata = (PRIORITY);                                                    \
		if (entry == DRAWMODE_SOURCE)                                               \
		{                                                                           \
			if (((1 << (pridata & 0x1f)) & pmask) == 0)                             \
			{                                                                       \
				if ((pridata & 0xc0) == (DRAWMODE_SHADOW_PRI << 6))                 \
					(DEST) = shadowtable[rgb_t(paldata[srcdata]).as_rgb15()];       \
				else                                                                \
					(DEST) = paldata[srcdata];                                      \
			}                                                                       \
			(PRIORITY) = 31;                                                        \
		}                                                                           \
		else if ((pridata & 0x80) == 0 && ((1 << (pridata & 0x1f)) & pmask) == 0)   \
		{                                                                           \
			(DEST) = shadowtable[rgb_t(DEST).as_rgb15()];                           \
			(PRIORITY) = pridata | (entry << 6);                                    \
		}                                                                           \
	}                                                                               \
}                                                                                   \
while (0)

/*-------------------------------------------------
    PIXEL_OP_REMAP_TRANSPEN_ALPHA - render all
    pixels except those matching 'transpen',
    mapping the pen to via the 'paldata' array;
    the resulting color is RGB alpha blended
    against the destination using 'alpha'
-------------------------------------------------*/

#define PIXEL_OP_REMAP_TRANSPEN_ALPHA32(DEST, SOURCE)                               \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
		(DEST) = alpha_blend_r32((DEST), paldata[srcdata], alpha_val);              \
}                                                                                   \
while (0)
#define PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY(DEST, PRIORITY, SOURCE)            \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = alpha_blend_r32((DEST), paldata[srcdata], alpha_val);          \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)


/***************************************************************************
    BASIC DRAWGFX CORE
***************************************************************************/

/*
    Input parameters:

        bitmap_t &dest - the bitmap to render to
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        gfx_element *gfx - pointer to the gfx_element to render
        u32 code - index of the entry within gfx_element
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to render to
        s32 desty - the top-left Y coordinate to render to
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/


template <typename BitmapType, typename FunctionClass>
inline void gfx_element::drawgfx_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(dest.cliprect().contains(cliprect));
		assert(code < elements());

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + width() - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = cliprect.left() - destx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + height() - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = cliprect.top() - desty;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx)
			srcx = width() - 1 - srcx;

		// apply Y flipping
		s32 dy = rowbytes();
		if (flipy)
		{
			srcy = height() - 1 - srcy;
			dy = -dy;
		}

		// fetch the source data
		const u8 *srcdata = get_data(code);

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// adjust srcdata to point to the first source pixel of the row
		srcdata += srcy * rowbytes() + srcx;

		// non-flipped 8bpp case
		if (!flipx)
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *destptr = &dest.pix(cury, destx);
				const u8 *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], srcptr[0]);
					pixel_op(destptr[1], srcptr[1]);
					pixel_op(destptr[2], srcptr[2]);
					pixel_op(destptr[3], srcptr[3]);

					srcptr += 4;
					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], srcptr[0]);
					srcptr++;
					destptr++;
				}
			}
		}

		// flipped 8bpp case
		else
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *destptr = &dest.pix(cury, destx);
				const u8 *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], srcptr[ 0]);
					pixel_op(destptr[1], srcptr[-1]);
					pixel_op(destptr[2], srcptr[-2]);
					pixel_op(destptr[3], srcptr[-3]);

					srcptr -= 4;
					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], srcptr[0]);
					srcptr--;
					destptr++;
				}
			}
		}
	} while (0);
}


template <typename BitmapType, typename PriorityType, typename FunctionClass>
inline void gfx_element::drawgfx_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, PriorityType &priority, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(priority.valid());
		assert(dest.cliprect().contains(cliprect));
		assert(code < elements());

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + width() - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = cliprect.left() - destx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + height() - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = cliprect.top() - desty;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx)
			srcx = width() - 1 - srcx;

		// apply Y flipping
		s32 dy = rowbytes();
		if (flipy)
		{
			srcy = height() - 1 - srcy;
			dy = -dy;
		}

		// fetch the source data
		const u8 *srcdata = get_data(code);

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// adjust srcdata to point to the first source pixel of the row
		srcdata += srcy * rowbytes() + srcx;

		// non-flipped 8bpp case
		if (!flipx)
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *priptr = &priority.pix(cury, destx);
				auto *destptr = &dest.pix(cury, destx);
				const u8 *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					pixel_op(destptr[1], priptr[1], srcptr[1]);
					pixel_op(destptr[2], priptr[2], srcptr[2]);
					pixel_op(destptr[3], priptr[3], srcptr[3]);

					srcptr += 4;
					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					srcptr++;
					destptr++;
					priptr++;
				}
			}
		}

		// flipped 8bpp case
		else
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *priptr = &priority.pix(cury, destx);
				auto *destptr = &dest.pix(cury, destx);
				const u8 *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[ 0]);
					pixel_op(destptr[1], priptr[1], srcptr[-1]);
					pixel_op(destptr[2], priptr[2], srcptr[-2]);
					pixel_op(destptr[3], priptr[3], srcptr[-3]);

					srcptr -= 4;
					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					srcptr--;
					destptr++;
					priptr++;
				}
			}
		}
	} while (0);
}



/***************************************************************************
    BASIC DRAWGFXZOOM CORE
***************************************************************************/

/*
    Input parameters:

        bitmap_t &dest - the bitmap to render to
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        gfx_element *gfx - pointer to the gfx_element to render
        u32 code - index of the entry within gfx_element
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to render to
        s32 desty - the top-left Y coordinate to render to
        u32 scalex - the 16.16 scale factor in the X dimension
        u32 scaley - the 16.16 scale factor in the Y dimension
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/


template <typename BitmapType, typename FunctionClass>
inline void gfx_element::drawgfxzoom_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute scaled size
		u32 dstwidth = (scalex * width() + 0x8000) >> 16;
		u32 dstheight = (scaley * height() + 0x8000) >> 16;
		if (dstwidth < 1 || dstheight < 1)
			break;

		// compute 16.16 source steps in dx and dy
		s32 dx = (width() << 16) / dstwidth;
		s32 dy = (height() << 16) / dstheight;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + dstwidth - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = (cliprect.left() - destx) * dx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + dstheight - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = (cliprect.top() - desty) * dy;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx)
		{
			srcx = (dstwidth - 1) * dx - srcx;
			dx = -dx;
		}

		// apply Y flipping
		if (flipy)
		{
			srcy = (dstheight - 1) * dy - srcy;
			dy = -dy;
		}

		// fetch the source data
		const u8 *srcdata = get_data(code);

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// iterate over pixels in Y
		for (s32 cury = desty; cury <= destendy; cury++)
		{
			auto *destptr = &dest.pix(cury, destx);
			const u8 *srcptr = srcdata + (srcy >> 16) * rowbytes();
			s32 cursrcx = srcx;
			srcy += dy;

			// iterate over unrolled blocks of 4
			for (s32 curx = 0; curx < numblocks; curx++)
			{
				pixel_op(destptr[0], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(destptr[1], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(destptr[2], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(destptr[3], srcptr[cursrcx >> 16]);
				cursrcx += dx;

				destptr += 4;
			}

			// iterate over leftover pixels
			for (s32 curx = 0; curx < leftovers; curx++)
			{
				pixel_op(destptr[0], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				destptr++;
			}
		}
	} while (0);
}


template <typename BitmapType, typename PriorityType, typename FunctionClass>
inline void gfx_element::drawgfxzoom_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, PriorityType &priority, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(priority.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute scaled size
		u32 dstwidth = (scalex * width() + 0x8000) >> 16;
		u32 dstheight = (scaley * height() + 0x8000) >> 16;
		if (dstwidth < 1 || dstheight < 1)
			break;

		// compute 16.16 source steps in dx and dy
		s32 dx = (width() << 16) / dstwidth;
		s32 dy = (height() << 16) / dstheight;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + dstwidth - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = (cliprect.left() - destx) * dx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + dstheight - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			return;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = (cliprect.top() - desty) * dy;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx)
		{
			srcx = (dstwidth - 1) * dx - srcx;
			dx = -dx;
		}

		// apply Y flipping
		if (flipy)
		{
			srcy = (dstheight - 1) * dy - srcy;
			dy = -dy;
		}

		// fetch the source data
		const u8 *srcdata = get_data(code);

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// iterate over pixels in Y
		for (s32 cury = desty; cury <= destendy; cury++)
		{
			auto *priptr = &priority.pix(cury, destx);
			auto *destptr = &dest.pix(cury, destx);
			const u8 *srcptr = srcdata + (srcy >> 16) * rowbytes();
			s32 cursrcx = srcx;
			srcy += dy;

			// iterate over unrolled blocks of 4
			for (s32 curx = 0; curx < numblocks; curx++)
			{
				pixel_op(destptr[0], priptr[0], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(destptr[1], priptr[1], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(destptr[2], priptr[2], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				pixel_op(destptr[3], priptr[3], srcptr[cursrcx >> 16]);
				cursrcx += dx;

				destptr += 4;
				priptr += 4;
			}

			// iterate over leftover pixels
			for (s32 curx = 0; curx < leftovers; curx++)
			{
				pixel_op(destptr[0], priptr[0], srcptr[cursrcx >> 16]);
				cursrcx += dx;
				destptr++;
				priptr++;
			}
		}
	} while (0);
}



/***************************************************************************
    BASIC COPYBITMAP CORE
***************************************************************************/

/*
    Input parameters:

        bitmap_t &dest - the bitmap to copy to
        bitmap_t &src - the bitmap to copy from (must be same bpp as dest)
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to copy to
        s32 desty - the top-left Y coordinate to copy to
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/

template <typename BitmapType, typename FunctionClass>
inline void copybitmap_core(BitmapType &dest, const BitmapType &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_COPYBITMAP);
	do {
		assert(dest.valid());
		assert(src.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// standard setup; dx counts bytes in X, dy counts pixels in Y
		s32 dx = 1;
		s32 dy = src.rowpixels();

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + src.width() - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = cliprect.left() - destx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + src.height() - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = cliprect.top() - desty;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx)
		{
			srcx = src.width() - 1 - srcx;
			dx = -dx;
		}

		// apply Y flipping
		if (flipy)
		{
			srcy = src.height() - 1 - srcy;
			dy = -dy;
		}

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// compute the address of the first source pixel of the first row
		const auto *srcdata = &src.pix(srcy, srcx);

		// non-flipped case
		if (!flipx)
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *destptr = &dest.pix(cury, destx);
				const auto *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], srcptr[0]);
					pixel_op(destptr[1], srcptr[1]);
					pixel_op(destptr[2], srcptr[2]);
					pixel_op(destptr[3], srcptr[3]);

					srcptr += 4;
					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], srcptr[0]);
					srcptr++;
					destptr++;
				}
			}
		}

		// flipped case
		else
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *destptr = &dest.pix(cury, destx);
				const auto *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], srcptr[ 0]);
					pixel_op(destptr[1], srcptr[-1]);
					pixel_op(destptr[2], srcptr[-2]);
					pixel_op(destptr[3], srcptr[-3]);

					srcptr -= 4;
					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], srcptr[0]);
					srcptr--;
					destptr++;
				}
			}
		}
	} while (0);
}


template <typename BitmapType, typename PriorityType, typename FunctionClass>
inline void copybitmap_core(BitmapType &dest, const BitmapType &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, PriorityType &priority, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_COPYBITMAP);
	do {
		assert(dest.valid());
		assert(src.valid());
		assert(priority.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// standard setup; dx counts bytes in X, dy counts pixels in Y
		s32 dx = 1;
		s32 dy = src.rowpixels();

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + src.width() - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = cliprect.left() - destx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + src.height() - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = cliprect.top() - desty;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		if (flipx)
		{
			srcx = src.width() - 1 - srcx;
			dx = -dx;
		}

		// apply Y flipping
		if (flipy)
		{
			srcy = src.height() - 1 - srcy;
			dy = -dy;
		}

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// compute the address of the first source pixel of the first row
		const auto *srcdata = &src.pix(srcy, srcx);

		// non-flipped case
		if (!flipx)
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *priptr = &priority.pix(cury, destx);
				auto *destptr = &dest.pix(cury, destx);
				const auto *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					pixel_op(destptr[1], priptr[1], srcptr[1]);
					pixel_op(destptr[2], priptr[2], srcptr[2]);
					pixel_op(destptr[3], priptr[3], srcptr[3]);

					srcptr += 4;
					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					srcptr++;
					destptr++;
					priptr++;
				}
			}
		}

		// flipped case
		else
		{
			// iterate over pixels in Y
			for (s32 cury = desty; cury <= destendy; cury++)
			{
				auto *priptr = &priority.pix(cury, destx);
				auto *destptr = &dest.pix(cury, destx);
				const auto *srcptr = srcdata;
				srcdata += dy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[ 0]);
					pixel_op(destptr[1], priptr[1], srcptr[-1]);
					pixel_op(destptr[2], priptr[2], srcptr[-2]);
					pixel_op(destptr[3], priptr[3], srcptr[-3]);

					srcptr -= 4;
					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					srcptr--;
					destptr++;
					priptr++;
				}
			}
		}
	} while (0);
}



/***************************************************************************
    BASIC COPYROZBITMAP CORE
***************************************************************************/

/*
    Input parameters:

        bitmap_t &dest - the bitmap to copy to
        bitmap_t &src - the bitmap to copy from (must be same bpp as dest)
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        s32 destx - the 16.16 source X position at destination pixel (0,0)
        s32 desty - the 16.16 source Y position at destination pixel (0,0)
        s32 incxx - the 16.16 amount to increment in source X for each destination X pixel
        s32 incyx - the 16.16 amount to increment in source Y for each destination X pixel
        s32 incxy - the 16.16 amount to increment in source X for each destination Y pixel
        s32 incyy - the 16.16 amount to increment in source Y for each destination Y pixel
        bool wraparound - true means wrap when hitting the edges of the source
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/

template <typename BitmapType, typename FunctionClass>
inline void copyrozbitmap_core(BitmapType &dest, const rectangle &cliprect, const BitmapType &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_COPYBITMAP);

	assert(dest.valid());
	assert(dest.valid());
	assert(dest.cliprect().contains(cliprect));
	assert(!wraparound || (src.width() & (src.width() - 1)) == 0);
	assert(!wraparound || (src.height() & (src.height() - 1)) == 0);

	// ignore empty/invalid cliprects
	if (cliprect.empty())
		return;

	// compute fixed-point 16.16 size of the source bitmap
	u32 srcfixwidth = src.width() << 16;
	u32 srcfixheight = src.height() << 16;

	// advance the starting coordinates to the top-left of the cliprect
	startx += cliprect.left() * incxx + cliprect.top() * incyx;
	starty += cliprect.left() * incxy + cliprect.top() * incyy;

	// compute how many blocks of 4 pixels we have
	u32 numblocks = cliprect.width() / 4;
	u32 leftovers = cliprect.width() - 4 * numblocks;

	// if incxy and incyx are 0, then we aren't rotating, just zooming
	if (incxy == 0 && incyx == 0)
	{
		// zoom-only, non-wraparound case
		if (!wraparound)
		{
			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *destptr = &dest.pix(cury, cliprect.left());
				s32 srcx = startx;
				s32 srcy = starty;

				starty += incyy;

				// check srcy for the whole row at once
				if (u32(srcy) < srcfixheight)
				{
					const auto *srcptr = &src.pix(srcy >> 16);

					// iterate over unrolled blocks of 4
					for (s32 curx = 0; curx < numblocks; curx++)
					{
						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[0], srcptr[srcx >> 16]);
						srcx += incxx;

						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[1], srcptr[srcx >> 16]);
						srcx += incxx;

						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[2], srcptr[srcx >> 16]);
						srcx += incxx;

						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[3], srcptr[srcx >> 16]);
						srcx += incxx;

						destptr += 4;
					}

					// iterate over leftover pixels
					for (s32 curx = 0; curx < leftovers; curx++)
					{
						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[0], srcptr[srcx >> 16]);
						srcx += incxx;
						destptr++;
					}
				}
			}
		}

		// zoom-only, wraparound case
		else
		{
			// convert srcfixwidth/height into a mask and apply
			srcfixwidth--;
			srcfixheight--;
			startx &= srcfixwidth;
			starty &= srcfixheight;

			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *destptr = &dest.pix(cury, cliprect.left());
				const auto *srcptr = &src.pix(starty >> 16);
				s32 srcx = startx;

				starty = (starty + incyy) & srcfixheight;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					pixel_op(destptr[1], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					pixel_op(destptr[2], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					pixel_op(destptr[3], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;
					destptr++;
				}
			}
		}
	}

	// full rotation case
	else
	{
		// full rotation, non-wraparound case
		if (!wraparound)
		{
			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *destptr = &dest.pix(cury, cliprect.left());
				s32 srcx = startx;
				s32 srcy = starty;

				startx += incyx;
				starty += incyy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[0], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[1], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[2], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[3], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[0], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;
					destptr++;
				}
			}
		}

		// zoom-only, wraparound case
		else
		{
			// convert srcfixwidth/height into a mask and apply
			srcfixwidth--;
			srcfixheight--;
			startx &= srcfixwidth;
			starty &= srcfixheight;

			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *destptr = &dest.pix(cury, cliprect.left());
				s32 srcx = startx;
				s32 srcy = starty;

				startx = (startx + incyx) & srcfixwidth;
				starty = (starty + incyy) & srcfixheight;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[0], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[1], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[2], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[3], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					destptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[0], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;
					destptr++;
				}
			}
		}
	}
}


template <typename BitmapType, typename PriorityType, typename FunctionClass>
inline void copyrozbitmap_core(BitmapType &dest, const rectangle &cliprect, const BitmapType &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, PriorityType &priority, FunctionClass pixel_op)
{
	auto profile = g_profiler.start(PROFILER_COPYBITMAP);

	assert(dest.valid());
	assert(dest.valid());
	assert(priority.valid());
	assert(dest.cliprect().contains(cliprect));
	assert(!wraparound || (src.width() & (src.width() - 1)) == 0);
	assert(!wraparound || (src.height() & (src.height() - 1)) == 0);

	// ignore empty/invalid cliprects
	if (cliprect.empty())
		return;

	// compute fixed-point 16.16 size of the source bitmap
	u32 srcfixwidth = src.width() << 16;
	u32 srcfixheight = src.height() << 16;

	// advance the starting coordinates to the top-left of the cliprect
	startx += cliprect.left() * incxx + cliprect.top() * incyx;
	starty += cliprect.left() * incxy + cliprect.top() * incyy;

	// compute how many blocks of 4 pixels we have
	u32 numblocks = cliprect.width() / 4;
	u32 leftovers = cliprect.width() - 4 * numblocks;

	// if incxy and incyx are 0, then we aren't rotating, just zooming
	if (incxy == 0 && incyx == 0)
	{
		// zoom-only, non-wraparound case
		if (!wraparound)
		{
			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *priptr = &priority.pix(cury, cliprect.left());
				auto *destptr = &dest.pix(cury, cliprect.left());
				s32 srcx = startx;
				s32 srcy = starty;

				starty += incyy;

				// check srcy for the whole row at once
				if (u32(srcy) < srcfixheight)
				{
					const auto *srcptr = &src.pix(srcy >> 16);

					// iterate over unrolled blocks of 4
					for (s32 curx = 0; curx < numblocks; curx++)
					{
						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[0], priptr[0], srcptr[srcx >> 16]);
						srcx += incxx;

						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[1], priptr[1], srcptr[srcx >> 16]);
						srcx += incxx;

						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[2], priptr[2], srcptr[srcx >> 16]);
						srcx += incxx;

						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[3], priptr[3], srcptr[srcx >> 16]);
						srcx += incxx;

						destptr += 4;
						priptr += 4;
					}

					// iterate over leftover pixels
					for (s32 curx = 0; curx < leftovers; curx++)
					{
						if (u32(srcx) < srcfixwidth)
							pixel_op(destptr[0], priptr[0], srcptr[srcx >> 16]);
						srcx += incxx;
						destptr++;
						priptr++;
					}
				}
			}
		}

		// zoom-only, wraparound case
		else
		{
			// convert srcfixwidth/height into a mask and apply
			srcfixwidth--;
			srcfixheight--;
			startx &= srcfixwidth;
			starty &= srcfixheight;

			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *priptr = &priority.pix(cury, cliprect.left());
				auto *destptr = &dest.pix(cury, cliprect.left());
				const auto *srcptr = &src.pix(starty >> 16);
				s32 srcx = startx;

				starty = (starty + incyy) & srcfixheight;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					pixel_op(destptr[1], priptr[1], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					pixel_op(destptr[2], priptr[2], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					pixel_op(destptr[3], priptr[3], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;

					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					pixel_op(destptr[0], priptr[0], srcptr[srcx >> 16]);
					srcx = (srcx + incxx) & srcfixwidth;
					destptr++;
					priptr++;
				}
			}
		}
	}

	// full rotation case
	else
	{
		// full rotation, non-wraparound case
		if (!wraparound)
		{
			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *priptr = &priority.pix(cury, cliprect.left());
				auto *destptr = &dest.pix(cury, cliprect.left());
				s32 srcx = startx;
				s32 srcy = starty;

				startx += incyx;
				starty += incyy;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[0], priptr[0], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[1], priptr[1], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[2], priptr[2], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[3], priptr[3], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;

					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)
					{
						const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
						pixel_op(destptr[0], priptr[0], srcptr[0]);
					}
					srcx += incxx;
					srcy += incxy;
					destptr++;
					priptr++;
				}
			}
		}

		// zoom-only, wraparound case
		else
		{
			// convert srcfixwidth/height into a mask and apply
			srcfixwidth--;
			srcfixheight--;
			startx &= srcfixwidth;
			starty &= srcfixheight;

			// iterate over pixels in Y
			for (s32 cury = cliprect.top(); cury <= cliprect.bottom(); cury++)
			{
				auto *priptr = &priority.pix(cury, cliprect.left());
				auto *destptr = &dest.pix(cury, cliprect.left());
				s32 srcx = startx;
				s32 srcy = starty;

				startx = (startx + incyx) & srcfixwidth;
				starty = (starty + incyy) & srcfixheight;

				// iterate over unrolled blocks of 4
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[1], priptr[1], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[2], priptr[2], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[3], priptr[3], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;

					destptr += 4;
					priptr += 4;
				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					const auto *srcptr = &src.pix(srcy >> 16, srcx >> 16);
					pixel_op(destptr[0], priptr[0], srcptr[0]);
					srcx = (srcx + incxx) & srcfixwidth;
					srcy = (srcy + incxy) & srcfixheight;
					destptr++;
					priptr++;
				}
			}
		}
	}
}



/***************************************************************************
    BASIC DRAWSCANLINE CORE
***************************************************************************/

/*
    Input parameters:

        bitmap_t &bitmap - the bitmap to copy to
        s32 destx - the X coordinate to copy to
        s32 desty - the Y coordinate to copy to
        s32 length - the total number of pixels to copy
        const UINTx *srcptr - pointer to memory containing the source pixels
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/

template <typename BitmapType, typename SourceType, typename FunctionClass>
inline void drawscanline_core(BitmapType &bitmap, s32 destx, s32 desty, s32 length, const SourceType *srcptr, FunctionClass pixel_op)
{
	assert(bitmap.valid());
	assert(destx >= 0);
	assert(destx + length <= bitmap.width());
	assert(desty >= 0);
	assert(desty < bitmap.height());
	assert(srcptr != nullptr);

	auto *destptr = &bitmap.pix(desty, destx);

	// iterate over unrolled blocks of 4
	while (length >= 4)
	{
		pixel_op(destptr[0], srcptr[0]);
		pixel_op(destptr[1], srcptr[1]);
		pixel_op(destptr[2], srcptr[2]);
		pixel_op(destptr[3], srcptr[3]);

		length -= 4;
		srcptr += 4;
		destptr += 4;
	}

	// iterate over leftover pixels
	while (length-- > 0)
	{
		pixel_op(destptr[0], srcptr[0]);
		srcptr++;
		destptr++;
	}
}


template <typename BitmapType, typename SourceType, typename PriorityType, typename FunctionClass>
inline void drawscanline_core(BitmapType &bitmap, s32 destx, s32 desty, s32 length, const SourceType *srcptr, PriorityType &priority, FunctionClass pixel_op)
{
	assert(bitmap.valid());
	assert(destx >= 0);
	assert(destx + length <= bitmap.width());
	assert(desty >= 0);
	assert(desty < bitmap.height());
	assert(srcptr != nullptr);
	assert(priority.valid());

	auto *priptr = &priority.pix(desty, destx);
	auto *destptr = &bitmap.pix(desty, destx);

	// iterate over unrolled blocks of 4
	while (length >= 4)
	{
		pixel_op(destptr[0], priptr[0], srcptr[0]);
		pixel_op(destptr[1], priptr[1], srcptr[1]);
		pixel_op(destptr[2], priptr[2], srcptr[2]);
		pixel_op(destptr[3], priptr[3], srcptr[3]);

		length -= 4;
		srcptr += 4;
		destptr += 4;
		priptr += 4;
	}

	// iterate over leftover pixels
	while (length-- > 0)
	{
		pixel_op(destptr[0], priptr[0], srcptr[0]);
		srcptr++;
		destptr++;
		priptr++;
	}
}


#endif  // MAME_EMU_DRAWGFXT_IPP
