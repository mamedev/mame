// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/sf7000_dsk.c

    sf7000 format

*********************************************************************/

#include <assert.h>

#include "formats/sf7000_dsk.h"

sf7000_format::sf7000_format() : upd765_format(formats)
{
}

const char *sf7000_format::name() const
{
	return "sf7";
}

const char *sf7000_format::description() const
{
	return "SF7 disk image";
}

const char *sf7000_format::extensions() const
{
	return "sf7";
}

const sf7000_format::format sf7000_format::formats[] = {
	{
	// mfm h=00 n=01 sc=10 gpl=2a d=ff
		floppy_image::FF_3, floppy_image::SSDD, floppy_image::MFM,
		2000, // 2us, 300rpm
		16, 40, 1,
		256, {},
		1, {},
		80, 50, 22, 42
	},
	{}
};

const floppy_format_type FLOPPY_SF7000_FORMAT = &floppy_image_format_creator<sf7000_format>;
