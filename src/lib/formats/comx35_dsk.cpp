// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/comx35_dsk.cpp

    COMX-35 disk image format

*********************************************************************/

#include "formats/comx35_dsk.h"

#include "ioprocs.h"

comx35_format::comx35_format() : wd177x_format(formats)
{
}

const char *comx35_format::name() const noexcept
{
	return "comx35";
}

const char *comx35_format::description() const noexcept
{
	return "COMX-35 disk image";
}

const char *comx35_format::extensions() const noexcept
{
	return "img";
}

const comx35_format::format comx35_format::formats[] = {
	{   //  70K 5 1/4 inch single density, single sided, 35 tracks
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 16, 35, 1, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	{   //  140K 5 1/4 inch single density, double sided, 35 tracks
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 16, 35, 2, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	{   //  140K 5 1/4 inch single density, double track density, single sided, 70 tracks
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
		4000, 16, 70, 1, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	{}
};

int comx35_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	// DOS double track density flag in sector 0
	uint8_t dbl_trk;
	auto const [err, actual] = read_at(io, 18, &dbl_trk, 1);
	if (err || (actual != 1))
		return -1;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;
		if (!variants.empty() && !has_variant(variants, f.variant))
			continue;
		if (size != uint64_t(compute_track_size(f)) * f.track_count * f.head_count)
			continue;
		if ((f.variant == floppy_image::DSSD && dbl_trk) || (f.variant == floppy_image::SSQD && !dbl_trk))
			continue;

		return i;
	}

	return -1;
}

floppy_image_format_t::desc_e *comx35_format::get_desc_fm(const format &f, int &current_size, int &end_gap_index) const
{
	static floppy_image_format_t::desc_e desc[23] = {
		/* 00 */ { FM, 0xff, 0 },
		/* 01 */ { SECTOR_LOOP_START, 0, 0 },
		/* 02 */ {   FM, 0x00, 6 },
		/* 03 */ {   CRC_CCITT_FM_START, 1 },
		/* 04 */ {     RAW, 0xf57e, 1 },
		/* 05 */ {     TRACK_ID_FM },
		/* 06 */ {     HEAD_ID_FM },
		/* 07 */ {     SECTOR_ID_FM },
		/* 08 */ {     SIZE_ID_FM },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   CRC, 1 },
		/* 11 */ {   FM, 0x00, 0 },
		/* 12 */ {   FM, 0x00, 6 },
		/* 13 */ {   CRC_CCITT_FM_START, 2 },
		/* 14 */ {     RAW, 0xf56f, 1 },
		/* 15 */ {     SECTOR_DATA_FM, -1 },
		/* 16 */ {   CRC_END, 2 },
		/* 17 */ {   CRC, 2 },
		/* 18 */ {   FM, 0x00, 0 },
		/* 19 */ { SECTOR_LOOP_END },
		/* 20 */ { FM, 0x00, 0 },
		/* 21 */ { RAWBITS, 0xaaaa, 0 },
		/* 22 */ { END }
	};

	desc[0].p2 = f.gap_1;
	desc[1].p2 = f.sector_count - 1;
	desc[11].p2 = f.gap_2;
	desc[18].p2 = f.gap_3;
	desc[21].p1 = 0xaaaa;

	current_size = (f.gap_1 + f.sector_base_size * f.sector_count + (6+1+4+2+f.gap_2+6+1+2+f.gap_3) * f.sector_count) * 16;

	end_gap_index = 20;

	return desc;
}

const comx35_format FLOPPY_COMX35_FORMAT;
