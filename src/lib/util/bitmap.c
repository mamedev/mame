/***************************************************************************

    bitmap.c

    Core bitmap routines.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "bitmap.h"



/***************************************************************************
    BITMAP ALLOCATION/CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    bitmap_alloc -- allocate memory for a new
    bitmap of the given format
-------------------------------------------------*/

bitmap_t *bitmap_alloc(int width, int height, bitmap_format format)
{
	return bitmap_alloc_slop(width, height, 0, 0, format);
}


/*-------------------------------------------------
    bitmap_alloc_slop -- allocate a new bitmap with
    additional slop on the borders
-------------------------------------------------*/

bitmap_t *bitmap_alloc_slop(int width, int height, int xslop, int yslop, bitmap_format format)
{
	int bpp = bitmap_format_to_bpp(format);
	size_t allocbytes;
	bitmap_t *bitmap;
	int rowpixels;

	/* fail if invalid format */
	if (bpp == 0)
		return NULL;

	/* allocate the bitmap itself */
	bitmap = malloc(sizeof(*bitmap));
	if (bitmap == NULL)
		return NULL;
	memset(bitmap, 0, sizeof(*bitmap));

	/* round the width to a multiple of 8 and add some padding */
	rowpixels = (width + 2 * xslop + 7) & ~7;

	/* allocate memory for the bitmap itself */
	allocbytes = rowpixels * (height + 2 * yslop) * bpp / 8;
	bitmap->alloc = malloc(allocbytes);
	if (bitmap->alloc == NULL)
	{
		free(bitmap);
		return NULL;
	}

	/* clear to 0 by default */
	memset(bitmap->alloc, 0, allocbytes);

	/* fill in the data */
	bitmap->format = format;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->bpp = bpp;
	bitmap->rowpixels = rowpixels;
	bitmap->base = (UINT8 *)bitmap->alloc + (rowpixels * yslop + xslop) * (bpp / 8);

	return bitmap;
}


/*-------------------------------------------------
    bitmap_wrap -- wrap an existing memory buffer
    as a bitmap
-------------------------------------------------*/

bitmap_t *bitmap_wrap(void *base, int width, int height, int rowpixels, bitmap_format format)
{
	int bpp = bitmap_format_to_bpp(format);
	bitmap_t *bitmap;

	/* fail if invalid format */
	if (bpp == 0)
		return NULL;

	/* allocate memory */
	bitmap = malloc(sizeof(*bitmap));
	if (bitmap == NULL)
		return NULL;
	memset(bitmap, 0, sizeof(*bitmap));

	/* fill in the data */
	bitmap->format = format;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->bpp = bpp;
	bitmap->rowpixels = rowpixels;
	bitmap->base = base;

	return bitmap;
}


/*-------------------------------------------------
    bitmap_set_palette -- associate a palette with
    a bitmap
-------------------------------------------------*/

void bitmap_set_palette(bitmap_t *bitmap, palette_t *palette)
{
	/* first dereference any existing palette */
	if (bitmap->palette != NULL)
	{
		palette_deref(bitmap->palette);
		bitmap->palette = NULL;
	}

	/* then reference any new palette */
	if (palette != NULL)
	{
		palette_ref(palette);
		bitmap->palette = palette;
	}
}


/*-------------------------------------------------
    bitmap_free -- release memory allocated for
    a bitmap
-------------------------------------------------*/

void bitmap_free(bitmap_t *bitmap)
{
	/* dereference the palette */
	if (bitmap->palette != NULL)
		palette_deref(bitmap->palette);

	/* free any allocated memory */
	if (bitmap->alloc != NULL)
		free(bitmap->alloc);

	/* free the bitmap */
	free(bitmap);
}



/***************************************************************************
    BITMAP DRAWING
***************************************************************************/

/*-------------------------------------------------
    bitmap_fill -- fill a bitmap with a solid
    color
-------------------------------------------------*/

void bitmap_fill(bitmap_t *dest, const rectangle *cliprect, UINT32 color)
{
	rectangle fill;
	int x, y;

	/* default to the whole bitmap */
	fill.min_x = fill.min_y = 0;
	fill.max_x = dest->width - 1;
	fill.max_y = dest->height - 1;

	/* if we have a cliprect, intersect with that */
	if (cliprect != NULL)
		sect_rect(&fill, cliprect);

	/* early out if nothing to do */
	if (fill.min_x > fill.max_x || fill.min_y > fill.max_y)
		return;

	/* based on the bpp go from there */
	switch (dest->bpp)
	{
		case 8:
			/* 8bpp always uses memset */
			for (y = fill.min_y; y <= fill.max_y; y++)
				memset(BITMAP_ADDR8(dest, y, fill.min_x), (UINT8)color, fill.max_x + 1 - fill.min_x);
			break;

		case 16:
			/* 16bpp can use memset if the bytes are equal */
			if ((UINT8)(color >> 8) == (UINT8)color)
			{
				for (y = fill.min_y; y <= fill.max_y; y++)
					memset(BITMAP_ADDR16(dest, y, fill.min_x), (UINT8)color, (fill.max_x + 1 - fill.min_x) * 2);
			}
			else
			{
				for (y = fill.min_y; y <= fill.max_y; y++)
				{
					UINT16 *destrow = BITMAP_ADDR16(dest, y, 0);
					for (x = fill.min_x; x <= fill.max_x; x++)
						destrow[x] = (UINT16)color;
				}
			}
			break;

		case 32:
			/* 32bpp can use memset if the bytes are equal */
			if ((UINT8)(color >> 8) == (UINT8)color && (UINT16)(color >> 16) == (UINT16)color)
			{
				for (y = fill.min_y; y <= fill.max_y; y++)
					memset(BITMAP_ADDR32(dest, y, fill.min_x), (UINT8)color, (fill.max_x + 1 - fill.min_x) * 4);
			}
			else
			{
				for (y = fill.min_y; y <= fill.max_y; y++)
				{
					UINT32 *destrow = BITMAP_ADDR32(dest, y, 0);
					for (x = fill.min_x; x <= fill.max_x; x++)
						destrow[x] = (UINT32)color;
				}
			}
			break;
	}
}



/***************************************************************************
    BITMAP UTILITIES
***************************************************************************/

/*-------------------------------------------------
    bitmap_format_to_bpp - given a format, return
    the bpp
-------------------------------------------------*/

int bitmap_format_to_bpp(bitmap_format format)
{
	/* choose a depth for the format */
	switch (format)
	{
		case BITMAP_FORMAT_INDEXED8:
			return 8;

		case BITMAP_FORMAT_INDEXED16:
		case BITMAP_FORMAT_RGB15:
		case BITMAP_FORMAT_YUY16:
			return 16;

		case BITMAP_FORMAT_INDEXED32:
		case BITMAP_FORMAT_RGB32:
		case BITMAP_FORMAT_ARGB32:
			return 32;

		default:
			break;
	}
	return 0;
}
