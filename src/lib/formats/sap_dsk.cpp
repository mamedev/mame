// license:BSD-3-Clause
// copyright-holders:AJR

#include "sap_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

static constexpr unsigned HEADER_LENGTH = 66;
static constexpr uint8_t MAGIC_XOR = 0xb3;

const sap_dsk_format FLOPPY_SAP_FORMAT;

inline uint16_t sap_dsk_format::accumulate_crc(uint16_t crc, uint8_t data) noexcept
{
	// Almost CRC16-CCITT, but not quite...
	for (int shift : {0, 4})
		crc = ((crc >> 4) & 0xfff) ^ 0x1081 * ((crc ^ (data >> shift)) & 0xf);
	return crc;
}

const char *sap_dsk_format::name() const noexcept
{
	return "sap";
}

const char *sap_dsk_format::description() const noexcept
{
	return "Thomson SAP disk image (Systeme d'Archivage Pukall)";
}

const char *sap_dsk_format::extensions() const noexcept
{
	return "sap";
}

int sap_dsk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t buffer[HEADER_LENGTH];
	auto [err, actual] = read_at(io, 0, buffer, HEADER_LENGTH);
	if (err || actual != HEADER_LENGTH)
		return 0;

	// Check ID string
	if (memcmp(&buffer[1], "SYSTEME D'ARCHIVAGE PUKALL S.A.P. (c) Alexandre PUKALL Avril 1998", HEADER_LENGTH - 1) != 0)
		return 0;

	// Check disk format
	if (form_factor == floppy_image::FF_35)
	{
		// 3.5" disks must have 80 double-density tracks
		if ((buffer[0] & 0x7f) != 0x01)
			return 0;
	}
	else if (form_factor == floppy_image::FF_525)
	{
		// 5.25" disks must have 40 tracks
		if ((buffer[0] & 0x7e) != 0x02)
			return 0;
	}
	else if (form_factor != floppy_image::FF_UNKNOWN)
		return 0;

	uint32_t expected_size = HEADER_LENGTH + uint32_t((buffer[0] & 0x01) ? 256 + 6 : 128 + 6) * ((buffer[0] & 0x02) ? 40 : 80) * 16 * ((buffer[0] & 0x80) ? 2 : 1);
	uint64_t actual_size;
	if (!io.length(actual_size) && expected_size == actual_size)
		return FIFID_SIGN | FIFID_SIZE;
	else
		return FIFID_SIGN;
}

bool sap_dsk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t disk_type;
	if (auto [err, actual] = read_at(io, 0, &disk_type, 1); err || actual != 1)
		return false;

	int track_count = (disk_type & 0x02) ? 40 : 80;
	int head_count = (disk_type & 0x80) ? 2 : 1;
	int max_tracks, max_heads;
	image.get_maximal_geometry(max_tracks, max_heads);
	if (max_heads < head_count || max_tracks < track_count)
		return false;

	switch (disk_type & 0x03)
	{
	case 0x01:
		image.set_form_variant(floppy_image::FF_35, (disk_type & 0x80) ? floppy_image::DSDD : floppy_image::SSDD);
		break;

	case 0x02:
		image.set_form_variant(floppy_image::FF_525, (disk_type & 0x80) ? floppy_image::DSSD : floppy_image::SSSD);
		break;

	case 0x03:
		image.set_form_variant(floppy_image::FF_525, (disk_type & 0x80) ? floppy_image::DSDD : floppy_image::SSDD);
		break;

	default:
		return false;
	}

	uint8_t sector_buf[16 * 1024];
	uint8_t sector_header[4];
	uint8_t sector_crc[2];
	desc_pc_sector sectors[16];

	size_t read_offset = HEADER_LENGTH;
	for (int head = 0; head < head_count; head++)
	{
		for (int track = 0; track < track_count; track++)
		{
			uint8_t *bufptr = &sector_buf[0];
			uint8_t track_size = 16;
			int sector_count = 0;
			bool is_mfm = disk_type & 0x01;
			while (track_size != 0)
			{
				if (auto [err, actual] = read_at(io, read_offset, &sector_header[0], 4); err || actual != 4)
					return false;
				if (sector_count == 0 && sector_header[0] == 0x01)
					is_mfm = false;
				uint8_t sector_size = (sector_header[0] & 0x02) ? 4 >> (sector_header[0] & 0x01) : 1;
				if (track_size < sector_size)
					return false;
				uint16_t sector_octets = sector_size * (is_mfm ? 256 : 128);
				if (auto [err, actual] = read_at(io, read_offset + 4, bufptr, sector_octets); err || actual != sector_octets)
					return false;
				if (auto [err, actual] = read_at(io, read_offset + 4 + sector_octets, &sector_crc[0], 2); err || actual != 2)
					return false;

				uint16_t crc = 0xffff;
				for (uint8_t data : sector_header)
					crc = accumulate_crc(crc, data);
				for (uint16_t i = 0; i < sector_octets; i++)
				{
					bufptr[i] ^= MAGIC_XOR;
					crc = accumulate_crc(crc, bufptr[i]);
				}

				sectors[sector_count].track = sector_header[2];
				sectors[sector_count].head = head;
				sectors[sector_count].sector = sector_header[3];
				sectors[sector_count].size = sector_size;
				sectors[sector_count].actual_size = sector_octets;
				sectors[sector_count].data = bufptr;
				// Teo claims this flag is set for protection with "all the gap bytes set to 0xF7"; actual purpose is a bit unclear
				sectors[sector_count].deleted = bool(sector_header[0] & 0x04);
				sectors[sector_count].bad_crc = crc != get_u16be(sector_crc);

				read_offset += sector_octets + 6;
				bufptr += sector_octets;
				track_size -= sector_size;
				sector_count++;
			}
			if (is_mfm)
				build_pc_track_mfm(track, head, image, 100000, sector_count, sectors, calc_default_pc_gap3_size(form_factor, sectors[0].actual_size));
			else
				build_pc_track_fm(track, head, image, 100000 / 2, sector_count, sectors, calc_default_pc_gap3_size(form_factor, sectors[0].actual_size));
		}
	}

	return true;
}
