// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/sdd_dsk.c

    Speccy-DOS SDD disk images

*********************************************************************/

#include "formats/sdd_dsk.h"

#include "ioprocs.h"


sdd_format::sdd_format() : wd177x_format(formats)
{
}

const char *sdd_format::name() const
{
	return "sdd";
}

const char *sdd_format::description() const
{
	return "SDD floppy disk image";
}

const char *sdd_format::extensions() const
{
	return "sdd";
}

int sdd_format::get_image_offset(const format &f, int head, int track) const
{
	return (f.track_count * head + track) * compute_track_size(f);
}

const sdd_format::format sdd_format::formats[] = {
	{   // 5"25 640K 80 track double sided double density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, {1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6}, 60, 22, 24
	},
	{   // 5"25 140K 35 track single sided double density
		floppy_image::FF_525,  floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 35, 1, 256, {}, -1, {1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6}, 60, 22, 24
	},
	{   // 5"25 400K 80 track double sided single density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, {1,8,5,2,9,6,3,10,7,4}, 40, 11, 10
	},
	{   // 3'5 640K 80 track double sided double density
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, {1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6}, 60, 22, 24
	},
	{   // 3'5 400K 80 track double sided single density
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, {1,8,5,2,9,6,3,10,7,4}, 40, 11, 10
	},
	{}
};

const sdd_format FLOPPY_SDD_FORMAT;
