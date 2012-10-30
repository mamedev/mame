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
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if(size == compute_track_size(f) * f.track_count * f.head_count)
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

int upd765_format::compute_track_size(const format &f) const
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

void upd765_format::build_sector_description(const format &f, UINT8 *sectdata, desc_s *sectors) const
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

bool upd765_format::supports_save() const
{
	return true;
}

bool upd765_format::save(io_generic *io, floppy_image *image)
{
	// Count the number of formats
	int formats_count;
	for(formats_count=0; formats[formats_count].form_factor; formats_count++);

	// Allocate the storage for the list of testable formats for a
	// given cell size
	int *candidates = global_alloc_array(int, formats_count);

	// Format we're finally choosing
	int chosen_candidate = -1;

	// Previously tested cell size
	int min_cell_size = 0;
	for(;;) {
		// Build the list of all formats for the immediatly superior cell size
		int cur_cell_size = 0;
		int candidates_count = 0;
		for(int i=0; i != formats_count; i++) {
			if(image->get_form_factor() == floppy_image::FF_UNKNOWN ||
			   image->get_form_factor() == formats[i].form_factor) {
				if(formats[i].cell_size == cur_cell_size)
					candidates[candidates_count++] = i;
				else if((!cur_cell_size || formats[i].cell_size < cur_cell_size) &&
						formats[i].cell_size > min_cell_size) {
					candidates[0] = i;
					candidates_count = 1;
					cur_cell_size = formats[i].cell_size;
				}
			}
		}

		min_cell_size = cur_cell_size;

		// No candidates with a cell size bigger than the previously
		// tested one, we're done
		if(!candidates_count)
			break;

		// Filter with track 0 head 0
		check_compatibility(image, candidates, candidates_count);

		// Nobody matches, try with the next cell size
		if(!candidates_count)
			continue;

		// We have a match at that cell size, we just need to find the
		// best one given the geometry

		// If there's only one, we're done
		if(candidates_count == 1) {
			chosen_candidate = candidates[0];
			break;
		}

		// Otherwise, find the best
		int tracks, heads;
		image->get_actual_geometry(tracks, heads);
		chosen_candidate = candidates[0];
		for(int i=1; i != candidates_count; i++) {
			const format &cc = formats[chosen_candidate];
			const format &cn = formats[candidates[i]];

			// Handling enough sides is better than not
			if(cn.head_count >= heads && cc.head_count < heads)
				goto change;
			else if(cc.head_count >= heads && cn.head_count < heads)
				goto dont_change;

			// Since we're limited to two heads, at that point head
			// count is identical for both formats.

			// Handling enough tracks is better than not
			if(cn.track_count >= tracks && cc.track_count < tracks)
				goto change;
			else if(cn.track_count >= tracks && cc.track_count < tracks)
				goto dont_change;

			// Both are on the same side of the track count, so closest is best
			if(cc.track_count < tracks && cn.track_count > cc.track_count)
				goto change;
			if(cc.track_count >= tracks && cn.track_count < cc.track_count)
				goto change;
			goto dont_change;

		change:
			chosen_candidate = candidates[i];
		dont_change:
			;
		}
		// We have a winner, bail out
		break;
	}

	// No match, pick the first one and be done with it
	if(chosen_candidate == -1)
		chosen_candidate = 0;


	const format &f = formats[chosen_candidate];
	int track_size = compute_track_size(f);

	UINT8 sectdata[40*512];
	desc_s sectors[40];
	build_sector_description(f, sectdata, sectors);

	for(int track=0; track < f.track_count; track++)
		for(int head=0; head < f.head_count; head++) {
			extract_sectors(image, f, sectors, track, head);
			io_generic_write(io, sectdata, (track*f.head_count + head)*track_size, track_size);
		}

	return true;
}

void upd765_format::check_compatibility(floppy_image *image, int *candidates, int &candidates_count)
{
	UINT8 bitstream[500000/8];
	UINT8 sectdata[50000];
	desc_xs sectors[256];
	int track_size;

	// Extract the sectors
	generate_bitstream_from_track(0, 0, formats[candidates[0]].cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sectdata, sizeof(sectdata));

	// Check compatibility with every candidate, copy in-place
	int *ok_cands = candidates;
	for(int i=0; i != candidates_count; i++) {
		const format &f = formats[candidates[i]];
		int ns = 0;
		for(int j=0; j<256; j++)
			if(sectors[j].data) {
				int sid;
				if(f.sector_base_id == -1) {
					for(sid=0; sid < f.sector_count; sid++)
						if(f.per_sector_id[sid] == j)
							break;
				} else
					sid = j - f.sector_base_id;
				if(sid < 0 || sid > f.sector_count)
					goto fail;
				if(f.sector_base_size) {
					if(sectors[j].size != f.sector_base_size)
						goto fail;
				} else {
					if(sectors[j].size != f.per_sector_size[sid])
						goto fail;
				}
				ns++;
			}
		if(ns == f.sector_count)
			*ok_cands++ = candidates[i];
	fail:
		;
	}
	candidates_count = ok_cands - candidates;
}


void upd765_format::extract_sectors(floppy_image *image, const format &f, desc_s *sdesc, int track, int head)
{
	UINT8 bitstream[500000/8];
	UINT8 sectdata[50000];
	desc_xs sectors[256];
	int track_size;

	// Extract the sectors
	generate_bitstream_from_track(track, head, f.cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sectdata, sizeof(sectdata));

	for(int i=0; i<f.sector_count; i++) {
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
