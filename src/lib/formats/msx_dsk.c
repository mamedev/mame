// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/msx_dsk.c

    MSX disk images

*********************************************************************/

#include "msx_dsk.h"

//msx_format::msx_format() : wd177x_format(formats)
msx_format::msx_format() : upd765_format(formats)
{
}

const char *msx_format::name() const
{
	return "msx";
}

const char *msx_format::description() const
{
	return "MSX disk image";
}

const char *msx_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const msx_format::format msx_format::formats[] = {
	{   /*  3 1/2 inch single sided double density */
		// 80x4e 12x00 3xf6 fc
		// 26x4e 12x00 3xf5 fe 2x00 01 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 02 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 03 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 04 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 05 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 06 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 07 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 08 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 54x4e 12x00 3xf5 fe 2x00 09 02 f7 24x4e 12x00 3xf5 fb 512xe5 f7
		// 1078x4e
		floppy_image::FF_35,  floppy_image::SSDD, floppy_image::MFM,
		2000,  9, 80, 1, 512, {}, 1, {}, 26, 24, 28, 80
	},
	{   /*  3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 26, 24, 28, 80
	},
	{   /*  3 1/2 inch double density - 81 tracks */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 81, 2, 512, {}, 1, {}, 26, 24, 28, 80
	},
	{}
};

const floppy_format_type FLOPPY_MSX_FORMAT = &floppy_image_format_creator<msx_format>;
