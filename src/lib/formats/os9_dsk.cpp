// license:BSD-3-Clause
// copyright-holders:tim lindner
/*********************************************************************

    formats/os9_dsk.c

    OS-9 disk images

    OS-9 Level 2 Technical Reference, Chapter 5, Random Block File
    Manager, page 2

    Available: http://www.colorcomputerarchive.com/coco/Documents/Manuals/
        Operating Systems/OS-9 Level 2 Manual (Tandy).pdf

*********************************************************************/

#include "emu.h"
#include "formats/os9_dsk.h"

#include "formats/imageutl.h"


os9_format::os9_format() : wd177x_format(formats)
{
}

const char *os9_format::name() const
{
	return "os9";
}

const char *os9_format::description() const
{
	return "OS-9 floppy disk image";
}

const char *os9_format::extensions() const
{
	return "dsk,os9";
}

int os9_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 75;
	return 0;
}

int os9_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint64_t size = io_generic_size(io);

	uint8_t os9_header[0x20];
	io_generic_read(io, os9_header, 0, 0x20);

	int os9_total_sectors = pick_integer_be(os9_header, 0x00, 3);
	int os9_heads = BIT(os9_header[0x10], 0) ? 2 : 1;
	int os9_sectors = pick_integer_be(os9_header, 0x11, 2);

	if (os9_total_sectors > 0 && os9_heads > 0 && os9_sectors > 0) {
		int os9_tracks = os9_total_sectors / os9_sectors / os9_heads;

		// now let's see if we have valid info
		if ((os9_tracks * os9_heads * os9_sectors * 256) == size) {
			for (int i=0; formats[i].form_factor; i++) {
				const format &f = formats[i];

				if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
					continue;

				if ((os9_tracks == f.track_count) && (os9_heads == f.head_count) && (os9_sectors == f.sector_count))
					return i;
			}
		}
	}

	return -1;
}

const os9_format::format os9_format::formats[] = {
	{   //  5"25 160K double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 35, 1, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{   //  5"25 160K double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 35, 2, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{   //  5"25 160K double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{   //  5"25 160K double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{   //  5"25 160K double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{   //  5"25 160K double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 80, 2, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{}
};

const floppy_format_type FLOPPY_OS9_FORMAT = &floppy_image_format_creator<os9_format>;
