// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/pc_dsk.c

    PC disk images

*********************************************************************/

#include <string.h>
#include <assert.h>

#include "formats/pc_dsk.h"
#include "formats/basicdsk.h"

struct pc_disk_sizes
{
	uint32_t image_size;
	int sectors;
	int heads;
};



static const struct pc_disk_sizes disk_sizes[] =
{
	{ 8*1*40*512,  8, 1},   /* 5 1/4 inch double density single sided */
	{ 8*2*40*512,  8, 2},   /* 5 1/4 inch double density */
	{ 9*1*40*512,  9, 1},   /* 5 1/4 inch double density single sided */
	{ 9*2*40*512,  9, 2},   /* 5 1/4 inch double density */
	{10*2*40*512, 10, 2},   /* 5 1/4 inch double density single sided */
	{ 9*2*80*512,  9, 2},   /* 80 tracks 5 1/4 inch drives rare in PCs */
	{ 9*2*80*512,  9, 2},   /* 3 1/2 inch double density */
	{15*2*80*512, 15, 2},   /* 5 1/4 inch high density (or japanese 3 1/2 inch high density) */
	{18*2*80*512, 18, 2},   /* 3 1/2 inch high density */
	{21*2*80*512, 21, 2},   /* 3 1/2 inch high density DMF */
	{36*2*80*512, 36, 2}    /* 3 1/2 inch enhanced density */
};



static floperr_t pc_dsk_compute_geometry(floppy_image_legacy *floppy, struct basicdsk_geometry *geometry)
{
	int i;
	uint64_t size;

	memset(geometry, 0, sizeof(*geometry));
	size = floppy_image_size(floppy);

	for (i = 0; i < ARRAY_LENGTH(disk_sizes); i++)
	{
		if (disk_sizes[i].image_size == size)
		{
			geometry->sectors = disk_sizes[i].sectors;
			geometry->heads = disk_sizes[i].heads;
			geometry->sector_length = 512;
			geometry->first_sector_id = 1;
			geometry->tracks = (int) (size / disk_sizes[i].sectors / disk_sizes[i].heads / geometry->sector_length);
			return FLOPPY_ERROR_SUCCESS;
		}
	}

	if (size >= 0x1a)
	{
		/*
		 * get info from boot sector.
		 * not correct on all disks
		 */
		uint8_t scl, spt, heads;
		floppy_image_read(floppy, &scl, 0x0c, 1);
		floppy_image_read(floppy, &spt, 0x18, 1);
		floppy_image_read(floppy, &heads, 0x1A, 1);

		if (size == ((uint64_t) scl) * spt * heads * 0x200)
		{
			geometry->sectors = spt;
			geometry->heads = heads;
			geometry->sector_length = 512;
			geometry->first_sector_id = 1;
			geometry->tracks = scl;
			return FLOPPY_ERROR_SUCCESS;
		}
	}

	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_IDENTIFY(pc_dsk_identify)
{
	floperr_t err;
	struct basicdsk_geometry geometry;

	err = pc_dsk_compute_geometry(floppy, &geometry);
	if (err)
		return err;

	*vote = geometry.heads ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(pc_dsk_construct)
{
	floperr_t err;
	struct basicdsk_geometry geometry;

	if (params)
	{
		/* create */
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads = params->lookup_int(PARAM_HEADS);
		geometry.tracks = params->lookup_int(PARAM_TRACKS);
		geometry.sectors = params->lookup_int(PARAM_SECTORS);
		geometry.first_sector_id = 1;
		geometry.sector_length = 512;
	}
	else
	{
		/* open */
		err = pc_dsk_compute_geometry(floppy, &geometry);
		if (err)
			return err;
	}

	return basicdsk_construct(floppy, &geometry);
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( pc )
	LEGACY_FLOPPY_OPTION( pc_dsk, "dsk,ima,img,ufi,360",        "PC floppy disk image", pc_dsk_identify, pc_dsk_construct, nullptr,
		HEADS([1]-2)
		TRACKS(40/[80])
		SECTORS(8/[9]/10/15/18/36))
LEGACY_FLOPPY_OPTIONS_END

pc_format::pc_format() : upd765_format(formats)
{
}

const char *pc_format::name() const
{
	return "pc";
}

const char *pc_format::description() const
{
	return "PC floppy disk image";
}

const char *pc_format::extensions() const
{
	return "dsk,ima,img,ufi,360";
}

int pc_format::identify(io_generic *io, uint32_t form_factor)
{
	uint64_t size = io_generic_size(io);

	/* some 360K images have a 512-byte header */
	if (size == 368640 + 0x200) {
		file_header_skip_bytes = 0x200;
	}

	/* some 1.44MB images have a 1024-byte footer */
	if (size == 1474560 + 0x400) {
		file_footer_skip_bytes = 0x400;
	}

	return upd765_format::identify(io, form_factor);
}

const pc_format::format pc_format::formats[] = {
	{   /*  160K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  8, 40, 1, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  320K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  8, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  180K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  9, 40, 1, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  360K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  360K 5 1/4 inch double density, 41 tracks */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 41, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  360K 5 1/4 inch double density, 42 tracks */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 42, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  400K 5 1/4 inch double density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, 1, {}, 80, 50, 22, 36
	},
	{   /*  720K 5 1/4 inch quad density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /* 1200K 5 1/4 inch high density */
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, 15, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{   /*  720K 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /* 1200K 3 1/2 inch high density (japanese variant) - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 15, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{   /* 1440K 3 1/2 inch high density */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 18, 80, 2, 512, {}, 1, {}, 80, 50, 22, 108
	},
	{   /* Microsoft DMF 1680K 3 1/2 inch high density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 21, 80, 2, 512, {}, 1, {}, 80, 50, 22, 0xc
	},
	{   /* 2880K 3 1/2 inch extended density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSED, floppy_image::MFM,
			500, 36, 80, 2, 512, {}, 1, {}, 80, 50, 41, 80
	},
	{}
};

const floppy_format_type FLOPPY_PC_FORMAT = &floppy_image_format_creator<pc_format>;
