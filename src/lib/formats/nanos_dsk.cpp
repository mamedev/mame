// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/nanos_dsk.c

    nanos format

*********************************************************************/

#include <assert.h>

#include "formats/nanos_dsk.h"

nanos_format::nanos_format() : upd765_format(formats)
{
}

const char *nanos_format::name() const
{
	return "nanos";
}

const char *nanos_format::description() const
{
	return "NANOS disk image";
}

const char *nanos_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
// 800K on HD which handles 1.2M, really?
const nanos_format::format nanos_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, // 1us, 360rpm
		5, 80, 2,
		1024, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_NANOS_FORMAT = &floppy_image_format_creator<nanos_format>;
