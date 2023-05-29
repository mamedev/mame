// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "roland_dsk.h"

roland_sdisk_format::roland_sdisk_format() : wd177x_format(formats)
{
}

const char *roland_sdisk_format::name() const
{
	return "roland_sdisk";
}

const char *roland_sdisk_format::description() const
{
	return "Roland S-Disk image";
}

const char *roland_sdisk_format::extensions() const
{
	return "out,w30";
}

const roland_sdisk_format::format roland_sdisk_format::formats[] = {
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000,
		9, 80, 2,
		512, {},
		1, {},
		80, 22, 24
	},
	// TODO: add HD format used by S-750/S-760/S-770
	{}
};

const roland_sdisk_format FLOPPY_ROLAND_SDISK_FORMAT;
