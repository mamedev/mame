// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/victor9k_dsk.cpp

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
    4           38-47*      30-39*      15          187.6        320
    5           48-59       40-51       14          175.3        342
    6           60-70       52-62       13          163.0        368
    7           71-79       63-74       12          149.6        401
    8           unused      75-79       11          144.0        417

    * The documentation for the Victor lists Zone 4 as ending with Track 48
    on side 1 and track 40 on side two. This is incorrect. The above table
    reflects disks analyzed from various machines and matches the assembly
    code in the floppy driver. Various written documents contain this
    documentation bug.

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

#include "formats/victor9k_dsk.h"

#include "coretmpl.h" // util::BIT
#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*

#include <cstring>
#include <iterator>


victor9k_format::victor9k_format()
{
}

const char *victor9k_format::name() const noexcept
{
	return "victor9k";
}

const char *victor9k_format::description() const noexcept
{
	return "Victor 9000 disk image";
}

const char *victor9k_format::extensions() const noexcept
{
	return "img";
}

int victor9k_format::find_size(util::random_read &io)
{
	uint64_t size;
	if (io.length(size))
		return -1;

	for (int i = 0; formats[i].sector_count; i++) {
		const format &f = formats[i];
		if(size == uint32_t(f.sector_count*f.sector_base_size))
			return i;
	}

	return -1;
}

int victor9k_format::find_size(util::random_read &io, uint32_t form_factor)
{
	return find_size(io);
}

int victor9k_format::identify(const floppy_image &image)
{
	for (int i = 0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(f.variant == image.get_variant())
			return i;
	}

	return -1;
}


int victor9k_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return FIFID_SIZE;

	return 0;
}

void victor9k_format::log_boot_sector(const uint8_t *data)
{
	// System disc ID
	osd_printf_verbose("System disc: %s\n", ((data[0] == 0xff) && (data[1] == 0x00)) ? "yes" : "no");

	// Load address
	osd_printf_verbose("Load address: %04x\n", (data[2] && data[3]));

	// Length
	osd_printf_verbose("Length: %04x\n", (data[4] <<8 | data[5]));

	// Entry offset
	osd_printf_verbose("Entry offset: %04x\n", (data[6] << 8 | data[7]));

	// Entry segment
	osd_printf_verbose("Entry segment: %04x\n", (data[8] << 8 | data[9]));

	// I.D.
	osd_printf_verbose("Disk I.D.: %s\n", (data[10] << 8 | data[17]));

	// Part number
	osd_printf_verbose("Part number: %s\n", (data[18] << 8 | data[25]));

	// Sector size
	osd_printf_verbose("Sector size: %04d\n", (data[26] << 8 | data[27]));

	// Data start
	osd_printf_verbose("Data start: %04x\n", (data[28] << 8 | data[29]));

	// Boot start
	osd_printf_verbose("Boot start: %04x\n", (data[30] << 8 | data[31]));

	//Disk type - flags
	osd_printf_verbose("%s sided\n", (data[32]) ? "Double" : "Single");

	//Sector Interleave
	osd_printf_verbose("Sector Interleave: %02x\n", data[33]);

	// Disc type
	switch (data[34]) {
		case 0x01: osd_printf_verbose("Disc type: CP/M\n"); break;
		case 0x10: osd_printf_verbose("Disc type: MS-DOS\n"); break;
		default: osd_printf_verbose("Disc type: unknown\n"); break;
	}

	//reserved
	osd_printf_verbose("Part number: %s\n", data[35] << 8 | data[37]);

	// Speed table
	osd_printf_verbose("Speed table:  ");
	for (int i = 38; i < 56; i++) {
		osd_printf_verbose("%02x ", data[i]);
	}
	osd_printf_verbose("\n");

	// Zone table
	osd_printf_verbose("Zone table:            ");
	for (int i = 56; i < 71; i++) {
		osd_printf_verbose("%02x ", data[i]);
	}
	osd_printf_verbose("\n");

	// Sector/track
	osd_printf_verbose("Sector/track:          ");
	for (int i = 71; i < 86; i++) {
		osd_printf_verbose("%02x ", data[i]);
	}
	osd_printf_verbose("\n");
}

floppy_image_format_t::desc_e* victor9k_format::get_sector_desc(const format &f, int &current_size, int sector_count)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_INTERLEAVE_SKEW, 0, 0},
		/* 01 */ { SECTOR_LOOP_START, 0, -1 },
		/* 02 */ {   SYNC_GCR5, 15 },
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

	current_size = 150 + (1+1+1+1)*10 + 8*8 + 50 + (1+f.sector_base_size+2)*10 + 8*8;

	current_size *= sector_count;
	return desc;
}

void victor9k_format::build_sector_description(const format &f, uint8_t *sectdata, uint32_t sect_offs, desc_s *sectors, int sector_count)
{
	for (int i = 0; i < sector_count; i++) {
		sectors[i].data = sectdata + sect_offs;
		sectors[i].size = f.sector_base_size;
		sectors[i].sector_id = i;

		sect_offs += sectors[i].size;
	}
}

bool victor9k_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	int const type = find_size(io, form_factor);
	if (type == -1)
		return false;

	const format &f = formats[type];

	osd_printf_verbose("Type: %d Head Count: %d Sector Count: %d\n", type,
		f.head_count, f.sector_count);

	uint64_t size;
	if (io.length(size))
		return false;

	auto const [err, img, actual] = read_at(io, 0, size);
	if (err || (actual != size))
		return false;

	log_boot_sector(&img[0]);

	int track_offset = 0;

	osd_printf_verbose("load Heads: %01d Tracks: %02d Sectors/track head[1]track[1]: %02d\n ",
		f.head_count, f.track_count, sectors_per_track[1][1]);

	for (int head = 0; head < f.head_count; head++) {
		for (int track = 0; track < f.track_count; track++) {
			int current_size = 0;
			int total_size = 200000000./cell_size[speed_zone[head][track]];
			int sector_count = sectors_per_track[head][track];
			int track_size = sector_count*f.sector_base_size;

			floppy_image_format_t::desc_e *desc = get_sector_desc(f, current_size, sector_count);

			int remaining_size = total_size - current_size;
			if(remaining_size < 0) {
				osd_printf_error("victor9k_format: Incorrect track layout, max_size=%d, current_size=%d\n", total_size, current_size);
				return false;
			}

			// Fixup the end gap
			desc[18].p2 = remaining_size / 8;
			desc[19].p2 = remaining_size & 7;
			desc[19].p1 >>= remaining_size & 0x01;

			desc_s sectors[40];

			build_sector_description(f, &img[0], track_offset, sectors, sector_count);
			generate_track(desc, track, head, sectors, sector_count, total_size, image);

			track_offset += track_size;
		}
	}

	image.set_variant(f.variant);

	return true;
}

int victor9k_format::get_rpm(int head, int track)
{
	osd_printf_verbose("Head: %1d TracK: %02d \n ", head, track);
	return rpm[speed_zone[head][track]];
}

int victor9k_format::get_image_offset(const format &f, int _head, int _track)
{
	int offset = 0;
	if (_head) {
		int first_side = 0;  //build up offset for first side
		for (int track = 0; track < f.track_count; track++) {
			offset += compute_track_size(f, first_side, track);
		}
	}
	for (int track = 0; track < _track; track++) {
		offset += compute_track_size(f, _head, track);
	}
	return offset;
}

int victor9k_format::compute_track_size(const format &f, int head, int track)
{
	return sectors_per_track[head][track] * f.sector_base_size;
}

const victor9k_format::format victor9k_format::formats[] = {
	{ //
		floppy_image::FF_525, floppy_image::SSDD, 1224, 80, 1, 512
	},
	{ //
		floppy_image::FF_525, floppy_image::DSDD, 2391, 80, 2, 512
	},
	{}
};

const uint32_t victor9k_format::cell_size[9] =
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
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
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
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8
	}
};

const int victor9k_format::rpm[9] =
{
	252, 267, 283, 300, 321, 342, 368, 401, 417
};

bool victor9k_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int type = victor9k_format::identify(image);
	uint64_t size;
	io.length(size); // FIXME: check for errors
	osd_printf_verbose("save type: %01d, size: %d\n", type, size);

	if (type == -1)
		return false;

	const format &f = formats[type];

	osd_printf_verbose("save Heads: %01d Tracks: %02d Sectors/track head[1]track[1]: %02d\n ",
		f.head_count, f.track_count, sectors_per_track[1][1]);
	for (int head = 0; head < f.head_count; head++) {
		for(int track=0; track < f.track_count; track++) {
			int sector_count = sectors_per_track[head][track];
			int track_size = compute_track_size(f, head, track);
			uint8_t sectdata[40*512];
			desc_s sectors[40];
			int offset = get_image_offset(f, head, track);
			osd_printf_verbose(">>type: %s, head: %d, track: %d, sector_count: %d, offset: %d, track_size: %d\n",
				f.sector_base_size, head, track, sector_count, offset, track_size);

			build_sector_description(f, sectdata, 0, sectors, sector_count);
			extract_sectors(image, f, sectors, track, head, sector_count);
			/*auto const [err, actual] =*/ write_at(io, offset, sectdata, track_size); // FIXME: check for errors
		}
	}

	return true;
}

void victor9k_format::extract_sectors(const floppy_image &image, const format &f, desc_s *sdesc, int track, int head, int sector_count)
{
	// Extract the sectors
	auto bitstream = generate_bitstream_from_track(track, head, cell_size[speed_zone[head][track]], image);
	auto sectors = extract_sectors_from_bitstream_victor_gcr5(bitstream);

	if (sectors.size() == 0) {
		for(int i=0; i<sector_count; i++) {
			std::vector<unsigned char> sector;
			sectors.push_back(sector);
		}
	}

	for (int i = 0; i<sector_count; i++) {
		desc_s &ds = sdesc[i];
		const auto &data = sectors[ds.sector_id];
		osd_printf_verbose("Head: %01d TracK: %02d Total Sectors: %02d Current Sector: %02d ",
			head, track, sector_count, i);
		if (data.empty()) {
			memset((uint8_t *)ds.data, 0, ds.size);
		} else if (data.size() < ds.size) {
			memcpy((uint8_t *)ds.data, data.data(), data.size() - 1);
			memset((uint8_t *)ds.data + data.size() - 1, 0, data.size() - ds.size);
			osd_printf_verbose("data.size(): %01d\n", data.size());
		} else
			memcpy((uint8_t *)ds.data, data.data(), ds.size);
		osd_printf_verbose("data.size(): %01d\n", data.size());
	}
}

const victor9k_format FLOPPY_VICTOR_9000_FORMAT;
