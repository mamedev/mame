// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc1600_dsk.c

    Luxor ABC 1600 disk image formats

*********************************************************************/

#include "formats/abc1600_dsk.h"

abc1600_format::abc1600_format() : wd177x_format(formats)
{
}

const char *abc1600_format::name() const
{
	return "abc1600";
}

const char *abc1600_format::description() const
{
	return "Luxor ABC 1600 disk image";
}

const char *abc1600_format::extensions() const
{
	return "img";
}

const abc1600_format::format abc1600_format::formats[] = {
	// track description
	// 55x4e 12x00 3xf5 fe 2x00 01 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 02 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 03 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 04 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 05 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 06 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 07 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 08 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 09 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0a 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0b 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0c 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0d 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0e 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0f 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 10 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 298x4e

	{   //  640K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 1, {}, 55, 22, 54
	},

	{}
};

const floppy_format_type FLOPPY_ABC1600_FORMAT = &floppy_image_format_creator<abc1600_format>;

int abc1600_format::get_image_offset(const format &f, int head, int track)
{
	int offset = 0;

	if(head) {
		for(int trk=0; trk < f.track_count; trk++) {
			const format &tf = get_track_format(f, 0, trk);
			offset += compute_track_size(tf);
		}
	}

	for(int trk=0; trk < track; trk++) {
		const format &tf = get_track_format(f, head, trk);
		offset += compute_track_size(tf);
	}

	return offset;
}
