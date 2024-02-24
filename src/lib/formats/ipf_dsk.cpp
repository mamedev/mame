// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "ipf_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <cstring>


struct ipf_format::ipf_decode {
	struct track_info {
		uint32_t cylinder = 0, head = 0, type = 0;
		uint32_t sigtype = 0, process = 0, reserved[3] = { 0, 0, 0 };
		uint32_t size_bytes = 0, size_cells = 0;
		uint32_t index_bytes = 0, index_cells = 0;
		uint32_t datasize_cells = 0, gapsize_cells = 0;
		uint32_t block_count = 0, weak_bits = 0;

		uint32_t data_size_bits = 0;

		bool info_set = false;

		const uint8_t *data = nullptr;
		uint32_t data_size = 0;
	};

	std::vector<track_info> tinfos;
	uint32_t tcount = 0;

	uint32_t type = 0, release = 0, revision = 0;
	uint32_t encoder_type = 0, encoder_revision = 0, origin = 0;
	uint32_t min_cylinder = 0, max_cylinder = 0, min_head = 0, max_head = 0;
	uint32_t credit_day = 0, credit_time = 0;
	uint32_t platform[4] = {}, extra[5] = {};

	uint32_t crc32r(const uint8_t *data, uint32_t size);

	bool parse_info(const uint8_t *info);
	bool parse_imge(const uint8_t *imge);
	bool parse_data(const uint8_t *data, uint32_t &pos, uint32_t max_extra_size);

	bool scan_one_tag(uint8_t *data, size_t size, uint32_t &pos, uint8_t *&tag, uint32_t &tsize);
	bool scan_all_tags(uint8_t *data, size_t size);
	static uint32_t rb(const uint8_t *&p, int count);

	track_info *get_index(uint32_t idx);

	void track_write_raw(std::vector<uint32_t>::iterator &tpos, const uint8_t *data, uint32_t cells, bool &context);
	void track_write_mfm(std::vector<uint32_t>::iterator &tpos, const uint8_t *data, uint32_t start_offset, uint32_t patlen, uint32_t cells, bool &context);
	void track_write_weak(std::vector<uint32_t>::iterator &tpos, uint32_t cells);
	bool generate_block_data(const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, std::vector<uint32_t>::iterator tlimit, bool &context);

	bool gap_description_to_reserved_size(const uint8_t *&data, const uint8_t *dlimit, uint32_t &res_size);
	bool generate_gap_from_description(const uint8_t *&data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, uint32_t size, bool pre, bool &context);
	bool generate_block_gap_0(uint32_t gap_cells, uint8_t pattern, uint32_t &spos, uint32_t ipos, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap_1(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap_2(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap_3(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap(uint32_t gap_type, uint32_t gap_cells, uint8_t pattern, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, bool &context);

	bool generate_block(const track_info &t, uint32_t idx, uint32_t ipos, std::vector<uint32_t> &track, uint32_t &pos, uint32_t &dpos, uint32_t &gpos, uint32_t &spos, bool &context);
	uint32_t block_compute_real_size(const track_info &t);

	void timing_set(std::vector<uint32_t> &track, uint32_t start, uint32_t end, uint32_t time);
	bool generate_timings(const track_info &t, std::vector<uint32_t> &track, const std::vector<uint32_t> &data_pos, const std::vector<uint32_t> &gap_pos);

	void rotate(std::vector<uint32_t> &track, uint32_t offset, uint32_t size);
	void mark_track_splice(std::vector<uint32_t> &track, uint32_t offset, uint32_t size);
	bool generate_track(track_info &t, floppy_image &image);
	bool generate_tracks(floppy_image &image);

	bool parse(uint8_t *data, size_t size, floppy_image &image);
};


const ipf_format FLOPPY_IPF_FORMAT;

const char *ipf_format::name() const noexcept
{
	return "ipf";
}

const char *ipf_format::description() const noexcept
{
	return "SPS floppy disk image";
}

const char *ipf_format::extensions() const noexcept
{
	return "ipf";
}

bool ipf_format::supports_save() const noexcept
{
	return false;
}

int ipf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	static const uint8_t refh[12] = { 0x43, 0x41, 0x50, 0x53, 0x00, 0x00, 0x00, 0x0c, 0x1c, 0xd5, 0x73, 0xba };
	uint8_t h[12];
	auto const [err, actual] = read_at(io, 0, h, 12);
	if(err || (12 != actual))
		return 0;

	if(!memcmp(h, refh, 12))
		return FIFID_SIGN;

	return 0;
}

bool ipf_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if(io.length(size))
		return false;
	auto const [err, data, actual] = read_at(io, 0, size);
	if(err || (actual != size))
		return false;
	ipf_decode dec;
	return dec.parse(data.get(), size, image);
}


uint32_t ipf_format::ipf_decode::rb(const uint8_t *&p, int count)
{
	uint32_t v = 0;
	for(int i=0; i<count; i++)
		v = (v << 8) | *p++;
	return v;
}

uint32_t ipf_format::ipf_decode::crc32r(const uint8_t *data, uint32_t size)
{
	// Reversed crc32
	uint32_t crc = 0xffffffff;
	for(uint32_t i=0; i != size; i++) {
		crc = crc ^ data[i];
		for(int j=0; j<8; j++)
			if(crc & 1)
				crc = (crc >> 1) ^ 0xedb88320;
			else
				crc = crc >> 1;
	}
	return ~crc;
}

bool ipf_format::ipf_decode::parse(uint8_t *data, size_t size, floppy_image &image)
{
	image.set_variant(floppy_image::DSDD); // Not handling anything else yet
	tcount = 84*2+1; // Usual max
	tinfos.resize(tcount);
	bool res = scan_all_tags(data, size);
	if(res)
		res = generate_tracks(image);
	tinfos.clear();
	return res;
}

bool ipf_format::ipf_decode::parse_info(const uint8_t *info)
{
	type = get_u32be(info+12);
	if(type != 1)
		return false;
	encoder_type = get_u32be(info+16); // 1 for CAPS, 2 for SPS
	encoder_revision = get_u32be(info+20); // 1 always
	release = get_u32be(info+24);
	revision = get_u32be(info+28);
	origin = get_u32be(info+32); // Original source reference
	min_cylinder = get_u32be(info+36);
	max_cylinder = get_u32be(info+40);
	min_head = get_u32be(info+44);
	max_head = get_u32be(info+48);
	credit_day = get_u32be(info+52);  // year*1e4 + month*1e2 + day
	credit_time = get_u32be(info+56); // hour*1e7 + min*1e5 + sec*1e3 + msec
	for(int i=0; i<4; i++)
		platform[i] = get_u32be(info+60+4*i);
	for(int i=0; i<5; i++)
		extra[i] = get_u32be(info+76+4*i);
	return true;
}

ipf_format::ipf_decode::track_info *ipf_format::ipf_decode::get_index(uint32_t idx)
{
	if(idx > 1000)
		return nullptr;
	if(idx >= tcount) {
		tinfos.resize(idx+1);
		tcount = idx+1;
	}

	return &tinfos[idx];
}

bool ipf_format::ipf_decode::parse_imge(const uint8_t *imge)
{
	track_info *t = get_index(get_u32be(imge+64));
	if(!t)
		return false;

	t->info_set = true;

	t->cylinder = get_u32be(imge+12);
	if(t->cylinder < min_cylinder || t->cylinder > max_cylinder)
		return false;

	t->head = get_u32be(imge+16);
	if(t->head < min_head || t->head > max_head)
		return false;

	t->type = get_u32be(imge+20);
	t->sigtype = get_u32be(imge+24); // 1 for 2us cells, no other value valid
	t->size_bytes = get_u32be(imge+28);
	t->index_bytes = get_u32be(imge+32);
	t->index_cells = get_u32be(imge+36);
	t->datasize_cells = get_u32be(imge+40);
	t->gapsize_cells = get_u32be(imge+44);
	t->size_cells = get_u32be(imge+48);
	t->block_count = get_u32be(imge+52);
	t->process = get_u32be(imge+56); // encoder process, always 0
	t->weak_bits = get_u32be(imge+60);
	t->reserved[0] = get_u32be(imge+68);
	t->reserved[1] = get_u32be(imge+72);
	t->reserved[2] = get_u32be(imge+76);

	return true;
}

bool ipf_format::ipf_decode::parse_data(const uint8_t *data, uint32_t &pos, uint32_t max_extra_size)
{
	track_info *t = get_index(get_u32be(data+24));
	if(!t)
		return false;

	t->data_size_bits = get_u32be(data+16);
	t->data = data+28;
	t->data_size = get_u32be(data+12);
	if(t->data_size > max_extra_size)
		return false;
	if(crc32r(t->data, t->data_size) != get_u32be(data+20))
		return false;
	pos += t->data_size;
	return true;
}

bool ipf_format::ipf_decode::scan_one_tag(uint8_t *data, size_t size, uint32_t &pos, uint8_t *&tag, uint32_t &tsize)
{
	if(size-pos < 12)
		return false;
	tag = &data[pos];
	tsize = get_u32be(tag+4);
	if(size-pos < tsize)
		return false;
	uint32_t crc = get_u32be(tag+8);
	tag[8] = tag[9] = tag[10] = tag[11] = 0;
	if(crc32r(tag, tsize) != crc)
		return false;
	pos += tsize;
	return true;
}

bool ipf_format::ipf_decode::scan_all_tags(uint8_t *data, size_t size)
{
	uint32_t pos = 0;
	while(pos != size) {
		uint8_t *tag;
		uint32_t tsize;

		if(!scan_one_tag(data, size, pos, tag, tsize))
			return false;

		switch(get_u32be(tag)) {
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

bool ipf_format::ipf_decode::generate_tracks(floppy_image &image)
{
	for(uint32_t i = 0; i != tcount; i++) {
		track_info &t = tinfos[i];
		if(t.info_set && t.data) {
			if(!generate_track(t, image))
				return false;

		} else if(t.info_set || t.data)
			return false;
	}
	return true;
}

void ipf_format::ipf_decode::rotate(std::vector<uint32_t> &track, uint32_t offset, uint32_t size)
{
	uint32_t done = 0;
	for(uint32_t bpos=0; done < size; bpos++) {
		uint32_t pos = bpos;
		uint32_t hold = track[pos];
		for(;;) {
			uint32_t npos = pos+offset;
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

void ipf_format::ipf_decode::mark_track_splice(std::vector<uint32_t> &track, uint32_t offset, uint32_t size)
{
	for(int i=0; i<3; i++) {
		uint32_t pos = (offset + i) % size;
		uint32_t v = track[pos];
		if((v & floppy_image::MG_MASK) == MG_0)
			v = (v & floppy_image::TIME_MASK) | MG_1;
		else if((v & floppy_image::MG_MASK) == MG_1)
			v = (v & floppy_image::TIME_MASK) | MG_0;
		track[pos] = v;
	}
}

void ipf_format::ipf_decode::timing_set(std::vector<uint32_t> &track, uint32_t start, uint32_t end, uint32_t time)
{
	for(uint32_t i=start; i != end; i++)
		track[i] = (track[i] & floppy_image::MG_MASK) | time;
}

bool ipf_format::ipf_decode::generate_timings(const track_info &t, std::vector<uint32_t> &track, const std::vector<uint32_t> &data_pos, const std::vector<uint32_t> &gap_pos)
{
	timing_set(track, 0, t.size_cells, 2000);

	switch(t.type) {
	case 2: break;

	case 3:
		if(t.block_count >= 4)
			timing_set(track, gap_pos[3], data_pos[4], 1890);
		if(t.block_count >= 5) {
			timing_set(track, data_pos[4], gap_pos[4], 1890);
			timing_set(track, gap_pos[4], data_pos[5], 1990);
		}
		if(t.block_count >= 6) {
			timing_set(track, data_pos[5], gap_pos[5], 1990);
			timing_set(track, gap_pos[5], data_pos[6], 2090);
		}
		if(t.block_count >= 7)
			timing_set(track, data_pos[6], gap_pos[6], 2090);
		break;

	case 4:
		timing_set(track, gap_pos[t.block_count-1], data_pos[0], 1890);
		timing_set(track, data_pos[0], gap_pos[0], 1890);
		timing_set(track, gap_pos[0], data_pos[1], 1990);
		if(t.block_count >= 2) {
			timing_set(track, data_pos[1], gap_pos[1], 1990);
			timing_set(track, gap_pos[1], data_pos[2], 2090);
		}
		if(t.block_count >= 3)
			timing_set(track, data_pos[2], gap_pos[2], 2090);
		break;

	case 5:
		if(t.block_count >= 6)
			timing_set(track, data_pos[5], gap_pos[5], 2100);
		break;

	case 6:
		if(t.block_count >= 2)
			timing_set(track, data_pos[1], gap_pos[1], 2200);
		if(t.block_count >= 3)
			timing_set(track, data_pos[2], gap_pos[2], 1800);
		break;

	case 7:
		if(t.block_count >= 2)
			timing_set(track, data_pos[1], gap_pos[1], 2100);
		break;

	case 8:
		if(t.block_count >= 2)
			timing_set(track, data_pos[1], gap_pos[1], 2200);
		if(t.block_count >= 3)
			timing_set(track, data_pos[2], gap_pos[2], 2100);
		if(t.block_count >= 5)
			timing_set(track, data_pos[4], gap_pos[4], 1900);
		if(t.block_count >= 6)
			timing_set(track, data_pos[5], gap_pos[5], 1800);
		if(t.block_count >= 7)
			timing_set(track, data_pos[6], gap_pos[6], 1700);
		break;

	case 9: {
		uint32_t mask = get_u32be(t.data + 32*t.block_count + 12);
		for(uint32_t i=1; i<t.block_count; i++)
			timing_set(track, data_pos[i], gap_pos[i], mask & (1 << (i-1)) ? 1900 : 2100);
		break;
	}

	default:
		return false;
	}

	return true;
}

bool ipf_format::ipf_decode::generate_track(track_info &t, floppy_image &image)
{
	if(!t.size_cells)
		return true;

	if(t.data_size < 32*t.block_count)
		return false;

	// Annoyingly enough, too small gaps are ignored, changing the
	// total track size.  Artifact stemming from the byte-only support
	// of old times?
	t.size_cells = block_compute_real_size(t);

	if(t.index_cells >= t.size_cells)
		return false;

	std::vector<uint32_t> track(t.size_cells);
	std::vector<uint32_t> data_pos(t.block_count+1);
	std::vector<uint32_t> gap_pos(t.block_count);
	std::vector<uint32_t> splice_pos(t.block_count);

	bool context = false;
	uint32_t pos = 0;
	for(uint32_t i = 0; i != t.block_count; i++) {
		if(!generate_block(t, i, i == t.block_count-1 ? t.size_cells - t.index_cells : 0xffffffff, track, pos, data_pos[i], gap_pos[i], splice_pos[i], context)) {
			return false;
		}
	}
	if(pos != t.size_cells) {
		return false;
	}

	data_pos[t.block_count] = pos;

	mark_track_splice(track, splice_pos[t.block_count-1], t.size_cells);

	if(!generate_timings(t, track, data_pos, gap_pos)) {
		return false;
	}

	if(t.index_cells)
		rotate(track, t.size_cells - t.index_cells, t.size_cells);

	generate_track_from_levels(t.cylinder, t.head, track, splice_pos[t.block_count-1] + t.index_cells, image);

	return true;
}

void ipf_format::ipf_decode::track_write_raw(std::vector<uint32_t>::iterator &tpos, const uint8_t *data, uint32_t cells, bool &context)
{
	for(uint32_t i=0; i != cells; i++)
		*tpos++ = data[i>>3] & (0x80 >> (i & 7)) ? MG_1 : MG_0;
	if(cells)
		context = tpos[-1] == MG_1;
}

void ipf_format::ipf_decode::track_write_mfm(std::vector<uint32_t>::iterator &tpos, const uint8_t *data, uint32_t start_offset, uint32_t patlen, uint32_t cells, bool &context)
{
	patlen *= 2;
	for(uint32_t i=0; i != cells; i++) {
		uint32_t pos = (i + start_offset) % patlen;
		bool bit = data[pos>>4] & (0x80 >> ((pos >> 1) & 7));
		if(pos & 1) {
			*tpos++ = bit ? MG_1 : MG_0;
			context = bit;
		} else
			*tpos++ = context || bit ? MG_0 : MG_1;
	}
}

void ipf_format::ipf_decode::track_write_weak(std::vector<uint32_t>::iterator &tpos, uint32_t cells)
{
	for(uint32_t i=0; i != cells; i++)
		*tpos++ = floppy_image::MG_N;
}

bool ipf_format::ipf_decode::generate_block_data(const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, std::vector<uint32_t>::iterator tlimit, bool &context)
{
	for(;;) {
		if(data >= dlimit)
			return false;
		uint8_t val = *data++;
		if((val >> 5) > dlimit-data)
			return false;
		uint32_t param = rb(data, val >> 5);
		uint32_t tleft = tlimit - tpos;
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

bool ipf_format::ipf_decode::generate_block_gap_0(uint32_t gap_cells, uint8_t pattern, uint32_t &spos, uint32_t ipos, std::vector<uint32_t>::iterator &tpos, bool &context)
{
	spos = ipos >= 16 && ipos+16 <= gap_cells ? ipos : gap_cells >> 1;
	track_write_mfm(tpos, &pattern, 0, 8, spos, context);
	uint32_t delta = 0;
	if(gap_cells & 1) {
		*tpos++ = MG_0;
		delta++;
	}
	track_write_mfm(tpos, &pattern, spos+delta-gap_cells, 8, gap_cells-spos-delta, context);
	return true;
}

bool ipf_format::ipf_decode::gap_description_to_reserved_size(const uint8_t *&data, const uint8_t *dlimit, uint32_t &res_size)
{
	res_size = 0;
	for(;;) {
		if(data >= dlimit)
			return false;
		uint8_t val = *data++;
		if((val >> 5) > dlimit-data)
			return false;
		uint32_t param = rb(data, val >> 5);
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

bool ipf_format::ipf_decode::generate_gap_from_description(const uint8_t *&data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, uint32_t size, bool pre, bool &context)
{
	const uint8_t *data1 = data;
	uint32_t res_size;
	if(!gap_description_to_reserved_size(data1, dlimit, res_size))
		return false;

	if(res_size > size)
		return false;
	uint8_t pattern[16];
	memset(pattern, 0, sizeof(pattern));
	uint32_t pattern_size = 0;

	uint32_t pos = 0, block_size = 0;
	for(;;) {
		uint8_t val = *data++;
		uint32_t param = rb(data, val >> 5);
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


bool ipf_format::ipf_decode::generate_block_gap_1(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context)
{
	if(ipos >= 16 && ipos < gap_cells-16)
		spos = ipos;
	else
		spos = 0;
	return generate_gap_from_description(data, dlimit, tpos, gap_cells, true, context);
}

bool ipf_format::ipf_decode::generate_block_gap_2(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context)
{
	if(ipos >= 16 && ipos < gap_cells-16)
		spos = ipos;
	else
		spos = gap_cells;
	return generate_gap_from_description(data, dlimit, tpos, gap_cells, false, context);
}

bool ipf_format::ipf_decode::generate_block_gap_3(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos,  bool &context)
{
	if(ipos >= 16 && ipos < gap_cells-16)
		spos = ipos;
	else {
		uint32_t presize, postsize;
		const uint8_t *data1 = data;
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
	uint32_t delta = 0;
	if(gap_cells & 1) {
		tpos[spos] = MG_0;
		delta++;
	}

	return generate_gap_from_description(data, dlimit, tpos+spos+delta, gap_cells - spos - delta, false, context);
}

bool ipf_format::ipf_decode::generate_block_gap(uint32_t gap_type, uint32_t gap_cells, uint8_t pattern, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, bool &context)
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

bool ipf_format::ipf_decode::generate_block(const track_info &t, uint32_t idx, uint32_t ipos, std::vector<uint32_t> &track, uint32_t &pos, uint32_t &dpos, uint32_t &gpos, uint32_t &spos, bool &context)
{
	const uint8_t *data = t.data;
	const uint8_t *data_end = t.data + t.data_size;
	const uint8_t *thead = data + 32*idx;
	uint32_t data_cells = get_u32be(thead);
	uint32_t gap_cells = get_u32be(thead+4);

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
	if(pos > t.size_cells)
		return false;
	if(!generate_block_data(data + get_u32be(thead+28), data_end, track.begin()+dpos, track.begin()+gpos, context))
		return false;
	if(!generate_block_gap(get_u32be(thead+20), gap_cells, get_u32be(thead+24), spos, ipos > gpos ? ipos-gpos : 0, data + get_u32be(thead+8), data_end, track.begin()+gpos, context))
		return false;
	spos += gpos;

	return true;
}

uint32_t ipf_format::ipf_decode::block_compute_real_size(const track_info &t)
{
	uint32_t size = 0;
	const uint8_t *thead = t.data;
	for(unsigned int i=0; i != t.block_count; i++) {
		uint32_t data_cells = get_u32be(thead);
		uint32_t gap_cells = get_u32be(thead+4);
		if(gap_cells < 8)
			gap_cells = 0;

		size += data_cells + gap_cells;
		thead += 32;
	}
	return size;
}
