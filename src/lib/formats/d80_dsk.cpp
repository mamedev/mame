// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d80_dsk.cpp

    Commodore 8050 sector disk image format

*********************************************************************/

#include "formats/d80_dsk.h"

#include "ioprocs.h"


namespace {

constexpr uint32_t GCR5_ZERO = 0x14a;

constexpr uint8_t D80_GAP2[] = { 30, 23, 21, 22 };

constexpr uint32_t STEP_SKEW[] = { 175'453'000, 153'616'000, 131'783'000, 109'950'000 };
constexpr uint32_t HEAD_SKEW[] = { 149'362'000, 127'534'000, 105'703'000, 83'870'000 };

} // anonymous namespace


d80_format::d80_format() : d64_format(file_formats), formats(nullptr)
{
}

d80_format::d80_format(const format *_formats) : d64_format(_formats), formats(nullptr)
{
}

const char *d80_format::name() const noexcept
{
	return "d80";
}

const char *d80_format::description() const noexcept
{
	return "Commodore 8050 disk image";
}

const char *d80_format::extensions() const noexcept
{
	return "d80";
}

const d80_format::format d80_format::file_formats[] = {
	{ // d80, dos 2.5, 77 tracks, head/stepper 100 tpi
		floppy_image::FF_525, floppy_image::SSQD, 2083, 77, 1, 256, 20, 0
	},
	{}
};

const uint32_t d80_format::d80_cell_size[] =
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

int d80_format::get_physical_track(const format &f, int head, int track) const
{
	return track;
}

uint32_t d80_format::get_cell_size(const format &f, int track) const
{
	return d80_cell_size[d80_speed_zone[track]];
}

int d80_format::get_sectors_per_track(const format &f, int track) const
{
	return d80_sectors_per_track[track];
}

int d80_format::get_gap2(const format &f, int head, int track) const
{
	return D80_GAP2[d80_speed_zone[track]];
}

int d80_format::get_disk_id_offset(const format &f) const
{
	// t39s0 +0x18
	return 0x44e18;
}

floppy_image_format_t::desc_e* d80_format::get_sector_desc(const format &f, int &current_size, int sector_count, uint8_t id1, uint8_t id2, int gap_2) const
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, -1 },
		/* 01 */ {   SYNC_GCR5, 4 },
		/* 02 */ {   GCR5, 0x08, 1 },
		/* 03 */ {   CRC, 1 },
		/* 04 */ {   CRC_CBM_START, 1 },
		/* 05 */ {     SECTOR_ID_GCR5 },
		/* 06 */ {     TRACK_ID_DOS25_GCR5 },
		/* 07 */ {     GCR5, id2, 1 },
		/* 08 */ {     GCR5, id1, 1 },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   GCR5, 0x0f, f.gap_1 },
		/* 11 */ {   SYNC_GCR5, 5 },
		/* 12 */ {   GCR5, 0x07, 1 },
		/* 13 */ {   CRC_CBM_START, 2 },
		/* 14 */ {   CRC_CBM_START, 3 },
		/* 15 */ {     SECTOR_DATA_GCR5, -1 },
		/* 16 */ {   CRC_END, 2 },
		/* 17 */ {   CRC_END, 3 },
		/* 18 */ {   CRC, 2 },
		/* 19 */ {   CRC, 3 },
		/* 20 */ {   GCR5, 0x00, 1 },
		/* 21 */ {   GCR5, 0x00, gap_2 },
		/* 22 */ { SECTOR_LOOP_END },
		/* 23 */ { GCR5, 0x00, 0 },
		/* 24 */ { RAWBITS, GCR5_ZERO, 0 },
		/* 25 */ { END }
	};

	desc[7].p1 = id2;
	desc[8].p1 = id1;
	desc[10].p2 = f.gap_1;
	desc[21].p2 = gap_2;

	current_size = 40 + (1+1+4)*10 + (f.gap_1)*10 + 50 + (1+f.sector_base_size+1+1+1)*10 + gap_2*10;

	current_size *= sector_count;
	return desc;
}

void d80_format::fix_end_gap(floppy_image_format_t::desc_e* desc, int remaining_size) const
{
	desc[23].p2 = remaining_size / 10;
	desc[24].p2 = remaining_size % 10;
	desc[24].p1 = GCR5_ZERO >> (10 - desc[24].p2);
}

uint32_t d80_format::get_track_skew(const format &f, int head, int track, uint32_t skew, int current_size, int gap2, int total_size) const
{
	uint64_t result = 0;

	for (int t = 0; t <= track; t++)
	{
		for (int h = 0; h < f.head_count && (t < track || h <= head); h++)
		{
			if (t || h)
				result += h ? HEAD_SKEW[d80_speed_zone[t]] : STEP_SKEW[d80_speed_zone[t]];
		}
	}

	return result % 200000000;
}

const d80_format FLOPPY_D80_FORMAT;
