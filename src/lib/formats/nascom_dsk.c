// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom 1/2/3

    Disk image format

***************************************************************************/

#include "nascom_dsk.h"

nascom_format::nascom_format() : wd177x_format(formats)
{
}

const char *nascom_format::name() const
{
	return "nascom";
}

const char *nascom_format::description() const
{
	return "Nascom disk image";
}

const char *nascom_format::extensions() const
{
	return "dsk";
}

const nascom_format::format nascom_format::formats[] =
{
	{   //  320k 5 1/4 inch double density single sided (NASDOS)
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 1, {}, 32, 22, 54
	},
	{   //  640k 5 1/4 inch double density double sided (NASDOS)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 1, {}, 32, 22, 54
	},
	{   //  385k 5 1/4 inch double density single sided (CP/M)
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 10, 77, 1, 512, {}, 1, {}, 32, 22, 32
	},
	{   //  770k 5 1/4 inch double density double sided (CP/M)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 10, 77, 2, 512, {}, 1, {}, 32, 22, 32
	},
	{}
};

const floppy_format_type FLOPPY_NASCOM_FORMAT = &floppy_image_format_creator<nascom_format>;
