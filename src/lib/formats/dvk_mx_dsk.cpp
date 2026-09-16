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

#include "formats/dvk_mx_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

constexpr uint32_t ID_PATTERN = 0xaaaaffaf;

const floppy_image_format_t::desc_e dvk_mx_format::dvk_mx_new_desc[] = {
	/* 01 */ { FM, 0x00, 8*2 }, // eight 0x0000 words
	/* 02 */ { FM, 0x00, 1 },
	/* 03 */ { FM, 0xf3, 1 },   // word 0x00f3
	/* 04 */ { FM, 0x00, 1 },
	/* 05 */ { TRACK_ID_FM },   // track number word
	/* 06 */ { SECTOR_LOOP_START, 0, 10 },  // 11 sectors
	/* 07 */ {   SECTOR_DATA_MX, -1 },
	/* 08 */ { SECTOR_LOOP_END },
	/* 09 */ { FM, 0x83, 1 },
	/* 10 */ { OFFSET_ID_FM },
	/* 11 */ { FM, 0x83, 1 },
	/* 12 */ { OFFSET_ID_FM },
	/* 13 */ { FM, 0x83, 1 },
	/* 14 */ { OFFSET_ID_FM },
	/* 15 */ { END }
};

const floppy_image_format_t::desc_e dvk_mx_format::dvk_mx_old_desc[] = {
	/* 01 */ { FM, 0x00, 30*2 },
	/* 02 */ { FM, 0x00, 1 },
	/* 03 */ { FM, 0xf3, 1 },   // word 0x00f3
	/* 04 */ { FM, 0x00, 1 },
	/* 05 */ { TRACK_ID_FM },   // track number word
	/* 06 */ { SECTOR_LOOP_START, 0, 10 },  // 11 sectors
	/* 07 */ {   SECTOR_DATA_MX, -1 },
	/* 08 */ { SECTOR_LOOP_END },
	/* 09 */ { FM, 0x00, 1 },
	/* 10 */ { FM, 0116, 1 },
	/* 11 */ { FM, 0x00, 4*2 }, // four 0x0000 words
	/* 12 */ { END }
};

dvk_mx_format::dvk_mx_format()
{
}

void dvk_mx_format::find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count)
{
	uint64_t size;
	if (io.length(size))
	{
		track_count = head_count = sector_count = 0;
		return;
	}

	switch (size)
	{
	case 112640:
		track_count = 40;
		sector_count = MX_SECTORS;
		head_count = 1;
		break;
	case 225280:
		track_count = 40;
		sector_count = MX_SECTORS;
		head_count = 2;
		break;
	case 450560:
		track_count = 80;
		sector_count = MX_SECTORS;
		head_count = 2;
		break;
	default:
		track_count = head_count = sector_count = 0;
		break;
	}
}

int dvk_mx_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t track_count, head_count, sector_count;

	find_size(io, track_count, head_count, sector_count);

	if (track_count)
	{
		uint8_t sectdata[512];
		/*auto const [err, actual] =*/ read_at(io, 512, sectdata, 512); // FIXME: check for errors and premature EOF
		// check value in RT-11 home block.  see src/tools/imgtool/modules/rt11.cpp
		if (get_u16le(&sectdata[0724]) == 6)
			return FIFID_SIGN|FIFID_SIZE;
		else
			return FIFID_SIZE;

	}

	return 0;
}

bool dvk_mx_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t track_count, head_count, sector_count;

	find_size(io, track_count, head_count, sector_count);
	if (track_count == 0) return false;

	uint8_t sectdata[MX_SECTORS * MX_SECTOR_SIZE];
	desc_s sectors[MX_SECTORS];
	for (int i = 0; i < sector_count; i++)
	{
		sectors[i].data = sectdata + MX_SECTOR_SIZE * i;
		sectors[i].size = MX_SECTOR_SIZE;
		sectors[i].sector_id = i;
	}

	int track_size = sector_count * MX_SECTOR_SIZE;
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			/*auto const [err, actual] =*/ read_at(io, (track * head_count + head) * track_size, sectdata, track_size); // FIXME: check for errors and premature EOF
			generate_track(dvk_mx_new_desc, track, head, sectors, sector_count, 45824, image);
		}
	}

	if (head_count == 1)
	{
		image.set_variant(floppy_image::SSDD);
	}
	else
	{
		if (track_count > 40)
		{
			image.set_variant(floppy_image::DSQD);
		}
		else
		{
			image.set_variant(floppy_image::DSDD);
		}
	}

	return true;
}

bool dvk_mx_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks;
	int heads;
	bool res = false;
	image.get_actual_geometry(tracks, heads);

	for (int cyl = 0; cyl < tracks; cyl++)
	{
		for (int head = 0; head < heads; head++)
		{
			auto bitstream = generate_bitstream_from_track(cyl, head, 4000, image, 0);
			uint8_t sector_data[MX_SECTORS * MX_SECTOR_SIZE];
			if (get_next_sector(bitstream, sector_data))
			{
				unsigned const offset_in_image = (cyl * heads + head) * MX_SECTOR_SIZE * MX_SECTORS;
				res = true;
				// FIXME: check for errors
				/*auto const [err, actual] =*/ write_at(io, offset_in_image, sector_data, MX_SECTOR_SIZE * MX_SECTORS);
			}
		}
	}
	return res;
}

bool dvk_mx_format::get_next_sector(const std::vector<bool> &bitstream, uint8_t *sector_data)
{
	uint32_t sr = 0;
	int pos = 0;
	while (pos < bitstream.size() && sr != ID_PATTERN)
	{
		sr = (sr << 1) | bitstream[pos];
		pos++;
	}
	if (pos == bitstream.size())
	{
		// End of track reached
		return false;
	}
	unsigned to_dump = MX_SECTORS * (MX_SECTOR_SIZE + 2);
	pos += 32; // skip track ID
	for (unsigned i = 0, k = 0; i < to_dump && pos < bitstream.size(); i++)
	{
		uint8_t byte = 0;
		unsigned j;
		for (j = 0; j < 8 && pos < bitstream.size(); j++)
		{
			bool bit = bitstream[pos + 1];
			pos += 2;
			byte <<= 1;
			byte |= bit;
		}
		if (j == 8 && (i % (MX_SECTOR_SIZE + 2)) < MX_SECTOR_SIZE)
		{
			sector_data[k ^ 1] = byte;
			k++;
		}
	}
	return true;
}

const dvk_mx_format FLOPPY_DVK_MX_FORMAT;
