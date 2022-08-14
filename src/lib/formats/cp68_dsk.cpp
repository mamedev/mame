// license:BSD-3-Clause copyright-holders:Barry Rodewald, 68bit, Michael R. Furman
/*
 * cp68_dsk.cpp  -  CP68 compatible disk images
 *
 *  Created on: 02/08/2022
 *
 * This CP68 floppy disk image support leverages the wd177x_format support with
 * the few differences handled by format variations.  This format is based off
 * flex_dsk.cpp with the necessary changes for the CP/68 disk format.
 *
 * The CP68 Disk format is as follows:
 *
 * Geometry:: 18 Sectors * 35 Tracks * 128 Bytes * 1 Side
 *
 * Track 0:: Boot Sectors 0, 1, 2; Directory Sectors 4-18
 * Tracks 1-34::  Sectors 1-18
 *
 * System Information Record:: Sector 2, Bytes $7A-$7F
 *         $7A - System Link Start Track
 *         $7B - System Link Start Sector
 *         $7C - System Link End Track
 *         $7D - System Link End Sector
 *         $7E - Free Chain Start Track
 *         $7F - Free Chain Start Sector
 *
 * The formats below include interleaved sectors to improve performance. The
 * interleave has been chosen to perform well on slower hardware and software
 * combinations while still offering some performance improvement. Tighter
 * interleaving may well be possible but it depends on the systems and
 * software.
 *
 * Note about Gap Bytes::
 *         The gap bytes used here for MAME are not standard.  The reason is
 *         with 4,000 time quanta for each cell only allows for 50,000 Cells
 *         total on the disk.  The original Gap Bytes as shown in the manual
 *         https://archive.org/details/CP-68_An_M6800_Operating_System_1979/page/80/mode/2up?q=GAP
 *         wind up with too many cells to fit on the disk. This has been
 *         adjusted to the minimum allowable according to the WD1771 data
 *         sheet of 8, 11, 11
 *
 * Note: This code is compatible with the Disk images from deramp.com (Mike
 * Douglas) at the following location.  It is _NOT_ compatible with the SimH
 * disk images:
 *
 * https://deramp.com/downloads/swtpc/software/CP68/
 */

#include "cp68_dsk.h"

#include "imageutl.h"

#include "ioprocs.h"

namespace
{
	class cp68_formats : public wd177x_format
	{
	public:
		struct sysinfo_sector_cp68
		{
			uint8_t unused1[122]{};
			uint8_t link_start_track = 0;
			uint8_t link_start_sector = 0;
			uint8_t link_end_track = 0;
			uint8_t link_end_sector = 0;
			uint8_t fc_start_track = 0;
			uint8_t fc_start_sector = 0;
		};

		static const format formats[];
		static const format formats_head1[];
		static const format formats_track0[];
		static const format formats_head1_track0[];
	};
}

cp68_format::cp68_format() : wd177x_format(cp68_formats::formats)
{
}

const char *cp68_format::name() const
{
	return "cp68";
}

const char *cp68_format::description() const
{
	return "CP/68 compatible disk image";
}

const char *cp68_format::extensions() const
{
	return "dsk";
}

int cp68_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_SIZE;
	return 0;
}


int cp68_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	uint8_t boot0[128];
	cp68_formats::sysinfo_sector_cp68 info;
	size_t actual;
	std::error_condition ec;

	// Look at the boot sector.
	ec = io.read_at(128 * 0, &boot0, sizeof(boot0), actual);
	if (ec || actual == 0)
		return -1;
	uint8_t boot0_sector_id = 1;
	//  uint8_t boot1_sector_id = 2;

	// This floppy format uses a strategy of looking for 6800 boot code to
	// set the numbering of the first two sectors. If this is shown to not
	// be practical in some common cases then a separate format variant
	// might be needed.
	if (boot0[0] == 0xbd && boot0[3] == 0x86)
	{
		// Found a 6800 jsr and ldaa, looks like a CP68 6800 boot sector.
		boot0_sector_id = 0;
	}

	for (int i=0; cp68_formats::formats[i].form_factor; i++) {
		const format &f = cp68_formats::formats[i];
		
		// Look at the system information sector.
		ec = io.read_at(f.sector_base_size * 2, &info, sizeof(struct cp68_formats::sysinfo_sector_cp68), actual);
		if (ec || actual == 0)
			continue;

		LOG_FORMATS("CP68 floppy dsk: size %d bytes, %d total sectors, %d remaining bytes, expected form factor %x\n", (uint32_t)size, (uint32_t)size / f.sector_base_size, (uint32_t)size % f.sector_base_size, form_factor);

		// Consistency checks.
		//   Free Chain
		if (info.fc_start_track < 1 || info.fc_start_track >= f.track_count)
			continue;
		else if (info.fc_start_sector < 1 || info.fc_start_sector > f.sector_count)
			continue;

		//   Init Linkage
		if (info.link_start_track < 1 || info.link_start_track >= f.track_count)
			continue;
		else if (info.link_end_track < 1 || info.link_end_track >= f.track_count)
			continue;

		if (info.link_start_sector < 1 || info.link_start_sector > f.sector_count)
			continue;
		else if (info.link_end_sector < 1 || info.link_end_sector > f.sector_count)
			continue;


		unsigned int format_size = 0;
		for (int track=0; track < f.track_count; track++) {
			for (int head=0; head < f.head_count; head++) {
				const format &tf = get_track_format(f, head, track);
				format_size += compute_track_size(tf);
			}
		}

		if (format_size != size)
			continue;

		// Check that the boot sector ID matches.
		const format &ft0 = cp68_formats::formats_track0[i];
		if (ft0.form_factor) {
			// There is a specialized track 0 format.
			if (ft0.sector_base_id == -1) {
				if (ft0.per_sector_id[0] != boot0_sector_id)
					continue;
			} else {
				if (ft0.sector_base_id != boot0_sector_id)
					continue;
			}
		} else {
			// Otherwise check the default track format.
			if (f.sector_base_id == -1) {
				if (f.per_sector_id[0] != boot0_sector_id)
					continue;
			} else {
				if (f.sector_base_id != boot0_sector_id)
					continue;
			}
		}

		LOG_FORMATS("CP68 matching format index %d\n", i);
		return i;
	}
	return -1;
}

const wd177x_format::format &cp68_format::get_track_format(const format &f, int head, int track) const
{
	int n = -1;

	for (int i = 0; cp68_formats::formats[i].form_factor; i++) {
		if (&cp68_formats::formats[i] == &f) {
			n = i;
			break;
		}
	}

	if (n < 0) {
		LOG_FORMATS("Error format not found\n");
		return f;
	}

	if (head >= f.head_count) {
		LOG_FORMATS("Error invalid head %d\n", head);
		return f;
	}

	if (track >= f.track_count) {
		LOG_FORMATS("Error invalid track %d\n", track);
		return f;
	}

	if (track > 0) {
		if (head == 1) {
			const format &fh1 = cp68_formats::formats_head1[n];
			if (!fh1.form_factor) {
				LOG_FORMATS("Error expected a head 1 format\n");
				return f;
			}
			return fh1;
		}
		return f;
	}

	// Track 0

	if (head == 1) {
		const format &fh1t0 = cp68_formats::formats_head1_track0[n];
		if (fh1t0.form_factor) {
			return fh1t0;
		}
		const format &fh1 = cp68_formats::formats_head1[n];
		if (fh1.form_factor) {
			return fh1;
		}
		LOG_FORMATS("Error expected a head 1 format\n");
		return f;
	}

	// Head 0

	const format &ft0 = cp68_formats::formats_track0[n];
	if (ft0.form_factor) {
		return ft0;
	}

	return f;
}


const cp68_formats::format cp68_formats::formats[] = {
	{ // 0 87.5K 5 1/4 inch single density cp68 1.0 format three boot sectors
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 18, 35, 1, 128, {}, -1, {1, 6, 11, 16, 3, 8, 13, 18, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14}, 8, 11, 11
	},
	{}
};

const cp68_formats::format cp68_formats::formats_head1[] = {
	{ // 0 87.5K 5 1/4 inch single density cp68 1.0 format three boot sectors
	},
	{}
};

const cp68_formats::format cp68_formats::formats_track0[] = {
	{ // 0 87.5K 5 1/4 inch single density cp68 1.0 format three boot sectors
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 18, 35, 1, 128, {}, -1, {0, 6, 11, 16, 2, 8, 13, 18, 5, 10, 15, 1, 7, 12, 17, 4, 9, 14}, 8, 11, 11
	},
	{}
};

const cp68_formats::format cp68_formats::formats_head1_track0[] = {
	{ // 0 87.5K 5 1/4 inch single density cp68 1.0 format three boot sectors
	},
	{}
};

const cp68_format FLOPPY_CP68_FORMAT;
