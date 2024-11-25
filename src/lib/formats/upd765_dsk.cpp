// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/upd765_dsk.cpp

    helper for simple upd765-formatted disk images

*********************************************************************/

#include "formats/upd765_dsk.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*

#include <cstring>


upd765_format::upd765_format(const format *_formats) : formats(_formats)
{
}

int upd765_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return -1;

	for(int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if(form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;
		if(!variants.empty() && !has_variant(variants, f.variant))
			continue;

		if(size == compute_track_size(f) * f.track_count * f.head_count)
			return i;
	}
	return -1;
}

int upd765_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if(type != -1)
		return FIFID_SIZE;
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

void upd765_format::build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const
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

floppy_image_format_t::desc_e* upd765_format::get_desc_fm(const format &f, int &current_size, int &end_gap_index) const
{
	static floppy_image_format_t::desc_e desc[26] = {
		/* 00 */ { FM, 0xff, f.gap_4a },
		/* 01 */ { FM, 0x00, 6 },
		/* 02 */ { RAW, 0xf77a, 1 },
		/* 03 */ { FM, 0xff, f.gap_1 },
		/* 04 */ { SECTOR_LOOP_START, 0, f.sector_count-1 },
		/* 05 */ {   FM, 0x00, 12 },
		/* 06 */ {   CRC_CCITT_FM_START, 1 },
		/* 07 */ {     RAW, 0xf57e, 1 },
		/* 08 */ {     TRACK_ID_FM },
		/* 09 */ {     HEAD_ID_FM },
		/* 10 */ {     SECTOR_ID_FM },
		/* 11 */ {     SIZE_ID_FM },
		/* 12 */ {   CRC_END, 1 },
		/* 13 */ {   CRC, 1 },
		/* 14 */ {   FM, 0xff, f.gap_2 },
		/* 15 */ {   FM, 0x00, 6 },
		/* 16 */ {   CRC_CCITT_FM_START, 2 },
		/* 17 */ {     RAW, 0xf56f, 1 },
		/* 18 */ {     SECTOR_DATA_FM, -1 },
		/* 19 */ {   CRC_END, 2 },
		/* 20 */ {   CRC, 2 },
		/* 21 */ {   FM, 0xff, f.gap_3 },
		/* 22 */ { SECTOR_LOOP_END },
		/* 23 */ { FM, 0xff, 0 },
		/* 24 */ { RAWBITS, 0xffff, 0 },
		/* 25 */ { END }
	};

	current_size = (f.gap_4a+6+1+f.gap_1)*16;
	if(f.sector_base_size)
		current_size += f.sector_base_size * f.sector_count * 16;
	else {
		for(int j=0; j != f.sector_count; j++)
			current_size += f.per_sector_size[j] * 16;
	}
	current_size += (12+1+4+2+f.gap_2+6+1+2+f.gap_3) * f.sector_count * 16;

	end_gap_index = 23;

	return desc;
}

floppy_image_format_t::desc_e* upd765_format::get_desc_mfm(const format &f, int &current_size, int &end_gap_index) const
{
	static floppy_image_format_t::desc_e desc[29] = {
		/* 00 */ { MFM, 0x4e, 0 },
		/* 01 */ { MFM, 0x00, 12 },
		/* 02 */ { RAW, 0x5224, 3 },
		/* 03 */ { MFM, 0xfc, 1 },
		/* 04 */ { MFM, 0x4e, 0 },
		/* 05 */ { SECTOR_LOOP_START, 0, 0 },
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
		/* 16 */ {   MFM, 0x4e, 0 },
		/* 17 */ {   MFM, 0x00, 12 },
		/* 18 */ {   CRC_CCITT_START, 2 },
		/* 19 */ {     RAW, 0x4489, 3 },
		/* 20 */ {     MFM, 0xfb, 1 },
		/* 21 */ {     SECTOR_DATA, -1 },
		/* 22 */ {   CRC_END, 2 },
		/* 23 */ {   CRC, 2 },
		/* 24 */ {   MFM, 0x4e, 0 },
		/* 25 */ { SECTOR_LOOP_END },
		/* 26 */ { MFM, 0x4e, 0 },
		/* 27 */ { RAWBITS, 0x9254, 0 },
		/* 28 */ { END }
	};

	desc[0].p2  = f.gap_4a;
	desc[4].p2  = f.gap_1;
	desc[5].p2  = f.sector_count-1;
	desc[16].p2 = f.gap_2;
	desc[24].p2 = f.gap_3;

	current_size = (f.gap_4a+12+3+1+f.gap_1)*16;
	if(f.sector_base_size)
		current_size += f.sector_base_size * f.sector_count * 16;
	else {
		for(int j=0; j != f.sector_count; j++)
			current_size += f.per_sector_size[j] * 16;
	}
	current_size += (12+3+1+4+2+f.gap_2+12+3+1+2+f.gap_3) * f.sector_count * 16;

	end_gap_index = 26;

	return desc;
}

bool upd765_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	int type = find_size(io, form_factor, variants);
	if(type == -1)
		return false;

	// format shouldn't exceed image geometry
	const format &f = formats[type];
	int img_tracks, img_heads;
	image.get_maximal_geometry(img_tracks, img_heads);
	if (f.track_count > img_tracks || f.head_count > img_heads)
		return false;

	floppy_image_format_t::desc_e *desc;
	int current_size;
	int end_gap_index;

	switch(f.encoding) {
	case floppy_image::FM:
		desc = get_desc_fm(f, current_size, end_gap_index);
		break;
	case floppy_image::MFM:
	default:
		desc = get_desc_mfm(f, current_size, end_gap_index);
		break;
	}

	int total_size = 200000000/f.cell_size;
	int remaining_size = total_size - current_size;
	if(remaining_size < 0) {
		osd_printf_error("upd765_format: Incorrect track layout, max_size=%d, current_size=%d\n", total_size, current_size);
		return false;
	}

	// Fixup the end gap
	desc[end_gap_index].p2 = remaining_size / 16;
	desc[end_gap_index + 1].p2 = remaining_size & 15;
	desc[end_gap_index + 1].p1 >>= 16-(remaining_size & 15);

	int track_size = compute_track_size(f);

	uint8_t sectdata[40*512];
	desc_s sectors[40];

	for(int track=0; track < f.track_count; track++)
		for(int head=0; head < f.head_count; head++) {
			build_sector_description(f, sectdata, sectors, track, head);
			/*auto const [err, actual] =*/ read_at(io, (track*f.head_count + head)*track_size, sectdata, track_size); // FIXME: check for errors and premature EOF
			generate_track(desc, track, head, sectors, f.sector_count, total_size, image);
		}

	image.set_form_variant(f.form_factor, f.variant);

	return true;
}

bool upd765_format::supports_save() const noexcept
{
	return true;
}

bool upd765_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	// Count the number of formats
	int formats_count;
	for(formats_count=0; formats[formats_count].form_factor; formats_count++) {};

	// Allocate the storage for the list of testable formats for a
	// given cell size
	std::vector<int> candidates;

	// Format we're finally choosing
	int chosen_candidate = -1;

	// Previously tested cell size
	int min_cell_size = 0;
	for(;;) {
		// Build the list of all formats for the immediately superior cell size
		int cur_cell_size = 0;
		candidates.clear();
		for(int i=0; i != formats_count; i++) {
			if(image.get_form_factor() == floppy_image::FF_UNKNOWN ||
				image.get_form_factor() == formats[i].form_factor) {
				if(formats[i].cell_size == cur_cell_size)
					candidates.push_back(i);
				else if((!cur_cell_size || formats[i].cell_size < cur_cell_size) &&
						formats[i].cell_size > min_cell_size) {
					candidates.clear();
					candidates.push_back(i);
					cur_cell_size = formats[i].cell_size;
				}
			}
		}

		min_cell_size = cur_cell_size;

		// No candidates with a cell size bigger than the previously
		// tested one, we're done
		if(candidates.empty())
			break;

		// Filter with track 0 head 0
		check_compatibility(image, candidates);

		// Nobody matches, try with the next cell size
		if(candidates.empty())
			continue;

		// We have a match at that cell size, we just need to find the
		// best one given the geometry

		// If there's only one, we're done
		if(candidates.size() == 1) {
			chosen_candidate = candidates[0];
			break;
		}

		// Otherwise, find the best
		int tracks, heads;
		image.get_actual_geometry(tracks, heads);
		chosen_candidate = candidates[0];
		for(unsigned int i=1; i != candidates.size(); i++) {
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
			else if(cc.track_count >= tracks && cn.track_count < tracks)
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

	uint8_t sectdata[40*512];
	desc_s sectors[40];

	for(int track=0; track < f.track_count; track++) {
		for(int head=0; head < f.head_count; head++) {
			build_sector_description(f, sectdata, sectors, track, head);
			extract_sectors(image, f, sectors, track, head);
			/*auto const [err, actual] =*/ write_at(io, (track*f.head_count + head)*track_size, sectdata, track_size); // FIXME: check for errors
		}
	}

	return true;
}

void upd765_format::check_compatibility(const floppy_image &image, std::vector<int> &candidates) const
{
	// Extract the sectors
	auto bitstream = generate_bitstream_from_track(0, 0, formats[candidates[0]].cell_size, image);
	std::vector<std::vector<uint8_t>> sectors;

	switch (formats[candidates[0]].encoding)
	{
	case floppy_image::FM:
		sectors = extract_sectors_from_bitstream_fm_pc(bitstream);
		break;
	case floppy_image::MFM:
		sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
		break;
	}

	// Check compatibility with every candidate, copy in-place
	int *ok_cands = &candidates[0];
	for(unsigned int i=0; i != candidates.size(); i++) {
		const format &f = formats[candidates[i]];
		int ns = 0;
		for(unsigned int j=0; j != sectors.size(); j++)
			if(!sectors[j].empty()) {
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
					if(sectors[j].size() != f.sector_base_size)
						goto fail;
				} else {
					if(sectors[j].size() != f.per_sector_size[sid])
						goto fail;
				}
				ns++;
			}
		if(ns == f.sector_count)
			*ok_cands++ = candidates[i];
	fail:
		;
	}
	candidates.resize(ok_cands - &candidates[0]);
}


void upd765_format::extract_sectors(const floppy_image &image, const format &f, desc_s *sdesc, int track, int head) const
{
	// Extract the sectors
	auto bitstream = generate_bitstream_from_track(track, head, f.cell_size, image);
	std::vector<std::vector<uint8_t>> sectors;

	switch (f.encoding)
	{
	case floppy_image::FM:
		sectors = extract_sectors_from_bitstream_fm_pc(bitstream);
		break;
	case floppy_image::MFM:
		sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
		break;
	}

	for(int i=0; i<f.sector_count; i++) {
		desc_s &ds = sdesc[i];
		if(ds.sector_id >= sectors.size() || sectors[ds.sector_id].empty())
			memset((void *)ds.data, 0, ds.size);

		else if(sectors[ds.sector_id].size() < ds.size) {
			memcpy((void *)ds.data, sectors[ds.sector_id].data(), sectors[ds.sector_id].size());
			memset((uint8_t *)ds.data + sectors[ds.sector_id].size(), 0, ds.size - sectors[ds.sector_id].size());

		} else
			memcpy((void *)ds.data, sectors[ds.sector_id].data(), ds.size);
	}
}
