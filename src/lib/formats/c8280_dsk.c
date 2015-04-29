// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/c8280_dsk.c

    Commodore 8280 disk image format

*********************************************************************/

#include "formats/c8280_dsk.h"

c8280_format::c8280_format() : wd177x_format(formats)
{
}

const char *c8280_format::name() const
{
	return "c8280";
}

const char *c8280_format::description() const
{
	return "Commodore 8280 disk image";
}

const char *c8280_format::extensions() const
{
	return "dsk";
}

const c8280_format::format c8280_format::formats[] = {
	// 80x4e 12x00 3xf6 fc
	// 50x4e 12x00 3xf5 fe 2x00 01 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 02 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 03 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 04 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 05 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 06 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 07 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 08 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 09 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 0a 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 0b 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 0c 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 0d 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 0e 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 0f 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 10 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 11 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 12 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 13 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 14 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 15 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 16 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 17 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 18 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 19 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 54x4e 12x00 3xf5 fe 2x00 1a 01 f7 22x4e 12x00 3xf5 fb 256xaa f7
	// 653x4e
	{
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		1200, 26, 77, 2, 256, {}, 1, {}, 50, 22, 54
	},

	{}
};

const floppy_format_type FLOPPY_C8280_FORMAT = &floppy_image_format_creator<c8280_format>;
