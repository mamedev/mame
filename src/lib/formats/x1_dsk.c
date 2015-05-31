// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Sharp X1

    Disk image format

***************************************************************************/

#include "x1_dsk.h"

x1_format::x1_format() : wd177x_format(formats)
{
}

const char *x1_format::name() const
{
	return "x1";
}

const char *x1_format::description() const
{
	return "Sharp X1 disk image";
}

const char *x1_format::extensions() const
{
	return "2d";
}

const x1_format::format x1_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, 1, {}, 32, 22, 54
	},
	{}
};

const floppy_format_type FLOPPY_X1_FORMAT = &floppy_image_format_creator<x1_format>;
