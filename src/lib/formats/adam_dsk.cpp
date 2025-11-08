// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Coleco Adam disk image format (new implementation)

    The Adam's Elementary Operating System (EOS) sees all mass storage
    devices as sequences of 1024-byte blocks addressed linearly through the
    ADAMnet bus. However, these blocks are actually stored on 40-track or
    80-track floppy disks in two logically consecutive 512-byte sectors.
    Furthermore, logically consecutive sectors are physically non-adjacent
    due to interleaving implemented at the level of the FDC MCU (the
    geometry is transparent to the operating system), and different native
    interleaving schemes were devised depending on the number of sectors
    per track. However, .dsk images are stored using the original 8-sector
    interleave (1,6,3,8,5,2,7,4) regardless of their capacity. The 3.5-inch
    formats, which use 9 or 18 sectors per track, thus require a convoluted
    permutation of the sectors to transform their native interleave to or
    from the non-native interleave.

	Track description for 160KB SSDD format:
	100x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	100x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	859x4e

***************************************************************************/

#include "adam_dsk.h"

#include "ioprocs.h"

#include <cstring>
#include <memory>


namespace {

struct format_desc
{
	uint32_t size;
	uint32_t form_factor;
	uint32_t variant;
	uint32_t cell_size;
	uint8_t track_count;
	uint8_t sector_count;
	uint8_t head_count;
};

const format_desc s_formats[] =
{
	{ 163840, floppy_image::FF_525, floppy_image::SSDD, 2000, 40, 8, 1 },
	{ 327680, floppy_image::FF_525, floppy_image::DSDD, 2000, 40, 8, 2 },
	{ 737280, floppy_image::FF_35, floppy_image::DSDD, 2000, 80, 9, 2 },
	{ 1474560, floppy_image::FF_35, floppy_image::DSHD, 1000, 80, 18, 2 },
	{ 0, floppy_image::FF_UNKNOWN, 0, 0, 0, 0, 0 }
};

const uint8_t s_interleave_9s[9] =
{
	0, 4, 8, 3, 7, 2, 6, 1, 5
};

const uint8_t s_interleave_18s[18] =
{
	0, 4, 8, 12, 16, 2, 6, 10, 14,
	1, 5, 9, 13, 17, 3, 7, 11, 15
};

const uint8_t s_cpm_boot[8] =
{
	0x18, 0x01, 0xe5, 0x3e, 0xc9, 0x32, 0x66, 0x00
};

constexpr uint8_t ZZZZ = 0x40; // a "don't care" value

const uint8_t s_eos_directory[0x38] =
{
	0x55, 0xaa, 0x00, 0xff, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, // magic numbers required by EOS
	'B',  'O',  'O',  'T',  0x03, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ,
	ZZZZ, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, ZZZZ, ZZZZ, ZZZZ, ZZZZ, ZZZZ, // covers block 0 and only block 0
	'D',  'I',  'R',  'E',  'C',  'T',  'O',  'R',  'Y',  0x03, ZZZZ, ZZZZ,
	ZZZZ, 0x01, 0x00, 0x00, 0x00 // begins with block 1
};

} // anonymous namespace


const adam_format FLOPPY_ADAM_FORMAT;

adam_format::adam_format()
{
}

const char *adam_format::name() const noexcept
{
	return "adam";
}

const char *adam_format::description() const noexcept
{
	return "Coleco Adam disk image";
}

const char *adam_format::extensions() const noexcept
{
	return "dsk";
}

bool adam_format::supports_save() const noexcept
{
	return true;
}

int adam_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	// Size must match exactly
	const format_desc *f = &s_formats[0];
	while (f->size != size)
	{
		if (!(++f)->size)
			return 0;
	}

	// Form factor and variant must agree if provided
	if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f->form_factor)
		return 0;
	if (!variants.empty() && !has_variant(variants, f->variant))
		return 0;

	// Size confirmed; now do a little extra work to confirm presence of Adam data
	// (since .dsk extension is shared by a lot of formats for other systems)

	// First, check for the beginning of a standalone bootloader commonly used on CP/M disks
	std::uint8_t buffer[0x38];
	auto const [err, actual] = read_at(io, 0, &buffer[0], 8);
	if (!err && actual == 8 && !std::memcmp(&buffer[0], s_cpm_boot, 8))
		return FIFID_SIZE | FIFID_HINT;

	// EOS bootloaders are varied enough that checking for their system calls is annoying inefficient
	// Instead, check in block 1 for the magic signature and the special BOOT and DIRECTORY entries
	// (which some CP/M disks also have as part of a fake directory structure)
	auto const [err2, actual2] = read_at(io, 0x40d, &buffer[0], 0x38);
	if (!err2 && actual2 == 0x38)
	{
		int i = 0;
		while (i < 0x38 && (s_eos_directory[i] == ZZZZ || s_eos_directory[i] == buffer[i]))
			i++;
		if (i == 0x38)
			return FIFID_SIZE | FIFID_HINT;
	}

	return FIFID_SIZE;
}

bool adam_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	// Find the size that matches
	const format_desc *f = &s_formats[0];
	while (f->size != size)
	{
		if (!(++f)->size)
			return false;
	}

	// Verify that the geometry will fit
	int max_tracks, max_heads;
	image.get_maximal_geometry(max_tracks, max_heads);
	if (f->track_count > max_tracks || f->head_count > max_heads)
		return false;

	const int track_size = f->sector_count * 512;
	const auto track_data = std::make_unique<uint8_t []>(track_size);
	desc_pc_sector sector_desc[18];

	const uint32_t cell_count = 200000000 / f->cell_size;
	unsigned sector_offset = 0;
	switch (f->form_factor)
	{
	case floppy_image::FF_525:
		// Sectors are stored in native order
		for (int sector = 0; sector < f->sector_count; sector++)
		{
			sector_desc[sector].sector = sector + 1;
			sector_desc[sector].size = 2;
			sector_desc[sector].actual_size = 512;
			sector_desc[sector].data = &track_data[uint32_t(sector) * 512];
			sector_desc[sector].deleted = false;
			sector_desc[sector].bad_data_crc = false;
			sector_desc[sector].bad_addr_crc = false;
		}

		// For double-sided 5.25" images, read one side after the other
		for (int head = 0; head < f->head_count; head++)
		{
			for (int track = 0; track < f->track_count; track++)
			{
				// Read in one entire track
				auto const [err, actual] = read_at(io, uint32_t(sector_offset) * 512, track_data.get(), track_size);
				if (err || (actual != track_size))
					return false;

				// Finish up the sector descriptions
				for (int sector = 0; sector < f->sector_count; sector++)
				{
					sector_desc[sector].track = track;
					sector_desc[sector].head = head;
				}

				// Create the track
				build_wd_track_mfm(track, head, image, cell_count, f->sector_count, sector_desc, 100, 100, 22);
				sector_offset += f->sector_count;
			}
		}
		break;

	case floppy_image::FF_35:
		// Set up the native interleave
		for (int i = 0; i < f->sector_count; i++)
		{
			const int sector = f->sector_count == 18 ? s_interleave_18s[i] : s_interleave_9s[i];
			sector_desc[sector].sector = sector + 1;
			sector_desc[sector].size = 2;
			sector_desc[sector].actual_size = 512;
			sector_desc[sector].data = &track_data[uint32_t(i) * 512];
			sector_desc[sector].deleted = false;
			sector_desc[sector].bad_data_crc = false;
			sector_desc[sector].bad_addr_crc = false;
		}

		// For 3.5" images, tracks are interleaved between sides
		for (int track = 0; track < f->track_count; track++)
		{
			for (int head = 0; head < f->head_count; head++)
			{
				for (int i = 0; i < f->sector_count; i++, sector_offset++)
				{
					// Apply the non-native sector interleave (5 modulo 8)
					const unsigned mangled_offset = sector_offset ^ ((sector_offset & 1) << 2);

					// Read in one sector at a time
					auto const [err, actual] = read_at(io, uint32_t(mangled_offset) * 512, &track_data[uint32_t(i) * 512], 512);
					if (err || (actual != 512))
						return false;

					// Also finish up the sector descriptions
					sector_desc[i].track = track;
					sector_desc[i].head = head;
				}

				// Create the track (TODO: verify gap sizes)
				build_wd_track_mfm(track, head, image, cell_count, f->sector_count, sector_desc, 84, 100, 22);
			}
		}
		break;

	default:
		// Never happens
		return false;
	}

	// Assign the form factor and variant
	image.set_form_variant(f->form_factor, f->variant);

	// Success
	return true;
}

bool adam_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks, heads;
	image.get_actual_geometry(tracks, heads);

	// Find an appropriate format
	// TODO: Analyze the cell structure of a track rather than rely on the variant field being set correctly
	const format_desc *f = &s_formats[0];
	while (f->form_factor != image.get_form_factor() || f->variant != image.get_variant()
			|| f->track_count > tracks || f->head_count > heads)
	{
		if (!(++f)->size)
			return false;
	}

	unsigned sector_offset = 0;
	switch (f->form_factor)
	{
	case floppy_image::FF_525:
		for (int head = 0; head < f->head_count; head++)
		{
			for (int track = 0; track < f->track_count; track++)
			{
				// Extract the sectors for each track
				const auto bitstream = generate_bitstream_from_track(track, head, f->cell_size, image);
				const auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
				if (sectors.size() > f->sector_count + 1)
					return false;

				for (int sector = 1; sector <= f->sector_count; sector++, sector_offset++)
				{
					const auto &s = sectors[sector];
					if (s.size() != 512)
						return false;

					// Write each sector in sequence
					auto const [err, actual] = write_at(io, uint32_t(sector_offset) * 512, s.data(), 512);
					if (err || (actual != 512))
						return false;
				}
			}
		}
		break;

	case floppy_image::FF_35:
		for (int track = 0; track < f->track_count; track++)
		{
			for (int head = 0; head < f->head_count; head++)
			{
				// Extract the sectors for each track
				const auto bitstream = generate_bitstream_from_track(track, head, f->cell_size, image);
				const auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
				if (sectors.size() > f->sector_count + 1)
					return false;

				for (int i = 0; i < f->sector_count; i++, sector_offset++)
				{
					// Resolve the native interleave
					const auto &s = sectors[(f->sector_count == 18 ? s_interleave_18s[i] : s_interleave_9s[i]) + 1];
					if (s.size() != 512)
						return false;

					// Apply the non-native sector interleave (5 modulo 8)
					const unsigned mangled_offset = sector_offset ^ ((sector_offset & 1) << 2);

					// Write each sector
					auto const [err, actual] = write_at(io, uint32_t(mangled_offset) * 512, s.data(), 512);
					if (err || (actual != 512))
						return false;
				}
			}
		}
		break;

	default:
		// Never happens
		return false;
	}

	// Success at last
	return true;
}
