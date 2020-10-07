// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    pngcmp.c

    PNG comparison (based on regrep.c)

****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>
#include "osdcore.h"
#include "png.h"

#include <new>

/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define BITMAP_SPACE            4

/***************************************************************************
    PROTOTYPES
***************************************************************************/

static int generate_png_diff(const std::string& imgfile1, const std::string& imgfile2, const std::string& outfilename);

/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	/* first argument is the directory */
	if (argc < 4)
	{
		fprintf(stderr, "Usage:\npngcmp <image1> <image2> <outfile>\n");
		return 10;
	}
	std::string imgfilename1(argv[1]);
	std::string imgfilename2(argv[2]);
	std::string outfilename(argv[3]);

	try {
		return generate_png_diff(imgfilename1, imgfilename2, outfilename);
	}
	catch(...)
	{
		printf("Exception occurred");
		return 1000;
	}
}

/*-------------------------------------------------
    generate_png_diff - create a new PNG file
    that shows multiple differing PNGs side by
    side with a third set of differences
-------------------------------------------------*/

static int generate_png_diff(const std::string& imgfile1, const std::string& imgfile2, const std::string& outfilename)
{
	bitmap_argb32 bitmap1;
	bitmap_argb32 bitmap2;
	bitmap_argb32 finalbitmap;
	int width, height, maxwidth;
	util::core_file::ptr file;
	osd_file::error filerr;
	util::png_error pngerr;
	int error = 100;

	/* open the source image */
	filerr = util::core_file::open(imgfile1, OPEN_FLAG_READ, file);
	if (filerr != osd_file::error::NONE)
	{
		printf("Could not open %s (%d)\n", imgfile1.c_str(), int(filerr));
		goto error;
	}

	/* load the source image */
	pngerr = util::png_read_bitmap(*file, bitmap1);
	file.reset();
	if (pngerr != util::png_error::NONE)
	{
		printf("Could not read %s (%d)\n", imgfile1.c_str(), int(pngerr));
		goto error;
	}

	/* open the source image */
	filerr = util::core_file::open(imgfile2, OPEN_FLAG_READ, file);
	if (filerr != osd_file::error::NONE)
	{
		printf("Could not open %s (%d)\n", imgfile2.c_str(), int(filerr));
		goto error;
	}

	/* load the source image */
	pngerr = util::png_read_bitmap(*file, bitmap2);
	file.reset();
	if (pngerr != util::png_error::NONE)
	{
		printf("Could not read %s (%d)\n", imgfile2.c_str(), int(pngerr));
		goto error;
	}

	/* if the sizes are different, we differ; otherwise start off assuming we are the same */
	bool bitmaps_differ;
	bitmaps_differ = (bitmap2.width() != bitmap1.width() || bitmap2.height() != bitmap1.height());

	/* compare scanline by scanline */
	for (int y = 0; y < bitmap2.height() && !bitmaps_differ; y++)
	{
		uint32_t const *base = &bitmap1.pix(y);
		uint32_t const *curr = &bitmap2.pix(y);

		/* scan the scanline */
		int x;
		for (x = 0; x < bitmap2.width(); x++)
			if (*base++ != *curr++)
				break;
		bitmaps_differ = (x != bitmap2.width());
	}

	if (bitmaps_differ)
	{
		/* determine the size of the final bitmap */
		height = width = 0;
		{
			/* determine the maximal width */
			maxwidth = std::max(bitmap1.width(), bitmap2.width());
			width = bitmap1.width() + BITMAP_SPACE + maxwidth + BITMAP_SPACE + maxwidth;

			/* add to the height */
			height += std::max(bitmap1.height(), bitmap2.height());
		}

		/* allocate the final bitmap */
		finalbitmap.allocate(width, height);

		/* now copy and compare each set of bitmaps */
		int curheight = std::max(bitmap1.height(), bitmap2.height());
		/* iterate over rows in these bitmaps */
		for (int y = 0; y < curheight; y++)
		{
			uint32_t const *src1 = (y < bitmap1.height()) ? &bitmap1.pix(y) : nullptr;
			uint32_t const *src2 = (y < bitmap2.height()) ? &bitmap2.pix(y) : nullptr;
			uint32_t *dst1 = &finalbitmap.pix(y);
			uint32_t *dst2 = &finalbitmap.pix(y, bitmap1.width() + BITMAP_SPACE);
			uint32_t *dstdiff = &finalbitmap.pix(y, bitmap1.width() + BITMAP_SPACE + maxwidth + BITMAP_SPACE);

			/* now iterate over columns */
			for (int x = 0; x < maxwidth; x++)
			{
				int pix1 = -1, pix2 = -2;

				if (src1 != nullptr && x < bitmap1.width())
					pix1 = dst1[x] = src1[x];
				if (src2 != nullptr && x < bitmap2.width())
					pix2 = dst2[x] = src2[x];
				dstdiff[x] = (pix1 != pix2) ? 0xffffffff : 0xff000000;
			}
		}

		/* write the final PNG */
		filerr = util::core_file::open(outfilename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, file);
		if (filerr != osd_file::error::NONE)
		{
			printf("Could not open %s (%d)\n", outfilename.c_str(), int(filerr));
			goto error;
		}
		pngerr = util::png_write_bitmap(*file, nullptr, finalbitmap, 0, nullptr);
		file.reset();
		if (pngerr != util::png_error::NONE)
		{
			printf("Could not write %s (%d)\n", outfilename.c_str(), int(pngerr));
			goto error;
		}
	}

	/* if we get here, we are error free */
	if (bitmaps_differ)
		error = 1;
	else
		error = 0;

error:
	if (error == -1)
		osd_file::remove(outfilename);
	return error;
}
