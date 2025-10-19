// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/adam_dsk.cpp

    Coleco Adam disk image format

*********************************************************************/

#include "formats/adam_dsk.h"

adam_format::adam_format() : wd177x_format(formats)
{
}

const char *adam_format::name() const noexcept
{
	return "adam";
}

const char *adam_format::description() const noexcept
{
	return "Coleco Adam disk image";
}

const char *adam_format::extensions() const noexcept
{
	return "dsk";
}

const adam_format::format adam_format::formats[] = {
	// track description
	// 100x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 100x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 859x4e
	{   /*  160K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  8, 40, 1, 512, {}, 1, {}, 100, 22, 100
	},

	// Unverified gap sizes -->
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

int adam_format::get_image_offset(const format &f, int head, int track) const
{
	return (track + (head ? f.track_count : 0)) * compute_track_size(f);
}

const adam_format FLOPPY_ADAM_FORMAT;
