// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Vector 06

    Disk image format

    TODO:
    - Gap sizes

***************************************************************************/

#include "vector06_dsk.h"

vector06_format::vector06_format() : wd177x_format(formats)
{
}

const char *vector06_format::name() const
{
	return "vector06";
}

const char *vector06_format::description() const
{
	return "Vector 06 disk image";
}

const char *vector06_format::extensions() const
{
	return "fdd";
}

const vector06_format::format vector06_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 5, 82, 2, 1024, {}, 1, {}, 80, 22, 24
	},
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 1, {}, 80, 22, 24
	},
	{}
};

const floppy_format_type FLOPPY_VECTOR06_FORMAT = &floppy_image_format_creator<vector06_format>;
