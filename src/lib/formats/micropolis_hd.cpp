// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/***************************************************************************

Micropolis MFM HDD format

See vgi_dsk.cpp for the related floppy format. The sector format is:
SYNC (0xFF), head, track, sector, 256 byte data, 4 byte ECC.

****************************************************************************/

#include "micropolis_hd.h"

static constexpr int SECTORS = 32;
static constexpr int SECTOR_SIZE = 256;

uint64_t micropolis_mfmhd_format::chs_to_offset(int cylinder, int head, int sector)
{
	return ((cylinder * m_param.heads + head) * SECTORS + sector) * SECTOR_SIZE;
}

std::error_condition micropolis_mfmhd_format::load(chd_file *chdfile, uint16_t *trackimage, int tracksize, int cylinder, int head)
{
	if (tracksize < 10416)
		return std::error_condition(std::errc::invalid_argument);
	if (cylinder >= m_param.cylinders || head >= m_param.heads)
		return std::error_condition(std::errc::invalid_argument);
	uint8_t sector_bytes[SECTOR_SIZE];
	int pos = 0;
	for (int sector = 0; sector < SECTORS; sector++) {
		uint64_t offset = chs_to_offset(cylinder, head, sector);
		std::error_condition ret = chdfile->read_bytes(offset, sector_bytes, std::size(sector_bytes));
		if (ret)
			return ret;

		mfm_encode(trackimage, pos, 0x00, 30);
		mfm_encode(trackimage, pos, 0xFF);
		mfm_encode(trackimage, pos, head);
		mfm_encode(trackimage, pos, cylinder);
		mfm_encode(trackimage, pos, sector);
		for (uint8_t byte : sector_bytes)
			mfm_encode(trackimage, pos, byte);
		mfm_encode(trackimage, pos, 0x00, 4); // ECC
		mfm_encode(trackimage, pos, 0x00, tracksize * (sector+1)/32 - pos);
	}
	mfm_encode(trackimage, pos, 0x00, tracksize - pos);
	return std::error_condition();
}

std::error_condition micropolis_mfmhd_format::save(chd_file *chdfile, uint16_t *trackimage, int tracksize, int cylinder, int head)
{
	if (tracksize < 10416)
		return std::error_condition(std::errc::invalid_argument);
	if (cylinder >= m_param.cylinders || head >= m_param.heads)
		return std::error_condition(std::errc::invalid_argument);
	uint8_t sector_bytes[SECTOR_SIZE];
	for (int sector = 0; sector < SECTORS; sector++) {
		int sector_start = tracksize * sector/32;
		int pos = sector_start + 16;
		while (pos++ < sector_start + 50) {
			if (trackimage[pos] == 0x5555)
				break;
		}
		if (trackimage[pos++] != 0x5555) {
			osd_printf_verbose("%s: Unable to find start of sector %d, %d, %d (CHS)\n", tag(), cylinder, head, sector);
			continue;
		}
		if (mfm_decode(trackimage[pos++]) != head) {
			osd_printf_verbose("%s: Wrong header of sector %d, %d, %d (CHS) head = %d\n", tag(), cylinder, head, sector, trackimage[pos-1]);
			continue;
		}
		if (mfm_decode(trackimage[pos++]) != cylinder) {
			osd_printf_verbose("%s: Wrong header of sector %d, %d, %d (CHS) track = %d\n", tag(), cylinder, head, sector, trackimage[pos-1]);
			continue;
		}
		if (mfm_decode(trackimage[pos++]) != sector) {
			osd_printf_verbose("%s: Wrong header of sector %d, %d, %d (CHS) sector = %d\n", tag(), cylinder, head, sector, trackimage[pos-1]);
			continue;
		}
		for (int i = 0; i < std::size(sector_bytes); i++)
			sector_bytes[i] = mfm_decode(trackimage[pos++]);

		uint64_t offset = chs_to_offset(cylinder, head, sector);
		std::error_condition ret = chdfile->write_bytes(offset, sector_bytes, std::size(sector_bytes));
		if (ret)
			return ret;
	}
	return std::error_condition();
}

bool micropolis_mfmhd_format::save_param(mfmhd_param_t type)
{
	switch (type)
	{
	case MFMHD_WPCOM:
	case MFMHD_RWC:
		return true;
	default:
		return false;
	}
}

const mfmhd_format_type MFMHD_MICROPOLIS_FORMAT = &mfmhd_image_format_creator<micropolis_mfmhd_format>;
