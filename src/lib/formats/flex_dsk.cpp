// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * flex_dsk.c  -  FLEX compatible disk images
 *
 *  Created on: 24/06/2014
 *
 * TODO This format does not yet handle double density disks with a single
 * density track 0. FLEX DSK files are generally 'fixed' to have a consistent
 * number of sectors per track which makes them easier to work with and gains
 * more storage space, and tools and emulators generally only work with these
 * formats. For now use single density disks, or patch the ROM to load the
 * boot sector(s) in double density and patch the drivers to use double
 * density on track zero. Drivers developed for emulators commonly have other
 * issues and need work anyway.
 */

#include "flex_dsk.h"
#include "formats/imageutl.h"

flex_format::flex_format() : wd177x_format(formats)
{
}

const char *flex_format::name() const
{
	return "flex";
}

const char *flex_format::description() const
{
	return "FLEX compatible disk image";
}

const char *flex_format::extensions() const
{
	return "dsk";
}

int flex_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 75;
	return 0;
}

int flex_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint64_t size = io_generic_size(io);
	uint8_t boot0[256], boot1[256];

	// Look at the boot sector.
	// Density, sides, link??
	io_generic_read(io, &boot0, 256 * 0, sizeof(boot0));
	io_generic_read(io, &boot1, 256 * 1, sizeof(boot1));
	// Look at the system information sector.
	io_generic_read(io, &info, 256 * 2, sizeof(struct sysinfo_sector));

	LOG_FORMATS("FLEX floppy dsk size %d %d %d\n", (uint32_t)size, (uint32_t)size / 256, (uint32_t)size % 256);

	LOG_FORMATS(" boot0:");
	for (int i = 0; i < 16; i++) {
	  LOG_FORMATS(" %02x", boot0[i]);
	}
	LOG_FORMATS("\n");

	LOG_FORMATS(" boot1:");
	for (int i = 0; i < 16; i++) {
	  LOG_FORMATS(" %02x", boot1[i]);
	}
	LOG_FORMATS("\n");


	// Check that the 'unused' area is actually unused.
	LOG_FORMATS(" unused1:");
	for (int i = 0; i < sizeof(info.unused1); i++) {
	  LOG_FORMATS(" %02x", info.unused1[i]);
	}
	LOG_FORMATS("\n");

	LOG_FORMATS(" disk_name: \"");
	for (int i = 0; i < sizeof(info.disk_name); i++) {
	  uint8_t ch = info.disk_name[i];
	  if (ch < 0x20 || ch > 0x7f) {
		LOG_FORMATS("[%02x]", ch);
	  } else {
		LOG_FORMATS("%c", ch);
	  }
	}
	if (info.disk_ext[0] || info.disk_ext[1] || info.disk_ext[2]) {
	  LOG_FORMATS(".");
	  for (int i = 0; i < sizeof(info.disk_ext); i++) {
		uint8_t ch = info.disk_ext[i];
		if (ch < 0x20 || ch > 0x7f) {
		  LOG_FORMATS("[%02x]", ch);
		} else {
		  LOG_FORMATS("%c", ch);
		}
	  }
	}
	LOG_FORMATS("\"\n");

	LOG_FORMATS(" fc_start_trk %d, fc_start_sec %d\n", info.fc_start_trk, info.fc_start_sec);
	LOG_FORMATS(" fc_end_trk: %d, fc_end_sec: %d\n", info.fc_end_trk, info.fc_end_sec);
	LOG_FORMATS(" free: %02x %02x\n", info.free[0], info.free[0]);

	LOG_FORMATS(" month %d day %d year %d\n", info.month, info.day, info.year);
	LOG_FORMATS(" last_trk %d, last_sec %d\n", info.last_trk, info.last_sec);

	LOG_FORMATS(" unused2:");
	for (int i = 0; i < 16; i++) {
	  LOG_FORMATS(" %02x", info.unused2[i]);
	}
	LOG_FORMATS("\n");

#if 0
	// Check that the first 'unused' area is zero filled.
	// Unfortunately an occasional dsk image has non-zero values here.
	for (int i = 0; i < sizeof(info.unused1); i++)
		if (info.unused1[i] != 0) return -1;
#endif

	// Consistency checks.
	if (info.fc_start_trk > info.last_trk || info.fc_end_trk > info.last_trk)
		return -1;
	if (info.fc_start_sec > info.last_sec || info.fc_end_sec > info.last_sec)
		return -1;
	if (info.month < 1 || info.month > 12 || info.day < 1 || info.day > 31)
		return -1;

	// FLEX sector numbers start at one generally, however the 6800 ROM
	// boot loaders load the boot code from track zero, side zero,
	// starting at sector zero. The boot code attempts to read multiple
	// sectors and a gap in the sector numbering appears to be used to
	// terminate the sequence. So if only one sector is to be loaded then
	// the sector numbering is 0, 2, 3, .... If two sectors are to be
	// loaded then the sector numbering is 0, 1, 3, 4 ... The boot loaders
	// for 6809 FLEX systems appear to load from sector one so do not have
	// this inconsistency to handle.
	boot0_sector_id = 1;
	boot1_sector_id = 2;

	// This floppy format uses a strategy of looking for 6800 boot code to
	// set the numbering of the first two sectors. If this is shown to not
	// be practical in some common cases then a separate format variant
	// might be needed.
	if (boot0[0] == 0x8e && boot0[3] == 0x20)
	{
		// Found a 6800 stack load and branch, looks like a 6800 boot sector.
		boot0_sector_id = 0;

		// Look for a link to the next sector, normal usage.
		if (boot1[0] != 0 || boot1[1] != 3)
		{
			// If not then assume it is a boot sector.
			boot1_sector_id = 1;
		}
	}
	LOG_FORMATS(" boot sector ids: %d %d\n",  boot0_sector_id, boot1_sector_id);

	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if(size != (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			continue;

		// Check consistency with the sysinfo record sector.
		if (f.track_count != info.last_trk + 1)
			continue;

		if (f.sector_count * f.head_count != info.last_sec)
			continue;

		return i;
	}
	return -1;
}

// FLEX numbers sectors on the second side of a track continuing from the
// first side which is a variation not handled by the generic code.
//
// FLEX generally numbers sectors starting at 1, however the 6800 boot sectors
// are numbers starting at zero.
//
void flex_format::build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const
{
	if(f.sector_base_id == -1) {
		for(int i=0; i<f.sector_count; i++) {
			int cur_offset = 0;
			for(int j=0; j<f.sector_count; j++)
				if(f.per_sector_id[j] < f.per_sector_id[i])
					cur_offset += f.sector_base_size ? f.sector_base_size : f.per_sector_size[j];
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			uint8_t sector_id = f.per_sector_id[i];
			if (track == 0 && head == 0 && sector_id <= 2) {
				if (sector_id == 0)
					sector_id = boot0_sector_id;
				else
					sector_id = boot1_sector_id;
			}
			sectors[i].sector_id = f.sector_count * head + sector_id;
		}
	} else {
		int cur_offset = 0;
		for(int i=0; i<f.sector_count; i++) {
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			cur_offset += sectors[i].size;
			uint8_t sector_id = i + f.sector_base_id;
			if (track == 0 && head == 0 && i < 2) {
				if (i == 0)
					sector_id = boot0_sector_id;
				else
					sector_id = boot1_sector_id;
			}
			sectors[i].sector_id = f.sector_count * head + sector_id;
		}
	}
}

// For FLEX just use track 1 rather than the generic code that looks a track
// 0. This is enough to avoid the odd sector numbering for the boot sectors,
// while following the generic code.
void flex_format::check_compatibility(floppy_image *image, std::vector<int> &candidates)
{
	uint8_t bitstream[500000/8];
	uint8_t sectdata[50000];
	desc_xs sectors[256];
	int track_size;

	// Extract the sectors
	generate_bitstream_from_track(1, 0, formats[candidates[0]].cell_size, bitstream, track_size, image);

	switch (formats[candidates[0]].encoding)
	{
	case floppy_image::FM:
		extract_sectors_from_bitstream_fm_pc(bitstream, track_size, sectors, sectdata, sizeof(sectdata));
		break;
	case floppy_image::MFM:
		extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sectdata, sizeof(sectdata));
		break;
	}

	// Check compatibility with every candidate, copy in-place
	int *ok_cands = &candidates[0];
	for(unsigned int i=0; i != candidates.size(); i++) {
		const format &f = formats[candidates[i]];
		int ns = 0;
		for(int j=0; j<256; j++)
			if(sectors[j].data) {
				int sid;
				if(f.sector_base_id == -1) {
					for(sid=0; sid < f.sector_count; sid++)
						if(f.per_sector_id[sid] == j)
							break;
				} else
					sid = j - f.sector_base_id;
				if(sid < 0 || sid > f.sector_count)
					goto fail;
				if(f.sector_base_size) {
					if(sectors[j].size != f.sector_base_size)
						goto fail;
				} else {
					if(sectors[j].size != f.per_sector_size[sid])
						goto fail;
				}
				ns++;
			}
		if(ns == f.sector_count)
			*ok_cands++ = candidates[i];
	fail:
		;
	}
	candidates.resize(ok_cands - &candidates[0]);
}

const flex_format::format flex_format::formats[] = {
	{ // 87.5K 5 1/4 inch single density - gaps unverified
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, 1, {}, 40, 16, 11
	},
	{ // 100K 5 1/4 inch single density - gaps unverified
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 1, {}, 40, 16, 11
	},
	{ // 200K 5 1/4 inch single density - gaps unverified
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, 1, {}, 40, 16, 11
	},
	{ // 175K 5 1/4 inch single density - gaps unverified
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 35, 2, 256, {}, 1, {}, 40, 16, 11
	},
	{ // 200K 5 1/4 inch single density - gaps unverified
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, 1, {}, 40, 16, 11
	},
	{ // 400K 5 1/4 inch single density - gaps unverified
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, 1, {}, 40, 16, 11
	},
	{ // 320K 5 1/4 inch double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, 1, {}, 80, 22, 24
	},
	{ // 320K 5 1/4 inch double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, 1, {}, 80, 22, 24
	},
	{ // 360K 5 1/4 inch quad density - gaps unverified
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, 1, {}, 80, 22, 24
	},
	{ // 720K 5 1/4 inch quad density - gaps unverified
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 18, 80, 2, 256, {}, 1, {}, 80, 22, 24
	},
	{ // 288.75K 8 inch single density - gaps unverified
		floppy_image::FF_8, floppy_image::SSSD, floppy_image::FM,
		2000, 15, 77, 1, 256, {}, 1, {}, 40, 12, 12
	},
	{ // 577.5K 8 inch single density - gaps unverified
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2000, 15, 77, 2, 256, {}, 1, {}, 40, 12, 12
	},
	{ // 500.5K 8 inch double density - gaps unverified
		floppy_image::FF_8, floppy_image::SSDD, floppy_image::MFM,
		1000, 26, 77, 1, 256, {}, 1, {}, 80, 22, 24
	},
	{ // 1001K 8 inch double density - gaps unverified
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		1000, 26, 77, 2, 256, {}, 1, {}, 80, 22, 24
	},
	{   /* 1440K 3 1/2 inch high density */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 36, 80, 2, 256, {}, 1, {}, 80, 22, 24
	},
	{}
};

const floppy_format_type FLOPPY_FLEX_FORMAT = &floppy_image_format_creator<flex_format>;
