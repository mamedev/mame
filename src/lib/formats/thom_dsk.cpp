// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "thom_dsk.h"

thomson_525_format::thomson_525_format() : wd177x_format(formats)
{
}

const char *thomson_525_format::name() const
{
	return "thomson_525";
}

const char *thomson_525_format::description() const
{
	return "Thomson 5.25 disk image";
}

const char *thomson_525_format::extensions() const
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
	{}
};


thomson_35_format::thomson_35_format() : wd177x_format(formats)
{
}

const char *thomson_35_format::name() const
{
	return "thomson_35";
}

const char *thomson_35_format::description() const
{
	return "Thomson 3.5 disk image";
}

const char *thomson_35_format::extensions() const
{
	return "fd";
}

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

const thomson_525_format FLOPPY_THOMSON_525_FORMAT;
const thomson_35_format FLOPPY_THOMSON_35_FORMAT;
