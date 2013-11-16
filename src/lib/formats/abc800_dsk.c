// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc800_dsk.c

    Luxor ABC 830/832/838 disk image formats

*********************************************************************/

#include "emu.h"
#include "formats/abc800_dsk.h"

abc800_format::abc800_format() : wd177x_format(formats)
{
}

const char *abc800_format::name() const
{
	return "abc800";
}

const char *abc800_format::description() const
{
	return "Luxor ABC 830/832/838 disk image";
}

const char *abc800_format::extensions() const
{
	return "dsk";
}

const abc800_format::format abc800_format::formats[] = {
/*	{   //  80K 5 1/4 inch single density double sided
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		2000,  8, 40, 2, 256, {}, 1, {}, 54, 22, 0
	},

	{   //  160K 5 1/4 inch double density double sided
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  8, 40, 2, 256, {}, 1, {}, 54, 22, 0
	},
*/
	// track description 
	// 55x4e 12x00 3xf5 fe 2x00 2x01 f7 22x4e 12x00 3xf5 fb 256xe5 f7 
	// 54x4e 12x00 3xf5 fe 2x00 02 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 03 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 04 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 05 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 06 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 07 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 08 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 09 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0a 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0b 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0c 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0d 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0e 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0f 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 10 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 298x4e
	{   //  640K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 1, {}, 54, 22, 0
	},
/*
	{   //  1M 8 inch double density
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 77, 2, 256, {}, 1, {}, 55, 22, 84
	},
*/
	{}
};

const floppy_format_type FLOPPY_ABC800_FORMAT = &floppy_image_format_creator<abc800_format>;
