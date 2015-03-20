// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/a5105_dsk.c

    a5105 format

*********************************************************************/

#include <assert.h>

#include "formats/a5105_dsk.h"

a5105_format::a5105_format() : upd765_format(formats)
{
}

const char *a5105_format::name() const
{
	return "a5105";
}

const char *a5105_format::description() const
{
	return "A5105 disk image";
}

const char *a5105_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
const a5105_format::format a5105_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		5, 80, 2,
		1024, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_A5105_FORMAT = &floppy_image_format_creator<a5105_format>;
