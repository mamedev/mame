// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/itt3030_dsk.c

    ITT3030 560K disk image format



*********************************************************************/

#include <assert.h>

#include "formats/itt3030_dsk.h"

itt3030_format::itt3030_format() : wd177x_format(formats)
{
}

const char *itt3030_format::name() const
{
	return "itt3030";
}

const char *itt3030_format::description() const
{
	return "ITT3030 disk image";
}

const char *itt3030_format::extensions() const
{
	return "dsk";
}

// gap info is a total guess
const itt3030_format::format itt3030_format::formats[] = {
	{   /*  5,25" DS DD 70 tracks 16 SPT 256 bytes/sector */
		floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 70, 2, 256, {}, 1, {}, 32, 22, 31
	},
	{}
};



const floppy_format_type FLOPPY_ITT3030_FORMAT = &floppy_image_format_creator<itt3030_format>;
