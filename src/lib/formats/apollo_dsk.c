// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/apollo_dsk.c

    apollo format

*********************************************************************/

#include <assert.h>

#include "formats/apollo_dsk.h"

apollo_format::apollo_format() : upd765_format(formats)
{
}

const char *apollo_format::name() const
{
	return "apollo";
}

const char *apollo_format::description() const
{
	return "APOLLO disk image";
}

const char *apollo_format::extensions() const
{
	return "afd";
}

// Unverified gap sizes
const apollo_format::format apollo_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, // 1us, 360rpm
		8, 77, 2,
		1024, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_APOLLO_FORMAT = &floppy_image_format_creator<apollo_format>;

int apollo_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);
	UINT32 expected_size = 77*2*8*1024;

	return ((size == expected_size) || (size == 0)) ? 1 : 0;
}

