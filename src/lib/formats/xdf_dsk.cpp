// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/xdf_dsk.cpp

    x68k bare-bones formats

*********************************************************************/

#include "formats/xdf_dsk.h"

xdf_format::xdf_format() : upd765_format(formats)
{
}

const char *xdf_format::name() const noexcept
{
	return "xdf";
}

const char *xdf_format::description() const noexcept
{
	return "XDF disk image";
}

const char *xdf_format::extensions() const noexcept
{
	return "xdf,hdm,2hd";
}

// Unverified gap sizes
const xdf_format::format xdf_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, // 1us, 360rpm
		8, 77, 2,
		1024, {},
		1, {},
		80, 50, 22, 84
	},
	{}
};

// 2HC format, used by NetBSD/x68k
_2hc_format::_2hc_format() : upd765_format(formats)
{
}

const char *_2hc_format::name() const noexcept
{
	return "2hc";
}

const char *_2hc_format::description() const noexcept
{
	return "2HC disk image";
}

const char *_2hc_format::extensions() const noexcept
{
	return "2hc";
}

// Unverified gap sizes
const _2hc_format::format _2hc_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, // 1us, 360rpm
		15, 80, 2,
		512, {},
		1, {},
		80, 50, 22, 84
	},
	{}
};

const xdf_format FLOPPY_XDF_FORMAT;
const _2hc_format FLOPPY_2HC_FORMAT;
