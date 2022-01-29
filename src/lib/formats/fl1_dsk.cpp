// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/fl1_dsk.c

    FloppyOne DOS disk images

*********************************************************************/

#include "formats/fl1_dsk.h"

fl1_format::fl1_format() : wd177x_format(formats)
{
}

const char *fl1_format::name() const
{
	return "fl1";
}

const char *fl1_format::description() const
{
	return "FloppyOne floppy disk image";
}

const char *fl1_format::extensions() const
{
	return "fl1";
}

int fl1_format::get_image_offset(const format &f, int head, int track)
{
	return (f.track_count * head + track) * compute_track_size(f);
}

const fl1_format::format fl1_format::formats[] = {
	{   // 5"25 800K 80 track double sided double density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 0, {}, 80, 22, 54
	},
	{   // 5"25 400K 80 track single sided double density
		floppy_image::FF_525,  floppy_image::SSQD, floppy_image::MFM,
		2000, 5, 80, 1, 1024, {}, 0, {}, 80, 22, 54
	},
	{   // 3'5 800K 80 track double sided double density
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 0, {}, 80, 22, 54
	},
	{   // 3'5 400K 80 track single sided double density
		floppy_image::FF_35,  floppy_image::SSDD, floppy_image::MFM,
		2000, 5, 80, 1, 1024, {}, 0, {}, 80, 22, 54
	},
	{}
};

const floppy_format_type FLOPPY_FL1_FORMAT = &floppy_image_format_creator<fl1_format>;
