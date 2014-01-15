// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ccvf_dsk.c

    Compucolor Virtual Floppy Disk Image format

*********************************************************************/

/*

	TODO:

	- parse ascii hex into binary on load()

*/

#include "emu.h"
#include "formats/ccvf_dsk.h"

ccvf_format::ccvf_format()
{
	formats = file_formats;
}

ccvf_format::ccvf_format(const format *_formats)
{
	formats = _formats;
}

const char *ccvf_format::name() const
{
	return "ccvf";
}

const char *ccvf_format::description() const
{
	return "Compucolor Virtual Floppy Disk Image";
}

const char *ccvf_format::extensions() const
{
	return "ccvf";
}

const ccvf_format::format ccvf_format::file_formats[] = {
	{
		floppy_image::FF_35, floppy_image::SSSD,
		4000, 10, 41, 1, 128, {}, -1, { 0,5,1,6,2,7,3,8,4,9 }
	},
	{}
};

int ccvf_format::identify(io_generic *io, UINT32 form_factor)
{
	char h[36];

	io_generic_read(io, h, 0, 36);
	if(!memcmp(h, "Compucolor Virtual Floppy Disk Image", 36))
		return 100;

	return 0;
}

floppy_image_format_t::desc_e* ccvf_format::get_desc_8n1(const format &f, int &current_size)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, -1 },
		/* 01 */ {   RAWBITS, 0xffff, 3 },
		/* 02 */ {   RAWBYTE, 0xff, 3 },
		/* 03 */ {   CRC_FCS_START, 1 },
		/* 04 */ {     _8N1, 0x55, 1 },
		/* 05 */ {     TRACK_ID_8N1 },
		/* 06 */ {     SECTOR_ID_8N1 },
		/* 07 */ {   CRC_END, 1 },
		/* 08 */ {   CRC, 1 },
		/* 09 */ {   _8N1, 0xff, 3 },
		/* 10 */ {   CRC_FCS_START, 2 },
		/* 11 */ {     _8N1, 0x5a, 1 },
		/* 12 */ {     SECTOR_DATA_8N1, -1 },
		/* 13 */ {   CRC_END, 2 },
		/* 14 */ {   CRC, 2 },
		/* 15 */ { SECTOR_LOOP_END },
		/* 16 */ { RAWBYTE, 0xff, 0 },
		/* 17 */ { RAWBITS, 0xffff, 0 },
		/* 18 */ { END }
	};

	current_size = 120 + (1+1+1+2)*10 + 3*10 + (1+f.sector_base_size+2)*10;

	current_size *= f.sector_count;
	return desc;
}

int ccvf_format::compute_track_size(const format &f) const
{
	int track_size;
	if(f.sector_base_size)
		track_size = f.sector_base_size * f.sector_count;
	else {
		track_size = 0;
		for(int i=0; i != f.sector_count; i++)
			track_size += f.per_sector_size[i];
	}
	return track_size;
}

void ccvf_format::build_sector_description(const format &f, UINT8 *sectdata, desc_s *sectors) const
{
	if(f.sector_base_id == -1) {
		for(int i=0; i<f.sector_count; i++) {
			int cur_offset = 0;
			for(int j=0; j<f.sector_count; j++)
				if(f.per_sector_id[j] < f.per_sector_id[i])
					cur_offset += f.sector_base_size ? f.sector_base_size : f.per_sector_size[j];
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			sectors[i].sector_id = f.per_sector_id[i];
		}
	} else {
		int cur_offset = 0;
		for(int i=0; i<f.sector_count; i++) {
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			cur_offset += sectors[i].size;
			sectors[i].sector_id = i + f.sector_base_id;
		}
	}
}

bool ccvf_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	const format &f = formats[0];
	int current_size;
	floppy_image_format_t::desc_e *desc = get_desc_8n1(f, current_size);

	int total_size = 200000000/f.cell_size;
	int remaining_size = total_size - current_size;
	if(remaining_size < 0)
		throw emu_fatalerror("ccvf_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);

	// Fixup the end gap
	desc[16].p2 = remaining_size / 16;
	desc[17].p2 = remaining_size & 15;
	desc[17].p1 >>= 16-(remaining_size & 15);

	int track_size = compute_track_size(f);

	UINT8 sectdata[40*512];
	desc_s sectors[40];
	build_sector_description(f, sectdata, sectors);

	for(int track=0; track < f.track_count; track++)
		for(int head=0; head < f.head_count; head++) {
			io_generic_read(io, sectdata, (track*f.head_count + head)*track_size, track_size);
			generate_track(desc, track, head, sectors, f.sector_count, total_size, image);
		}

	image->set_variant(f.variant);

	return true;
}

void ccvf_format::extract_sectors(floppy_image *image, const format &f, desc_s *sdesc, int track, int head)
{
	// TODO
}

bool ccvf_format::save(io_generic *io, floppy_image *image)
{
	// TODO
	return false;
}

bool ccvf_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_CCVF_FORMAT = &floppy_image_format_creator<ccvf_format>;
