// license:GPL-2.0+
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    Acorn - BBC Micro, Electron, Archimedes

    Disk image formats

***************************************************************************/

#include "acorn_dsk.h"

acorn_ssd_format::acorn_ssd_format() : wd177x_format(formats)
{
}

const char *acorn_ssd_format::name() const
{
	return "ssd";
}

const char *acorn_ssd_format::description() const
{
	return "Acorn SSD disk image";
}

const char *acorn_ssd_format::extensions() const
{
	return "ssd,bbc,img";
}

int acorn_ssd_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint8_t cat[8];
	uint32_t sectors0, sectors2;
	uint64_t size = io_generic_size(io);

	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// test for Torch CPN - test pattern at sector &0018
		io_generic_read(io, cat, 0x32800, 8);
		if (memcmp(cat, "\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd", 4) == 0 && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// read sector count from side 0 catalogue
		io_generic_read(io, cat, 0x100, 8);
		sectors0 = ((cat[6] & 3) << 8) + cat[7];
		LOG_FORMATS("ssd: sector count 0: %d %s\n", sectors0, sectors0 % 10 != 0 ? "invalid" : "");

		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 <= f.track_count * f.sector_count)) {
			if (f.head_count == 2)
			{
				// read sector count from side 2 catalogue
				io_generic_read(io, cat, compute_track_size(f) * f.track_count + 0x100, 8); // sequential
				sectors2 = ((cat[6] & 3) << 8) + cat[7];

				// exception case for Acorn CP/M System Disc 1
				io_generic_read(io, cat, 0x367ec, 8);
				if (memcmp(cat, "/M  ", 4) == 0) sectors2 = ((cat[6] & 3) << 8) + cat[7];

				LOG_FORMATS("ssd: sector count 2: %d %s\n", sectors2, sectors2 % 10 != 0 ? "invalid" : "");
			}
			else
			{
				sectors2 = sectors0;
			}

			if (sectors0 > 0 && sectors0 % 10 == 0 && sectors2 > 0 && sectors2 % 10 == 0 && size <= (sectors0 + sectors2) * 256)
				return i;
		}
	}
	LOG_FORMATS("ssd: no match\n");
	return -1;
}

int acorn_ssd_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 90;
	return 0;
}

int acorn_ssd_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
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
		4000, 10, 40, 1, 256,{}, 0,{}, 40, 10, 10
	},
	{ // 200k 80 track single sided single density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::FM,
		4000, 10, 80, 1, 256,{}, 0,{}, 40, 10, 10
	},
	{ // 200k 40 track double sided single density
		floppy_image::FF_35, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256,{}, 0,{}, 40, 10, 10
	},
	{ // 400k 80 track double sided single density
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::FM,
		4000, 10, 80, 2, 256,{}, 0,{}, 40, 10, 10
	},
	{ // 100k 40 track single sided single density
		floppy_image::FF_3, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256,{}, 0,{}, 40, 10, 10
	},
	{}
};


acorn_dsd_format::acorn_dsd_format() : wd177x_format(formats)
{
}

const char *acorn_dsd_format::name() const
{
	return "dsd";
}

const char *acorn_dsd_format::description() const
{
	return "Acorn DSD disk image";
}

const char *acorn_dsd_format::extensions() const
{
	return "dsd";
}

int acorn_dsd_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint8_t cat[8];
	uint32_t sectors0, sectors2;
	uint64_t size = io_generic_size(io);

	for (int i = 0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// test for Torch CPN - test pattern at sector &0018
		io_generic_read(io, cat, 0x1200, 8);
		if (memcmp(cat, "\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd", 4) == 0 && size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count)
			return i;

		// read sector count from side 0 catalogue
		io_generic_read(io, cat, 0x100, 8);
		sectors0 = ((cat[6] & 3) << 8) + cat[7];
		LOG_FORMATS("dsd: sector count 0: %d %s\n", sectors0, sectors0 % 10 != 0 ? "invalid" : "");

		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 <= f.track_count * f.sector_count)) {
			// read sector count from side 2 catalogue
			io_generic_read(io, cat, 0xb00, 8); // interleaved
			sectors2 = ((cat[6] & 3) << 8) + cat[7];

			// exception case for Acorn CP/M System Disc 1
			io_generic_read(io, cat, 0x97ec, 8);
			if (memcmp(cat, "/M  ", 4) == 0) sectors2 = ((cat[6] & 3) << 8) + cat[7];

			LOG_FORMATS("dsd: sector count 2: %d %s\n", sectors2, sectors2 % 10 != 0 ? "invalid" : "");

			if (sectors0 > 0 && sectors0 % 10 == 0 && sectors2 > 0 && sectors2 % 10 == 0 && size <= (sectors0 + sectors2) * 256)
				return i;
		}
	}
	LOG_FORMATS("dsd: no match\n");
	return -1;
}

int acorn_dsd_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 90;
	return 0;
}

int acorn_dsd_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
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
		4000, 10, 80, 2, 256,{}, -1,{ 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{ // 200k 40 track double sided single density (interleaved)
		floppy_image::FF_35, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256,{}, -1,{ 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{}
};


opus_ddos_format::opus_ddos_format() : wd177x_format(formats)
{
}

const char *opus_ddos_format::name() const
{
	return "ddos";
}

const char *opus_ddos_format::description() const
{
	return "Opus DDOS disk image";
}

const char *opus_ddos_format::extensions() const
{
	return "dds";
}

int opus_ddos_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint8_t cat[8];
	uint32_t sectors0, sectors2;

	// read sector count from side 0 catalogue
	io_generic_read(io, cat, 0x1000, 8);
	sectors0 = (cat[1] << 8) + cat[2];
	LOG_FORMATS("ddos: sector count 0: %d %s\n", sectors0, sectors0 % 18 != 0 ? "invalid" : "");

	uint64_t size = io_generic_size(io);
	for (int i = 0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 <= f.track_count * f.sector_count)) {
			if (f.head_count == 2)
			{
				// read sector count from side 2 catalogue
				io_generic_read(io, cat, compute_track_size(f) * f.track_count + 0x1000, 8); // sequential
				sectors2 = (cat[1] << 8) + cat[2];
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
	LOG_FORMATS("ddos: no match\n");
	return -1;
}

int opus_ddos_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 90;
	return 0;
}

int opus_ddos_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const opus_ddos_format::format opus_ddos_format::formats[] =
{
	{ // 180k 40 track single sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::MFM,
		4000, 18, 40, 1, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 360k 80 track single sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		4000, 18, 80, 1, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 360k 40 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::MFM,
		4000, 18, 40, 2, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 720k 80 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		4000, 18, 80, 2, 256, {}, 0, {}, 36, 22, 27
	},
	{}
};


acorn_adfs_old_format::acorn_adfs_old_format() : wd177x_format(formats)
{
}

const char *acorn_adfs_old_format::name() const
{
	return "adfs_o";
}

const char *acorn_adfs_old_format::description() const
{
	return "Acorn ADFS (OldMap) disk image";
}

const char *acorn_adfs_old_format::extensions() const
{
	return "adf,ads,adm,adl";
}

int acorn_adfs_old_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint8_t map[3];
	uint32_t sectors;
	uint8_t oldmap[4];

	// read sector count from free space map
	io_generic_read(io, map, 0xfc, 3);
	sectors = map[0] + (map[1] << 8) + (map[2] << 16);
	LOG_FORMATS("adfs_o: sector count %d %s\n", sectors, sectors % 16 != 0 ? "invalid" : "");

	// read map identifier
	io_generic_read(io, oldmap, 0x201, 4);
	LOG_FORMATS("adfs_o: map identifier %s %s\n", oldmap, memcmp(oldmap, "Hugo", 4) != 0 ? "invalid" : "");

	uint64_t size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// valid images will have map identifier 'Hugo' and sector counts adfs-s = 0x280; adfs-m = 0x500; adfs-l = 0xa00; adfs-dos = 0xaa0; though many adfs-s images are incorrect
		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && memcmp(oldmap, "Hugo", 4) == 0 && (sectors == 0x280 || sectors == 0x500 || sectors == 0xa00 || sectors == 0xaa0 || size == 819200)) {
			return i;
		}
	}
	LOG_FORMATS("adfs_o: no match\n");
	return -1;
}

int acorn_adfs_old_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 100;
	return 0;
}

int acorn_adfs_old_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
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

const char *acorn_adfs_new_format::name() const
{
	return "adfs_n";
}

const char *acorn_adfs_new_format::description() const
{
	return "Acorn ADFS (NewMap) disk image";
}

const char *acorn_adfs_new_format::extensions() const
{
	return "adf";
}

int acorn_adfs_new_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint8_t dform[4];
	uint8_t eform[4];

	// read map identifiers for D and E formats
	io_generic_read(io, dform, 0x401, 4);
	LOG_FORMATS("adfs_n: map identifier (D format) %s %s\n", dform, (memcmp(dform, "Hugo", 4) != 0 && memcmp(dform, "Nick", 4) != 0) ? "invalid" : "");
	io_generic_read(io, eform, 0x801, 4);
	LOG_FORMATS("adfs_n: map identifier (E format) %s %s\n", eform, memcmp(eform, "Nick", 4) != 0 ? "invalid" : "");

	uint64_t size = io_generic_size(io);
	for (int i = 0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// no further checks for 1600K images
		if ((size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && size == 0x190000) {
			return i;
		}

		// valid 800K images will have map identifier Nick, Arthur D format still use Hugo
		if ((size <= (uint64_t)compute_track_size(f) * f.track_count * f.head_count) && (memcmp(dform, "Hugo", 4) == 0 || memcmp(dform, "Nick", 4) == 0 || memcmp(eform, "Nick", 4) == 0)) {
			return i;
		}
	}
	LOG_FORMATS("adfs_n: no match\n");
	return -1;
}

int acorn_adfs_new_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 100;
	return 0;
}

int acorn_adfs_new_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const acorn_adfs_new_format::format acorn_adfs_new_format::formats[] =
{
	{ // D,E - 800K 3 1/2 inch 80 track double sided double density - gaps unverified
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, -1, { 0,1,2,3,4 }, 32, 22, 90
	},
	{ // F - 1600K 3 1/2 inch 80 track double sided quad density
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		2000, 10, 80, 2, 1024, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 50, 22, 90
	},
	{}
};


acorn_dos_format::acorn_dos_format() : wd177x_format(formats)
{
}

const char *acorn_dos_format::name() const
{
	return "dos";
}

const char *acorn_dos_format::description() const
{
	return "Acorn DOS disk image";
}

const char *acorn_dos_format::extensions() const
{
	return "img";
}

int acorn_dos_format::find_size(io_generic *io, uint32_t form_factor)
{
	uint8_t type;

	uint64_t size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if (size == (uint64_t)compute_track_size(f) * f.track_count * f.head_count) {
			// read media type ID from FAT - Acorn DOS = 0xfd
			io_generic_read(io, &type, 0, 1);
			LOG_FORMATS("dos: 800k media type id %02X %s\n", type, type != 0xfd ? "invalid" : "");
			if (type == 0xfd) return i;
		}
	}
	LOG_FORMATS("dos: no match\n");
	return -1;
}

int acorn_dos_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 90;
	return 0;
}

int acorn_dos_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
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

const char *opus_ddcpm_format::name() const
{
	return "ddcpm";
}

const char *opus_ddcpm_format::description() const
{
	return "Opus DD CP/M disk image";
}

const char *opus_ddcpm_format::extensions() const
{
	return "ssd";
}

bool opus_ddcpm_format::supports_save() const
{
	return false;
}

int opus_ddcpm_format::identify(io_generic *io, uint32_t form_factor)
{
	uint8_t h[8];

	io_generic_read(io, h, 0, 8);

	if (io_generic_size(io) == 819200 && memcmp(h, "Slogger ", 8) == 0)
		return 100;
	LOG_FORMATS("ddcpm: no match\n");
	return 0;
}

bool opus_ddcpm_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
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
	int spt, bps;

	for (int head = 0; head < 2; head++) {
		for (int track = 0; track < 80; track++) {
			bool mfm = track > 2 || head;
			bps = mfm ? 512 : 256;
			spt = 10;
			desc_pc_sector sects[10];
			uint8_t sectdata[10*512];

			io_generic_read(io, sectdata, head * 80 * spt * 512 + track * spt * 512, spt * 512);

			for (int i = 0; i < spt; i++) {
				sects[i].track = track;
				sects[i].head = head;
				sects[i].sector = i;
				sects[i].size =  mfm ? 2 : 1;
				sects[i].actual_size = bps;
				sects[i].data = sectdata + i * 512;
				sects[i].deleted = false;
				sects[i].bad_crc = false;
			}

			if (mfm)
				build_wd_track_mfm(track, head, image, 100000, 10, sects, 60, 43, 22);
			else
				build_wd_track_fm(track, head, image, 50000, 10, sects, 40, 10, 10);
		}
	}

	return true;
}

bool opus_ddcpm_format::save(io_generic *io, floppy_image *image)
{
	return false;
}


const floppy_format_type FLOPPY_ACORN_SSD_FORMAT = &floppy_image_format_creator<acorn_ssd_format>;
const floppy_format_type FLOPPY_ACORN_DSD_FORMAT = &floppy_image_format_creator<acorn_dsd_format>;
const floppy_format_type FLOPPY_ACORN_DOS_FORMAT = &floppy_image_format_creator<acorn_dos_format>;
const floppy_format_type FLOPPY_ACORN_ADFS_OLD_FORMAT = &floppy_image_format_creator<acorn_adfs_old_format>;
const floppy_format_type FLOPPY_ACORN_ADFS_NEW_FORMAT = &floppy_image_format_creator<acorn_adfs_new_format>;
const floppy_format_type FLOPPY_OPUS_DDOS_FORMAT = &floppy_image_format_creator<opus_ddos_format>;
const floppy_format_type FLOPPY_OPUS_DDCPM_FORMAT = &floppy_image_format_creator<opus_ddcpm_format>;
