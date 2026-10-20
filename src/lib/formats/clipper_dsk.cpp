// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/clipper_dsk.cpp

    PDC Clipper 3" disk image format

*********************************************************************/

#include "formats/clipper_dsk.h"

clipper_format::clipper_format() : upd765_format(formats)
{
}

const char *clipper_format::name() const noexcept
{
	return "clipper";
}

const char *clipper_format::description() const noexcept
{
	return "PDC Clipper disk image";
}

const char *clipper_format::extensions() const noexcept
{
	return "dsk";
}

const clipper_format::format clipper_format::formats[] = {
	{   // 205K 3 inch single sided double density
		floppy_image::FF_3, floppy_image::SSDD, floppy_image::MFM,
		2000, 5, 41, 1, 1024, {}, 1, {}, 80, 50, 22, 128
	},
	{}
};

const clipper_format FLOPPY_CLIPPER_FORMAT;
