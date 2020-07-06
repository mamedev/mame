// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/d40_dsk.c

    Didaktik D40/D80 disk images

*********************************************************************/

#include <cassert>

#include "formats/d40_dsk.h"

d40_format::d40_format() : wd177x_format(formats)
{
}

const char *d40_format::name() const
{
	return "d40";
}

const char *d40_format::description() const
{
	return "Didaktik D40/D80 floppy disk image";
}

const char *d40_format::extensions() const
{
	return "d40,d80";
}

const d40_format::format d40_format::formats[] = {
	{   //  400K 5"25 double density, gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, 1, {}, 50, 22, 36
	},
	{   //  720K 3'5 double density, gaps unverified
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_D40D80_FORMAT = &floppy_image_format_creator<d40_format>;
