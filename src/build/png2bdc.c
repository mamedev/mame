// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    png2bdc.c

    Super-simple PNG to BDC file generator

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
#include <new>
#include "png.h"


//**************************************************************************
//  CONSTANTS & DEFINES
//**************************************************************************

#define CACHED_CHAR_SIZE        12
#define CACHED_HEADER_SIZE      16



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// a render_font contains information about a single character in a font
struct render_font_char
{
	render_font_char() : width(0), xoffs(0), yoffs(0), bmwidth(0), bmheight(0) { }

	INT32               width;              // width from this character to the next
	INT32               xoffs, yoffs;       // X and Y offset from baseline to top,left of bitmap
	INT32               bmwidth, bmheight;  // width and height of bitmap
	bitmap_argb32 *     bitmap;             // pointer to the bitmap containing the raw data
};


// a render_font contains information about a font
struct render_font
{
	render_font() : height(0), yoffs(0) { }

	int                 height;             // height of the font, from ascent to descent
	int                 yoffs;              // y offset from baseline to descent
	render_font_char    chars[65536];       // array of characters
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline int pixel_is_set(bitmap_argb32 &bitmap, int y, int x)
{
	return (bitmap.pix32(y, x) & 0xffffff) == 0;
}



//**************************************************************************
//  MAIN
//**************************************************************************

//-------------------------------------------------
//  write_data - write data to the given file and
//  throw an exception if an error occurs
//-------------------------------------------------

static void write_data(core_file &file, UINT8 *base, UINT8 *end)
{
	UINT32 bytes_written = core_fwrite(&file, base, end - base);
	if (bytes_written != end - base)
	{
		fprintf(stderr, "Error writing to destination file\n");
		throw;
	}
}


//-------------------------------------------------
//  render_font_save_cached - write the cached
//  data out to the file
//-------------------------------------------------

static bool render_font_save_cached(render_font &font, const char *filename, UINT32 hash)
{
	// attempt to open the file
	core_file *file;
	file_error filerr = core_fopen(filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	if (filerr != FILERR_NONE)
		return true;

	try
	{
		// determine the number of characters
		int numchars = 0;
		for (int chnum = 0; chnum < 65536; chnum++)
			if (font.chars[chnum].width > 0)
				numchars++;

		// write the header
		dynamic_buffer tempbuffer(65536);
		UINT8 *dest = &tempbuffer[0];
		*dest++ = 'f';
		*dest++ = 'o';
		*dest++ = 'n';
		*dest++ = 't';
		*dest++ = hash >> 24;
		*dest++ = hash >> 16;
		*dest++ = hash >> 8;
		*dest++ = hash & 0xff;
		*dest++ = font.height >> 8;
		*dest++ = font.height & 0xff;
		*dest++ = font.yoffs >> 8;
		*dest++ = font.yoffs & 0xff;
		*dest++ = numchars >> 24;
		*dest++ = numchars >> 16;
		*dest++ = numchars >> 8;
		*dest++ = numchars & 0xff;
		write_data(*file, tempbuffer, dest);

		// write the empty table to the beginning of the file
		dynamic_buffer chartable(numchars * CACHED_CHAR_SIZE + 1, 0);
		write_data(*file, &chartable[0], &chartable[numchars * CACHED_CHAR_SIZE]);

		// loop over all characters
		int tableindex = 0;
		for (int chnum = 0; chnum < 65536; chnum++)
		{
			render_font_char &ch = font.chars[chnum];
			if (ch.width > 0)
			{
				// write out a bit-compressed bitmap if we have one
				if (ch.bitmap != NULL)
				{
					// write the data to the tempbuffer
					dest = tempbuffer;
					UINT8 accum = 0;
					UINT8 accbit = 7;

					// bit-encode the character data
					for (int y = 0; y < ch.bmheight; y++)
					{
						int desty = y + font.height + font.yoffs - ch.yoffs - ch.bmheight;
						const UINT32 *src = (desty >= 0 && desty < font.height) ? &ch.bitmap->pix32(desty) : NULL;
						for (int x = 0; x < ch.bmwidth; x++)
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

					// flush any extra
					if (accbit != 7)
						*dest++ = accum;

					// write the data
					write_data(*file, tempbuffer, dest);

					// free the bitmap and texture
					global_free(ch.bitmap);
					ch.bitmap = NULL;
				}

				// compute the table entry
				dest = &chartable[tableindex++ * CACHED_CHAR_SIZE];
				*dest++ = chnum >> 8;
				*dest++ = chnum & 0xff;
				*dest++ = ch.width >> 8;
				*dest++ = ch.width & 0xff;
				*dest++ = ch.xoffs >> 8;
				*dest++ = ch.xoffs & 0xff;
				*dest++ = ch.yoffs >> 8;
				*dest++ = ch.yoffs & 0xff;
				*dest++ = ch.bmwidth >> 8;
				*dest++ = ch.bmwidth & 0xff;
				*dest++ = ch.bmheight >> 8;
				*dest++ = ch.bmheight & 0xff;
			}
		}

		// seek back to the beginning and rewrite the table
		core_fseek(file, CACHED_HEADER_SIZE, SEEK_SET);
		write_data(*file, &chartable[0], &chartable[numchars * CACHED_CHAR_SIZE]);

		// all done
		core_fclose(file);
		return false;
	}
	catch (...)
	{
		core_fclose(file);
		osd_rmfile(filename);
		return true;
	}
}


//-------------------------------------------------
//  bitmap_to_chars - convert a bitmap to
//  characters in the given font
//-------------------------------------------------

static bool bitmap_to_chars(bitmap_argb32 &bitmap, render_font &font)
{
	// loop over rows
	int rowstart = 0;
	while (rowstart < bitmap.height())
	{
		// find the top of the row
		for ( ; rowstart < bitmap.height(); rowstart++)
			if (pixel_is_set(bitmap, rowstart, 0))
				break;
		if (rowstart >= bitmap.height())
			break;

		// find the bottom of the row
		int rowend;
		for (rowend = rowstart + 1; rowend < bitmap.height(); rowend++)
			if (!pixel_is_set(bitmap, rowend, 0))
			{
				rowend--;
				break;
			}

		// find the baseline
		int baseline;
		for (baseline = rowstart; baseline <= rowend; baseline++)
			if (pixel_is_set(bitmap, baseline, 1))
				break;
		if (baseline > rowend)
		{
			fprintf(stderr, "No baseline found between rows %d-%d\n", rowstart, rowend);
			break;
		}

		// set or confirm the height
		if (font.height == 0)
		{
			font.height = rowend - rowstart + 1;
			font.yoffs = baseline - rowend;
		}
		else
		{
			if (font.height != rowend - rowstart + 1)
			{
				fprintf(stderr, "Inconsistent font height at rows %d-%d\n", rowstart, rowend);
				break;
			}
			if (font.yoffs != baseline - rowend)
			{
				fprintf(stderr, "Inconsistent baseline at rows %d-%d\n", rowstart, rowend);
				break;
			}
		}

		// decode the starting character
		int chstart = 0;
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				chstart = (chstart << 1) | pixel_is_set(bitmap, rowstart + y, 2 + x);

		// print info
//      printf("Row %d-%d, baseline %d, character start %X\n", rowstart, rowend, baseline, chstart);

		// scan the column to find characters
		int colstart = 0;
		while (colstart < bitmap.width())
		{
			render_font_char &ch = font.chars[chstart];

			// find the start of the character
			for ( ; colstart < bitmap.width(); colstart++)
				if (pixel_is_set(bitmap, rowend + 2, colstart))
					break;
			if (colstart >= bitmap.width())
				break;

			// find the end of the character
			int colend;
			for (colend = colstart + 1; colend < bitmap.width(); colend++)
				if (!pixel_is_set(bitmap, rowend + 2, colend))
				{
					colend--;
					break;
				}

			// skip char which code is already registered
			if (ch.width <= 0)
			{
				// print info
//              printf("  Character %X - width = %d\n", chstart, colend - colstart + 1);

				// allocate a bitmap
				ch.bitmap = global_alloc(bitmap_argb32(colend - colstart + 1, font.height));

				// plot the character
				for (int y = rowstart; y <= rowend; y++)
					for (int x = colstart; x <= colend; x++)
						ch.bitmap->pix32(y - rowstart, x - colstart) = pixel_is_set(bitmap, y, x) ? 0xffffffff : 0x00000000;

				// set the character parameters
				ch.width = colend - colstart + 1;
				ch.xoffs = 0;
				ch.yoffs = font.yoffs;
				ch.bmwidth = ch.bitmap->width();
				ch.bmheight = ch.bitmap->height();
			}

			// next character
			chstart++;
			colstart = colend + 1;
		}

		// next row
		rowstart = rowend + 1;
	}

	// return non-zero (TRUE) if we errored
	return (rowstart < bitmap.height());
}


//-------------------------------------------------
//  main - main entry point
//-------------------------------------------------

int main(int argc, char *argv[])
{
	// validate arguments
	if (argc < 3)
	{
		fprintf(stderr, "Usage:\n%s <input.png> [<input2.png> [...]] <output.bdc>\n", argv[0]);
		return 1;
	}
	const char *bdcname = argv[argc - 1];

	// iterate over input files
	static render_font font;
	bool error = false;
	for (int curarg = 1; curarg < argc - 1; curarg++)
	{
		// load the png file
		const char *pngname = argv[curarg];
		core_file *file;
		file_error filerr = core_fopen(pngname, OPEN_FLAG_READ, &file);
		if (filerr != FILERR_NONE)
		{
			fprintf(stderr, "Error %d attempting to open PNG file\n", filerr);
			error = true;
			break;
		}

		bitmap_argb32 bitmap;
		png_error pngerr = png_read_bitmap(file, bitmap);
		core_fclose(file);
		if (pngerr != PNGERR_NONE)
		{
			fprintf(stderr, "Error %d reading PNG file\n", pngerr);
			error = true;
			break;
		}

		// parse the PNG into characters
		error = bitmap_to_chars(bitmap, font);
		if (error)
			break;
	}

	// write out the resulting font
	if (!error)
		error = render_font_save_cached(font, bdcname, 0);

	// cleanup after ourselves
	return error ? 1 : 0;
}
