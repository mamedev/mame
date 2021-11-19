// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    VDK

    Disk image format

    Used by Paul Burgin's PC-Dragon emulator

***************************************************************************/

#include "vdk_dsk.h"

#include "ioprocs.h"


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

int vdk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	size_t actual;
	uint8_t id[2];
	io.read_at(0, id, 2, actual);

	if (id[0] == 'd' && id[1] == 'k')
		return 50;
	else
		return 0;
}

bool vdk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	size_t actual;
	if (io.seek(0, SEEK_SET))
		return false;

	uint8_t header[0x100];
	io.read(header, 0x100, actual);

	int const header_size = header[3] * 0x100 + header[2];
	int const track_count = header[8];
	int const head_count = header[9];

	if (io.seek(header_size, SEEK_SET))
		return false;

	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count ; head++)
		{
			desc_pc_sector sectors[SECTOR_COUNT];
			uint8_t sector_data[SECTOR_COUNT * SECTOR_SIZE];
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

				io.read(sectors[i].data, SECTOR_SIZE, actual);

				sector_offset += SECTOR_SIZE;
			}

			build_wd_track_mfm(track, head, image, 100000, SECTOR_COUNT, sectors, 22, 32, 24);
		}
	}

	return true;
}

bool vdk_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	size_t actual;
	if (io.seek(0, SEEK_SET))
		return false;

	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	// write header
	uint8_t header[12];

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

	io.write(header, sizeof(header), actual);

	// write disk data
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			auto bitstream = generate_bitstream_from_track(track, head, 2000, image);
			auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);

			for (int i = 0; i < SECTOR_COUNT; i++)
				io.write(sectors[FIRST_SECTOR_ID + i].data(), SECTOR_SIZE, actual);
		}
	}

	return true;
}

bool vdk_format::supports_save() const
{
	return true;
}

const floppy_format_type FLOPPY_VDK_FORMAT = &floppy_image_format_creator<vdk_format>;
