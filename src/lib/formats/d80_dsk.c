// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d80_dsk.c

    Commodore 8050 sector disk image format

*********************************************************************/

#include "emu.h"
#include "formats/d80_dsk.h"

d80_format::d80_format() : d64_format(file_formats)
{
}

d80_format::d80_format(const format *_formats) : d64_format(_formats)
{
}

const char *d80_format::name() const
{
	return "d80";
}

const char *d80_format::description() const
{
	return "Commodore 8050 disk image";
}

const char *d80_format::extensions() const
{
	return "d80";
}

const d80_format::format d80_format::file_formats[] = {
	{ // d80, dos 2.5, 77 tracks, head/stepper 100 tpi
		floppy_image::FF_525, floppy_image::SSQD, 2083, 77, 1, 256, 9, 8
	},
	{}
};

const UINT32 d80_format::d80_cell_size[] =
{
	2667, // 12MHz/2/16
	2500, // 12MHz/2/15
	2333, // 12MHz/2/14
	2167  // 12MHz/2/13
};

const int d80_format::d80_sectors_per_track[] =
{
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, //  1-39
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,                         // 40-53
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,                                     // 54-64
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,                             // 65-77
	23, 23, 23, 23, 23, 23, 23                                                      // 78-84
};

const int d80_format::d80_speed_zone[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  1-39
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                   // 40-53
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                            // 54-64
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 65-77
	0, 0, 0, 0, 0, 0, 0                                         // 78-84
};

int d80_format::get_physical_track(const format &f, int head, int track)
{
	return track;
}

UINT32 d80_format::get_cell_size(const format &f, int track)
{
	return d80_cell_size[d80_speed_zone[track]];
}

int d80_format::get_sectors_per_track(const format &f, int track)
{
	return d80_sectors_per_track[track];
}

int d80_format::get_disk_id_offset(const format &f)
{
	// t39s0 +0x18
	return 0x44e18;
}

floppy_image_format_t::desc_e* d80_format::get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, -1 },
		/* 01 */ {   RAWBYTE, 0xff, 5 },
		/* 02 */ {   GCR5, 0x08, 1 },
		/* 03 */ {   CRC, 1 },
		/* 04 */ {   CRC_CBM_START, 1 },
		/* 05 */ {     SECTOR_ID_GCR5 },
		/* 06 */ {     TRACK_ID_DOS25_GCR5 },
		/* 07 */ {     GCR5, id2, 1 },
		/* 08 */ {     GCR5, id1, 1 },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   GCR5, 0x0f, 2 },
		/* 11 */ {   RAWBYTE, 0x55, f.gap_1 },
		/* 12 */ {   RAWBYTE, 0xff, 5 },
		/* 13 */ {   GCR5, 0x07, 1 },
		/* 14 */ {   CRC_CBM_START, 2 },
		/* 15 */ {     SECTOR_DATA_GCR5, -1 },
		/* 16 */ {   CRC_END, 2 },
		/* 17 */ {   CRC, 2 },
		/* 18 */ {   GCR5, 0x00, 2 },
		/* 19 */ {   RAWBYTE, 0x55, gap_2 },
		/* 20 */ { SECTOR_LOOP_END },
		/* 21 */ { RAWBYTE, 0x55, 0 },
		/* 22 */ { RAWBITS, 0x5555, 0 },
		/* 23 */ { END }
	};

	current_size = 40 + (1+1+4+2)*10 + (f.gap_1)*8 + 40 + (1+f.sector_base_size+1+2)*10 + gap_2*8;

	current_size *= sector_count;
	return desc;
}

const floppy_format_type FLOPPY_D80_FORMAT = &floppy_image_format_creator<d80_format>;
