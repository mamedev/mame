// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Juku E5101

    Disk image format

***************************************************************************/

#include "juku_dsk.h"

juku_format::juku_format() : wd177x_format(formats)
{
}

const char *juku_format::name() const
{
	return "juku";
}

const char *juku_format::description() const
{
	return "Juku disk image";
}

const char *juku_format::extensions() const
{
	return "juk";
}

const juku_format::format juku_format::formats[] =
{
	{   //  800k 5 1/4 inch double density single sided - gaps unverified (CP/M)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 1, 512, {}, 1, {}, 32, 22, 35
	},
	{   //  800k 5 1/4 inch double density double sided - gaps unverified (CP/M)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 32, 22, 35
	},
	{}
};

const floppy_format_type FLOPPY_JUKU_FORMAT = &floppy_image_format_creator<juku_format>;
