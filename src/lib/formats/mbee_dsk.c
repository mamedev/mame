// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/mbee_dsk.c

    Microbee disk image format

*********************************************************************/

#include "emu.h"
#include "formats/mbee_dsk.h"

mbee_format::mbee_format() : wd177x_format(formats)
{
}

const char *mbee_format::name() const
{
	return "mbee";
}

const char *mbee_format::description() const
{
	return "Microbee disk image";
}

const char *mbee_format::extensions() const
{
	return "ss80,ds40,ds80,ds82,ds84";
}

// Unverified gap sizes
const mbee_format::format mbee_format::formats[] = {
	{   /*  ss80 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::SSDD, floppy_image::MFM,
		2000,  10, 80, 1, 512, {}, 1, {}, 32, 22, 31
	},
	{   /*  ds40 5.25 inch double density */
		floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 40, 2, 512, {}, 1, {}, 32, 22, 31
	},
	{   /*  ds80 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 80, 2, 512, {}, 21, {}, 32, 22, 31
	},
	{   /*  ds82,ds84 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 80, 2, 512, {}, 1, {}, 32, 22, 31
	},
	{}
};

const floppy_format_type FLOPPY_MBEE_FORMAT = &floppy_image_format_creator<mbee_format>;
