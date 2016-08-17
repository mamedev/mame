// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/kc85_dsk.c

    kc85 format

*********************************************************************/

#include <assert.h>

#include "formats/kc85_dsk.h"

kc85_format::kc85_format() : upd765_format(formats)
{
}

const char *kc85_format::name() const
{
	return "kc85";
}

const char *kc85_format::description() const
{
	return "KC85 disk image";
}

const char *kc85_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
const kc85_format::format kc85_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		5, 80, 2,
		1024, {},
		1, {},
		80, 50, 22, 80
	},
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		9, 80, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		16, 80, 2,
		256, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_KC85_FORMAT = &floppy_image_format_creator<kc85_format>;
