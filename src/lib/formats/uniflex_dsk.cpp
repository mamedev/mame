// license:BSD-3-Clause
// copyright-holders:68bit
/*
 * uniflex_dsk.cpp  -  UniFLEX compatible disk images
 *
 * The UniFLEX floppy disk format is distinctily different to the FLEX format,
 * the sector size is 512 byte sectors versus 256 byte sectors and the format
 * of the disk information is different, and the file system format is
 * different.
 */

#include "uniflex_dsk.h"
#include "imageutl.h"

#include "ioprocs.h"
#include "multibyte.h"


uniflex_format::uniflex_format() : wd177x_format(formats)
{
}

const char *uniflex_format::name() const noexcept
{
	return "uniflex";
}

const char *uniflex_format::description() const noexcept
{
	return "UniFLEX compatible disk image";
}

const char *uniflex_format::extensions() const noexcept
{
	return "dsk";
}

int uniflex_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int const type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_SIZE;

	return 0;
}

int uniflex_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	// Look at the SIR sector, the second sector.
	uint8_t sir[192];
	auto const [err, actual] = read_at(io, 1 * 512, sir, sizeof(sir)); // FIXME: check for premature EOF
	if (err)
		return -1;

	uint16_t fdn_block_count = get_u16be(&sir[0x10]);
	uint32_t last_block_number = get_u24be(&sir[0x12]);
	uint32_t free_block_count = get_u24be(&sir[0x15]);
	uint16_t fdn_free_count = get_u16be(&sir[0x15]); // FIXME: offset should be different from previous?

	uint16_t volume_number = get_u16be(&sir[0x36]);
	uint8_t disk_density = sir[0x3a];
	uint8_t disk_side_info = sir[0x3b];

	uint32_t volbc_start_addr = get_u24be(&sir[0x3c]);
	uint16_t swap_size = get_u16be(&sir[0x3f]);

	LOG_FORMATS("UniFLEX floppy dsk size %d\n", (uint32_t)size);
	LOG_FORMATS(" time = %u %u\n", get_u32be(&sir[0x08]), get_u32be(&sir[0x0c]));
	LOG_FORMATS(" fdn_block_count: %d\n", fdn_block_count);

	LOG_FORMATS(" file system name: ");
	for (int i = 0; i < 14; i++)
		LOG_FORMATS(" %02x", sir[0x1a + i]);
	LOG_FORMATS("\n");

	LOG_FORMATS(" volume name: ");
	for (int i = 0; i < 14; i++)
		LOG_FORMATS(" %02x", sir[0x28 + i]);
	LOG_FORMATS("\n");

	LOG_FORMATS(" last_block_number: %d\n", last_block_number);
	LOG_FORMATS(" free_block_count: %d\n", free_block_count);
	LOG_FORMATS(" fdn_free_count: %d\n", fdn_free_count);
	LOG_FORMATS(" volume_number: %d\n", volume_number);
	LOG_FORMATS(" disk density: %02x, side info %02x\n", disk_density, disk_side_info);
	LOG_FORMATS(" volbc_start_addr: %d\n", volbc_start_addr);
	LOG_FORMATS(" swap_size: %d\n", swap_size);

	// The first eight bytes appear to be zeros.
	if (get_u64be(&sir[0x00]) != 0)
		return -1;

	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if(size != (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			continue;

		// Check consistency with the SIR sector.
		if(last_block_number + 1 + swap_size != f.sector_count * f.track_count * f.head_count)
			continue;
		if(f.head_count == 2 && disk_side_info == 0)
			continue;
		if(f.encoding == floppy_image::MFM && disk_density == 0)
			continue;
		if(f.encoding == floppy_image::FM && disk_density != 0)
			continue;

		return i;
	}
	return -1;
}

// UniFLEX numbers sectors on the second side of a track continuing from the
// first side which is a variation not handled by the generic code.
void uniflex_format::build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const
{
	if(f.sector_base_id == -1) {
		for(int i=0; i<f.sector_count; i++) {
			int cur_offset = 0;
			for(int j=0; j<f.sector_count; j++)
				if(f.per_sector_id[j] < f.per_sector_id[i])
					cur_offset += f.sector_base_size ? f.sector_base_size : f.per_sector_size[j];
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			sectors[i].sector_id = f.sector_count * head + f.per_sector_id[i];
		}
	} else {
		int cur_offset = 0;
		for(int i=0; i<f.sector_count; i++) {
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			cur_offset += sectors[i].size;
			sectors[i].sector_id = f.sector_count * head + i + f.sector_base_id;
		}
	}
}

const uniflex_format::format uniflex_format::formats[] = {
	{ // 616K 8 inch double density - gaps unverified
		floppy_image::FF_8, floppy_image::SSDD, floppy_image::MFM,
		1000, 16, 77, 1, 512, {}, 1, {}, 32, 22, 50
	},
	{ // 1232K 8 inch double density - gaps unverified
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		1000, 16, 77, 2, 512, {}, 1, {}, 32, 22, 50
	},

	{}
};

const uniflex_format FLOPPY_UNIFLEX_FORMAT;
