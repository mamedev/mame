// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d64_dsk.c

    Commodore 4040/1541/1551 sector disk image format

    http://unusedino.de/ec64/technical/formats/d64.html

*********************************************************************/

#include "emu.h" // emu_fatalerror, fatalerror
#include "formats/d64_dsk.h"

d64_format::d64_format()
{
	formats = file_formats;
}

d64_format::d64_format(const format *_formats)
{
	formats = _formats;
}

const char *d64_format::name() const
{
	return "d64";
}

const char *d64_format::description() const
{
	return "Commodore 4040/1541/1551 disk image";
}

const char *d64_format::extensions() const
{
	return "d64";
}

const d64_format::format d64_format::file_formats[] = {
	{ // d64, dos 2, 35 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 683, 35, 1, 256, 9, 8
	},
	{ // d64, dos 2, 40 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 768, 40, 1, 256, 9, 8
	},
	{ // d64, dos 2, 42 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 802, 42, 1, 256, 9, 8
	},
	{}
};

const UINT32 d64_format::cell_size[] =
{
	4000, // 16MHz/16/4
	3750, // 16MHz/15/4
	3500, // 16MHz/14/4
	3250  // 16MHz/13/4
};

const int d64_format::sectors_per_track[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, //  1-17
	19, 19, 19, 19, 19, 19, 19,                                         // 18-24
	18, 18, 18, 18, 18, 18,                                             // 25-30
	17, 17, 17, 17, 17,                                                 // 31-35
	17, 17, 17, 17, 17,                                                 // 36-40
	17, 17                                                              // 41-42
};

const int d64_format::speed_zone[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  1-17
	2, 2, 2, 2, 2, 2, 2,                               // 18-24
	1, 1, 1, 1, 1, 1,                                  // 25-30
	0, 0, 0, 0, 0,                                     // 31-35
	0, 0, 0, 0, 0,                                     // 36-40
	0, 0                                               // 41-42
};

int d64_format::find_size(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);
	for(int i=0; formats[i].sector_count; i++) {
		const format &f = formats[i];
		if(size == (UINT32) f.sector_count*f.sector_base_size*f.head_count)
			return i;
		if(size == (UINT32) (f.sector_count*f.sector_base_size*f.head_count) + f.sector_count)
			return i;
	}
	return -1;
}

int d64_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 50;

	return 0;
}

int d64_format::get_physical_track(const format &f, int head, int track)
{
	// skip halftracks
	return track * 2;
}

int d64_format::get_disk_id_offset(const format &f)
{
	// t18s0 +0xa2
	return 0x165a2;
}

void d64_format::get_disk_id(const format &f, io_generic *io, UINT8 &id1, UINT8 &id2)
{
	UINT8 id[2];
	io_generic_read(io, id, get_disk_id_offset(f), 2);
	id1 = id[0];
	id2 = id[1];
}

int d64_format::get_image_offset(const format &f, int _head, int _track)
{
	int offset = 0;
	if (_head) {
		for (int track = 0; track < f.track_count; track++) {
			offset += compute_track_size(f, track);
		}
	}
	for (int track = 0; track < _track; track++) {
		offset += compute_track_size(f, track);
	}
	return offset;
}

int d64_format::compute_track_size(const format &f, int track)
{
	return this->get_sectors_per_track(f, track) * f.sector_base_size;
}

UINT32 d64_format::get_cell_size(const format &f, int track)
{
	return cell_size[speed_zone[track]];
}

int d64_format::get_sectors_per_track(const format &f, int track)
{
	return sectors_per_track[track];
}

floppy_image_format_t::desc_e* d64_format::get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2)
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

void d64_format::build_sector_description(const format &f, UINT8 *sectdata, UINT32 sect_offs, UINT32 error_offs, desc_s *sectors, int sector_count) const
{
	for (int i = 0; i < sector_count; i++) {
		sectors[i].data = sectdata + sect_offs;
		sectors[i].size = f.sector_base_size;
		sectors[i].sector_id = i;
		sectors[i].sector_info = sectdata[error_offs];

		sect_offs += sectors[i].size;
		error_offs++;
	}
}

void d64_format::fix_end_gap(floppy_image_format_t::desc_e* desc, int remaining_size)
{
	desc[21].p2 = remaining_size / 8;
	desc[22].p2 = remaining_size & 7;
	desc[22].p1 >>= remaining_size & 0x01;
}

bool d64_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int type = find_size(io, form_factor);
	if(type == -1)
		return false;

	const format &f = formats[type];

	UINT64 size = io_generic_size(io);
	dynamic_buffer img;

	if(size == (UINT32)f.sector_count*f.sector_base_size) {
		img.resize(size + f.sector_count);
		memset(&img[0], ERROR_00, size + f.sector_count);
	}
	else {
		img.resize(size);
	}

	io_generic_read(io, &img[0], 0, size);

	int track_offset = 0, error_offset = f.sector_count*f.sector_base_size;

	UINT8 id1 = 0, id2 = 0;
	get_disk_id(f, io, id1, id2);

	for (int head = 0; head < f.head_count; head++) {
		for (int track = 0; track < f.track_count; track++) {
			int current_size = 0;
			int total_size = 200000000./this->get_cell_size(f, track);
			int physical_track = this->get_physical_track(f, head, track);
			int sector_count = this->get_sectors_per_track(f, track);
			int track_size = sector_count*f.sector_base_size;
			int gap2 = this->get_gap2(f, head, track);

			floppy_image_format_t::desc_e *desc = this->get_sector_desc(f, current_size, sector_count, id1, id2, gap2);

			int remaining_size = total_size - current_size;
			if(remaining_size < 0)
				throw emu_fatalerror("d64_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);

			this->fix_end_gap(desc, remaining_size);

			desc_s sectors[40];

			build_sector_description(f, &img[0], track_offset, error_offset, sectors, sector_count);
			generate_track(desc, physical_track, head, sectors, sector_count, total_size, image);

			track_offset += track_size;
			error_offset += sector_count;
		}
	}

	image->set_variant(f.variant);

	return true;
}

bool d64_format::save(io_generic *io, floppy_image *image)
{
	const format &f = formats[0];

	for(int head=0; head < f.head_count; head++) {
		for(int track=0; track < f.track_count; track++) {
			int sector_count = this->get_sectors_per_track(f, track);
			int track_size = compute_track_size(f, track);
			UINT8 sectdata[40*256] = { 0 };
			desc_s sectors[40];
			int offset = get_image_offset(f, head, track);

			build_sector_description(f, sectdata, 0, 0, sectors, sector_count);
			extract_sectors(image, f, sectors, track, head, sector_count);
			io_generic_write(io, sectdata, offset, track_size);
		}
	}

	return true;
}

void d64_format::extract_sectors(floppy_image *image, const format &f, desc_s *sdesc, int track, int head, int sector_count)
{
	UINT8 bitstream[500000/8];
	UINT8 sectdata[50000];
	desc_xs sectors[256];
	int physical_track = this->get_physical_track(f, head, track);
	int cell_size = this->get_cell_size(f, track);
	int track_size;

	// Extract the sectors
	generate_bitstream_from_track(physical_track, head, cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_gcr5(bitstream, track_size, sectors, sectdata, sizeof(sectdata), head, f.track_count);

	for(int i=0; i<sector_count; i++) {
		desc_s &ds = sdesc[i];
		desc_xs &xs = sectors[ds.sector_id];
		if(!xs.data)
			memset((void *)ds.data, 0, ds.size);
		else if(xs.size < ds.size) {
			memcpy((void *)ds.data, xs.data, xs.size);
			memset((UINT8 *)ds.data + xs.size, 0, xs.size - ds.size);
		} else
			memcpy((void *)ds.data, xs.data, ds.size);
	}
}

const floppy_format_type FLOPPY_D64_FORMAT = &floppy_image_format_creator<d64_format>;
