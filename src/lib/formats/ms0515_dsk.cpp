// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/ms0515_dsk.cpp

    ms0515 format

*********************************************************************/

#include "formats/ms0515_dsk.h"

ms0515_format::ms0515_format() : wd177x_format(formats)
{
}

const char *ms0515_format::name() const
{
	return "ms0515";
}

const char *ms0515_format::description() const
{
	return "MS 0515 disk image";
}

const char *ms0515_format::extensions() const
{
	return "img";
}

// gap sizes taken from FORML.SAV
const ms0515_format::format ms0515_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, // 2us, 300rpm
		10, 80, 1,
		512, {},
		1, {},
		50, 22, 30
	},
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, // 2us, 300rpm
		10, 80, 2,
		512, {},
		1, {},
		50, 22, 30
	},
	{}
};

const floppy_format_type FLOPPY_MS0515_FORMAT = &floppy_image_format_creator<ms0515_format>;
