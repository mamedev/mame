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
    cell 2.13 usec


	Boot Disc Label Format
	Track 0 Sector 0

	Byte
	Offset         Name                Description

	0              System disc ID      literally, ff,00h for a system
	                                   disc

	2              Load address        paragraph   to   load   booted
	                                   program at. If zero then  boot
	                                   loads in high memory.

	4              Length              paragraph count to load.

	6              Entry offset        I.P.  value  for  transfer  of
	                                   control.

	8              Entry segment       C.S.  value  for  transfer  of
	                                   control.

	10             I.D.                disc identifier.

	18             Part number         system identifier  - displayed
	                                   by early versions of boot.

	26             Sector size         byte count for sectors.

	28             Data start          first   data  sector  on  disc
	                                   (absolute sectors).

	30             Boot start          first   absolute   sector   of
	                                   program  for boot to  load  at
	                                   'load  address'  for  'length'
	                                   paragraphs.

	32             Flags               indicators:
	                                        bit  meaning
	                                       15-12 interleave    factor
	                                             (0-15)
	                                         0   0=single sided
	                                             1=double sided

	34             Disc type           00 = CP/M
	                                   01 = MS-DOS

	35             Reserved

	38             Speed table         information  for speed control
	                                   proc.

	56             Zone table          high track for each zone.

	71             Sector/track        sectors  per  track  for  each
	                                   zone.
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

void victor9k_format::log_boot_sector(UINT8 *data)
{
	// System disc ID
	logerror("System disc: %s\n", ((data[0] == 0xff) && (data[1] == 0x00)) ? "yes" : "no");

	// Load address
	logerror("Load address: %04x\n", (data[1] << 8) | data[2]);

	// Length
	logerror("Length: %04x\n", (data[3] << 8) | data[4]);

	// Entry offset
	logerror("Entry offset: %04x\n", (data[5] << 8) | data[6]);

	// Entry segment
	logerror("Entry segment: %04x\n", (data[7] << 8) | data[8]);

	// I.D.
	//logerror("I.D.: %s\n", data[10]);

	// Part number
	//logerror("Part number: %s\n", data[18]);

	// Sector size
	logerror("Sector size: %04x\n", (data[25] << 8) | data[26]);

	// Data start
	logerror("Data start: %04x\n", (data[27] << 8) | data[28]);

	// Boot start
	logerror("Boot start: %04x\n", (data[29] << 8) | data[30]);

	// Flags
	logerror("%s sided\n", BIT(data[33], 0) ? "Double" : "Single");
	logerror("Interleave factor: %u\n", data[32] >> 4);

	// Disc type
	switch (data[34]) {
	case 0x00: logerror("Disc type: CP/M\n"); break;
	case 0x01: logerror("Disc type: MS-DOS\n"); break;
	default: logerror("Disc type: unknown\n"); break;
	}

	// Speed table
	logerror("Speed table:  ");
	for (int i = 38; i < 56; i++) {
		logerror("%02x ", data[i]);
	}
	logerror("\n");

	// Zone table
	logerror("Zone table:            ");
	for (int i = 56; i < 71; i++) {
		logerror("%02x ", data[i]);
	}
	logerror("\n");

	// Sector/track
	logerror("Sector/track:          ");
	for (int i = 71; i < 86; i++) {
		logerror("%02x ", data[i]);
	}
	logerror("\n");
}

floppy_image_format_t::desc_e* victor9k_format::get_sector_desc(const format &f, int &current_size, int sector_count)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_INTERLEAVE_SKEW, 0, 0},
		/* 01 */ { SECTOR_LOOP_START, 0, -1 },
		/* 02 */ {   SYNC_GCR5, 9 },
		/* 03 */ {   GCR5, 0x07, 1 },
		/* 04 */ {   CRC_VICTOR_HDR_START, 1 },
		/* 05 */ {     TRACK_ID_VICTOR_GCR5 },
		/* 06 */ {     SECTOR_ID_GCR5 },
		/* 07 */ {   CRC_END, 1 },
		/* 08 */ {   CRC, 1 },
		/* 09 */ {   RAWBYTE, 0x55, 8 },
		/* 10 */ {   SYNC_GCR5, 5 },
		/* 11 */ {   GCR5, 0x08, 1 },
		/* 12 */ {   CRC_VICTOR_DATA_START, 2 },
		/* 13 */ {     SECTOR_DATA_GCR5, -1 },
		/* 14 */ {   CRC_END, 2 },
		/* 15 */ {   CRC, 2 },
		/* 16 */ {   RAWBYTE, 0x55, 8 },
		/* 17 */ { SECTOR_LOOP_END },
		/* 18 */ { RAWBYTE, 0x55, 0 },
		/* 19 */ { RAWBITS, 0x5555, 0 },
		/* 20 */ { END }
	};

	current_size = 90 + (1+1+1+1)*10 + 8*8 + 50 + (1+f.sector_base_size+2)*10 + 8*8;

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

	log_boot_sector(img);

	int track_offset = 0;

	for (int head = 0; head < f.head_count; head++) {
		for (int track = 0; track < f.track_count; track++) {
			int current_size = 0;
			int total_size = 200000000./cell_size[speed_zone[head][track]];
			int sector_count = sectors_per_track[head][track];
			int track_size = sector_count*f.sector_base_size;

			floppy_image_format_t::desc_e *desc = get_sector_desc(f, current_size, sector_count);

			int remaining_size = total_size - current_size;
			if(remaining_size < 0)
				throw emu_fatalerror("victor9k_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);

			// Fixup the end gap
			desc[18].p2 = remaining_size / 8;
			desc[19].p2 = remaining_size & 7;
			desc[19].p1 >>= remaining_size & 0x01;

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

int victor9k_format::get_rpm(int head, int track)
{
	return rpm[speed_zone[head][track]];
}

const victor9k_format::format victor9k_format::formats[] = {
	{ //
		floppy_image::FF_525, floppy_image::SSDD, 1224, 80, 1, 512
	},
	{ //
		floppy_image::FF_525, floppy_image::DSDD, 2448, 80, 2, 512
	},
	{}
};

const UINT32 victor9k_format::cell_size[9] =
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
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
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
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
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

const int victor9k_format::rpm[9] =
{
	252, 267, 283, 300, 320, 342, 368, 401, 417
};

const floppy_format_type FLOPPY_VICTOR_9000_FORMAT = &floppy_image_format_creator<victor9k_format>;
