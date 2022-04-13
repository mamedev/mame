// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline

    Disk image format

***************************************************************************/

#include "rc759_dsk.h"

rc759_format::rc759_format() : wd177x_format(formats)
{
}

const char *rc759_format::name() const
{
	return "rc759";
}

const char *rc759_format::description() const
{
	return "RC759 disk image";
}

const char *rc759_format::extensions() const
{
	return "img";
}

const rc759_format::format rc759_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, 8, 77, 2, 1024, {}, 1, {}, 50, 22, 54
	},
	{}
};

const rc759_format FLOPPY_RC759_FORMAT;
