// license:BSD-3-Clause
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    Acorn - BBC Micro, Electron, Archimedes

    Disk image formats

***************************************************************************/

#include "acorn_dsk.h"
#include "imageutl.h"

#include "coretmpl.h"
#include "ioprocs.h"
#include "multibyte.h"

#include <cstring>
#include <tuple>


acorn_ssd_format::acorn_ssd_format() : wd177x_format(formats)
{
}

const char *acorn_ssd_format::name() const noexcept
{
	return "ssd";
}

const char *acorn_ssd_format::description() const noexcept
{
	return "Acorn SSD disk image";
}

const char *acorn_ssd_format::extensions() const noexcept
{
	return "ssd,bbc,img";
}

int acorn_ssd_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t cat[8];
	uint32_t sectors0, sectors2;
	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// test for Torch CPN - test pattern at sector &0018
		read_at(io, 0x32800, cat, 8); // FIXME: check for errors and premature EOF
		if (memcmp(cat, "\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd", 8) == 0 && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// test for HADFS - test pattern at sector 70
		read_at(io, 0x04610, cat, 8); // FIXME: check for errors and premature EOF
		if (memcmp(cat, "\x00\x28\x43\x29\x4a\x47\x48\x00", 8) == 0 && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// test for Kenda SD - offset &0962 = 0 SD/1 DD, offset &0963 = disk size blocks / 4 (block size = 1K, ie. 0x400 bytes), reserved tracks = 3, ie. 0x1e00 bytes, soft stagger = 2 sectors, ie. 0x200 bytes
		read_at(io, 0x0960, cat, 8); // FIXME: check for errors and premature EOF
		if (cat[2] == 0 && ((uint64_t)cat[3] * 4 * 0x400 + 0x2000) == size && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
		{
			// valid blocks for single sided
			if (f.head_count == 1 && (cat[3] == 0x17 || cat[3] == 0x30))
				return i;
			// valid blocks for double sided
			if (f.head_count == 2 && (cat[3] == 0x2f || cat[3] == 0x62))
				return i;
		}

		// read sector count from side 0 catalogue
		read_at(io, 0x100, cat, 8); // FIXME: check for errors and premature EOF
		sectors0 = get_u16be(&cat[6]) & 0x3ff;
		LOG_FORMATS("ssd: sector count 0: %d %s\n", sectors0, sectors0 % 10 != 0 ? "invalid" : "");

		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 <= f.track_count * f.sector_count))
		{
			if (f.head_count == 2)
			{
				// read sector count from side 2 catalogue
				read_at(io, (uint64_t)compute_track_size(f) * f.track_count + 0x100, cat, 8); // sequential
				sectors2 = get_u16be(&cat[6]) & 0x3ff;

				// exception case for Acorn CP/M System Disc 1
				read_at(io, 0x367ec, cat, 8); // FIXME: check for errors and premature EOF
				if (memcmp(cat, "/M  ", 4) == 0)
					sectors2 = get_u16be(&cat[6]) & 0x3ff;

				LOG_FORMATS("ssd: sector count 2: %d %s\n", sectors2, sectors2 % 10 != 0 ? "invalid" : "");
			}
			else
			{
				sectors2 = sectors0;
			}

			if (sectors0 > 0 && sectors2 > 0 && size <= (sectors0 + sectors2) * 256)
				return i;
		}
	}
	return -1;
}

int acorn_ssd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_SIZE | FIFID_HINT;

	return 0;
}

int acorn_ssd_format::get_image_offset(const format &f, int head, int track) const
{
	return (f.track_count * head + track) * compute_track_size(f);
}

const acorn_ssd_format::format acorn_ssd_format::formats[] =
{
	{ // 100k 40 track single sided single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 200k 80 track single sided single density
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 200k 40 track double sided single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 400k 80 track double sided single density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 100k 40 track single sided single density
		floppy_image::FF_35, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 200k 80 track single sided single density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 200k 40 track double sided single density
		floppy_image::FF_35, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 400k 80 track double sided single density
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 100k 40 track single sided single density
		floppy_image::FF_3, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 40, 10, 10
	},
	{}
};


acorn_dsd_format::acorn_dsd_format() : wd177x_format(formats)
{
}

const char *acorn_dsd_format::name() const noexcept
{
	return "dsd";
}

const char *acorn_dsd_format::description() const noexcept
{
	return "Acorn DSD disk image";
}

const char *acorn_dsd_format::extensions() const noexcept
{
	return "dsd";
}

int acorn_dsd_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t cat[8];
	uint32_t sectors0, sectors2;
	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// test for Torch CPN - test pattern at sector &0018
		read_at(io, 0x1200, cat, 8); // FIXME: check for errors and premature EOF
		if (memcmp(cat, "\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd", 8) == 0 && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// test for HADFS - test pattern at sector 70
		read_at(io, 0x08c10, cat, 8); // FIXME: check for errors and premature EOF
		if (memcmp(cat, "\x00\x28\x43\x29\x4a\x47\x48\x00", 8) == 0 && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// test for FLEX - from System Information Record
		read_at(io, 0x0226, cat, 2); // FIXME: check for errors and premature EOF
		if ((memcmp(cat, "\x4f\x14", 2) == 0 || memcmp(cat, "\x4f\x0a", 2) == 0) && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// read sector count from side 0 catalogue
		read_at(io, 0x100, cat, 8); // FIXME: check for errors and premature EOF
		sectors0 = get_u16be(&cat[6]) & 0x3ff;
		LOG_FORMATS("dsd: sector count 0: %d %s\n", sectors0, sectors0 % 10 != 0 ? "invalid" : "");

		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 <= f.track_count * f.sector_count))
		{
			// read sector count from side 2 catalogue
			read_at(io, 0xb00, cat, 8); // interleaved
			sectors2 = get_u16be(&cat[6]) & 0x3ff;

			// exception case for Acorn CP/M System Disc 1
			read_at(io, 0x97ec, cat, 8); // FIXME: check for errors and premature EOF
			if (memcmp(cat, "/M  ", 4) == 0)
				sectors2 = get_u16be(&cat[6]) & 0x3ff;

			LOG_FORMATS("dsd: sector count 2: %d %s\n", sectors2, sectors2 % 10 != 0 ? "invalid" : "");

			if (sectors0 > 0 && sectors2 > 0 && size <= (sectors0 + sectors2) * 256)
				return i;
		}
	}
	return -1;
}

int acorn_dsd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_HINT | FIFID_SIZE;

	return 0;
}

int acorn_dsd_format::get_image_offset(const format &f, int head, int track) const
{
	return (track * f.head_count + head) * compute_track_size(f);
}

const acorn_dsd_format::format acorn_dsd_format::formats[] =
{
	{ // 400k 80 track double sided single density (interleaved)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{ // 200k 40 track double sided single density (interleaved)
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{ // 400k 80 track double sided single density (interleaved)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{ // 200k 40 track double sided single density (interleaved)
		floppy_image::FF_35, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{}
};


opus_ddos_format::opus_ddos_format() : wd177x_format(formats)
{
}

const char *opus_ddos_format::name() const noexcept
{
	return "ddos";
}

const char *opus_ddos_format::description() const noexcept
{
	return "Opus DDOS disk image";
}

const char *opus_ddos_format::extensions() const noexcept
{
	return "dds";
}

int opus_ddos_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t cat[8];
	uint32_t sectors0, sectors2;

	// read sector count from side 0 catalogue
	read_at(io, 0x1000, cat, 8); // FIXME: check for errors and premature EOF
	sectors0 = get_u16be(&cat[1]);
	LOG_FORMATS("ddos: sector count 0: %d %s\n", sectors0, sectors0 % 18 != 0 ? "invalid" : "");

	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 <= f.track_count * f.sector_count))
		{
			if (f.head_count == 2)
			{
				// read sector count from side 2 catalogue
				read_at(io, (uint64_t)compute_track_size(f) * f.track_count + 0x1000, cat, 8); // sequential
				sectors2 = get_u16be(&cat[1]);
				LOG_FORMATS("ddos: sector count 2: %d %s\n", sectors2, sectors2 % 18 != 0 ? "invalid" : "");
			}
			else
			{
				sectors2 = sectors0;
			}

			if (sectors0 % 18 == 0 && sectors2 % 18 == 0)
				return i;
		}
	}
	return -1;
}

int opus_ddos_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_HINT | FIFID_SIZE;

	return 0;
}

int opus_ddos_format::get_image_offset(const format &f, int head, int track) const
{
	return (f.track_count * head + track) * compute_track_size(f);
}

const opus_ddos_format::format opus_ddos_format::formats[] =
{
	{ // 180k 40 track single sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 360k 80 track single sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 360k 40 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 720k 80 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 80, 2, 256, {}, 0, {}, 36, 22, 27
	},
	{}
};


acorn_adfs_old_format::acorn_adfs_old_format() : wd177x_format(formats)
{
}

const char *acorn_adfs_old_format::name() const noexcept
{
	return "adfs_o";
}

const char *acorn_adfs_old_format::description() const noexcept
{
	return "Acorn ADFS (OldMap) disk image";
}

const char *acorn_adfs_old_format::extensions() const noexcept
{
	return "adf,ads,adm,adl";
}

int acorn_adfs_old_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	std::error_condition err;
	size_t actual;
	uint8_t map[3];
	uint32_t sectors;
	uint8_t oldmap[4];

	// read sector count from free space map
	std::tie(err, actual) = read_at(io, 0xfc, map, 3);
	if (err || (3 != actual))
		return -1;
	sectors = get_u24le(map);
	LOG_FORMATS("adfs_o: sector count %d %s\n", sectors, sectors % 16 != 0 ? "invalid" : "");

	// read map identifier
	std::tie(err, actual) = read_at(io, 0x201, oldmap, 4);
	if (err || (4 != actual))
		return -1;
	LOG_FORMATS("adfs_o: map identifier %s %s\n", oldmap, memcmp(oldmap, "Hugo", 4) != 0 ? "invalid" : "");

	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// valid images will have map identifier 'Hugo' and sector counts adfs-s = 0x280; adfs-m = 0x500; adfs-l = 0xa00; adfs-dos = 0xaa0; though many adfs-s images are incorrect
		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && memcmp(oldmap, "Hugo", 4) == 0 && (sectors == 0x280 || sectors == 0x500 || sectors == 0xa00 || sectors == 0xaa0 || size == 819200)) {
			return i;
		}
	}
	return -1;
}

int acorn_adfs_old_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_HINT | FIFID_SIZE;

	return 0;
}

int acorn_adfs_old_format::get_image_offset(const format &f, int head, int track) const
{
	return (track * f.head_count + head) * compute_track_size(f);
}

const acorn_adfs_old_format::format acorn_adfs_old_format::formats[] =
{
	{ // M - 320K 5 1/4 inch 80 track single sided double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // S - 160K 5 1/4 inch 40 track single sided double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // L - 640K 5 1/4 inch 80 track double sided double density (interleaved)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 42, 22, 57
	},
	{ // M - 320K 3 1/2 inch 80 track single sided double density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // S - 160K 3 1/2 inch 40 track single sided double density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // L - 640K 3 1/2 inch 80 track double sided double density (interleaved)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 42, 22, 57
	},
	{}
};


acorn_adfs_new_format::acorn_adfs_new_format() : wd177x_format(formats)
{
}

const char *acorn_adfs_new_format::name() const noexcept
{
	return "adfs_n";
}

const char *acorn_adfs_new_format::description() const noexcept
{
	return "Acorn ADFS (NewMap) disk image";
}

const char *acorn_adfs_new_format::extensions() const noexcept
{
	return "adf";
}

int acorn_adfs_new_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	std::error_condition err;
	size_t actual;
	uint8_t dform[4];
	uint8_t eform[4];

	// read map identifiers for D and E formats
	std::tie(err, actual) = read_at(io, 0x401, dform, 4);
	if (err || (4 != actual))
		return -1;
	LOG_FORMATS("adfs_n: map identifier (D format) %s %s\n", dform, (memcmp(dform, "Hugo", 4) != 0 && memcmp(dform, "Nick", 4) != 0) ? "invalid" : "");
	std::tie(err, actual) = read_at(io, 0x801, eform, 4);
	if (err || (4 != actual))
		return -1;
	LOG_FORMATS("adfs_n: map identifier (E format) %s %s\n", eform, memcmp(eform, "Nick", 4) != 0 ? "invalid" : "");

	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// no further checks for 1600K images
		if ((size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && size == 0x190000)
			return i;

		// valid 800K images will have map identifier Nick, Arthur D format still use Hugo
		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (memcmp(dform, "Hugo", 4) == 0 || memcmp(dform, "Nick", 4) == 0 || memcmp(eform, "Nick", 4) == 0))
			return i;
	}
	return -1;
}

int acorn_adfs_new_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_HINT | FIFID_SIZE;

	return 0;
}

int acorn_adfs_new_format::get_image_offset(const format &f, int head, int track) const
{
	return (track * f.head_count + head) * compute_track_size(f);
}

const acorn_adfs_new_format::format acorn_adfs_new_format::formats[] =
{
	{ // D,E - 800K 3 1/2 inch 80 track double sided double density - gaps unverified
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, -1, { 0,1,2,3,4 }, 32, 22, 90
	},
	{ // F - 1600K 3 1/2 inch 80 track double sided high density
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 10, 80, 2, 1024, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 50, 22, 90
	},
	{}
};


acorn_dos_format::acorn_dos_format() : wd177x_format(formats)
{
}

const char *acorn_dos_format::name() const noexcept
{
	return "dos";
}

const char *acorn_dos_format::description() const noexcept
{
	return "Acorn DOS disk image";
}

const char *acorn_dos_format::extensions() const noexcept
{
	return "img";
}

int acorn_dos_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i=0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if (size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
		{
			// read media type ID from FAT - Acorn DOS = 0xfd
			uint8_t type;
			auto const [err, actual] = read_at(io, 0, &type, 1);
			if (!err && (1 == actual))
			{
				LOG_FORMATS("dos: 800k media type id %02X %s\n", type, type != 0xfd ? "invalid" : "");
				if (type == 0xfd)
					return i;
			}
		}
	}
	return -1;
}

int acorn_dos_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_HINT | FIFID_SIZE;

	return 0;
}

int acorn_dos_format::get_image_offset(const format &f, int head, int track) const
{
	return (track * f.head_count + head) * compute_track_size(f);
}

const acorn_dos_format::format acorn_dos_format::formats[] =
{
	{ // 800K 5 1/4 inch 80 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, -1, { 1,2,3,4,5 }, 32, 22, 90
	},
	{}
};


opus_ddcpm_format::opus_ddcpm_format()
{
}

const char *opus_ddcpm_format::name() const noexcept
{
	return "ddcpm";
}

const char *opus_ddcpm_format::description() const noexcept
{
	return "Opus DD CP/M disk image";
}

const char *opus_ddcpm_format::extensions() const noexcept
{
	return "ssd";
}

int opus_ddcpm_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[8];
	auto const [err, actual] = read_at(io, 0, h, 8);
	if (err || (8 != actual))
		return 0;

	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == 819200 && memcmp(h, "Slogger ", 8) == 0)
		return FIFID_SIGN | FIFID_SIZE;

	return 0;
}

bool opus_ddcpm_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	// Double density discs formatted with DDCPM :
	//
	// Tracks 0 - 2 formatted Single Density
	// Tracks 3 - 159 formatted Double Density
	//
	// Single density tracks are 10 x 256 byte sectors per track
	// Sector skew of 2
	//
	// Double Density tracks are 10 x 512 byte sectors per track
	// Sector skew of 1
	// Sector interleave of 2
	//
	for (int head = 0; head < 2; head++)
	{
		for (int track = 0; track < 80; track++)
		{
			bool const mfm = track > 2 || head;
			int const bps = mfm ? 512 : 256;
			int const spt = 10;
			desc_pc_sector sects[10];
			uint8_t sectdata[10*512];

			/*auto const [err, actual] =*/ read_at(io, head * 80 * spt * 512 + track * spt * 512, sectdata, spt * 512); // FIXME: check for errors and premature EOF

			for (int i = 0; i < spt; i++)
			{
				sects[i].track = track;
				sects[i].head = head;
				sects[i].sector = i;
				sects[i].size =  mfm ? 2 : 1;
				sects[i].actual_size = bps;
				sects[i].data = sectdata + i * 512;
				sects[i].deleted = false;
				sects[i].bad_data_crc = false;
				sects[i].bad_addr_crc = false;
			}

			if (mfm)
				build_wd_track_mfm(track, head, image, 100000, 10, sects, 60, 43, 22);
			else
				build_wd_track_fm(track, head, image, 50000, 10, sects, 40, 10, 10);
		}
	}

	return true;
}


cumana_dfs_format::cumana_dfs_format() : wd177x_format(formats)
{
}

const char *cumana_dfs_format::name() const noexcept
{
	return "cdfs";
}

const char *cumana_dfs_format::description() const noexcept
{
	return "Cumana DFS disk image";
}

const char *cumana_dfs_format::extensions() const noexcept
{
	return "img";
}

int cumana_dfs_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if (size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
		{
			uint8_t info;
			auto const [err, actual] = read_at(io, 14, &info, 1);
			if (err || (actual != 1))
				return -1;
			if (f.head_count == (util::BIT(info, 6) ? 2 : 1) && f.track_count == (util::BIT(info, 7) ? 80 : 40))
				return i;
		}
	}
	return -1;
}

int cumana_dfs_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_HINT | FIFID_SIZE;

	return 0;
}

int cumana_dfs_format::get_image_offset(const format &f, int head, int track) const
{
	return (track * f.head_count + head) * compute_track_size(f);
}

const cumana_dfs_format::format cumana_dfs_format::formats[] =
{
	{ // 180K 5 1/4 inch 40 track single sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 9, 40, 1, 512, {}, -1, { 0,3,6,1,4,7,2,5,8 }, 32, 22, 90
	},
	{ // 360K 5 1/4 inch 40 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 40, 2, 512, {}, -1, { 0,3,6,1,4,7,2,5,8 }, 32, 22, 90
	},
	{ // 360K 5 1/4 inch 80 track single sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 9, 80, 1, 512, {}, -1, { 0,3,6,1,4,7,2,5,8 }, 32, 22, 90
	},
	{ // 720K 5 1/4 inch 80 track single double double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, -1, { 0,3,6,1,4,7,2,5,8 }, 32, 22, 90
	},
	{}
};


const acorn_ssd_format FLOPPY_ACORN_SSD_FORMAT;
const acorn_dsd_format FLOPPY_ACORN_DSD_FORMAT;
const acorn_dos_format FLOPPY_ACORN_DOS_FORMAT;
const acorn_adfs_old_format FLOPPY_ACORN_ADFS_OLD_FORMAT;
const acorn_adfs_new_format FLOPPY_ACORN_ADFS_NEW_FORMAT;
const opus_ddos_format FLOPPY_OPUS_DDOS_FORMAT;
const opus_ddcpm_format FLOPPY_OPUS_DDCPM_FORMAT;
const cumana_dfs_format FLOPPY_CUMANA_DFS_FORMAT;
