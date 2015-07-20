// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Spectravideo SVI-318/328

    Disk image format

***************************************************************************/

#include "svi_dsk.h"

svi_format::svi_format()
{
}

const char *svi_format::name() const
{
	return "svi";
}

const char *svi_format::description() const
{
	return "SVI-318/328 disk image";
}

const char *svi_format::extensions() const
{
	return "dsk";
}

int svi_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);

	if (size == 172032 || size == 346112)
		return 50;

	return 0;
}

bool svi_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT64 size = io_generic_size(io);
	int head_count;

	switch (size)
	{
	case 172032: head_count = 1; break;
	case 346112: head_count = 2; break;
	default: return false;
	}

	int file_offset = 0;

	for (int track = 0; track < 40; track++)
	{
		for (int head = 0; head < head_count ; head++)
		{
			int sector_count = (track == 0 && head == 0) ? 18 : 17;
			int sector_size = (track == 0 && head == 0) ? 128 : 256;

			desc_pc_sector sectors[20];
			UINT8 sector_data[5000];
			int sector_offset = 0;

			for (int i = 0; i < sector_count; i++)
			{
				sectors[i].track = track;
				sectors[i].head = head;
				sectors[i].sector = i + 1;
				sectors[i].actual_size = sector_size;
				sectors[i].size = sector_size >> 8;
				sectors[i].deleted = false;
				sectors[i].bad_crc = false;
				sectors[i].data = &sector_data[sector_offset];

				io_generic_read(io, sectors[i].data, file_offset, sector_size);

				sector_offset += sector_size;
				file_offset += sector_size;
			}

			if (track == 0 && head == 0)
				build_wd_track_fm(track, head, image, 50000, sector_count, sectors, 10, 26, 11);
			else
				build_wd_track_mfm(track, head, image, 100000, sector_count, sectors, 32, 50, 22);
		}
	}

	return true;
}

bool svi_format::save(io_generic *io, floppy_image *image)
{
	UINT8 bitstream[500000/8];
	UINT8 sector_data[50000];
	desc_xs sectors[256];
	int track_size;
	UINT64 file_offset = 0;

	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	// initial fm track
	generate_bitstream_from_track(0, 0, 4000, bitstream, track_size, image);
	extract_sectors_from_bitstream_fm_pc(bitstream, track_size, sectors, sector_data, sizeof(sector_data));

	for (int i = 0; i < 18; i++)
	{
		io_generic_write(io, sectors[i + 1].data, file_offset, 128);
		file_offset += 128;
	}

	// rest are mfm tracks
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			// skip track 0, head 0
			if (track == 0) { if (head_count == 1) break; else head++; }

			generate_bitstream_from_track(track, head, 2000, bitstream, track_size, image);
			extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sector_data, sizeof(sector_data));

			for (int i = 0; i < 17; i++)
			{
				io_generic_write(io, sectors[i + 1].data, file_offset, 256);
				file_offset += 256;
			}
		}
	}

	return true;
}

bool svi_format::supports_save() const
{
	return true;
}

const floppy_format_type FLOPPY_SVI_FORMAT = &floppy_image_format_creator<svi_format>;
