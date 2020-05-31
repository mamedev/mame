// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/trd_dsk.c

    TRD disk images

*********************************************************************/

#include <cassert>

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
	{   // 5"25 640K 80 track double sided double density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		// GAP4A - 0(!!!) bytes, GAP1 - 10 bytes, GAP2 - 22 bytes, GAP3 - 60 bytes, GAP4B - upto track end
		2000, 16, 80, 2, 256, {}, -1, { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 }, 10, 22, 60
	},
	{   // 5"25 400K 80 track double sided single density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::FM,
		// GAPs and interleave probably not correct
		4000, 10, 80, 2, 256, {}, 1, {}, 40, 10, 10
	},
	{   // 5"25 280K 80 track double sided single density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::FM,
		// GAPs and interleave probably not correct
		4000,  7, 80, 2, 256, {}, 1, {}, 40, 10, 10
	},
	{}
};

const floppy_format_type FLOPPY_TRD_FORMAT = &floppy_image_format_creator<trd_format>;
