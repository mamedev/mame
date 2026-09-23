// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe SDF Disk Image

***************************************************************************/

#include "coupe_sdf.h"
#include "ioprocs.h"
#include <array>

coupe_sdf_format::coupe_sdf_format()
{
}

const char *coupe_sdf_format::name() const noexcept
{
	return "coupe_sdf";
}

const char *coupe_sdf_format::description() const noexcept
{
	return "SAM Coupe SDF disk image";
}

const char *coupe_sdf_format::extensions() const noexcept
{
	return "sdf";
}

int coupe_sdf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t file_size;
	if (io.length(file_size))
		return 0;

	const uint64_t cylinder_size = SDF_TRACK_SIZE * SDF_SIDES;

	if (file_size % cylinder_size)
		return 0;

	const unsigned cylinders = file_size / cylinder_size;

	// 80 to 83 cylinders are supported
	if (cylinders < 80 || cylinders > 83)
		return 0;

	std::array<uint8_t, SDF_TRACK_SIZE> data;

	const unsigned tracks = file_size / SDF_TRACK_SIZE;

	for (unsigned track = 0; track < tracks; track++)
	{
		auto const [err, actual] = read_at(io, uint64_t(track) * SDF_TRACK_SIZE, data.data(), data.size());

		if (err || actual != data.size())
			return 0;

		const unsigned sector_count = data[0];

		size_t pos = 1;

		// iterate over all sectors and verify that they fit into a track
		for (unsigned sector = 0; sector < sector_count; sector++)
		{
			// 8 byte sector descriptor
			if ((pos + 8) > data.size())
				return 0;

			const unsigned size_code = data[pos + 5];

			// sector sizes larger than this don't fit into a track
			if (size_code > 5)
				return 0;

			const size_t sector_size = 128U << size_code;

			// verify that we're still inside the allowed size
			if ((pos + 8 + sector_size) > data.size())
				return 0;

			pos += 8 + sector_size;
		}
	}

	return FIFID_SIZE | FIFID_STRUCT;
}

bool coupe_sdf_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t file_size;
	if (io.length(file_size))
		return false;

	const unsigned tracks = file_size / SDF_TRACK_SIZE;
	const unsigned cylinders = tracks / SDF_SIDES;

	for (unsigned head = 0; head < SDF_SIDES; head++)
	{
		for (unsigned cylinder = 0; cylinder < cylinders; cylinder++)
		{
			std::array<uint8_t, SDF_TRACK_SIZE> track_data;
			const int track_index = (head * cylinders) + cylinder;

			auto const [err, actual] = read_at(io, uint64_t(track_index) * SDF_TRACK_SIZE, track_data.data(), track_data.size());

			if (err || actual != track_data.size())
				return false;

			// the first byte of the track contains the sector count
			const int sector_count = track_data[0];

			std::vector<desc_pc_sector> sectors;
			sectors.reserve(sector_count);

			const uint8_t *p = track_data.data() + 1;

			for (int i = 0; i < sector_count; i++)
			{
				// sector description, 8 bytes
				const uint8_t idstatus   = p[0];
				const uint8_t datastatus = p[1];
				const uint8_t id_cyl     = p[2];
				const uint8_t id_head    = p[3];
				const uint8_t sector     = p[4];
				const uint8_t size       = p[5];

				const int actual_size = 128 << size;
				const uint8_t *data = p + 8;

				const bool bad_id_crc = (idstatus & SDF_CRC_ERROR) != 0;
				const bool data_not_found = (datastatus & SDF_RECORD_NOT_FOUND) != 0;

				sectors.push_back({
					id_cyl,
					id_head,
					sector,
					size,
					actual_size,
					(bad_id_crc || data_not_found) ? nullptr : const_cast<uint8_t *>(data),
					bool(datastatus & SDF_DELETED_DATA),
					bool(datastatus & SDF_CRC_ERROR),
					bad_id_crc,
					false
				});

				p = data + actual_size;
			}

			build_wd_track_mfm(cylinder, head, image, 100000, sectors.size(), sectors.data(), 24, 60, 22);
		}
	}

	return true;
}

const coupe_sdf_format FLOPPY_COUPE_SDF_FORMAT;
