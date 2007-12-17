/***************************************************************************

    Super-simple PNG to BDC file generator

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Format of PNG data:

    Multiple rows of characters. A black pixel means "on". All other colors
    mean "off". Each row looks like this:

    * 8888  ***    *
    * 4444 *   *  **
    * 2222 *   *   *
    * 1111 *   *   *
    *      *   *   *
    **      ***   ***
    *
    *

           ****** ****

    The column of pixels on the left-hand side (column 0) indicates the
    character cell height. This column must be present on each row and
    the height must be consistent for each row.

    Protruding one pixel into column 1 is the baseline indicator. There
    should only be one row with a pixel in column 1 for each line, and
    that pixel row index should be consistent for each row.

    In columns 2-5 are a 4-hex-digit starting character index number. This
    is encoded as binary value. Each column is 4 pixels tall and represents
    one binary digit. The character index number is the unicode character
    number of the first character encoded in this row; subsequent
    characters in the row are at increasing character indices.

    Starting in column 6 and beyond are the actual character bitmaps.
    Below them, in the second row after the last row of the character,
    is a solid line that indicates the width of the character, and also
    where the character bitmap begins and ends.

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "osdcore.h"
#include "png.h"


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define CACHED_CHAR_SIZE		12
#define CACHED_HEADER_SIZE		16



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a render_font contains information about a single character in a font */
typedef struct _render_font_char render_font_char;
struct _render_font_char
{
	INT32				width;				/* width from this character to the next */
	INT32				xoffs, yoffs;		/* X and Y offset from baseline to top,left of bitmap */
	INT32				bmwidth, bmheight;	/* width and height of bitmap */
	bitmap_t *			bitmap;				/* pointer to the bitmap containing the raw data */
};


/* a render_font contains information about a font */
typedef struct _render_font render_font;
struct _render_font
{
	int					height;				/* height of the font, from ascent to descent */
	int					yoffs;				/* y offset from baseline to descent */
	render_font_char 	chars[65536];		/* array of characters */
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE int pixel_is_set(bitmap_t *bitmap, int y, int x)
{
	return (*BITMAP_ADDR32(bitmap, y, x) & 0xffffff) == 0;
}



/***************************************************************************
    MAIN
***************************************************************************/

static int render_font_save_cached(render_font *font, const char *filename, UINT32 hash)
{
	file_error filerr;
	render_font_char *ch;
	UINT32 bytes_written;
	UINT8 *tempbuffer;
	UINT8 *chartable;
	core_file *file;
	int numchars;
	UINT8 *dest;
	int tableindex;
	int chnum;

	/* attempt to open the file */
	filerr = core_fopen(filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	if (filerr != FILERR_NONE)
		return 1;

	/* determine the number of characters */
	numchars = 0;
	for (chnum = 0; chnum < 65536; chnum++)
		if (font->chars[chnum].width > 0)
			numchars++;

	/* allocate an array to hold the character data */
	chartable = malloc(numchars * CACHED_CHAR_SIZE);

	/* allocate a temp buffer to compress into */
	tempbuffer = malloc(65536);
	if (chartable == NULL || tempbuffer == NULL)
		goto error;

	memset(chartable, 0, numchars * CACHED_CHAR_SIZE);

	/* write the header */
	dest = tempbuffer;
	*dest++ = 'f';
	*dest++ = 'o';
	*dest++ = 'n';
	*dest++ = 't';
	*dest++ = hash >> 24;
	*dest++ = hash >> 16;
	*dest++ = hash >> 8;
	*dest++ = hash & 0xff;
	*dest++ = font->height >> 8;
	*dest++ = font->height & 0xff;
	*dest++ = font->yoffs >> 8;
	*dest++ = font->yoffs & 0xff;
	*dest++ = numchars >> 24;
	*dest++ = numchars >> 16;
	*dest++ = numchars >> 8;
	*dest++ = numchars & 0xff;
	bytes_written = core_fwrite(file, tempbuffer, dest - tempbuffer);
	if (bytes_written != dest - tempbuffer)
		goto error;

	/* write the empty table to the beginning of the file */
	bytes_written = core_fwrite(file, chartable, numchars * CACHED_CHAR_SIZE);
	if (bytes_written != numchars * CACHED_CHAR_SIZE)
		goto error;

	/* loop over all characters */
	tableindex = 0;
	for (chnum = 0; chnum < 65536; chnum++)
	{
		ch = &font->chars[chnum];
		if (ch->width > 0)
		{
			UINT8 accum, accbit;
			int x, y;

			/* write out a bit-compressed bitmap if we have one */
			if (ch->bitmap != NULL)
			{
				/* write the data to the tempbuffer */
				dest = tempbuffer;
				accum = 0;
				accbit = 7;

				/* bit-encode the character data */
				for (y = 0; y < ch->bmheight; y++)
				{
					int desty = y + font->height + font->yoffs - ch->yoffs - ch->bmheight;
					const UINT32 *src = (desty >= 0 && desty < font->height) ? BITMAP_ADDR32(ch->bitmap, desty, 0) : NULL;
					for (x = 0; x < ch->bmwidth; x++)
					{
						if (src != NULL && src[x] != 0)
							accum |= 1 << accbit;
						if (accbit-- == 0)
						{
							*dest++ = accum;
							accum = 0;
							accbit = 7;
						}
					}
				}

				/* flush any extra */
				if (accbit != 7)
					*dest++ = accum;

				/* write the data */
				bytes_written = core_fwrite(file, tempbuffer, dest - tempbuffer);
				if (bytes_written != dest - tempbuffer)
					goto error;

				/* free the bitmap and texture */
				bitmap_free(ch->bitmap);
				ch->bitmap = NULL;
			}

			/* compute the table entry */
			dest = &chartable[tableindex++ * CACHED_CHAR_SIZE];
			*dest++ = chnum >> 8;
			*dest++ = chnum & 0xff;
			*dest++ = ch->width >> 8;
			*dest++ = ch->width & 0xff;
			*dest++ = ch->xoffs >> 8;
			*dest++ = ch->xoffs & 0xff;
			*dest++ = ch->yoffs >> 8;
			*dest++ = ch->yoffs & 0xff;
			*dest++ = ch->bmwidth >> 8;
			*dest++ = ch->bmwidth & 0xff;
			*dest++ = ch->bmheight >> 8;
			*dest++ = ch->bmheight & 0xff;
		}
	}

	/* seek back to the beginning and rewrite the table */
	core_fseek(file, CACHED_HEADER_SIZE, SEEK_SET);
	bytes_written = core_fwrite(file, chartable, numchars * CACHED_CHAR_SIZE);
	if (bytes_written != numchars * CACHED_CHAR_SIZE)
		goto error;

	/* all done */
	core_fclose(file);
	free(tempbuffer);
	free(chartable);
	return 0;

error:
	core_fclose(file);
	osd_rmfile(filename);
	free(tempbuffer);
	free(chartable);
	return 1;
}


/*-------------------------------------------------
    bitmap_to_chars - convert a bitmap to
    characters in the given font
-------------------------------------------------*/

static int bitmap_to_chars(bitmap_t *bitmap, render_font *font)
{
	int rowstart = 0;
	int x, y;

	/* loop over rows */
	while (rowstart < bitmap->height)
	{
		int rowend, baseline, colstart;
		int chstart;

		/* find the top of the row */
		for ( ; rowstart < bitmap->height; rowstart++)
			if (pixel_is_set(bitmap, rowstart, 0))
				break;
		if (rowstart >= bitmap->height)
			break;

		/* find the bottom of the row */
		for (rowend = rowstart + 1; rowend < bitmap->height; rowend++)
			if (!pixel_is_set(bitmap, rowend, 0))
			{
				rowend--;
				break;
			}

		/* find the baseline */
		for (baseline = rowstart; baseline <= rowend; baseline++)
			if (pixel_is_set(bitmap, baseline, 1))
				break;
		if (baseline > rowend)
		{
			fprintf(stderr, "No baseline found between rows %d-%d\n", rowstart, rowend);
			break;
		}

		/* set or confirm the height */
		if (font->height == 0)
		{
			font->height = rowend - rowstart + 1;
			font->yoffs = baseline - rowend;
		}
		else
		{
			if (font->height != rowend - rowstart + 1)
			{
				fprintf(stderr, "Inconsistent font height at rows %d-%d\n", rowstart, rowend);
				break;
			}
			if (font->yoffs != baseline - rowend)
			{
				fprintf(stderr, "Inconsistent baseline at rows %d-%d\n", rowstart, rowend);
				break;
			}
		}

		/* decode the starting character */
		chstart = 0;
		for (x = 0; x < 4; x++)
			for (y = 0; y < 4; y++)
				chstart = (chstart << 1) | pixel_is_set(bitmap, rowstart + y, 2 + x);

		/* print info */
//      printf("Row %d-%d, baseline %d, character start %X\n", rowstart, rowend, baseline, chstart);

		/* scan the column to find characters */
		colstart = 0;
		while (colstart < bitmap->width)
		{
			render_font_char *ch = &font->chars[chstart];
			int colend;

			/* find the start of the character */
			for ( ; colstart < bitmap->width; colstart++)
				if (pixel_is_set(bitmap, rowend + 2, colstart))
					break;
			if (colstart >= bitmap->width)
				break;

			/* find the end of the character */
			for (colend = colstart + 1; colend < bitmap->width; colend++)
				if (!pixel_is_set(bitmap, rowend + 2, colend))
				{
					colend--;
					break;
				}

			/* print info */
//          printf("  Character %X - width = %d\n", chstart, colend - colstart + 1);

			/* allocate a bitmap */
			ch->bitmap = bitmap_alloc(colend - colstart + 1, font->height, BITMAP_FORMAT_ARGB32);
			if (ch->bitmap == NULL)
			{
				fprintf(stderr, "Error allocating character bitmap (%dx%d)\n", colend - colstart + 1, font->height);
				continue;
			}

			/* plot the character */
			for (y = rowstart; y <= rowend; y++)
				for (x = colstart; x <= colend; x++)
					*BITMAP_ADDR32(ch->bitmap, y - rowstart, x - colstart) = pixel_is_set(bitmap, y, x) ? 0xffffffff : 0x00000000;

			/* set the character parameters */
			ch->width = colend - colstart + 1;
			ch->xoffs = 0;
			ch->yoffs = font->yoffs;
			ch->bmwidth = ch->bitmap->width;
			ch->bmheight = ch->bitmap->height;

			/* next character */
			chstart++;
			colstart = colend + 1;
		}

		/* next row */
		rowstart = rowend + 1;
	}

	/* return non-zero (TRUE) if we errored */
	return (rowstart < bitmap->height);
}


/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	const char *bdcname;
	render_font *font;
	int error = FALSE;
	int curarg;

	/* validate arguments */
    if (argc < 3)
    {
    	fprintf(stderr, "Usage:\npng2bdf <input.png> [<input2.png> [...]] <output.bdc>\n");
    	return 1;
    }
    bdcname = argv[argc - 1];

	/* allocate a font */
	font = malloc(sizeof(*font));
	if (font == NULL)
		return 1;
	memset(font, 0, sizeof(*font));

	/* iterate over input files */
	for (curarg = 1; curarg < argc - 1; curarg++)
	{
		const char *pngname = argv[curarg];
		file_error filerr;
		png_error pngerr;
		bitmap_t *bitmap;
		core_file *file;

	    /* load the png file */
		filerr = core_fopen(pngname, OPEN_FLAG_READ, &file);
		if (filerr != FILERR_NONE)
	    {
	    	fprintf(stderr, "Error %d attempting to open PNG file\n", filerr);
	    	error = TRUE;
	    	break;
	    }
		pngerr = png_read_bitmap(file, &bitmap);
		core_fclose(file);
		if (pngerr != PNGERR_NONE)
		{
			fprintf(stderr, "Error %d reading PNG file\n", pngerr);
			error = TRUE;
			break;
		}

		/* parse the PNG into characters */
		error = bitmap_to_chars(bitmap, font);
		bitmap_free(bitmap);
		if (error)
			break;
	}

	/* write out the resulting font */
	if (!error)
		render_font_save_cached(font, bdcname, 0);

	/* cleanup after ourselves */
	free(font);
    return error;
}
