// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Thompson CD90-640 FDC

    Disk image format

    TODO:
    - Gap sizes unverified for FM

***************************************************************************/

#include "cd90_640_dsk.h"

cd90_640_format::cd90_640_format() : wd177x_format(formats)
{
}

const char *cd90_640_format::name() const
{
	return "cd90_640";
}

const char *cd90_640_format::description() const
{
	return "CD90-640 disk image";
}

const char *cd90_640_format::extensions() const
{
	return "fd";
}

const cd90_640_format::format cd90_640_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 16, 40, 1, 128, {}, 1, {}, 14, 11, 12
	},
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 1, {}, 31, 22, 44
	},
	{
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 16, 40, 2, 128, {}, 1, {}, 14, 11, 12
	},
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, 1, {}, 31, 22, 44
	},
	{}
};

const floppy_format_type FLOPPY_CD90_640_FORMAT = &floppy_image_format_creator<cd90_640_format>;
