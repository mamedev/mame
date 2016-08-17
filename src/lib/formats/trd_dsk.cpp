// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/trd_dsk.c

    TRD disk images

*********************************************************************/

#include <assert.h>

#include "formats/trd_dsk.h"

trd_format::trd_format() : wd177x_format(formats)
{
}

const char *trd_format::name() const
{
	return "trd";
}

const char *trd_format::description() const
{
	return "TRD floppy disk image";
}

const char *trd_format::extensions() const
{
	return "trd";
}

const trd_format::format trd_format::formats[] = {
	{   //  5"25 640K double density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		// GAP4A - 0(!) bytes, GAP1 - 10 bytes, GAP2 - 22 bytes, GAP3 - 60 bytes, GAP4B - upto track end
		2000,  16, 80, 2, 256, {}, 1, {}, 10, 22, 60
	},
	{}
};

const floppy_format_type FLOPPY_TRD_FORMAT = &floppy_image_format_creator<trd_format>;
