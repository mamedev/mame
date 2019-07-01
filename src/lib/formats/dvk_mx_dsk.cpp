// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    formats/dvk_mx_dsk.cpp

    Floppies used by DVK MX: controller

    http://torlus.com/floppy/forum/viewtopic.php?f=19&t=1384

    Track format is almost entirely driver-dependent (only sync word
    0x00f3 is mandatory), because hardware always reads or writes
    entire track.  'old' format is used by stock driver, 'new' --
    by 3rd party one. Formatting tools produce yet other variants...

    Floppy drives were 40- and 80-track, double-sided.  'new' driver
    also supports single-sided floppies.

************************************************************************/

#include <assert.h>

#include "flopimg.h"
#include "formats/dvk_mx_dsk.h"

const floppy_image_format_t::desc_e dvk_mx_format::dvk_mx_new_desc[] = {
	/* 01 */ { FM, 0x00, 8*2 },	// eight 0x0000 words
	/* 03 */ { FM, 0x00, 1 },
	/* 02 */ { FM, 0xf3, 1 },	// word 0x00f3
	/* 05 */ { FM, 0x00, 1 },
	/* 04 */ { TRACK_ID_FM },	// track number word
	/* 05 */ { SECTOR_LOOP_START, 0, 10 },	// 11 sectors
	/* 07 */ {   SECTOR_DATA_MX, -1 },
	/* 10 */ { SECTOR_LOOP_END },
	/* 13 */ { FM, 0x83, 1 },
	/* 12 */ { OFFSET_ID_FM },
	/* 15 */ { FM, 0x83, 1 },
	/* 14 */ { OFFSET_ID_FM },
	/* 17 */ { FM, 0x83, 1 },
	/* 16 */ { OFFSET_ID_FM },
	/* 18 */ { END }
};

const floppy_image_format_t::desc_e dvk_mx_format::dvk_mx_old_desc[] = {
	/* 01 */ { FM, 0x00, 30*2 },
	/* 03 */ { FM, 0x00, 1 },
	/* 02 */ { FM, 0xf3, 1 },	// word 0x00f3
	/* 05 */ { FM, 0x00, 1 },
	/* 04 */ { TRACK_ID_FM },	// track number word
	/* 06 */ { SECTOR_LOOP_START, 0, 10 },	// 11 sectors
	/* 07 */ {   SECTOR_DATA_MX, -1 },
	/* 10 */ { SECTOR_LOOP_END },
	/* 13 */ { FM, 0x83, 1 },
	/* 11 */ { FM, 0x01, 1 },
	/* 15 */ { FM, 0x83, 1 },
	/* 14 */ { FM, 0x01, 1 },
	/* 16 */ { END }
};

dvk_mx_format::dvk_mx_format()
{
}

const char *dvk_mx_format::name() const
{
	return "mx";
}

const char *dvk_mx_format::description() const
{
	return "DVK MX: floppy image";
}

const char *dvk_mx_format::extensions() const
{
	return "mx";
}

bool dvk_mx_format::supports_save() const
{
	return false;
}

void dvk_mx_format::find_size(io_generic *io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count)
{
	uint64_t size = io_generic_size(io);

	switch (size)
	{
	case 112640:
		track_count = 40;
		sector_count = 11;
		head_count = 1;
		break;
	case 225280:
		track_count = 40;
		sector_count = 11;
		head_count = 2;
		break;
	case 450560:
		track_count = 80;
		sector_count = 11;
		head_count = 2;
		break;
	default:
		track_count = head_count = sector_count = 0;
		break;
	}
}

int dvk_mx_format::identify(io_generic *io, uint32_t form_factor)
{
	uint8_t track_count, head_count, sector_count;

	find_size(io, track_count, head_count, sector_count);

	if (track_count)
	{
		uint8_t sectdata[512];
		io_generic_read(io, sectdata, 512, 512);
		// check value in RT-11 home block.  see src/tools/imgtool/modules/rt11.cpp
		if (pick_integer_le(sectdata, 0724, 2) == 6)
			return 100;
		else
			return 75;

	}

	return 0;
}

bool dvk_mx_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	uint8_t track_count, head_count, sector_count;

	find_size(io, track_count, head_count, sector_count);
	if (track_count == 0) return false;

	uint8_t sectdata[11 * 256];
	desc_s sectors[11];
	for (int i = 0; i < sector_count; i++)
	{
		sectors[i].data = sectdata + 256 * i;
		sectors[i].size = 256;
		sectors[i].sector_id = i;
	}

	int track_size = sector_count * 256;
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			io_generic_read(io, sectdata, (track * head_count + head) * track_size, track_size);
			generate_track(dvk_mx_new_desc, track, head, sectors, sector_count, 45824, image);
		}
	}

	if (head_count == 1)
	{
		image->set_variant(floppy_image::SSDD);
	}
	else
	{
		if (track_count > 40)
		{
			image->set_variant(floppy_image::DSQD);
		}
		else
		{
			image->set_variant(floppy_image::DSDD);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_DVK_MX_FORMAT = &floppy_image_format_creator<dvk_mx_format>;
