// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/*********************************************************************

Micropolis VGI disk image format

The format is essentially an HCS-ordered sector image, but with raw
275 byte sectors. The sector format is: SYNC (0xFF), track, sector,
10 byte "user data", 256 byte data, checksum, 4 byte ECC, ECC present
flag.

http://www.bitsavers.org/pdf/micropolis/metafloppy/1084-01_1040_1050_Users_Manual_Apr79.pdf

*********************************************************************/

#include "vgi_dsk.h"
#include "ioprocs.h"

#include <cstring>

static constexpr int TRACK_SIZE = 100'000;
static constexpr int HALF_BITCELL_SIZE = 2000;

micropolis_vgi_format::micropolis_vgi_format() : floppy_image_format_t()
{
}

struct format {
	int head_count;
	int track_count;
	uint32_t variant;
};

static const format formats[] = {
	{1, 35, floppy_image::SSDD16}, // MOD-I
	{2, 35, floppy_image::DSDD16},
	{1, 77, floppy_image::SSQD16}, // MOD-II
	{2, 77, floppy_image::DSQD16},
	{}
};

static format find_format(int file_size)
{
	for (int i = 0; formats[i].head_count; i++)
		if (file_size == formats[i].head_count * formats[i].track_count * 16 * 275)
			return formats[i];
	return {};
}

int micropolis_vgi_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t file_size;
	if (io.length(file_size))
		return 0;
	format fmt = find_format(file_size);
	if (!fmt.head_count)
		return 0;
	if (!has_variant(variants, fmt.variant))
		return 0;
	return FIFID_SIZE;
}

bool micropolis_vgi_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t file_size;
	if (io.length(file_size))
		return false;
	const format fmt = find_format(file_size);
	if (!fmt.head_count)
		return false;
	image.set_variant(fmt.variant);

	std::vector<uint32_t> buf;
	uint8_t sector_bytes[275];
	for (int head = 0; head < fmt.head_count; head++) {
		for (int track = 0; track < fmt.track_count; track++) {
			for (int sector = 0; sector < 16; sector++) {
				for (int i = 0; i < 40; i++)
					mfm_w(buf, 8, 0, HALF_BITCELL_SIZE);
				auto const [err, actual] = read(io, sector_bytes, std::size(sector_bytes));
				if (err || (actual != std::size(sector_bytes)))
					return false;
				for (int i = 0; i < std::size(sector_bytes); i++)
					mfm_w(buf, 8, sector_bytes[i], HALF_BITCELL_SIZE);
				while (buf.size() < TRACK_SIZE/16 * (sector+1))
					mfm_w(buf, 8, 0, HALF_BITCELL_SIZE);
			}
			generate_track_from_levels(track, head, buf, 0, image);
			buf.clear();
		}
	}
	return true;
}

bool micropolis_vgi_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	uint32_t variant = image.get_variant();
	format fmt = {};
	for (int i = 0; formats[i].head_count; i++)
		if (variant == formats[i].variant)
			fmt = formats[i];
	if (!fmt.head_count) {
		int heads, tracks;
		image.get_actual_geometry(tracks, heads);
		if (heads == 0 && tracks == 0)
			return false; // Brand-new image; we don't know the size yet
		for (int i = 0; formats[i].head_count; i++)
			if (heads <= formats[i].head_count && tracks <= formats[i].track_count)
				fmt = formats[i];
	}
	if (!fmt.head_count)
		return false;

	if (io.seek(0, SEEK_SET))
		return false;
	uint8_t sector_bytes[275];
	for (int head = 0; head < fmt.head_count; head++) {
		for (int track = 0; track < fmt.track_count; track++) {
			std::vector<bool> bitstream = generate_bitstream_from_track(track, head, HALF_BITCELL_SIZE, image);
			for (int sector = 0; sector < 16; sector++) {
				int sector_start = TRACK_SIZE/16 * sector;
				uint32_t pos = sector_start + 512 - 16;
				uint16_t shift_reg = 0;
				while (pos < sector_start + 60*16 && pos < bitstream.size()) {
					shift_reg = (shift_reg << 1) | bitstream[pos++];
					if (shift_reg == 0x5554)
						break;
				}
				if (shift_reg == 0x5554) {
					pos--;
					for (int i = 0; i < std::size(sector_bytes); i++)
						sector_bytes[i] = sbyte_mfm_r(bitstream, pos);
				} else {
					memset(sector_bytes, 0, std::size(sector_bytes));
				}

				auto const [err, actual] = write(io, sector_bytes, std::size(sector_bytes));
				if (err)
					return false;
			}
		}
	}
	return true;
}

const micropolis_vgi_format FLOPPY_VGI_FORMAT;
