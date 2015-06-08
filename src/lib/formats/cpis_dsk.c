// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/cpis_dsk.c

    Telenova Compis disk images

*********************************************************************/

#include "cpis_dsk.h"

cpis_format::cpis_format() : upd765_format(formats)
{
}

const char *cpis_format::name() const
{
	return "cpis";
}

const char *cpis_format::description() const
{
	return "COMPIS disk image";
}

const char *cpis_format::extensions() const
{
	return "dsk,img";
}

// Unverified gap sizes
const cpis_format::format cpis_format::formats[] = {
	{   /*  320K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  8, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  360K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  640K 5 1/4 inch quad density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  8, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  720K 5 1/4 inch quad density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /* 1200K 5 1/4 inch quad density */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		1200, 15, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{}
};

const floppy_format_type FLOPPY_CPIS_FORMAT = &floppy_image_format_creator<cpis_format>;
