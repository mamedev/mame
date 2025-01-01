// license: BSD-3-Clause
// copyright-holders: Dirk Best, Märt Põder
/***************************************************************************

    Juku E5101/E5104

    Disk image format

***************************************************************************/

#include "juku_dsk.h"

juku_format::juku_format() : wd177x_format(formats)
{
}

const char *juku_format::name() const noexcept
{
	return "juku";
}

const char *juku_format::description() const noexcept
{
	return "Juku disk image";
}

const char *juku_format::extensions() const noexcept
{
	return "juk";
}

const juku_format::format juku_format::formats[] =
{
	{   //  386k 5.25" double density single sided - gaps unverified (CP/M)
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 80, 1, 512, {}, 1, {}, 32, 22, 35
	},
	{   //  786k 5.25" double density "outout" double sided - gaps unverified (CP/M)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 32, 22, 35
	},
	{}
};

const juku_format FLOPPY_JUKU_FORMAT;
