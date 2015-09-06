// license:GPL-2.0+
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    BBC Micro

    Disk image formats

***************************************************************************/

#include "bbc_dsk.h"
#include "basicdsk.h"

LEGACY_FLOPPY_OPTIONS_START(bbc)
	LEGACY_FLOPPY_OPTION( ssd40, "bbc,img,ssd", "BBC 40t SSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([10])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( ssd80, "bbc,img,ssd", "BBC 80t SSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([10])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( dsd40, "dsd", "BBC 40t DSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([10])
		SECTOR_LENGTH([256])
		INTERLEAVE([0])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( dsd80, "dsd", "BBC 80t DSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([10])
		SECTOR_LENGTH([256])
		INTERLEAVE([0])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

/********************************************************************/

bbc_ssd_525_format::bbc_ssd_525_format() : wd177x_format(formats)
{
}

const char *bbc_ssd_525_format::name() const
{
	return "ssd";
}

const char *bbc_ssd_525_format::description() const
{
	return "BBC Micro 5.25\" disk image";
}

const char *bbc_ssd_525_format::extensions() const
{
	return "bbc,img,ssd";
}

int bbc_ssd_525_format::find_size(io_generic *io, UINT32 form_factor)
{
	char cat[8];
	io_generic_read(io, cat, 256, 8);
	UINT64 sectors = ((cat[6] & 3) << 8) + cat[7]; // sector count from catalogue
	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if((size <= (UINT64)compute_track_size(f) * f.track_count * f.head_count) && (sectors == f.track_count * f.sector_count))
			return i;
	}
	return -1;
}

const bbc_ssd_525_format::format bbc_ssd_525_format::formats[] =
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
	{}
};


bbc_dsd_525_format::bbc_dsd_525_format() : wd177x_format(formats)
{
}

const char *bbc_dsd_525_format::name() const
{
	return "dsd";
}

const char *bbc_dsd_525_format::description() const
{
	return "BBC Micro 5.25\" disk image";
}

const char *bbc_dsd_525_format::extensions() const
{
	return "dsd";
}

int bbc_dsd_525_format::find_size(io_generic *io, UINT32 form_factor)
{
	char cat[8];
	io_generic_read(io, cat, 256, 8);
	UINT64 sectors = ((cat[6] & 3) << 8) + cat[7]; // sector count from catalogue
	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if((size <= (UINT64)compute_track_size(f) * f.track_count * f.head_count) && (sectors == f.track_count * f.sector_count))
			return i;
	}
	return -1;
}

const bbc_dsd_525_format::format bbc_dsd_525_format::formats[] =
{
	{ // 200k 40 track double sided single density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{ // 400k 80 track double sided single density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9 }, 40, 10, 10
	},
	{}
};


bbc_adf_525_format::bbc_adf_525_format() : wd177x_format(formats)
{
}

const char *bbc_adf_525_format::name() const
{
	return "adf";
}

const char *bbc_adf_525_format::description() const
{
	return "BBC Micro 5.25\" ADFS disk image";
}

const char *bbc_adf_525_format::extensions() const
{
	return "adf,ads,adm,adl,img";
}

const bbc_adf_525_format::format bbc_adf_525_format::formats[] =
{
	{ // 160K 40 track single sided double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 320K 80 track single sided double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 640K 80 track double sided double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 60, 22, 43
	},
	{}
};


bbc_adf_35_format::bbc_adf_35_format() : wd177x_format(formats)
{
}

const char *bbc_adf_35_format::name() const
{
	return "adf";
}

const char *bbc_adf_35_format::description() const
{
	return "BBC Micro 3.5\" ADFS disk image";
}

const char *bbc_adf_35_format::extensions() const
{
	return "adf,ads,adm,adl,img";
}

const bbc_adf_35_format::format bbc_adf_35_format::formats[] = {
	{ // 160K 3 1/2 inch 40 track single sided double density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 320K 3 1/2 inch 80 track single sided double density
		floppy_image::FF_35, floppy_image::SSQD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, 0, {}, 60, 22, 43
	},
	{ // 640K 3 1/2 inch 80 track double sided double density
		floppy_image::FF_35, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 60, 22, 43
	},
	{}
};


const floppy_format_type FLOPPY_BBC_SSD_525_FORMAT = &floppy_image_format_creator<bbc_ssd_525_format>;
const floppy_format_type FLOPPY_BBC_DSD_525_FORMAT = &floppy_image_format_creator<bbc_dsd_525_format>;
const floppy_format_type FLOPPY_BBC_ADF_525_FORMAT = &floppy_image_format_creator<bbc_adf_525_format>;
const floppy_format_type FLOPPY_BBC_ADF_35_FORMAT = &floppy_image_format_creator<bbc_adf_35_format>;
