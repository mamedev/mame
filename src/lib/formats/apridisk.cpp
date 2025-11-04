// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    APRIDISK

    Disk image format for the ACT Apricot

***************************************************************************/

#include "apridisk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include "osdcore.h" // osd_printf_error


apridisk_format::apridisk_format()
{
}

const char *apridisk_format::name() const noexcept
{
	return "apridisk";
}

const char *apridisk_format::description() const noexcept
{
	return "APRIDISK disk image";
}

const char *apridisk_format::extensions() const noexcept
{
	return "dsk";
}

int apridisk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[APR_HEADER_SIZE];
	auto const [err, actual] = read_at(io, 0, header, APR_HEADER_SIZE);
	if (err || (APR_HEADER_SIZE != actual))
		return 0;

	const char magic[] = "ACT Apricot disk image\x1a\x04";

	if (memcmp(header, magic, sizeof(magic) - 1) == 0)
		return 100;
	else
		return 0;
}

bool apridisk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	desc_pc_sector sectors[80][2][18];
	std::unique_ptr<uint8_t []> sector_data(new uint8_t [MAX_SECTORS * SECTOR_SIZE]);
	uint8_t *data_ptr = sector_data.get();
	int track_count = 0, head_count = 0, sector_count = 0;

	uint64_t file_size;
	if (io.length(file_size))
		return false;

	uint64_t file_offset = APR_HEADER_SIZE;

	while (file_offset < file_size)
	{
		// read sector header
		uint8_t sector_header[16];
		read_at(io, file_offset, sector_header, 16); // FIXME: check for errors and premature EOF

		uint32_t type = get_u32le(&sector_header[0]);
		uint16_t compression = get_u16le(&sector_header[4]);
		uint16_t header_size = get_u16le(&sector_header[6]);
		uint32_t data_size = get_u32le(&sector_header[8]);

		file_offset += header_size;

		switch (type)
		{
		case APR_SECTOR:
			uint8_t head = sector_header[12];
			uint8_t sector = sector_header[13];
			uint8_t track = (uint8_t) get_u16le(&sector_header[14]);

			track_count = std::max(track_count, int(unsigned(track)));
			head_count = std::max(head_count, int(unsigned(head)));
			sector_count = std::max(sector_count, int(unsigned(sector)));

			// build sector info
			sectors[track][head][sector - 1].head = head;
			sectors[track][head][sector - 1].sector = sector;
			sectors[track][head][sector - 1].track = track;
			sectors[track][head][sector - 1].size = SECTOR_SIZE >> 8;
			sectors[track][head][sector - 1].actual_size = SECTOR_SIZE;
			sectors[track][head][sector - 1].deleted = false;
			sectors[track][head][sector - 1].bad_crc = false;

			// read sector data
			switch (compression)
			{
			case APR_COMPRESSED:
				{
					uint8_t comp[3];
					read_at(io, file_offset, comp, 3); // FIXME: check for errors and premature EOF
					uint16_t length = get_u16le(&comp[0]);

					if (length != SECTOR_SIZE)
					{
						osd_printf_error("apridisk_format: Invalid compression length %04x\n", length);
						return false;
					}

					memset(data_ptr, comp[2], SECTOR_SIZE);
				}
				break;

			case APR_UNCOMPRESSED:
				read_at(io, file_offset, data_ptr, SECTOR_SIZE); // FIXME: check for errors and premature EOF
				break;

			default:
				osd_printf_error("apridisk_format: Invalid compression %04x\n", compression);
				return false;
			}

			sectors[track][head][sector - 1].data = data_ptr;
			data_ptr += SECTOR_SIZE;

			break;

		}

		file_offset += data_size;
	}

	// track/head index are zero-based, so increase the final count by 1
	track_count++;
	head_count++;

	int cell_count = (sector_count == 18 ? 200000 : 100000);

	// now build our final track info
	for (int track = 0; track < track_count; track++)
		for (int head = 0; head < head_count; head++)
			build_pc_track_mfm(track, head, image, cell_count, sector_count, sectors[track][head], 84, 80, 50, 22);

	return true;
}

const apridisk_format FLOPPY_APRIDISK_FORMAT;
