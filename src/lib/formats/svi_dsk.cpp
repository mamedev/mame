// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Spectravideo SVI-318/328

    Disk image format

***************************************************************************/

#include "svi_dsk.h"

#include "ioprocs.h"


svi_format::svi_format()
{
}

const char *svi_format::name() const noexcept
{
	return "svi";
}

const char *svi_format::description() const noexcept
{
	return "SVI-318/328 disk image";
}

const char *svi_format::extensions() const noexcept
{
	return "dsk";
}

int svi_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == 172032 || size == 346112)
		return FIFID_SIZE;

	return 0;
}

bool svi_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	int head_count;
	switch (size)
	{
	case 172032: head_count = 1; break;
	case 346112: head_count = 2; break;
	default: return false;
	}

	if (io.seek(0, SEEK_SET))
		return false;

	for (int track = 0; track < 40; track++)
	{
		for (int head = 0; head < head_count ; head++)
		{
			int sector_count = (track == 0 && head == 0) ? 18 : 17;
			int sector_size = (track == 0 && head == 0) ? 128 : 256;

			desc_pc_sector sectors[20];
			uint8_t sector_data[5000];
			int sector_offset = 0;

			for (int i = 0; i < sector_count; i++)
			{
				sectors[i].track = track;
				sectors[i].head = head;
				sectors[i].sector = i + 1;
				sectors[i].actual_size = sector_size;
				sectors[i].size = sector_size >> 8;
				sectors[i].deleted = false;
				sectors[i].bad_data_crc = false;
				sectors[i].bad_addr_crc = false;
				sectors[i].data = &sector_data[sector_offset];

				/*auto const [err, actual] =*/ read(io, sectors[i].data, sector_size); // FIXME: check for errors and premature EOF

				sector_offset += sector_size;
			}

			if (track == 0 && head == 0)
				build_wd_track_fm(track, head, image, 50000, sector_count, sectors, 10, 26, 11);
			else
				build_wd_track_mfm(track, head, image, 100000, sector_count, sectors, 32, 50, 22);
		}
	}

	return true;
}

bool svi_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	if (io.seek(0, SEEK_SET))
		return false;

	int track_count, head_count;
	image.get_actual_geometry(track_count, head_count);

	// initial fm track
	auto bitstream = generate_bitstream_from_track(0, 0, 4000, image);
	auto sectors = extract_sectors_from_bitstream_fm_pc(bitstream);

	for (int i = 0; i < 18; i++)
	{
		/*auto const [err, actual] =*/ write(io, sectors[i + 1].data(), 128); // FIXME: check for errors
	}

	// rest are mfm tracks
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			// skip track 0, head 0
			if (track == 0) { if (head_count == 1) break; else head++; }

			bitstream = generate_bitstream_from_track(track, head, 2000, image);
			sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);

			for (int i = 0; i < 17; i++)
			{
				/*auto const [err, actual] =*/ write(io, sectors[i + 1].data(), 256); // FIXME: check for errors
			}
		}
	}

	return true;
}

bool svi_format::supports_save() const noexcept
{
	return true;
}

const svi_format FLOPPY_SVI_FORMAT;
