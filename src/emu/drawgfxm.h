// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/*********************************************************************

    drawgfxm.h

    Macros implementing drawgfx core operations. Drivers can use
    these if they need custom behavior not provided by the existing
    drawgfx functions.
**********************************************************************

    How to use these macros:

    There are two sets of macros. The PIXEL_OP* macros are simple
    per-pixel operations, designed to take a SOURCE pixel and
    copy it to the DEST, perhaps updating the PRIORITY pixel as
    well. On their own, they are not particularly useful.

    The second set of macros represents the core gfx/bitmap walking
    and rendering code. These macros generally take the target pixel
    type (u8, u16, u32), one of the PIXEL_OP* macros,
    and a priority bitmap pixel type (u8, u16, u32, or the
    special type NO_PRIORITY).

    Although the code may look inefficient at first, the compiler is
    able to easily optimize out unused cases due to the way the
    macros are written, leaving behind just the cases we are
    interested in.

    The general approach for using these macros is:

    my_drawing_function(params)
    {
        // ensure that all the required parameters for the mega
        // macro are defined (if they are not needed, just declare
        // the variables and set them equal to a known constant
        // value to help the compiler)

        // set up any additional variables needed by the PIXEL_OP*
        // macro you want to use (each macro has its own
        // requirements)

        MEGA_MACRO(BITMAP_TYPE, PIXEL_OP, PRIORITY_TYPE);
    }

*********************************************************************/

#pragma once

#ifndef __DRAWGFXM_H__
#define __DRAWGFXM_H__

/* special priority type meaning "none" */
struct NO_PRIORITY { char dummy[3]; };

extern bitmap_ind8 drawgfx_dummy_priority_bitmap;
#define DECLARE_NO_PRIORITY bitmap_t &priority = drawgfx_dummy_priority_bitmap;


/* macros for using the optional priority */
#define PRIORITY_VALID(x)       (sizeof(x) != sizeof(NO_PRIORITY))
#define PRIORITY_ADDR(p,t,y,x)  (PRIORITY_VALID(t) ? (&(p).pixt<t>(y, x)) : nullptr)
#define PRIORITY_ADVANCE(t,p,i) do { if (PRIORITY_VALID(t)) (p) += (i); } while (0)


/***************************************************************************
    PIXEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    PIXEL_OP_COPY_OPAQUE - render all pixels
    regardless of pen, copying directly
-------------------------------------------------*/

#define PIXEL_OP_COPY_OPAQUE(DEST, PRIORITY, SOURCE)                                \
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

#define PIXEL_OP_COPY_TRANSPEN(DEST, PRIORITY, SOURCE)                              \
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

#define PIXEL_OP_COPY_TRANSALPHA(DEST, PRIORITY, SOURCE)                            \
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

#define PIXEL_OP_REMAP_OPAQUE(DEST, PRIORITY, SOURCE)                               \
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

#define PIXEL_OP_REBASE_OPAQUE(DEST, PRIORITY, SOURCE)                              \
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

#define PIXEL_OP_REMAP_TRANSPEN(DEST, PRIORITY, SOURCE)                             \
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

#define PIXEL_OP_REBASE_TRANSPEN(DEST, PRIORITY, SOURCE)                            \
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

#define PIXEL_OP_REMAP_TRANSMASK(DEST, PRIORITY, SOURCE)                            \
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

#define PIXEL_OP_REBASE_TRANSMASK(DEST, PRIORITY, SOURCE)                           \
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
-------------------------------------------------*/

#define PIXEL_OP_REBASE_TRANSTABLE16(DEST, PRIORITY, SOURCE)                        \
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
#define PIXEL_OP_REMAP_TRANSTABLE32(DEST, PRIORITY, SOURCE)                         \
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
				(DEST) = color + srcdata;                                           \
			(PRIORITY) = 31;                                                        \
		}                                                                           \
		else if ((pridata & 0x80) == 0 && ((1 << (pridata & 0x1f)) & pmask) == 0)   \
		{                                                                           \
			(DEST) = shadowtable[DEST];                                             \
			(PRIORITY) = pridata | 0x80;                                            \
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
				(DEST) = paldata[srcdata];                                          \
			(PRIORITY) = 31;                                                        \
		}                                                                           \
		else if ((pridata & 0x80) == 0 && ((1 << (pridata & 0x1f)) & pmask) == 0)   \
		{                                                                           \
			(DEST) = shadowtable[rgb_t(DEST).as_rgb15()];                           \
			(PRIORITY) = pridata | 0x80;                                            \
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

#define PIXEL_OP_REMAP_TRANSPEN_ALPHA32(DEST, PRIORITY, SOURCE)                     \
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
    Assumed input parameters or local variables:

        bitmap_t &dest - the bitmap to render to
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        gfx_element *gfx - pointer to the gfx_element to render
        u32 code - index of the entry within gfx_element
        u32 color - index of the color within gfx_element
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to render to
        s32 desty - the top-left Y coordinate to render to
        bitmap_t &priority - the priority bitmap (even if PRIORITY_TYPE is NO_PRIORITY, at least needs a dummy)
*/


#define DRAWGFX_CORE(PIXEL_TYPE, PIXEL_OP, PRIORITY_TYPE)                           \
do {                                                                                \
	g_profiler.start(PROFILER_DRAWGFX);                                             \
	do {                                                                            \
		const u8 *srcdata;                                                          \
		s32 destendx, destendy;                                                     \
		s32 srcx, srcy;                                                             \
		s32 curx, cury;                                                             \
		s32 dy;                                                                     \
																					\
		assert(dest.valid());                                                       \
		assert(!PRIORITY_VALID(PRIORITY_TYPE) || priority.valid());                 \
		assert(dest.cliprect().contains(cliprect));                                 \
		assert(code < elements());                                                  \
																					\
		/* ignore empty/invalid cliprects */                                        \
		if (cliprect.empty())                                                       \
			break;                                                                  \
																					\
		/* compute final pixel in X and exit if we are entirely clipped */          \
		destendx = destx + width() - 1;                                             \
		if (destx > cliprect.right() || destendx < cliprect.left())                 \
			break;                                                                  \
																					\
		/* apply left clip */                                                       \
		srcx = 0;                                                                   \
		if (destx < cliprect.left())                                                \
		{                                                                           \
			srcx = cliprect.left() - destx;                                         \
			destx = cliprect.left();                                                \
		}                                                                           \
																					\
		/* apply right clip */                                                      \
		if (destendx > cliprect.right())                                            \
			destendx = cliprect.right();                                            \
																					\
		/* compute final pixel in Y and exit if we are entirely clipped */          \
		destendy = desty + height() - 1;                                            \
		if (desty > cliprect.bottom() || destendy < cliprect.top())                 \
			break;                                                                  \
																					\
		/* apply top clip */                                                        \
		srcy = 0;                                                                   \
		if (desty < cliprect.top())                                                 \
		{                                                                           \
			srcy = cliprect.top() - desty;                                          \
			desty = cliprect.top();                                                 \
		}                                                                           \
																					\
		/* apply bottom clip */                                                     \
		if (destendy > cliprect.bottom())                                           \
			destendy = cliprect.bottom();                                           \
																					\
		/* apply X flipping */                                                      \
		if (flipx)                                                                  \
			srcx = width() - 1 - srcx;                                              \
																					\
		/* apply Y flipping */                                                      \
		dy = rowbytes();                                                            \
		if (flipy)                                                                  \
		{                                                                           \
			srcy = height() - 1 - srcy;                                             \
			dy = -dy;                                                               \
		}                                                                           \
																					\
		/* fetch the source data */                                                 \
		srcdata = get_data(code);                                                   \
																					\
		/* compute how many blocks of 4 pixels we have */                           \
		u32 numblocks = (destendx + 1 - destx) / 4;                                 \
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;                     \
																					\
		/* adjust srcdata to point to the first source pixel of the row */          \
		srcdata += srcy * rowbytes() + srcx;                                        \
																					\
		/* non-flipped 8bpp case */                                                 \
		if (!flipx)                                                                 \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = desty; cury <= destendy; cury++)                            \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, destx); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, destx);          \
				const u8 *srcptr = srcdata;                                         \
				srcdata += dy;                                                      \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					PIXEL_OP(destptr[1], priptr[1], srcptr[1]);                     \
					PIXEL_OP(destptr[2], priptr[2], srcptr[2]);                     \
					PIXEL_OP(destptr[3], priptr[3], srcptr[3]);                     \
																					\
					srcptr += 4;                                                    \
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					srcptr++;                                                       \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
																					\
		/* flipped 8bpp case */                                                     \
		else                                                                        \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = desty; cury <= destendy; cury++)                            \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, destx); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, destx);          \
				const u8 *srcptr = srcdata;                                         \
				srcdata += dy;                                                      \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[ 0]);                    \
					PIXEL_OP(destptr[1], priptr[1], srcptr[-1]);                    \
					PIXEL_OP(destptr[2], priptr[2], srcptr[-2]);                    \
					PIXEL_OP(destptr[3], priptr[3], srcptr[-3]);                    \
																					\
					srcptr -= 4;                                                    \
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					srcptr--;                                                       \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	} while (0);                                                                    \
	g_profiler.stop();                                                              \
} while (0)



/***************************************************************************
    BASIC DRAWGFXZOOM CORE
***************************************************************************/

/*
    Assumed input parameters or local variables:

        bitmap_t &dest - the bitmap to render to
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        gfx_element *gfx - pointer to the gfx_element to render
        u32 code - index of the entry within gfx_element
        u32 color - index of the color within gfx_element
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to render to
        s32 desty - the top-left Y coordinate to render to
        u32 scalex - the 16.16 scale factor in the X dimension
        u32 scaley - the 16.16 scale factor in the Y dimension
        bitmap_t &priority - the priority bitmap (even if PRIORITY_TYPE is NO_PRIORITY, at least needs a dummy)
*/


#define DRAWGFXZOOM_CORE(PIXEL_TYPE, PIXEL_OP, PRIORITY_TYPE)                       \
do {                                                                                \
	g_profiler.start(PROFILER_DRAWGFX);                                             \
	do {                                                                            \
		const u8 *srcdata;                                                          \
		u32 dstwidth, dstheight;                                                    \
		s32 destendx, destendy;                                                     \
		s32 srcx, srcy;                                                             \
		s32 curx, cury;                                                             \
		s32 dx, dy;                                                                 \
																					\
		assert(dest.valid());                                                       \
		assert(!PRIORITY_VALID(PRIORITY_TYPE) || priority.valid());                 \
		assert(dest.cliprect().contains(cliprect));                                 \
																					\
		/* ignore empty/invalid cliprects */                                        \
		if (cliprect.empty())                                                       \
			break;                                                                  \
																					\
		/* compute scaled size */                                                   \
		dstwidth = (scalex * width() + 0x8000) >> 16;                               \
		dstheight = (scaley * height() + 0x8000) >> 16;                             \
		if (dstwidth < 1 || dstheight < 1)                                          \
			break;                                                                  \
																					\
		/* compute 16.16 source steps in dx and dy */                               \
		dx = (width() << 16) / dstwidth;                                            \
		dy = (height() << 16) / dstheight;                                          \
																					\
		/* compute final pixel in X and exit if we are entirely clipped */          \
		destendx = destx + dstwidth - 1;                                            \
		if (destx > cliprect.right() || destendx < cliprect.left())                 \
			break;                                                                  \
																					\
		/* apply left clip */                                                       \
		srcx = 0;                                                                   \
		if (destx < cliprect.left())                                                \
		{                                                                           \
			srcx = (cliprect.left() - destx) * dx;                                  \
			destx = cliprect.left();                                                \
		}                                                                           \
																					\
		/* apply right clip */                                                      \
		if (destendx > cliprect.right())                                            \
			destendx = cliprect.right();                                            \
																					\
		/* compute final pixel in Y and exit if we are entirely clipped */          \
		destendy = desty + dstheight - 1;                                           \
		if (desty > cliprect.bottom() || destendy < cliprect.top())                 \
		{                                                                           \
			g_profiler.stop();                                                      \
			return;                                                                 \
		}                                                                           \
																					\
		/* apply top clip */                                                        \
		srcy = 0;                                                                   \
		if (desty < cliprect.top())                                                 \
		{                                                                           \
			srcy = (cliprect.top() - desty) * dy;                                   \
			desty = cliprect.top();                                                 \
		}                                                                           \
																					\
		/* apply bottom clip */                                                     \
		if (destendy > cliprect.bottom())                                           \
			destendy = cliprect.bottom();                                           \
																					\
		/* apply X flipping */                                                      \
		if (flipx)                                                                  \
		{                                                                           \
			srcx = (dstwidth - 1) * dx - srcx;                                      \
			dx = -dx;                                                               \
		}                                                                           \
																					\
		/* apply Y flipping */                                                      \
		if (flipy)                                                                  \
		{                                                                           \
			srcy = (dstheight - 1) * dy - srcy;                                     \
			dy = -dy;                                                               \
		}                                                                           \
																					\
		/* fetch the source data */                                                 \
		srcdata = get_data(code);                                                   \
																					\
		/* compute how many blocks of 4 pixels we have */                           \
		u32 numblocks = (destendx + 1 - destx) / 4;                                 \
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;                     \
																					\
		/* iterate over pixels in Y */                                              \
		for (cury = desty; cury <= destendy; cury++)                                \
		{                                                                           \
			PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, destx); \
			PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, destx);              \
			const u8 *srcptr = srcdata + (srcy >> 16) * rowbytes();                 \
			s32 cursrcx = srcx;                                                     \
			srcy += dy;                                                             \
																					\
			/* iterate over unrolled blocks of 4 */                                 \
			for (curx = 0; curx < numblocks; curx++)                                \
			{                                                                       \
				PIXEL_OP(destptr[0], priptr[0], srcptr[cursrcx >> 16]);             \
				cursrcx += dx;                                                      \
				PIXEL_OP(destptr[1], priptr[1], srcptr[cursrcx >> 16]);             \
				cursrcx += dx;                                                      \
				PIXEL_OP(destptr[2], priptr[2], srcptr[cursrcx >> 16]);             \
				cursrcx += dx;                                                      \
				PIXEL_OP(destptr[3], priptr[3], srcptr[cursrcx >> 16]);             \
				cursrcx += dx;                                                      \
																					\
				destptr += 4;                                                       \
				PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                         \
			}                                                                       \
																					\
			/* iterate over leftover pixels */                                      \
			for (curx = 0; curx < leftovers; curx++)                                \
			{                                                                       \
				PIXEL_OP(destptr[0], priptr[0], srcptr[cursrcx >> 16]);             \
				cursrcx += dx;                                                      \
				destptr++;                                                          \
				PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                         \
			}                                                                       \
		}                                                                           \
	} while (0);                                                                    \
	g_profiler.stop();                                                              \
} while (0)



/***************************************************************************
    BASIC COPYBITMAP CORE
***************************************************************************/

/*
    Assumed input parameters or local variables:

        bitmap_t &dest - the bitmap to copy to
        bitmap_t &src - the bitmap to copy from (must be same bpp as dest)
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to copy to
        s32 desty - the top-left Y coordinate to copy to
        bitmap_t &priority - the priority bitmap (even if PRIORITY_TYPE is NO_PRIORITY, at least needs a dummy)
*/

#define COPYBITMAP_CORE(PIXEL_TYPE, PIXEL_OP, PRIORITY_TYPE)                        \
do {                                                                                \
	g_profiler.start(PROFILER_COPYBITMAP);                                          \
	do {                                                                            \
		const PIXEL_TYPE *srcdata;                                                  \
		u32 numblocks, leftovers;                                                   \
		s32 destendx, destendy;                                                     \
		s32 srcx, srcy;                                                             \
		s32 curx, cury;                                                             \
		s32 dx, dy;                                                                 \
																					\
		assert(dest.valid());                                                       \
		assert(src.valid());                                                        \
		assert(!PRIORITY_VALID(PRIORITY_TYPE) || priority.valid());                 \
		assert(dest.cliprect().contains(cliprect));                                 \
																					\
		/* ignore empty/invalid cliprects */                                        \
		if (cliprect.empty())                                                       \
			break;                                                                  \
																					\
		/* standard setup; dx counts bytes in X, dy counts pixels in Y */           \
		dx = 1;                                                                     \
		dy = src.rowpixels();                                                       \
																					\
		/* compute final pixel in X and exit if we are entirely clipped */          \
		destendx = destx + src.width() - 1;                                         \
		if (destx > cliprect.right() || destendx < cliprect.left())                 \
			break;                                                                  \
																					\
		/* apply left clip */                                                       \
		srcx = 0;                                                                   \
		if (destx < cliprect.left())                                                \
		{                                                                           \
			srcx = cliprect.left() - destx;                                         \
			destx = cliprect.left();                                                \
		}                                                                           \
																					\
		/* apply right clip */                                                      \
		if (destendx > cliprect.right())                                            \
			destendx = cliprect.right();                                            \
																					\
		/* compute final pixel in Y and exit if we are entirely clipped */          \
		destendy = desty + src.height() - 1;                                        \
		if (desty > cliprect.bottom() || destendy < cliprect.top())                 \
			break;                                                                  \
																					\
		/* apply top clip */                                                        \
		srcy = 0;                                                                   \
		if (desty < cliprect.top())                                                 \
		{                                                                           \
			srcy = cliprect.top() - desty;                                          \
			desty = cliprect.top();                                                 \
		}                                                                           \
																					\
		/* apply bottom clip */                                                     \
		if (destendy > cliprect.bottom())                                           \
			destendy = cliprect.bottom();                                           \
																					\
		/* apply X flipping */                                                      \
		if (flipx)                                                                  \
		{                                                                           \
			srcx = src.width() - 1 - srcx;                                          \
			dx = -dx;                                                               \
		}                                                                           \
																					\
		/* apply Y flipping */                                                      \
		if (flipy)                                                                  \
		{                                                                           \
			srcy = src.height() - 1 - srcy;                                         \
			dy = -dy;                                                               \
		}                                                                           \
																					\
		/* compute how many blocks of 4 pixels we have */                           \
		numblocks = (destendx + 1 - destx) / 4;                                     \
		leftovers = (destendx + 1 - destx) - 4 * numblocks;                         \
																					\
		/* compute the address of the first source pixel of the first row */        \
		srcdata = &src.pixt<PIXEL_TYPE>(srcy, srcx);                                \
																					\
		/* non-flipped case */                                                      \
		if (!flipx)                                                                 \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = desty; cury <= destendy; cury++)                            \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, destx); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, destx);          \
				const PIXEL_TYPE *srcptr = srcdata;                                 \
				srcdata += dy;                                                      \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					PIXEL_OP(destptr[1], priptr[1], srcptr[1]);                     \
					PIXEL_OP(destptr[2], priptr[2], srcptr[2]);                     \
					PIXEL_OP(destptr[3], priptr[3], srcptr[3]);                     \
																					\
					srcptr += 4;                                                    \
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					srcptr++;                                                       \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
																					\
		/* flipped case */                                                          \
		else                                                                        \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = desty; cury <= destendy; cury++)                            \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, destx); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, destx);          \
				const PIXEL_TYPE *srcptr = srcdata;                                 \
				srcdata += dy;                                                      \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[ 0]);                    \
					PIXEL_OP(destptr[1], priptr[1], srcptr[-1]);                    \
					PIXEL_OP(destptr[2], priptr[2], srcptr[-2]);                    \
					PIXEL_OP(destptr[3], priptr[3], srcptr[-3]);                    \
																					\
					srcptr -= 4;                                                    \
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					srcptr--;                                                       \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	} while (0);                                                                    \
	g_profiler.stop();                                                              \
} while (0)



/***************************************************************************
    BASIC COPYROZBITMAP CORE
***************************************************************************/

/*
    Assumed input parameters or local variables:

        bitmap_t &dest - the bitmap to copy to
        bitmap_t &src - the bitmap to copy from (must be same bpp as dest)
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        s32 destx - the 16.16 source X position at destination pixel (0,0)
        s32 desty - the 16.16 source Y position at destination pixel (0,0)
        s32 incxx - the 16.16 amount to increment in source X for each destination X pixel
        s32 incyx - the 16.16 amount to increment in source Y for each destination X pixel
        s32 incxy - the 16.16 amount to increment in source X for each destination Y pixel
        s32 incyy - the 16.16 amount to increment in source Y for each destination Y pixel
        int wraparound - non-zero means wrap when hitting the edges of the source
        bitmap_t &priority - the priority bitmap (even if PRIORITY_TYPE is NO_PRIORITY, at least needs a dummy)
*/

#define COPYROZBITMAP_CORE(PIXEL_TYPE, PIXEL_OP, PRIORITY_TYPE)                     \
do {                                                                                \
	u32 srcfixwidth, srcfixheight;                                                  \
	u32 numblocks, leftovers;                                                       \
	s32 curx, cury;                                                                 \
																					\
	g_profiler.start(PROFILER_COPYBITMAP);                                          \
																					\
	assert(dest.valid());                                                           \
	assert(dest.valid());                                                           \
	assert(!PRIORITY_VALID(PRIORITY_TYPE) || priority.valid());                     \
	assert(dest.cliprect().contains(cliprect));                                     \
	assert(!wraparound || (src.width() & (src.width() - 1)) == 0);                  \
	assert(!wraparound || (src.height() & (src.height() - 1)) == 0);                \
																					\
	/* ignore empty/invalid cliprects */                                            \
	if (cliprect.empty())                                                           \
		break;                                                                      \
																					\
	/* compute fixed-point 16.16 size of the source bitmap */                       \
	srcfixwidth = src.width() << 16;                                                \
	srcfixheight = src.height() << 16;                                              \
																					\
	/* advance the starting coordinates to the top-left of the cliprect */          \
	startx += cliprect.left() * incxx + cliprect.top() * incyx;                     \
	starty += cliprect.left() * incxy + cliprect.top() * incyy;                     \
																					\
	/* compute how many blocks of 4 pixels we have */                               \
	numblocks = cliprect.width() / 4;                                               \
	leftovers = cliprect.width() - 4 * numblocks;                                   \
																					\
	/* if incxy and incyx are 0, then we aren't rotating, just zooming */           \
	if (incxy == 0 && incyx == 0)                                                   \
	{                                                                               \
		/* zoom-only, non-wraparound case */                                        \
		if (!wraparound)                                                            \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = cliprect.top(); cury <= cliprect.bottom(); cury++)          \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, cliprect.left()); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, cliprect.left()); \
				const PIXEL_TYPE *srcptr;                                           \
				s32 srcx = startx;                                                  \
				s32 srcy = starty;                                                  \
																					\
				starty += incyy;                                                    \
																					\
				/* check srcy for the whole row at once */                          \
				if ((u32)srcy < srcfixheight)                                       \
				{                                                                   \
					srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16);                     \
																					\
					/* iterate over unrolled blocks of 4 */                         \
					for (curx = 0; curx < numblocks; curx++)                        \
					{                                                               \
						if (u32(srcx) < srcfixwidth)                                \
							PIXEL_OP(destptr[0], priptr[0], srcptr[srcx >> 16]);    \
						srcx += incxx;                                              \
																					\
						if (u32(srcx) < srcfixwidth)                                \
							PIXEL_OP(destptr[1], priptr[1], srcptr[srcx >> 16]);    \
						srcx += incxx;                                              \
																					\
						if (u32(srcx) < srcfixwidth)                                \
							PIXEL_OP(destptr[2], priptr[2], srcptr[srcx >> 16]);    \
						srcx += incxx;                                              \
																					\
						if (u32(srcx) < srcfixwidth)                                \
							PIXEL_OP(destptr[3], priptr[3], srcptr[srcx >> 16]);    \
						srcx += incxx;                                              \
																					\
						destptr += 4;                                               \
						PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                 \
					}                                                               \
																					\
					/* iterate over leftover pixels */                              \
					for (curx = 0; curx < leftovers; curx++)                        \
					{                                                               \
						if (u32(srcx) < srcfixwidth)                                \
							PIXEL_OP(destptr[0], priptr[0], srcptr[srcx >> 16]);    \
						srcx += incxx;                                              \
						destptr++;                                                  \
						PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                 \
					}                                                               \
				}                                                                   \
			}                                                                       \
		}                                                                           \
																					\
		/* zoom-only, wraparound case */                                            \
		else                                                                        \
		{                                                                           \
			/* convert srcfixwidth/height into a mask and apply */                  \
			srcfixwidth--;                                                          \
			srcfixheight--;                                                         \
			startx &= srcfixwidth;                                                  \
			starty &= srcfixheight;                                                 \
																					\
			/* iterate over pixels in Y */                                          \
			for (cury = cliprect.top(); cury <= cliprect.bottom(); cury++)          \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, cliprect.left()); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, cliprect.left()); \
				const PIXEL_TYPE *srcptr = &src.pixt<PIXEL_TYPE>(starty >> 16);     \
				s32 srcx = startx;                                                  \
																					\
				starty = (starty + incyy) & srcfixheight;                           \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[srcx >> 16]);            \
					srcx = (srcx + incxx) & srcfixwidth;                            \
																					\
					PIXEL_OP(destptr[1], priptr[1], srcptr[srcx >> 16]);            \
					srcx = (srcx + incxx) & srcfixwidth;                            \
																					\
					PIXEL_OP(destptr[2], priptr[2], srcptr[srcx >> 16]);            \
					srcx = (srcx + incxx) & srcfixwidth;                            \
																					\
					PIXEL_OP(destptr[3], priptr[3], srcptr[srcx >> 16]);            \
					srcx = (srcx + incxx) & srcfixwidth;                            \
																					\
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					PIXEL_OP(destptr[0], priptr[0], srcptr[srcx >> 16]);            \
					srcx = (srcx + incxx) & srcfixwidth;                            \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	}                                                                               \
																					\
	/* full rotation case */                                                        \
	else                                                                            \
	{                                                                               \
		/* full rotation, non-wraparound case */                                    \
		if (!wraparound)                                                            \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = cliprect.top(); cury <= cliprect.bottom(); cury++)          \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, cliprect.left()); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, cliprect.left()); \
				const PIXEL_TYPE *srcptr;                                           \
				s32 srcx = startx;                                                  \
				s32 srcy = starty;                                                  \
																					\
				startx += incyx;                                                    \
				starty += incyy;                                                    \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					if (u32(srcx) < srcfixwidth && (u32)srcy < srcfixheight)        \
					{                                                               \
						srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);     \
						PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                 \
					}                                                               \
					srcx += incxx;                                                  \
					srcy += incxy;                                                  \
																					\
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)        \
					{                                                               \
						srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);     \
						PIXEL_OP(destptr[1], priptr[1], srcptr[0]);                 \
					}                                                               \
					srcx += incxx;                                                  \
					srcy += incxy;                                                  \
																					\
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)        \
					{                                                               \
						srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);     \
						PIXEL_OP(destptr[2], priptr[2], srcptr[0]);                 \
					}                                                               \
					srcx += incxx;                                                  \
					srcy += incxy;                                                  \
																					\
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)        \
					{                                                               \
						srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);     \
						PIXEL_OP(destptr[3], priptr[3], srcptr[0]);                 \
					}                                                               \
					srcx += incxx;                                                  \
					srcy += incxy;                                                  \
																					\
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					if (u32(srcx) < srcfixwidth && u32(srcy) < srcfixheight)        \
					{                                                               \
						srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);     \
						PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                 \
					}                                                               \
					srcx += incxx;                                                  \
					srcy += incxy;                                                  \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
																					\
		/* zoom-only, wraparound case */                                            \
		else                                                                        \
		{                                                                           \
			/* convert srcfixwidth/height into a mask and apply */                  \
			srcfixwidth--;                                                          \
			srcfixheight--;                                                         \
			startx &= srcfixwidth;                                                  \
			starty &= srcfixheight;                                                 \
																					\
			/* iterate over pixels in Y */                                          \
			for (cury = cliprect.top(); cury <= cliprect.bottom(); cury++)          \
			{                                                                       \
				PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, cury, cliprect.left()); \
				PIXEL_TYPE *destptr = &dest.pixt<PIXEL_TYPE>(cury, cliprect.left()); \
				const PIXEL_TYPE *srcptr;                                           \
				s32 srcx = startx;                                                  \
				s32 srcy = starty;                                                  \
																					\
				startx = (startx + incyx) & srcfixwidth;                            \
				starty = (starty + incyy) & srcfixheight;                           \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);         \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					srcx = (srcx + incxx) & srcfixwidth;                            \
					srcy = (srcy + incxy) & srcfixheight;                           \
																					\
					srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);         \
					PIXEL_OP(destptr[1], priptr[1], srcptr[0]);                     \
					srcx = (srcx + incxx) & srcfixwidth;                            \
					srcy = (srcy + incxy) & srcfixheight;                           \
																					\
					srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);         \
					PIXEL_OP(destptr[2], priptr[2], srcptr[0]);                     \
					srcx = (srcx + incxx) & srcfixwidth;                            \
					srcy = (srcy + incxy) & srcfixheight;                           \
																					\
					srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);         \
					PIXEL_OP(destptr[3], priptr[3], srcptr[0]);                     \
					srcx = (srcx + incxx) & srcfixwidth;                            \
					srcy = (srcy + incxy) & srcfixheight;                           \
																					\
					destptr += 4;                                                   \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                     \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					srcptr = &src.pixt<PIXEL_TYPE>(srcy >> 16, srcx >> 16);         \
					PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                     \
					srcx = (srcx + incxx) & srcfixwidth;                            \
					srcy = (srcy + incxy) & srcfixheight;                           \
					destptr++;                                                      \
					PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                     \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	}                                                                               \
	g_profiler.stop();                                                              \
} while (0)



/***************************************************************************
    BASIC DRAWSCANLINE CORE
***************************************************************************/

/*
    Assumed input parameters or local variables:

        bitmap_t &bitmap - the bitmap to copy to
        s32 destx - the X coordinate to copy to
        s32 desty - the Y coordinate to copy to
        s32 length - the total number of pixels to copy
        const UINTx *srcptr - pointer to memory containing the source pixels
        bitmap_t &priority - the priority bitmap (even if PRIORITY_TYPE is NO_PRIORITY, at least needs a dummy)
*/

#define DRAWSCANLINE_CORE(PIXEL_TYPE, PIXEL_OP, PRIORITY_TYPE)                      \
do {                                                                                \
	assert(bitmap.valid());                                                         \
	assert(destx >= 0);                                                             \
	assert(destx + length <= bitmap.width());                                       \
	assert(desty >= 0);                                                             \
	assert(desty < bitmap.height());                                                \
	assert(srcptr != nullptr);                                                      \
	assert(!PRIORITY_VALID(PRIORITY_TYPE) || priority.valid());                     \
																					\
	{                                                                               \
		PRIORITY_TYPE *priptr = PRIORITY_ADDR(priority, PRIORITY_TYPE, desty, destx); \
		PIXEL_TYPE *destptr = &bitmap.pixt<PIXEL_TYPE>(desty, destx);               \
																					\
		/* iterate over unrolled blocks of 4 */                                     \
		while (length >= 4)                                                         \
		{                                                                           \
			PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                             \
			PIXEL_OP(destptr[1], priptr[1], srcptr[1]);                             \
			PIXEL_OP(destptr[2], priptr[2], srcptr[2]);                             \
			PIXEL_OP(destptr[3], priptr[3], srcptr[3]);                             \
																					\
			length -= 4;                                                            \
			srcptr += 4;                                                            \
			destptr += 4;                                                           \
			PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 4);                             \
		}                                                                           \
																					\
		/* iterate over leftover pixels */                                          \
		while (length-- > 0)                                                        \
		{                                                                           \
			PIXEL_OP(destptr[0], priptr[0], srcptr[0]);                             \
			srcptr++;                                                               \
			destptr++;                                                              \
			PRIORITY_ADVANCE(PRIORITY_TYPE, priptr, 1);                             \
		}                                                                           \
	}                                                                               \
} while (0)



/***************************************************************************
    BASIC EXTRACTSCANLINE CORE
***************************************************************************/

/*
    Assumed input parameters:

        bitmap_t &bitmap - the bitmap to extract from
        s32 srcx - the X coordinate to begin extraction
        s32 srcy - the Y coordinate to begin extraction
        s32 length - the total number of pixels to extract
        UINTx *destptr - pointer to memory to receive the extracted pixels
*/

#define EXTRACTSCANLINE_CORE(PIXEL_TYPE)                                            \
do {                                                                                \
	assert(bitmap.valid());                                                         \
	assert(srcx >= 0);                                                              \
	assert(srcx + length <= bitmap.width());                                        \
	assert(srcy >= 0);                                                              \
	assert(srcy < bitmap.height());                                                 \
	assert(destptr != nullptr);                                                     \
																					\
	{                                                                               \
		const PIXEL_TYPE *srcptr = &bitmap.pixt<PIXEL_TYPE>(srcy, srcx);            \
																					\
		/* iterate over unrolled blocks of 4 */                                     \
		while (length >= 4)                                                         \
		{                                                                           \
			destptr[0] = srcptr[0];                                                 \
			destptr[1] = srcptr[1];                                                 \
			destptr[2] = srcptr[2];                                                 \
			destptr[3] = srcptr[3];                                                 \
			length -= 4;                                                            \
			srcptr += 4;                                                            \
			destptr += 4;                                                           \
		}                                                                           \
																					\
		/* iterate over leftover pixels */                                          \
		while (length > 0)                                                          \
		{                                                                           \
			destptr[0] = srcptr[0];                                                 \
			length--;                                                               \
			srcptr++;                                                               \
			destptr++;                                                              \
		}                                                                           \
	}                                                                               \
} while (0)


#endif  /* __DRAWGFXM_H__ */
