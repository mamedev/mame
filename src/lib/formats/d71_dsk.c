// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d71_dsk.c

    Commodore 1571 sector disk image format

*********************************************************************/

#include <assert.h>

#include "formats/d71_dsk.h"

d71_format::d71_format() : d64_format(formats)
{
}

const char *d71_format::name() const
{
	return "d71";
}

const char *d71_format::description() const
{
	return "Commodore 1571 disk image";
}

const char *d71_format::extensions() const
{
	return "d71";
}

const d71_format::format d71_format::formats[] = {
	{ // d71, dos 2, 35 tracks, 2 heads, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::DSSD, 683, 35, 2, 256, 9, 8
	},
	{}
};

const floppy_format_type FLOPPY_D71_FORMAT = &floppy_image_format_creator<d71_format>;
