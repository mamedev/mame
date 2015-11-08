// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/pyldin_dsk.c

    pyldin format

*********************************************************************/

#include <assert.h>
#include "formats/pyldin_dsk.h"

pyldin_format::pyldin_format() : upd765_format(formats)
{
}

const char *pyldin_format::name() const
{
	return "pyldin";
}

const char *pyldin_format::description() const
{
	return "PYLDIN disk image";
}

const char *pyldin_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
// 720K on HD which handles 1.2M, really?
const pyldin_format::format pyldin_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, // 1us, 360rpm
		9, 80, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_PYLDIN_FORMAT = &floppy_image_format_creator<pyldin_format>;
