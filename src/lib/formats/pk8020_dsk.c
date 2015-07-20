// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    PK-8020

    Disk image format

    TODO:
    - Verify gap sizes

***************************************************************************/

#include "pk8020_dsk.h"

pk8020_format::pk8020_format() : wd177x_format(formats)
{
}

const char *pk8020_format::name() const
{
	return "pk8020";
}

const char *pk8020_format::description() const
{
	return "PK-8020 disk image";
}

const char *pk8020_format::extensions() const
{
	return "kdi";
}

const pk8020_format::format pk8020_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 1, {}, 60, 22, 24
	},
	{}
};

const floppy_format_type FLOPPY_PK8020_FORMAT = &floppy_image_format_creator<pk8020_format>;
