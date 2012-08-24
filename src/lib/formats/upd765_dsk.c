/***************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

/*********************************************************************

    formats/upd765_dsk.h

    helper for simple upd765-formatted disk images

*********************************************************************/

#include "emu.h"
#include "formats/upd765_dsk.h"

upd765_format::upd765_format(const format *_formats)
{
	formats = _formats;
}

int upd765_format::find_size(io_generic *io, UINT32 form_factor)
{
	int size = io_generic_size(io);
	for(int i=0; formats[i].form_factor; i++) {
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != formats[i].form_factor)
			continue;

		int format_size;
		if(formats[i].sector_base_size)
			format_size = formats[i].sector_base_size * formats[i].sector_count;
		else {
			format_size = 0;
			for(int j=0; j != formats[i].sector_count; j++)
				format_size += formats[i].per_sector_size[j];
		}

		format_size *= formats[i].track_count * formats[i].head_count;

		if(size == format_size)
			return i;
	}
	return -1;
}

int upd765_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 50;
	return 0;
}

bool upd765_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int type = find_size(io, form_factor);
	if(type == -1)
		return false;

	const format &f = formats[type];

	floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { MFM, 0x4e, f.gap_4a },
		/* 01 */ { MFM, 0x00, 12 },
		/* 02 */ { RAW, 0x5224, 3 },
		/* 03 */ { MFM, 0xfc, 1 },
		/* 04 */ { MFM, 0x4e, f.gap_1 },
		/* 05 */ { SECTOR_LOOP_START, 0, f.sector_count-1 },
		/* 06 */ {   MFM, 0x00, 12 },
		/* 07 */ {   CRC_CCITT_START, 1 },
		/* 08 */ {     RAW, 0x4489, 3 },
		/* 09 */ {     MFM, 0xfe, 1 },
		/* 10 */ {     TRACK_ID },
		/* 11 */ {     HEAD_ID },
		/* 12 */ {     SECTOR_ID },
		/* 13 */ {     SIZE_ID },
		/* 14 */ {   CRC_END, 1 },
		/* 15 */ {   CRC, 1 },
		/* 16 */ {   MFM, 0x4e, f.gap_2 },
		/* 17 */ {   MFM, 0x00, 12 },
		/* 18 */ {   CRC_CCITT_START, 2 },
		/* 19 */ {     RAW, 0x4489, 3 },
		/* 20 */ {     MFM, 0xfb, 1 },
		/* 21 */ {     SECTOR_DATA, -1 },
		/* 22 */ {   CRC_END, 2 },
		/* 23 */ {   CRC, 2 },
		/* 24 */ {   MFM, 0x4e, f.gap_3 },
		/* 25 */ { SECTOR_LOOP_END },
		/* 26 */ { MFM, 0x4e, 0 },
		/* 27 */ { RAWBITS, 0x9254, 0 },
		/* 28 */ { END }
	};

	int current_size = (f.gap_4a+12+3+1+f.gap_1)*16;
	if(f.sector_base_size)
		current_size += f.sector_base_size * f.sector_count * 16;
	else {
		for(int j=0; j != f.sector_count; j++)
			current_size += f.per_sector_size[j] * 16;
	}
	current_size += (12+3+1+4+2+f.gap_2+12+3+1+2+f.gap_3) * f.sector_count * 16;

	int total_size = 200000000/f.cell_size;
	int remaining_size = total_size - current_size;
	if(remaining_size < 0)
		throw emu_fatalerror("upd765_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);

	// Fixup the end gap
	desc[26].p2 = remaining_size / 16;
	desc[27].p2 = remaining_size & 15;
	desc[27].p1 >>= 16-(remaining_size & 15);

	int track_size;
	if(f.sector_base_size)
		track_size = f.sector_base_size * f.sector_count;
	else {
		track_size = 0;
		for(int i=0; i != f.sector_count; i++)
			track_size += f.per_sector_size[i];
	}

	UINT8 sectdata[40*512];
	desc_s sectors[40];
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

	for(int track=0; track < f.track_count; track++)
		for(int head=0; head < f.head_count; head++) {
			io_generic_read(io, sectdata, (track*f.head_count + head)*track_size, track_size);
			generate_track(desc, track, head, sectors, f.sector_count, total_size, image);
		}

	image->set_variant(f.variant);

	return true;
}


bool upd765_format::save(io_generic *io, floppy_image *image)
{
	return true;
}

bool upd765_format::supports_save() const
{
	return true;
}

