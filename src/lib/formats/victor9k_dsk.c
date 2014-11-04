// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

	formats/victor9k_dsk.c

	Victor 9000 sector disk image format

*********************************************************************/

/*

	Sector format
	-------------

	Header sync
	Sector header (header ID, track ID, sector ID, and checksum)
	Gap 1
	Data Sync
	Data field (data sync, data ID, data bytes, and checksum)
	Gap 2

	Track format
	------------

	ZONE        LOWER HEAD  UPPER HEAD  SECTORS     ROTATIONAL   RPM
	NUMBER      TRACKS      TRACKS      PER TRACK   PERIOD (MS)

	0           0-3         unused      19          237.9        252
	1           4-15        0-7         18          224.5        267
	2           16-26       8-18        17          212.2        283
	3           27-37       19-29       16          199.9        300
	4           38-48       30-40       15          187.6        320
	5           49-59       41-51       14          175.3        342
	6           60-70       52-62       13          163.0        368
	7           71-79       63-74       12          149.6        401
	8           unused      75-79       11          144.0        417

	Interleave factor 3

*/

#include "emu.h"
#include "formats/victor9k_dsk.h"

victor9k_format::victor9k_format()
{
}

const char *victor9k_format::name() const
{
	return "victor9k";
}

const char *victor9k_format::description() const
{
	return "Victor 9000 disk image";
}

const char *victor9k_format::extensions() const
{
	return "img";
}

int victor9k_format::find_size(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].sector_count; i++) {
		const format &f = formats[i];
		if(size == (UINT32) f.sector_count*f.sector_base_size*f.head_count)
			return i;
	}
	return -1;
}

int victor9k_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 50;

	return 0;
}

floppy_image_format_t::desc_e* victor9k_format::get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, -1 },
		/* 01 */ {   RAWBYTE, 0xff, 5 },
		/* 02 */ {   GCR5, 0x08, 1 },
		/* 03 */ {   CRC, 1 },
		/* 04 */ {   CRC_CBM_START, 1 },
		/* 05 */ {     SECTOR_ID_GCR5 },
		/* 06 */ {     TRACK_ID_DOS2_GCR5 },
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

void victor9k_format::build_sector_description(const format &f, UINT8 *sectdata, offs_t sect_offs, desc_s *sectors, int sector_count) const
{
	for (int i = 0; i < sector_count; i++) {
		sectors[i].data = sectdata + sect_offs;
		sectors[i].size = f.sector_base_size;
		sectors[i].sector_id = i;

		sect_offs += sectors[i].size;
	}
}

bool victor9k_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int type = find_size(io, form_factor);
	if(type == -1)
		return false;

	const format &f = formats[type];

	UINT64 size = io_generic_size(io);
	dynamic_buffer img;
	img.resize(size);

	io_generic_read(io, img, 0, size);

	int track_offset = 0;

	UINT8 id1 = 0xde, id2 = 0xad; // TODO

	for (int head = 0; head < f.head_count; head++) {
		for (int track = 0; track < f.track_count; track++) {
			int current_size = 0;
			int total_size = 200000000./cell_size[speed_zone[head][track]];
			int sector_count = sectors_per_track[head][track];
			int track_size = sector_count*f.sector_base_size;

			floppy_image_format_t::desc_e *desc = get_sector_desc(f, current_size, sector_count, id1, id2, f.gap_2);

			int remaining_size = total_size - current_size;
			if(remaining_size < 0)
				throw emu_fatalerror("victor9k_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);

			// Fixup the end gap
			desc[21].p2 = remaining_size / 8;
			desc[22].p2 = remaining_size & 7;
			desc[22].p1 >>= remaining_size & 0x01;

			desc_s sectors[40];

			build_sector_description(f, img, track_offset, sectors, sector_count);
			generate_track(desc, track, head, sectors, sector_count, total_size, image);

			track_offset += track_size;
		}
	}

	image->set_variant(f.variant);

	return true;
}

bool victor9k_format::supports_save() const
{
	return false;
}

const victor9k_format::format victor9k_format::formats[] = {
	{ //
		floppy_image::FF_525, floppy_image::SSDD, 1224, 80, 1, 512, 9, 8
	},
	{ //
		floppy_image::FF_525, floppy_image::DSDD, 2448, 80, 2, 512, 9, 8
	},
	{}
};

const UINT32 victor9k_format::cell_size[] =
{
	1789, 1896, 2009, 2130, 2272, 2428, 2613, 2847, 2961
};

const int victor9k_format::sectors_per_track[2][80] =
{
	{
		19, 19, 19, 19,
		18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		12, 12, 12, 12, 12, 12, 12, 12, 12
	},
	{
		18, 18, 18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
		11, 11, 11, 11, 11
	}
};

const int victor9k_format::speed_zone[2][80] =
{
	{
		0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7
	},
	{
		1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8
	}
};

const floppy_format_type FLOPPY_VICTOR_9000_FORMAT = &floppy_image_format_creator<victor9k_format>;
