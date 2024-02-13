// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/idpart_dsk.cpp

    Iskra Delta Partner format

*********************************************************************/

#include "formats/idpart_dsk.h"

idpart_format::idpart_format() : upd765_format(formats)
{
}

const char *idpart_format::name() const noexcept
{
	return "idpart";
}

const char *idpart_format::description() const noexcept
{
	return "Iskra Delta Partner disk image";
}

const char *idpart_format::extensions() const noexcept
{
	return "img";
}

// Unverified gap sizes.
const idpart_format::format idpart_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200,
		18, 73, 2,
		256, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const idpart_format FLOPPY_IDPART_FORMAT;
