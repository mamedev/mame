// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VDK

    Disk image format

    Used by Paul Burgin's PC-Dragon emulator

***************************************************************************/

#include "emu.h"
#include "vdk_dsk.h"

vdk_format::vdk_format()
{
}

const char *vdk_format::name() const
{
	return "vdk";
}

const char *vdk_format::description() const
{
	return "VDK disk image";
}

const char *vdk_format::extensions() const
{
	return "vdk";
}

int vdk_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 id[2];
	io_generic_read(io, id, 0, 2);

	if (id[0] == 'd' && id[1] == 'k')
		return 50;
	else
		return 0;
}

bool vdk_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 header[0x100];
	io_generic_read(io, header, 0, 0x100);

	int header_size = header[3] * 0x100 + header[2];
	int track_count = header[8];
	int head_count = header[9];

	int file_offset = header_size;

	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count ; head++)
		{
			desc_pc_sector sectors[SECTOR_COUNT];
			UINT8 sector_data[SECTOR_COUNT * SECTOR_SIZE];
			int sector_offset = 0;

			for (int i = 0; i < SECTOR_COUNT; i++)
			{
				sectors[i].track = track;
				sectors[i].head = head;
				sectors[i].sector = FIRST_SECTOR_ID + i;
				sectors[i].actual_size = SECTOR_SIZE;
				sectors[i].size = SECTOR_SIZE >> 8;
				sectors[i].deleted = false;
				sectors[i].bad_crc = false;
				sectors[i].data = &sector_data[sector_offset];

				io_generic_read(io, sectors[i].data, file_offset, SECTOR_SIZE);

				sector_offset += SECTOR_SIZE;
				file_offset += SECTOR_SIZE;
			}

			build_wd_track_mfm(track, head, image, 100000, SECTOR_COUNT, sectors, 22, 32, 24);
		}
	}

	return true;
}

bool vdk_format::save(io_generic *io, floppy_image *image)
{
	UINT8 bitstream[500000/8];
	UINT8 sector_data[50000];
	desc_xs sectors[256];
	UINT64 file_offset = 0;

	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	// write header
	UINT8 header[12];

	header[0] = 'd';
	header[1] = 'k';
	header[2] = sizeof(header) % 0x100;
	header[3] = sizeof(header) / 0x100;
	header[4] = 0x10;
	header[5] = 0x10;
	header[6] = 'M';
	header[7] = 0x01;
	header[8] = track_count;
	header[9] = head_count;
	header[10] = 0;
	header[11] = 0;

	io_generic_write(io, header, file_offset, sizeof(header));
	file_offset += sizeof(header);

	// write disk data
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			int track_size;
			generate_bitstream_from_track(track, head, 2000, bitstream, track_size, image);
			extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sector_data, sizeof(sector_data));

			for (int i = 0; i < SECTOR_COUNT; i++)
			{
				io_generic_write(io, sectors[FIRST_SECTOR_ID + i].data, file_offset, SECTOR_SIZE);
				file_offset += SECTOR_SIZE;
			}
		}
	}

	return true;
}

bool vdk_format::supports_save() const
{
	return true;
}

const floppy_format_type FLOPPY_VDK_FORMAT = &floppy_image_format_creator<vdk_format>;
