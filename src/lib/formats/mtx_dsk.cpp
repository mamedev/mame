// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Memotech MTX

    Disk image format

***************************************************************************/

#include "mtx_dsk.h"

mtx_format::mtx_format() : wd177x_format(formats)
{
}

const char *mtx_format::name() const
{
	return "mtx";
}

const char *mtx_format::description() const
{
	return "Memotech MTX disk image";
}

const char *mtx_format::extensions() const
{
	return "mfloppy";
}

const mtx_format::format mtx_format::formats[] =
{
	{ // 320k 5 1/4 inch double density single sided (Type 03)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, 1, {}, 32, 22, 54
	},
	{ // 640k 5 1/4 inch double density double sided (Type 07)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 1, {}, 32, 22, 54
	},
	{ // 320k 3 1/2 inch double density single sided (Type 03)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, 1, {}, 32, 22, 54
	},
	{ // 640k 3 1/2 inch double density double sided (Type 07)
		floppy_image::FF_35, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 1, {}, 32, 22, 54
	},
	{}
};

const floppy_format_type FLOPPY_MTX_FORMAT = &floppy_image_format_creator<mtx_format>;
