// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, tim lindner
/*********************************************************************

    formats/sdf_dsk.cpp

    SDF disk images. Format created by Darren Atkinson for use with
    his CoCoSDC floppy disk emulator.

    http://cocosdc.blogspot.com/p/sd-card-socket-sd-card-socket-is-push.html

    Saving shares DMK's FM/MFM decoder and its preservation limits.
    SDF output is limited to 80 cylinders, two sides, 31 sectors per
    track, and a 6250-byte SD/DD revolution with doubled FM bytes.

*********************************************************************/

#include "sdf_dsk.h"

#include "hashing.h"
#include "ioprocs.h"
#include "multibyte.h"

#include <algorithm>


sdf_format::sdf_format()
{
}


const char *sdf_format::name() const noexcept
{
	return "sdf";
}


const char *sdf_format::description() const noexcept
{
	return "SDF disk image";
}


const char *sdf_format::extensions() const noexcept
{
	return "sdf";
}


int sdf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	return dmk_format::identify(io, form_factor, variants) ? FIFID_SIGN | FIFID_SIZE | FIFID_STRUCT : 0;
}


bool sdf_format::read_header(util::random_read &io, disk_header &disk) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	uint8_t header[HEADER_SIZE];
	auto const [err, actual] = read_at(io, 0, header, HEADER_SIZE);
	if (err || actual != HEADER_SIZE)
		return false;

	if (header[0] != 'S' || header[1] != 'D' || header[2] != 'F' || header[3] != '1')
		return false;
	if (!header[4] || header[4] > 80 || (header[5] != 1 && header[5] != 2))
		return false;

	disk.tracks = header[4];
	disk.heads = header[5];
	disk.track_size = dmk_format::TRACK_HEADER_SIZE + TRACK_SIZE;
	disk.flags = 0; // SDF always doubles FM bytes, including on mixed-density tracks.
	return size == HEADER_SIZE + uint64_t(disk.tracks) * disk.heads * TOTAL_TRACK_SIZE;
}


bool sdf_format::read_track(util::random_read &io, const disk_header &disk, int track, int head,
	std::vector<uint8_t> &data, std::vector<int> &dams) const
{
	std::vector<uint8_t> record(TOTAL_TRACK_SIZE);
	auto const [err, actual] = read_at(io, HEADER_SIZE + (disk.heads * track + head) * TOTAL_TRACK_SIZE, record.data(), record.size());
	if (err || actual != record.size() || record[0] > SECTOR_SLOT_COUNT)
		return false;

	// Keep only the revolution, replacing SDF's header with a DMK IDAM table.
	data.assign(dmk_format::TRACK_HEADER_SIZE, 0);
	data.insert(data.end(), record.begin() + TRACK_HEADER_SIZE, record.begin() + TRACK_HEADER_SIZE + TRACK_SIZE);
	dams.clear();
	for (int i = 0; i < record[0]; i++)
	{
		uint16_t const id = get_u16le(&record[8 * (i + 1)]);
		uint16_t const dam = get_u16le(&record[8 * (i + 1) + 2]);
		bool const fm = id & 0x4000;
		int const stride = fm ? 2 : 1;
		// SDF points at the first field byte after FE/FB, rather than the mark.
		int const id_field = id & 0x3fff;
		int const data_field = dam & 0x3fff;
		if (id_field < TRACK_HEADER_SIZE || id_field >= TRACK_HEADER_SIZE + TRACK_SIZE)
			return false;
		if (data_field && (data_field < TRACK_HEADER_SIZE || data_field >= TRACK_HEADER_SIZE + TRACK_SIZE))
			return false;
		int const id_offset = TRACK_HEADER_SIZE + (id_field - TRACK_HEADER_SIZE - stride + TRACK_SIZE) % TRACK_SIZE;
		int const dam_offset = TRACK_HEADER_SIZE + (data_field - TRACK_HEADER_SIZE - stride + TRACK_SIZE) % TRACK_SIZE;
		if (data_field && (record[dam_offset] < 0xf8 || record[dam_offset] > 0xfb))
			return false;

		int const adjustment = TRACK_HEADER_SIZE - dmk_format::TRACK_HEADER_SIZE;
		put_u16le(&data[i * 2], (id_offset - adjustment) | (fm ? 0 : 0x8000));
		dams.push_back((dam & 0x3fff) ? dam_offset - adjustment : 0);
	}
	return true;
}


bool sdf_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks, heads;
	image.get_actual_geometry(tracks, heads);
	if (tracks > 80 || image.get_form_factor() == floppy_image::FF_8
		|| image.get_variant() == floppy_image::DSHD || image.get_variant() == floppy_image::DSED)
		return false;

	return dmk_format::save(io, variants, image);
}


bool sdf_format::write_image(util::random_read_write &io, int tracks, int heads, bool initial_mfm,
	std::vector<decoded_track> &data) const
{
	uint8_t header[HEADER_SIZE] = { 'S', 'D', 'F', '1' };
	header[4] = tracks;
	header[5] = heads;
	std::vector<std::vector<uint8_t>> records(data.size());
	for (unsigned index = 0; index < data.size(); index++)
	{
		auto &track = data[index];
		int const length = int(track.data.size()) - dmk_format::TRACK_HEADER_SIZE;
		if (!length || track.dams.size() > SECTOR_SLOT_COUNT)
			return false;

		// Density/phase changes can add or lose a byte during decoding.  Fit
		// SDF's fixed revolution by adjusting only filler outside syncs and
		// recorded fields, including fields which cross index.
		if (length != TRACK_SIZE)
		{
			std::vector<bool> field(length, false);
			auto protect = [&](int begin, int count)
			{
				for (int i = 0; i < count; i++)
					field[(begin - dmk_format::TRACK_HEADER_SIZE + i + length) % length] = true;
			};
			for (unsigned sector = 0; sector < track.dams.size(); sector++)
			{
				uint16_t const id = get_u16le(&track.data[sector * 2]);
				int const stride = (id & 0x8000) ? 1 : 2;
				int const idam = id & 0x3fff;
				int const sync = stride == 1 ? 15 : 12;
				protect(idam - sync, sync + 7 * stride);
				if (track.dams[sector])
				{
					int const size_offset = dmk_format::TRACK_HEADER_SIZE + (idam - dmk_format::TRACK_HEADER_SIZE + 4 * stride) % length;
					int const size = 128 << (track.data[size_offset] & 3);
					protect(track.dams[sector] - sync, sync + (size + 3) * stride);
				}
			}
			int const remove = std::max(0, length - TRACK_SIZE);
			int run = 0;
			int gap = -1;
			for (int i = 0; i < length; i++)
			{
				uint8_t const value = track.data[dmk_format::TRACK_HEADER_SIZE + i];
				run = !field[i] && (value == 0x4e || value == 0xff) ? run + 1 : 0;
				if (run >= std::max(2, remove))
				{
					gap = dmk_format::TRACK_HEADER_SIZE + i - run + 1;
					break;
				}
			}
			if (gap < 0)
				return false;
			int const change = TRACK_SIZE - length;
			for (unsigned sector = 0; sector < track.dams.size(); sector++)
			{
				uint16_t id = get_u16le(&track.data[sector * 2]);
				if ((id & 0x3fff) >= gap)
					put_u16le(&track.data[sector * 2], (id & 0x8000) | ((id & 0x3fff) + change));
				if (track.dams[sector] >= gap)
					track.dams[sector] += change;
			}
			if (remove)
				track.data.erase(track.data.begin() + gap, track.data.begin() + gap + remove);
			else
				track.data.insert(track.data.begin() + gap + 1, change, track.data[gap]);
		}

		auto &record = records[index];
		record.assign(TOTAL_TRACK_SIZE, 0);
		record[0] = track.dams.size();
		std::copy(track.data.begin() + dmk_format::TRACK_HEADER_SIZE, track.data.end(), record.begin() + TRACK_HEADER_SIZE);
		auto offset = [](int position) { return TRACK_HEADER_SIZE + (position - TRACK_HEADER_SIZE + TRACK_SIZE) % TRACK_SIZE; };
		for (unsigned sector = 0; sector < track.dams.size(); sector++)
		{
			uint16_t const id = get_u16le(&track.data[sector * 2]);
			bool const fm = !(id & 0x8000);
			int const stride = fm ? 2 : 1;
			int const adjustment = TRACK_HEADER_SIZE - dmk_format::TRACK_HEADER_SIZE;
			int const idam = (id & 0x3fff) + adjustment;
			int const dam = track.dams[sector] ? track.dams[sector] + adjustment : 0;
			auto *entry = &record[8 * (sector + 1)];
			// Include the recorded CRC bytes: a correct field has remainder zero.
			auto bad_crc = [&](int mark, int bytes)
			{
				util::crc16_creator crc(fm ? 0xffff : 0xcdb4); // MFM includes three A1 sync bytes.
				for (int i = 0; i < bytes; i++)
					crc.append(&record[offset(mark + i * stride)], 1);
				return crc.finish() != util::crc16_t(0);
			};
			put_u16le(entry, offset(idam + stride) | (fm ? 0x4000 : 0) | (bad_crc(idam, 7) ? 0x8000 : 0));
			for (int i = 0; i < 4; i++)
				entry[4 + i] = record[offset(idam + (i + 1) * stride)];
			if (!dam)
				continue;

			int const size = 128 << (entry[7] & 3);
			put_u16le(entry + 2, offset(dam + stride) | ((record[dam] == 0xf8 || record[dam] == 0xfa) ? 0x4000 : 0)
				| (bad_crc(dam, size + 3) ? 0x8000 : 0));
			// Mark nested sectors, including a data field which wraps through index.
			for (unsigned other = 0; other < track.dams.size(); other++)
			{
				int const other_id = (get_u16le(&track.data[other * 2]) & 0x3fff) + adjustment;
				int const distance = (other_id - offset(dam + stride) + TRACK_SIZE) % TRACK_SIZE;
				if (distance < size * stride)
					header[7] = 1;
			}
		}
	}

	// Validate every record before writing any part of the image.
	auto const [header_error, header_actual] = write_at(io, 0, header, sizeof(header));
	if (header_error || header_actual != sizeof(header))
		return false;
	for (unsigned index = 0; index < records.size(); index++)
	{
		auto const [err, actual] = write_at(io, HEADER_SIZE + uint64_t(index) * TOTAL_TRACK_SIZE, records[index].data(), TOTAL_TRACK_SIZE);
		if (err || actual != TOTAL_TRACK_SIZE)
			return false;
	}
	return true;
}


const sdf_format FLOPPY_SDF_FORMAT;
