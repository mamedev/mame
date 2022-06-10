// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/tim011_dsk.cpp

    TIM 011 format

*********************************************************************/

#include "formats/tim011_dsk.h"

tim011_format::tim011_format() : upd765_format(formats)
{
}

const char *tim011_format::name() const
{
	return "tim011";
}

const char *tim011_format::description() const
{
	return "TIM 011 disk image";
}

const char *tim011_format::extensions() const
{
	return "img";
}

// Unverified gap sizes.
const tim011_format::format tim011_format::formats[] = {
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000,
		5, 80, 2,
		1024, {},
		17, {},
		80, 50, 22, 80
	},
	{}
};

const tim011_format FLOPPY_TIM011_FORMAT;
