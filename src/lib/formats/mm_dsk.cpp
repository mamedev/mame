// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/mm_dsk.c

    mm format

*********************************************************************/

#include <assert.h>

#include "formats/mm_dsk.h"

mm1_format::mm1_format() : upd765_format(formats)
{
}

const char *mm1_format::name() const
{
	return "mm1";
}

const char *mm1_format::description() const
{
	return "Nokia MikroMikko 1 disk image";
}

const char *mm1_format::extensions() const
{
	return "dsk";
}

mm2_format::mm2_format() : upd765_format(formats)
{
}

const char *mm2_format::name() const
{
	return "mm2";
}

const char *mm2_format::description() const
{
	return "Nokia MikroMikko 2 disk image";
}

const char *mm2_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const mm1_format::format mm1_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		8, 80, 2,
		512, {},
		-1, { 1,4,7,2,5,8,3,6 },
		80, 50, 22, 80
	},
	{}
};

// Unverified gap sizes
const mm2_format::format mm2_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, // 2us, 300rpm
		9, 40, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	// 40 tracks but 18 sectors implying HD density at 300rpm, i.e. on
	// 3.5" media?  That makes no sense
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1000, // 1us, 300rpm, otherwise it just won't fit
		18, 40, 2, // That line is just nonsense
		512, {},
		1, {},
		80, 50, 22, 80
	}
};

const floppy_format_type FLOPPY_MM1_FORMAT = &floppy_image_format_creator<mm1_format>;
const floppy_format_type FLOPPY_MM2_FORMAT = &floppy_image_format_creator<mm2_format>;
