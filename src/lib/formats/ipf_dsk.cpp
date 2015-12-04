// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include <assert.h>
#include "ipf_dsk.h"

const floppy_format_type FLOPPY_IPF_FORMAT = &floppy_image_format_creator<ipf_format>;

ipf_format::ipf_format()
{
}

const char *ipf_format::name() const
{
	return "ipf";
}

const char *ipf_format::description() const
{
	return "SPS floppy disk image";
}

const char *ipf_format::extensions() const
{
	return "ipf";
}

bool ipf_format::supports_save() const
{
	return false;
}

int ipf_format::identify(io_generic *io, UINT32 form_factor)
{
	static const UINT8 refh[12] = { 0x43, 0x41, 0x50, 0x53, 0x00, 0x00, 0x00, 0x0c, 0x1c, 0xd5, 0x73, 0xba };
	UINT8 h[12];
	io_generic_read(io, h, 0, 12);

	if(!memcmp(h, refh, 12))
		return 100;

	return 0;
}

bool ipf_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT64 size = io_generic_size(io);
	dynamic_buffer data(size);
	io_generic_read(io, &data[0], 0, size);
	bool res = parse(data, image);
	return res;
}

UINT32 ipf_format::r32(const UINT8 *p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}


UINT32 ipf_format::rb(const UINT8 *&p, int count)
{
	UINT32 v = 0;
	for(int i=0; i<count; i++)
		v = (v << 8) | *p++;
	return v;
}

UINT32 ipf_format::crc32r(const UINT8 *data, UINT32 size)
{
	// Reversed crc32
	UINT32 crc = 0xffffffff;
	for(UINT32 i=0; i != size; i++) {
		crc = crc ^ data[i];
		for(int j=0; j<8; j++)
			if(crc & 1)
				crc = (crc >> 1) ^ 0xedb88320;
			else
				crc = crc >> 1;
	}
	return ~crc;
}

bool ipf_format::parse(dynamic_buffer &data, floppy_image *image)
{
	image->set_variant(floppy_image::DSDD); // Not handling anything else yet
	tcount = 84*2+1; // Usual max
	tinfos = global_alloc_array_clear(track_info, tcount);
	bool res = scan_all_tags(data);
	if(res)
		res = generate_tracks(image);
	global_free_array(tinfos);
	tinfos = nullptr;
	return res;
}

bool ipf_format::parse_info(const UINT8 *info)
{
	type = r32(info+12);
	if(type != 1)
		return false;
	encoder_type = r32(info+16); // 1 for CAPS, 2 for SPS
	encoder_revision = r32(info+20); // 1 always
	release = r32(info+24);
	revision = r32(info+28);
	origin = r32(info+32); // Original source reference
	min_cylinder = r32(info+36);
	max_cylinder = r32(info+40);
	min_head = r32(info+44);
	max_head = r32(info+48);
	credit_day = r32(info+52);  // year*1e4 + month*1e2 + day
	credit_time = r32(info+56); // hour*1e7 + min*1e5 + sec*1e3 + msec
	for(int i=0; i<4; i++)
		platform[i] = r32(info+60+4*i);
	for(int i=0; i<5; i++)
		extra[i] = r32(info+76+4*i);
	return true;
}

ipf_format::track_info *ipf_format::get_index(UINT32 idx)
{
	if(idx > 1000)
		return nullptr;
	if(idx >= tcount) {
		auto ti1 = global_alloc_array_clear(track_info, idx+1);
		memcpy(ti1, tinfos, tcount*sizeof(tinfos));
		global_free_array(tinfos);
		tcount = idx+1;
		tinfos = ti1;
	}

	return tinfos+idx;
}

bool ipf_format::parse_imge(const UINT8 *imge)
{
	track_info *t = get_index(r32(imge+64));
	if(!t)
		return false;

	t->info_set = true;

	t->cylinder = r32(imge+12);
	if(t->cylinder < min_cylinder || t->cylinder > max_cylinder)
		return false;

	t->head = r32(imge+16);
	if(t->head < min_head || t->head > max_head)
		return false;

	t->type = r32(imge+20);
	t->sigtype = r32(imge+24); // 1 for 2us cells, no other value valid
	t->size_bytes = r32(imge+28);
	t->index_bytes = r32(imge+32);
	t->index_cells = r32(imge+36);
	t->datasize_cells = r32(imge+40);
	t->gapsize_cells = r32(imge+44);
	t->size_cells = r32(imge+48);
	t->block_count = r32(imge+52);
	t->process = r32(imge+56); // encoder process, always 0
	t->weak_bits = r32(imge+60);
	t->reserved[0] = r32(imge+68);
	t->reserved[1] = r32(imge+72);
	t->reserved[2] = r32(imge+76);

	return true;
}

bool ipf_format::parse_data(const UINT8 *data, UINT32 &pos, UINT32 max_extra_size)
{
	track_info *t = get_index(r32(data+24));
	if(!t)
		return false;

	t->data_size_bits = r32(data+16);
	t->data = data+28;
	t->data_size = r32(data+12);
	if(t->data_size > max_extra_size)
		return false;
	if(crc32r(t->data, t->data_size) != r32(data+20))
		return false;
	pos += t->data_size;
	return true;
}

bool ipf_format::scan_one_tag(dynamic_buffer &data, UINT32 &pos, UINT8 *&tag, UINT32 &tsize)
{
	if(data.size()-pos < 12)
		return false;
	tag = &data[pos];
	tsize = r32(tag+4);
	if(data.size()-pos < tsize)
		return false;
	UINT32 crc = r32(tag+8);
	tag[8] = tag[9] = tag[10] = tag[11] = 0;
	if(crc32r(tag, tsize) != crc)
		return false;
	pos += tsize;
	return true;
}

bool ipf_format::scan_all_tags(dynamic_buffer &data)
{
	UINT32 pos = 0;
	UINT32 size = data.size();
	while(pos != size) {
		UINT8 *tag;
		UINT32 tsize;

		if(!scan_one_tag(data, pos, tag, tsize))
			return false;

		switch(r32(tag)) {
		case 0x43415053: // CAPS
			if(tsize != 12)
				return false;
			break;

		case 0x494e464f: // INFO
			if(tsize != 96)
				return false;
			if(!parse_info(tag))
				return false;
			break;

		case 0x494d4745: // IMGE
			if(tsize != 80)
				return false;
			if(!parse_imge(tag))
				return false;
			break;

		case 0x44415441: // DATA
			if(tsize != 28)
				return false;
			if(!parse_data(tag, pos, size-pos))
				return false;
			break;

		default:
			return false;
		}
	}
	return true;
}

bool ipf_format::generate_tracks(floppy_image *image)
{
	for(UINT32 i = 0; i != tcount; i++) {
		track_info *t = tinfos + i;
		if(t->info_set && t->data) {
			if(!generate_track(t, image))
				return false;

		} else if(t->info_set || t->data)
			return false;
	}
	return true;
}

void ipf_format::rotate(std::vector<UINT32> &track, UINT32 offset, UINT32 size)
{
	UINT32 done = 0;
	for(UINT32 bpos=0; done < size; bpos++) {
		UINT32 pos = bpos;
		UINT32 hold = track[pos];
		for(;;) {
			UINT32 npos = pos+offset;
			if(npos >= size)
				npos -= size;
			if(npos == bpos)
				break;
			track[pos] = track[npos];
			pos = npos;
			done++;
		}
		track[pos] = hold;
		done++;
	}
}

void ipf_format::mark_track_splice(std::vector<UINT32> &track, UINT32 offset, UINT32 size)
{
	for(int i=0; i<3; i++) {
		UINT32 pos = (offset + i) % size;
		UINT32 v = track[pos];
		if((v & floppy_image::MG_MASK) == MG_0)
			v = (v & floppy_image::TIME_MASK) | MG_1;
		else if((v & floppy_image::MG_MASK) == MG_1)
			v = (v & floppy_image::TIME_MASK) | MG_0;
		track[pos] = v;
	}
}

void ipf_format::timing_set(std::vector<UINT32> &track, UINT32 start, UINT32 end, UINT32 time)
{
	for(UINT32 i=start; i != end; i++)
		track[i] = (track[i] & floppy_image::MG_MASK) | time;
}

bool ipf_format::generate_timings(track_info *t, std::vector<UINT32> &track, const std::vector<UINT32> &data_pos, const std::vector<UINT32> &gap_pos)
{
	timing_set(track, 0, t->size_cells, 2000);

	switch(t->type) {
	case 2: break;

	case 3:
		if(t->block_count >= 4)
			timing_set(track, gap_pos[3], data_pos[4], 1890);
		if(t->block_count >= 5) {
			timing_set(track, data_pos[4], gap_pos[4], 1890);
			timing_set(track, gap_pos[4], data_pos[5], 1990);
		}
		if(t->block_count >= 6) {
			timing_set(track, data_pos[5], gap_pos[5], 1990);
			timing_set(track, gap_pos[5], data_pos[6], 2090);
		}
		if(t->block_count >= 7)
			timing_set(track, data_pos[6], gap_pos[6], 2090);
		break;

	case 4:
		timing_set(track, gap_pos[t->block_count-1], data_pos[0], 1890);
		timing_set(track, data_pos[0], gap_pos[0], 1890);
		timing_set(track, gap_pos[0], data_pos[1], 1990);
		if(t->block_count >= 2) {
			timing_set(track, data_pos[1], gap_pos[1], 1990);
			timing_set(track, gap_pos[1], data_pos[2], 2090);
		}
		if(t->block_count >= 3)
			timing_set(track, data_pos[2], gap_pos[2], 2090);
		break;

	case 5:
		if(t->block_count >= 6)
			timing_set(track, data_pos[5], gap_pos[5], 2100);
		break;

	case 6:
		if(t->block_count >= 2)
			timing_set(track, data_pos[1], gap_pos[1], 2200);
		if(t->block_count >= 3)
			timing_set(track, data_pos[2], gap_pos[2], 1800);
		break;

	case 7:
		if(t->block_count >= 2)
			timing_set(track, data_pos[1], gap_pos[1], 2100);
		break;

	case 8:
		if(t->block_count >= 2)
			timing_set(track, data_pos[1], gap_pos[1], 2200);
		if(t->block_count >= 3)
			timing_set(track, data_pos[2], gap_pos[2], 2100);
		if(t->block_count >= 5)
			timing_set(track, data_pos[4], gap_pos[4], 1900);
		if(t->block_count >= 6)
			timing_set(track, data_pos[5], gap_pos[5], 1800);
		if(t->block_count >= 7)
			timing_set(track, data_pos[6], gap_pos[6], 1700);
		break;

	case 9: {
		UINT32 mask = r32(t->data + 32*t->block_count + 12);
		for(UINT32 i=1; i<t->block_count; i++)
			timing_set(track, data_pos[i], gap_pos[i], mask & (1 << (i-1)) ? 1900 : 2100);
		break;
	}

	default:
		return false;
	}

	return true;
}

bool ipf_format::generate_track(track_info *t, floppy_image *image)
{
	if(!t->size_cells)
		return true;

	if(t->data_size < 32*t->block_count)
		return false;

	// Annoyingly enough, too small gaps are ignored, changing the
	// total track size.  Artifact stemming from the byte-only support
	// of old times?
	t->size_cells = block_compute_real_size(t);

	if(t->index_cells >= t->size_cells)
		return false;

	std::vector<UINT32> track(t->size_cells);
	std::vector<UINT32> data_pos(t->block_count+1);
	std::vector<UINT32> gap_pos(t->block_count);
	std::vector<UINT32> splice_pos(t->block_count);

	bool context = false;
	UINT32 pos = 0;
	for(UINT32 i = 0; i != t->block_count; i++) {
		if(!generate_block(t, i, i == t->block_count-1 ? t->size_cells - t->index_cells : 0xffffffff, track, pos, data_pos[i], gap_pos[i], splice_pos[i], context)) {
			return false;
		}
	}
	if(pos != t->size_cells) {
		return false;
	}

	data_pos[t->block_count] = pos;

	mark_track_splice(track, splice_pos[t->block_count-1], t->size_cells);

	if(!generate_timings(t, track, data_pos, gap_pos)) {
		return false;
	}

	if(t->index_cells)
		rotate(track, t->size_cells - t->index_cells, t->size_cells);

	generate_track_from_levels(t->cylinder, t->head, track, splice_pos[t->block_count-1] + t->index_cells, image);

	return true;
}

void ipf_format::track_write_raw(std::vector<UINT32>::iterator &tpos, const UINT8 *data, UINT32 cells, bool &context)
{
	for(UINT32 i=0; i != cells; i++)
		*tpos++ = data[i>>3] & (0x80 >> (i & 7)) ? MG_1 : MG_0;
	if(cells)
		context = tpos[-1] == MG_1;
}

void ipf_format::track_write_mfm(std::vector<UINT32>::iterator &tpos, const UINT8 *data, UINT32 start_offset, UINT32 patlen, UINT32 cells, bool &context)
{
	patlen *= 2;
	for(UINT32 i=0; i != cells; i++) {
		UINT32 pos = (i + start_offset) % patlen;
		bool bit = data[pos>>4] & (0x80 >> ((pos >> 1) & 7));
		if(pos & 1) {
			*tpos++ = bit ? MG_1 : MG_0;
			context = bit;
		} else
			*tpos++ = context || bit ? MG_0 : MG_1;
	}
}

void ipf_format::track_write_weak(std::vector<UINT32>::iterator &tpos, UINT32 cells)
{
	for(UINT32 i=0; i != cells; i++)
		*tpos++ = floppy_image::MG_N;
}

bool ipf_format::generate_block_data(const UINT8 *data, const UINT8 *dlimit, std::vector<UINT32>::iterator tpos, std::vector<UINT32>::iterator tlimit, bool &context)
{
	for(;;) {
		if(data >= dlimit)
			return false;
		UINT8 val = *data++;
		if((val >> 5) > dlimit-data)
			return false;
		UINT32 param = rb(data, val >> 5);
		UINT32 tleft = tlimit - tpos;
		switch(val & 0x1f) {
		case 0: // End of description
			return !tleft;

		case 1: // Raw bytes
			if(8*param > tleft)
				return false;
			track_write_raw(tpos, data, 8*param, context);
			data += param;
			break;

		case 2: // MFM-decoded data bytes
		case 3: // MFM-decoded gap bytes
			if(16*param > tleft)
				return false;
			track_write_mfm(tpos, data, 0, 8*param, 16*param, context);
			data += param;
			break;

		case 5: // Weak bytes
			if(16*param > tleft)
				return false;
			track_write_weak(tpos, 16*param);
			context = 0;
			break;

		default:
			return false;
		}
	}
}

bool ipf_format::generate_block_gap_0(UINT32 gap_cells, UINT8 pattern, UINT32 &spos, UINT32 ipos, std::vector<UINT32>::iterator &tpos, bool &context)
{
	spos = ipos >= 16 && ipos+16 <= gap_cells ? ipos : gap_cells >> 1;
	track_write_mfm(tpos, &pattern, 0, 8, spos, context);
	UINT32 delta = 0;
	if(gap_cells & 1) {
		*tpos++ = MG_0;
		delta++;
	}
	track_write_mfm(tpos, &pattern, spos+delta-gap_cells, 8, gap_cells-spos-delta, context);
	return true;
}

bool ipf_format::gap_description_to_reserved_size(const UINT8 *&data, const UINT8 *dlimit, UINT32 &res_size)
{
	res_size = 0;
	for(;;) {
		if(data >= dlimit)
			return false;
		UINT8 val = *data++;
		if((val >> 5) > dlimit-data)
			return false;
		UINT32 param = rb(data, val >> 5);
		switch(val & 0x1f) {
		case 0:
			return true;
		case 1:
			res_size += param*2;
			break;
		case 2:
			data += (param+7)/8;
			break;
		default:
			return false;
		}
	}
}

bool ipf_format::generate_gap_from_description(const UINT8 *&data, const UINT8 *dlimit, std::vector<UINT32>::iterator tpos, UINT32 size, bool pre, bool &context)
{
	const UINT8 *data1 = data;
	UINT32 res_size;
	if(!gap_description_to_reserved_size(data1, dlimit, res_size))
		return false;

	if(res_size > size)
		return false;
	UINT8 pattern[16];
	memset(pattern, 0, sizeof(pattern));
	UINT32 pattern_size = 0;

	UINT32 pos = 0, block_size = 0;
	for(;;) {
		UINT8 val = *data++;
		UINT32 param = rb(data, val >> 5);
		switch(val & 0x1f) {
		case 0:
			return size == pos;

		case 1:
			if(block_size)
				return false;
			block_size = param*2;
			pattern_size = 0;
			break;

		case 2:
			// You can't have a pattern at the start of a pre-slice
			// gap if there's a size afterwards
			if(pre && res_size && !block_size)
				return false;
			// You can't have two consecutive patterns
			if(pattern_size)
				return false;
			pattern_size = param;
			if(pattern_size > sizeof(pattern)*8)
				return false;

			memcpy(pattern, data, (pattern_size+7)/8);
			data += (pattern_size+7)/8;
			if(pre) {
				if(!block_size)
					block_size = size;
				else if(pos + block_size == res_size)
					block_size = size - pos;
				if(pos + block_size > size)
					return false;
				//              printf("pat=%02x size=%d pre\n", pattern[0], block_size);
				track_write_mfm(tpos, pattern, 0, pattern_size, block_size, context);
				pos += block_size;
			} else {
				if(pos == 0 && block_size && res_size != size)
					block_size = size - (res_size-block_size);
				if(!block_size)
					block_size = size - res_size;
				if(pos + block_size > size)
					return false;
				//              printf("pat=%02x block_size=%d size=%d res_size=%d post\n", pattern[0], block_size, size, res_size);
				track_write_mfm(tpos, pattern, -block_size, pattern_size, block_size, context);
				pos += block_size;
			}
			block_size = 0;
			break;
		}
	}
}


bool ipf_format::generate_block_gap_1(UINT32 gap_cells, UINT32 &spos, UINT32 ipos, const UINT8 *data, const UINT8 *dlimit, std::vector<UINT32>::iterator &tpos, bool &context)
{
	if(ipos >= 16 && ipos < gap_cells-16)
		spos = ipos;
	else
		spos = 0;
	return generate_gap_from_description(data, dlimit, tpos, gap_cells, true, context);
}

bool ipf_format::generate_block_gap_2(UINT32 gap_cells, UINT32 &spos, UINT32 ipos, const UINT8 *data, const UINT8 *dlimit, std::vector<UINT32>::iterator &tpos, bool &context)
{
	if(ipos >= 16 && ipos < gap_cells-16)
		spos = ipos;
	else
		spos = gap_cells;
	return generate_gap_from_description(data, dlimit, tpos, gap_cells, false, context);
}

bool ipf_format::generate_block_gap_3(UINT32 gap_cells, UINT32 &spos, UINT32 ipos, const UINT8 *data, const UINT8 *dlimit, std::vector<UINT32>::iterator &tpos,  bool &context)
{
	if(ipos >= 16 && ipos < gap_cells-16)
		spos = ipos;
	else {
		UINT32 presize, postsize;
		const UINT8 *data1 = data;
		if(!gap_description_to_reserved_size(data1, dlimit, presize))
			return false;
		if(!gap_description_to_reserved_size(data1, dlimit, postsize))
			return false;
		if(presize+postsize > gap_cells)
			return false;

		spos = presize + (gap_cells - presize - postsize)/2;
	}
	if(!generate_gap_from_description(data, dlimit, tpos, spos, true, context))
		return false;
	UINT32 delta = 0;
	if(gap_cells & 1) {
		tpos[spos] = MG_0;
		delta++;
	}

	return generate_gap_from_description(data, dlimit, tpos+spos+delta, gap_cells - spos - delta, false, context);
}

bool ipf_format::generate_block_gap(UINT32 gap_type, UINT32 gap_cells, UINT8 pattern, UINT32 &spos, UINT32 ipos, const UINT8 *data, const UINT8 *dlimit, std::vector<UINT32>::iterator tpos, bool &context)
{
	switch(gap_type) {
	case 0:
		return generate_block_gap_0(gap_cells, pattern, spos, ipos, tpos, context);
	case 1:
		return generate_block_gap_1(gap_cells, spos, ipos, data, dlimit, tpos, context);
	case 2:
		return generate_block_gap_2(gap_cells, spos, ipos, data, dlimit, tpos, context);
	case 3:
		return generate_block_gap_3(gap_cells, spos, ipos, data, dlimit, tpos, context);
	default:
		return false;
	}
}

bool ipf_format::generate_block(track_info *t, UINT32 idx, UINT32 ipos, std::vector<UINT32> &track, UINT32 &pos, UINT32 &dpos, UINT32 &gpos, UINT32 &spos, bool &context)
{
	const UINT8 *data = t->data;
	const UINT8 *data_end = t->data + t->data_size;
	const UINT8 *thead = data + 32*idx;
	UINT32 data_cells = r32(thead);
	UINT32 gap_cells = r32(thead+4);

	if(gap_cells < 8)
		gap_cells = 0;

	// +8  = gap description offset / datasize in bytes (when gap type = 0)
	// +12 =                      1 / gap size in bytes (when gap type = 0)
	// +16 = 1
	// +20 = gap type
	// +24 = type 0 gap pattern (8 bits) / speed mask for sector 0 track type 9
	// +28 = data description offset

	dpos = pos;
	gpos = dpos + data_cells;
	pos = gpos + gap_cells;
	if(pos > t->size_cells)
		return false;
	if(!generate_block_data(data + r32(thead+28), data_end, track.begin()+dpos, track.begin()+gpos, context))
		return false;
	if(!generate_block_gap(r32(thead+20), gap_cells, r32(thead+24), spos, ipos > gpos ? ipos-gpos : 0, data + r32(thead+8), data_end, track.begin()+gpos, context))
		return false;
	spos += gpos;

	return true;
}

UINT32 ipf_format::block_compute_real_size(track_info *t)
{
	UINT32 size = 0;
	const UINT8 *thead = t->data;
	for(unsigned int i=0; i != t->block_count; i++) {
		UINT32 data_cells = r32(thead);
		UINT32 gap_cells = r32(thead+4);
		if(gap_cells < 8)
			gap_cells = 0;

		size += data_cells + gap_cells;
		thead += 32;
	}
	return size;
}
