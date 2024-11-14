// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "thom_dsk.h"

thomson_525_format::thomson_525_format() : wd177x_format(formats)
{
}

const char *thomson_525_format::name() const noexcept
{
	return "thomson_525";
}

const char *thomson_525_format::description() const noexcept
{
	return "Thomson 5.25 disk image";
}

const char *thomson_525_format::extensions() const noexcept
{
	return "fd";
}

const thomson_525_format::format thomson_525_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000,
		16, 40, 1,
		128, {},
		1, {},
		17, 22, 12
	},
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,
		16, 40, 1,
		256, {},
		1, {},
		31, 22, 44
	},
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,
		16, 40, 2,
		256, {},
		1, {},
		31, 22, 44
	},
	{}
};

int thomson_525_format::get_image_offset(const format &f, int head, int track) const
{
	return (track + (head ? f.track_count : 0)) * compute_track_size(f);
}



thomson_35_format::thomson_35_format() : wd177x_format(formats)
{
}

const char *thomson_35_format::name() const noexcept
{
	return "thomson_35";
}

const char *thomson_35_format::description() const noexcept
{
	return "Thomson 3.5 disk image";
}

const char *thomson_35_format::extensions() const noexcept
{
	return "fd";
}

// 1280K .fd images exist but are not supported. They represent a notional type
// of 4-sided disk that can be inserted into 2 drives at once.
const thomson_35_format::format thomson_35_format::formats[] = {
	{
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000,
		16, 80, 1,
		256, {},
		1, {},
		31, 22, 44
	},
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000,
		16, 80, 2,
		256, {},
		1, {},
		31, 22, 44
	},
	{}
};

int thomson_35_format::get_image_offset(const format &f, int head, int track) const
{
	return (track + (head ? f.track_count : 0)) * compute_track_size(f);
}

const thomson_525_format FLOPPY_THOMSON_525_FORMAT;
const thomson_35_format FLOPPY_THOMSON_35_FORMAT;
