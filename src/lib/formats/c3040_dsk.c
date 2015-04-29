// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/c3040_dsk.c

    Commodore 3040 sector disk image format

*********************************************************************/

#include <assert.h>

#include "formats/c3040_dsk.h"

c3040_format::c3040_format() : d64_format(file_formats)
{
}

const char *c3040_format::name() const
{
	return "c3040";
}

const char *c3040_format::description() const
{
	return "Commodore 3040 disk image";
}

const char *c3040_format::extensions() const
{
	return "d67";
}

const c3040_format::format c3040_format::file_formats[] = {
	{ // d67, dos 1, 35 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 690, 35, 1, 256, 9, 8
	},
	{}
};

const int c3040_format::c3040_sectors_per_track[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, //  1-17
	20, 20, 20, 20, 20, 20, 20,                                         // 18-24
	18, 18, 18, 18, 18, 18,                                             // 25-30
	17, 17, 17, 17, 17,                                                 // 31-35
	17, 17, 17, 17, 17,                                                 // 36-40
	17, 17                                                              // 41-42
};

const int c3040_format::c3040_gap2[] =
{
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, // 1-17
	8, 8, 8, 8, 8, 8, 8,                                                // 18-24
	15, 15, 15, 15, 15, 15,                                             // 25-30
	12, 12, 12, 12, 12                                                  // 31-35
};

floppy_image_format_t::desc_e* c3040_format::get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, -1 },
		/* 01 */ {   SYNC_GCR5, 2 },
		/* 02 */ {   GCR5, 0x08, 1 },
		/* 03 */ {   CRC, 1 },
		/* 04 */ {   CRC_CBM_START, 1 },
		/* 05 */ {     SECTOR_ID_GCR5 },
		/* 06 */ {     TRACK_ID_DOS2_GCR5 },
		/* 07 */ {     GCR5, id2, 1 },
		/* 08 */ {     GCR5, id1, 1 },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   GCR5, 0x00, f.gap_1 },
		/* 11 */ {   SYNC_GCR5, 2 },
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

	current_size = 20 + (1+1+4)*10 + f.gap_1*10 + 20 + (1+f.sector_base_size+1)*10 + gap_2*10;

	current_size *= sector_count;
	return desc;
}

void c3040_format::fix_end_gap(floppy_image_format_t::desc_e* desc, int remaining_size)
{
	desc[19].p2 = remaining_size / 10;
	desc[20].p2 = remaining_size % 10;
	desc[20].p1 >>= remaining_size & 0x01;
}

const floppy_format_type FLOPPY_C3040_FORMAT = &floppy_image_format_creator<c3040_format>;
