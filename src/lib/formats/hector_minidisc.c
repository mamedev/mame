// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Hector Minidisc

    Disk image format

***************************************************************************/

#include "hector_minidisc.h"

hmd_format::hmd_format() : upd765_format(formats)
{
}

const char *hmd_format::name() const
{
	return "hector_minidisc";
}

const char *hmd_format::description() const
{
	return "Hector Minidisc disk image";
}

const char *hmd_format::extensions() const
{
	return "hmd";
}

const hmd_format::format hmd_format::formats[] =
{
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 70, 2, 512, {}, 1, {}, 80, 50, 22, 54
	},
	{}
};

const floppy_format_type FLOPPY_HMD_FORMAT = &floppy_image_format_creator<hmd_format>;
