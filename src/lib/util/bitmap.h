/***************************************************************************

    bitmap.h

    Core bitmap routines.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "osdcore.h"
#include "palette.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* bitmap_format describes the various bitmap formats we use */
enum _bitmap_format
{
	BITMAP_FORMAT_INVALID = 0,		/* invalid format */
	BITMAP_FORMAT_INDEXED8,			/* 8bpp indexed */
	BITMAP_FORMAT_INDEXED16,		/* 16bpp indexed */
	BITMAP_FORMAT_INDEXED32,		/* 32bpp indexed */
	BITMAP_FORMAT_RGB15,			/* 15bpp 5-5-5 RGB */
	BITMAP_FORMAT_RGB32,			/* 32bpp 8-8-8 RGB */
	BITMAP_FORMAT_ARGB32,			/* 32bpp 8-8-8-8 ARGB */
	BITMAP_FORMAT_YUY16,			/* 16bpp 8-8 Y/Cb, Y/Cr in sequence */
	BITMAP_FORMAT_LAST
};
typedef enum _bitmap_format bitmap_format;


/* rectangles describe a bitmap portion */
typedef struct _rectangle rectangle;
struct _rectangle
{
	int				min_x;			/* minimum X, or left coordinate */
	int				max_x;			/* maximum X, or right coordinate (inclusive) */
	int				min_y;			/* minimum Y, or top coordinate */
	int				max_y;			/* maximum Y, or bottom coordinate (inclusive) */
};


/* bitmaps describe a rectangular array of pixels */
typedef struct _bitmap_t bitmap_t;
struct _bitmap_t
{
	void *			alloc;			/* pointer to allocated pixel memory */
	void *			base;			/* pointer to pixel (0,0) (adjusted for padding) */
	int				rowpixels;		/* pixels per row (including padding) */
	int 			width;			/* width of the bitmap */
	int				height;			/* height of the bitmap */
	bitmap_format	format;			/* format of the bitmap */
	int				bpp;			/* bits per pixel */
	palette_t *		palette;		/* optional palette */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* Macros for accessing bitmap pixels */
#define BITMAP_ADDR(bitmap, type, y, x)	\
	((type *)(bitmap)->base + (y) * (bitmap)->rowpixels + (x))

#define BITMAP_ADDR8(bitmap, y, x)	BITMAP_ADDR(bitmap, UINT8, y, x)
#define BITMAP_ADDR16(bitmap, y, x)	BITMAP_ADDR(bitmap, UINT16, y, x)
#define BITMAP_ADDR32(bitmap, y, x)	BITMAP_ADDR(bitmap, UINT32, y, x)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- bitmap allocation ----- */

/* allocate a new bitmap of the given dimensions and format */
bitmap_t *bitmap_alloc(int width, int height, bitmap_format format);

/* allocate a new bitmap with additional slop on the borders */
bitmap_t *bitmap_alloc_slop(int width, int height, int xslop, int yslop, bitmap_format format);

/* wrap a bitmap object around an existing array of data */
bitmap_t *bitmap_wrap(void *base, int width, int height, int rowpixels, bitmap_format format);

/* associate a palette with a bitmap */
void bitmap_set_palette(bitmap_t *bitmap, palette_t *palette);

/* free allocated data for a bitmap */
void bitmap_free(bitmap_t *bitmap);



/* ----- bitmap drawing ----- */

/* fill a bitmap with a single color, clipping to the given rectangle */
void bitmap_fill(bitmap_t *dest, const rectangle *clip, rgb_t color);



/* ----- bitmap utilities ----- */

/* return the number of bits per pixel for a given bitmap format */
int bitmap_format_to_bpp(bitmap_format format);




/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    sect_rect - compute the intersection of two
    rectangles
-------------------------------------------------*/

INLINE void sect_rect(rectangle *dst, const rectangle *src)
{
	if (src->min_x > dst->min_x) dst->min_x = src->min_x;
	if (src->max_x < dst->max_x) dst->max_x = src->max_x;
	if (src->min_y > dst->min_y) dst->min_y = src->min_y;
	if (src->max_y < dst->max_y) dst->max_y = src->max_y;
}


/*-------------------------------------------------
    union_rect - compute the union of two
    rectangles
-------------------------------------------------*/

INLINE void union_rect(rectangle *dst, const rectangle *src)
{
	if (src->min_x < dst->min_x) dst->min_x = src->min_x;
	if (src->max_x > dst->max_x) dst->max_x = src->max_x;
	if (src->min_y < dst->min_y) dst->min_y = src->min_y;
	if (src->max_y > dst->max_y) dst->max_y = src->max_y;
}


/*-------------------------------------------------
    plot_box - draw a filled rectangle into a
    bitmap of arbitrary depth
-------------------------------------------------*/

INLINE void plot_box(bitmap_t *bitmap, int x, int y, int width, int height, UINT32 color)
{
	rectangle clip;
	clip.min_x = x;
	clip.min_y = y;
	clip.max_x = x + width - 1;
	clip.max_y = y + height - 1;
	bitmap_fill(bitmap, &clip, color);
}


#endif	/* __BITMAP_H__ */
