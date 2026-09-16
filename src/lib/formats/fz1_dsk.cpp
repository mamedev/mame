// license:BSD-3-Clause
// copyright-holders:Devin Acker

#include "formats/fz1_dsk.h"

fz1_format::fz1_format() : upd765_format(formats)
{
}

const char *fz1_format::name() const noexcept
{
	return "fz1";
}

const char *fz1_format::description() const noexcept
{
	return "FZ-1 floppy disk image";
}

const char *fz1_format::extensions() const noexcept
{
	return "img";
}

const fz1_format::format fz1_format::formats[] =
{
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1200,
		8,
		80,
		2,
		1024, {},
		1, {},
		80, 50, 22, 84 // gaps unverified
	},
	{}
};

const fz1_format FLOPPY_FZ1_FORMAT;
