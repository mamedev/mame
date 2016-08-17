// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/smx_dsk.c

    Specialist MX format

*********************************************************************/

#include <assert.h>

#include "formats/smx_dsk.h"

smx_format::smx_format() : wd177x_format(formats)
{
}

const char *smx_format::name() const
{
	return "smx";
}

const char *smx_format::description() const
{
	return "Specialist MX/Orion/B2M disk image";
}

const char *smx_format::extensions() const
{
	return "odi,cpm,img";
}

// Unverified gap sizes
const smx_format::format smx_format::formats[] =
{
	{   //  Specialist MX/Orion/B2M disk image
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 1, {}, 100, 22, 20
	},
	{   //  Lucksian Key Orion disk image
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 100, 22, 20
	},
	{}
};

const floppy_format_type FLOPPY_SMX_FORMAT = &floppy_image_format_creator<smx_format>;
