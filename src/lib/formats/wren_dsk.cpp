// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Wren Executive

    Disk image format

***************************************************************************/

#include "wren_dsk.h"


wren_format::wren_format() : wd177x_format(formats)
{
}

const char *wren_format::name() const noexcept
{
	return "wren";
}

const char *wren_format::description() const noexcept
{
	return "Wren Executive disk image";
}

const char *wren_format::extensions() const noexcept
{
	return "img";
}

const wren_format::format wren_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, 1, {}, 20, 22, 24 // gaps unverified
	},
	{}
};


const wren_format FLOPPY_WREN_FORMAT;
