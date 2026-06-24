// license:BSD-3-Clause
// copyright-holders:Devin Acker

#include "formats/s900_dsk.h"

s900_format::s900_format() : upd765_format(formats)
{
}

const char *s900_format::name() const noexcept
{
	return "s900";
}

const char *s900_format::description() const noexcept
{
	return "Akai S900 floppy disk image";
}

const char *s900_format::extensions() const noexcept
{
	return "img";
}

const s900_format::format s900_format::formats[] =
{
	{	// 800kb 2DD (S900/S950)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000,
		5,
		80,
		2,
		1024, {},
		1, {},
		// gaps 1/2/4a based on uPD7265/72066 datasheets, gap 3 from s900 firmware
		// (sony format has no gap 4A or IAM)
		0, 32, 22, 116
	},
	{	// 1600kb 2HD (S950)
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000,
		10,
		80,
		2,
		1024, {},
		1, {},
		0, 32, 22, 116
	},
	{}
};

const s900_format FLOPPY_S900_FORMAT;
