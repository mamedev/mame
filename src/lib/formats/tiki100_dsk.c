// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/tiki100_dsk.c

    TIKI 100 disk image format

*********************************************************************/

#include "emu.h"
#include "formats/tiki100_dsk.h"

tiki100_format::tiki100_format() : wd177x_format(formats)
{
}

const char *tiki100_format::name() const
{
	return "tiki100";
}

const char *tiki100_format::description() const
{
	return "TIKI 100 disk image";
}

const char *tiki100_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
// double sided disks have t0s0,t0s1,t1s0,t1s1... format
const tiki100_format::format tiki100_format::formats[] = {
	{   //  90K 5 1/4 inch single density single sided
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 18, 40, 1, 128, {}, 1, {}, 16, 11, 8
	},
	{   //  200K 5 1/4 inch double density single sided
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{   //  400K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{   //  800K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{}
};

floppy_image_format_t::desc_e* tiki100_format::get_desc_fm(const format &f, int &current_size, int &end_gap_index)
{
	static floppy_image_format_t::desc_e desc[23] = {
		/* 00 */ { FM, 0xff, f.gap_1 },
		/* 01 */ { SECTOR_LOOP_START, 0, f.sector_count-1 },
		/* 02 */ {   FM, 0x00, 4 }, // NOTE here is the difference to wd177x_format
		/* 03 */ {   CRC_CCITT_FM_START, 1 },
		/* 04 */ {     RAW, 0xf57e, 1 },
		/* 05 */ {     TRACK_ID_FM },
		/* 06 */ {     HEAD_ID_FM },
		/* 07 */ {     SECTOR_ID_FM },
		/* 08 */ {     SIZE_ID_FM },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   CRC, 1 },
		/* 11 */ {   FM, 0xff, f.gap_2 },
		/* 12 */ {   FM, 0x00, 6 },
		/* 13 */ {   CRC_CCITT_FM_START, 2 },
		/* 14 */ {     RAW, 0xf56f, 1 },
		/* 15 */ {     SECTOR_DATA_FM, -1 },
		/* 16 */ {   CRC_END, 2 },
		/* 17 */ {   CRC, 2 },
		/* 18 */ {   FM, 0xff, f.gap_3 },
		/* 19 */ { SECTOR_LOOP_END },
		/* 20 */ { FM, 0xff, 0 },
		/* 21 */ { RAWBITS, 0xffff, 0 },
		/* 22 */ { END }
	};

	current_size = f.gap_1*16;
	if(f.sector_base_size)
		current_size += f.sector_base_size * f.sector_count * 16;
	else {
		for(int j=0; j != f.sector_count; j++)
			current_size += f.per_sector_size[j] * 16;
	}
	current_size += (4+1+4+2+f.gap_2+6+1+2+f.gap_3) * f.sector_count * 16;

	end_gap_index = 20;

	return desc;
}

const floppy_format_type FLOPPY_TIKI100_FORMAT = &floppy_image_format_creator<tiki100_format>;
