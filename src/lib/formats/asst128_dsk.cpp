// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/asst128_dsk.c

    asst128 format

*********************************************************************/

#include <assert.h>

#include "formats/asst128_dsk.h"

asst128_format::asst128_format() : upd765_format(formats)
{
}

const char *asst128_format::name() const
{
	return "asst128";
}

const char *asst128_format::description() const
{
	return "ASST128 disk image";
}

const char *asst128_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
const asst128_format::format asst128_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000,  9, 80, 1, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_ASST128_FORMAT = &floppy_image_format_creator<asst128_format>;
