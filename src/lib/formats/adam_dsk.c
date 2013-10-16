// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/adam_dsk.c

    Coleco Adam disk image format

*********************************************************************/

#include "emu.h"
#include "formats/adam_dsk.h"

adam_format::adam_format() : wd177x_format(formats)
{
}

const char *adam_format::name() const
{
	return "adam";
}

const char *adam_format::description() const
{
	return "Coleco Adam disk image";
}

const char *adam_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const adam_format::format adam_format::formats[] = {
	{   /*  160K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  8, 40, 1, 512, {}, 1, {}, 100, 22, 84
	},
	{   /*  320K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  8, 40, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{   /*  640K 5 1/4 inch quad density */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  8, 80, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{   /*  720K 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{   /* 1440K 3 1/2 inch high density */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 18, 80, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{}
};

const floppy_format_type FLOPPY_ADAM_FORMAT = &floppy_image_format_creator<adam_format>;
