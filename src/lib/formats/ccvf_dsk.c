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
		(1./(9600*8))*1000000000, 10, 41, 1, 128, {}, 0, { 0,5,1,6,2,7,3,8,4,9 }
	},
	{}
};

int ccvf_format::identify(io_generic *io, UINT32 form_factor)
{
/*	char h[36];

	io_generic_read(io, h, 0, 36);
	if(!memcmp(h, "Compucolor Virtual Floppy Disk Image", 36))
		return 100;

	return 0;*/
	return 100;
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

bool ccvf_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	const format &f = formats[0];

	UINT64 size = io_generic_size(io);
	UINT8 *img = global_alloc_array(UINT8, size);
	io_generic_read(io, img, 0, size);

	int pos = 0;
	int total_size = 200000000/f.cell_size;
	
	for(int track=0; track < f.track_count; track++) {
		UINT32 *buffer = global_alloc_array_clear(UINT32, total_size);
		int offset = 0;
	
		for (int i=0; i<1920 && pos<size; i++, pos++) {
			for (int bit=7; bit>=0; bit--) {
				bit_w(buffer, offset++, BIT(img[pos], bit), f.cell_size);
			}
		}

		if (offset < total_size) {
			// pad the remainder of the track with sync
			int count = (total_size-offset);
			for (int i=0; i<count;i++) {
				bit_w(buffer, offset++, (track > 0) ? 1 : 0, f.cell_size);
			}
		}

		generate_track_from_levels(track, 0, buffer, total_size, 0, image);
		global_free(buffer);
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
