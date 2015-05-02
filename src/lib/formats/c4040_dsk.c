// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/c4040_dsk.c

    Commodore 4040 sector disk image format

*********************************************************************/

#include <assert.h>

#include "formats/c4040_dsk.h"

c4040_format::c4040_format() : d64_format(file_formats)
{
}

const char *c4040_format::name() const
{
	return "c4040";
}

const char *c4040_format::description() const
{
	return "Commodore 4040 disk image";
}

const char *c4040_format::extensions() const
{
	return "d64";
}

const c4040_format::format c4040_format::file_formats[] = {
	{ // c4040, dos 2, 35 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 683, 35, 1, 256, 9, 8
	},
	{}
};

const int c4040_format::c4040_gap2[] =
{
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, // 1-17
	19, 19, 19, 19, 19, 19, 19,                                         // 18-24
	15, 15, 15, 15, 15, 15,                                             // 25-30
	12, 12, 12, 12, 12                                                  // 31-35
};

floppy_image_format_t::desc_e* c4040_format::get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, -1 },
		/* 01 */ {   SYNC_GCR5, 4 },
		/* 02 */ {   GCR5, 0x08, 1 },
		/* 03 */ {   CRC, 1 },
		/* 04 */ {   CRC_CBM_START, 1 },
		/* 05 */ {     SECTOR_ID_GCR5 },
		/* 06 */ {     TRACK_ID_DOS2_GCR5 },
		/* 07 */ {     GCR5, id2, 1 },
		/* 08 */ {     GCR5, id1, 1 },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   GCR5, 0x00, f.gap_1 },
		/* 11 */ {   SYNC_GCR5, 4 },
		/* 12 */ {   GCR5, 0x07, 1 },
		/* 13 */ {   CRC_CBM_START, 2 },
		/* 14 */ {     SECTOR_DATA_GCR5, -1 },
		/* 15 */ {   CRC_END, 2 },
		/* 16 */ {   CRC, 2 },
		/* 17 */ {   GCR5, 0x00, gap_2 },
		/* 18 */ { SECTOR_LOOP_END },
		/* 19 */ { GCR5, 0x00, 0 },
		/* 20 */ { RAWBITS, 0x14a, 0 },
		/* 21 */ { END }
	};

	desc[17].p2 = gap_2; // TODO why?!?

	current_size = 40 + (1+1+4)*10 + f.gap_1*10 + 40 + (1+f.sector_base_size+1)*10 + gap_2*10;

	current_size *= sector_count;
	return desc;
}

void c4040_format::fix_end_gap(floppy_image_format_t::desc_e* desc, int remaining_size)
{
	desc[19].p2 = remaining_size / 10;
	desc[20].p2 = remaining_size % 10;
	desc[20].p1 >>= remaining_size & 0x01;
}

const floppy_format_type FLOPPY_C4040_FORMAT = &floppy_image_format_creator<c4040_format>;
