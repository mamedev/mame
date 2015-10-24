// license:GPL-2.0+
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    BBC Micro

    Disk image formats

***************************************************************************/

#include "bbc_dsk.h"

bbc_dfs_format::bbc_dfs_format() : wd177x_format(formats)
{
}

const char *bbc_dfs_format::name() const
{
	return "dfs";
}

const char *bbc_dfs_format::description() const
{
	return "Acorn DFS disk image";
}

const char *bbc_dfs_format::extensions() const
{
	return "bbc,img,ssd,dsd";
}

int bbc_dfs_format::find_size(io_generic *io, UINT32 form_factor)
{
	UINT8 cat[8];
	UINT32 sectors0, sectors2;

	// read sector count from side 0 catalogue
	io_generic_read(io, cat, 0x100, 8);
	sectors0 = ((cat[6] & 3) << 8) + cat[7];

	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if ((size <= (UINT64)compute_track_size(f) * f.track_count * f.head_count) && (sectors0 == f.track_count * f.sector_count)) {
			if (f.head_count == 2)
			{
				// read sector count from side 2 catalogue
				if (f.sector_base_id == -1)
					io_generic_read(io, cat, 0xb00, 8); // interleaved
				else
					io_generic_read(io, cat, compute_track_size(f) * f.track_count + 0x100, 8); // sequential
				sectors2 = ((cat[6] & 3) << 8) + cat[7];
			}
			else
			{
				sectors2 = sectors0;
			}

			if (sectors0 == sectors2)
				return i;
		}
	}
	return -1;
}

int bbc_dfs_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 90;
	return 0;
}

int bbc_dfs_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const bbc_dfs_format::format bbc_dfs_format::formats[] =
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
	{ // 200k 40 track double sided single density (interleaved)
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{ // 400k 80 track double sided single density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, 0, {}, 40, 10, 10
	},
	{ // 400k 80 track double sided single density (interleaved)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{}
};


bbc_adfs_format::bbc_adfs_format() : wd177x_format(formats)
{
}

const char *bbc_adfs_format::name() const
{
	return "adfs";
}

const char *bbc_adfs_format::description() const
{
	return "Acorn ADFS disk image";
}

const char *bbc_adfs_format::extensions() const
{
	return "adf,ads,adm,adl";
}

int bbc_adfs_format::find_size(io_generic *io, UINT32 form_factor)
{
	UINT8 map[3];
	UINT32 sectors;

	// read sector count from free space map
	io_generic_read(io, map, 0xfc, 3);
	sectors = map[0] + (map[1] << 8) + (map[2] << 16);

	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		// valid images will have sector counts adfs-s = 0x280; adfs-m = 0x500; adfs-l = 0xa00; though many adfs-s images are incorrect
		if ((size == (UINT64)compute_track_size(f) * f.track_count * f.head_count) && (sectors == 0x280 || sectors == 0x500 || sectors == 0xa00)) {
			return i;
		}
	}
	return -1;
}

int bbc_adfs_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 100;
	return 0;
}

int bbc_adfs_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const bbc_adfs_format::format bbc_adfs_format::formats[] =
{
	{ // 160K 5 1/4 inch 40 track single sided double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 320K 5 1/4 inch 80 track single sided double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 640K 5 1/4 inch 80 track double sided double density (interleaved)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 60, 22, 43
	},
	{ // 160K 3 1/2 inch 40 track single sided double density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 320K 3 1/2 inch 80 track single sided double density
		floppy_image::FF_35, floppy_image::SSQD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 640K 3 1/2 inch 80 track double sided double density (interleaved)
		floppy_image::FF_35, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 60, 22, 43
	},
	{}
};


bbc_dos_format::bbc_dos_format() : wd177x_format(formats)
{
}

const char *bbc_dos_format::name() const
{
	return "dos";
}

const char *bbc_dos_format::description() const
{
	return "Acorn DOS disk image";
}

const char *bbc_dos_format::extensions() const
{
	return "img,adl";
}

int bbc_dos_format::find_size(io_generic *io, UINT32 form_factor)
{
	UINT8 cat[3];
	UINT32 sectors;

	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if (size == (UINT64)compute_track_size(f) * f.track_count * f.head_count) {
			switch (size)
			{
				case 640 * 1024: // 640K Acorn (Bootable) DOS Format
					// read sector count from free space map - Acorn DOS = 0xaa0
					io_generic_read(io, cat, 0xfc, 3);
					sectors = cat[0] + (cat[1] << 8) + (cat[2] << 16);
					if (sectors == 0xaa0) {
						// read media type ID from FAT - Acorn DOS = 0xff
						if (f.sector_base_id == -1)
							io_generic_read(io, cat, 0x2000, 1); // interleaved
						else
							io_generic_read(io, cat, 0x1000, 1); // sequential
						if (cat[0] == 0xff) return i;
					}
					break;
				case 800 * 1024: // 800K Acorn DOS Format
					// read media type ID from FAT - Acorn DOS = 0xfd
					io_generic_read(io, cat, 0, 1);
					if (cat[0] == 0xfd) return i;
					break;
			}
		}
	}
	return -1;
}

int bbc_dos_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 100;
	return 0;
}

int bbc_dos_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const bbc_dos_format::format bbc_dos_format::formats[] =
{
	{ // 640K 5 1/4 inch 80 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 640K 5 1/4 inch 80 track double sided double density (interleaved) - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 60, 22, 43
	},
	{ // 800K 5 1/4 inch 80 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 0, {}, 60, 22, 43
	},
	{}
};


bbc_cpm_format::bbc_cpm_format() : wd177x_format(formats)
{
}

const char *bbc_cpm_format::name() const
{
	return "cpm";
}

const char *bbc_cpm_format::description() const
{
	return "Acorn CP/M disk image";
}

const char *bbc_cpm_format::extensions() const
{
	return "img,ssd,dsd";
}

int bbc_cpm_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[8];

	io_generic_read(io, h, 0, 8);

	int type = find_size(io, form_factor);

	if(type != -1 && (memcmp(h, "Acorn CP", 8) == 0 || memcmp(h, "Slogger ", 8) == 0)) {
		return 100;
	}
	return 0;
}

int bbc_cpm_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const bbc_cpm_format::format bbc_cpm_format::formats[] =
{
	{ // 400K 5 1/4 inch 80 track double sided single density - gaps unverified
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 5, 80, 2, 512, {}, 0, {}, 40, 10, 10
	},
	{ // 400k 5 1/4 inch 80 track double sided single density (interleaved) - gaps unverified
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 5, 80, 2, 512, {}, -1, { 0,1,2,3,4 }, 40, 10, 10
	},
	{ // 800K 5 1/4 inch 80 track double sided double density - gaps unverified
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 0, {}, 60, 22, 43
	},
	{}
};

const floppy_format_type FLOPPY_BBC_DFS_FORMAT = &floppy_image_format_creator<bbc_dfs_format>;
const floppy_format_type FLOPPY_BBC_ADFS_FORMAT = &floppy_image_format_creator<bbc_adfs_format>;
const floppy_format_type FLOPPY_BBC_DOS_FORMAT = &floppy_image_format_creator<bbc_dos_format>;
const floppy_format_type FLOPPY_BBC_CPM_FORMAT = &floppy_image_format_creator<bbc_cpm_format>;
