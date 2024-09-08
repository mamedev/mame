// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    flopimg.cpp

    Floppy disk image abstraction code (new implementation)

****************************************************************************/

#include "flopimg.h"

#include "ioprocs.h"
#include "multibyte.h"
#include "strformat.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <utility>


floppy_image::floppy_image(int _tracks, int _heads, uint32_t _form_factor)
{
	tracks = _tracks;
	heads = _heads;

	form_factor = _form_factor;
	variant = 0;

	track_array.resize(tracks*4+1);
	for(int i=0; i<tracks*4+1; i++)
		track_array[i].resize(heads);
}

floppy_image::~floppy_image()
{
}

void floppy_image::set_variant(uint32_t _variant)
{
	variant = _variant;

	// Initialize hard sectors
	index_array.clear();

	uint32_t sectors;
	switch(variant) {
	case SSDD16:
	case SSQD16:
	case DSDD16:
	case DSQD16:
		sectors = 16;
		break;
	default:
		sectors = 0;
	}
	if(sectors) {
		uint32_t sector_angle = 200000000/sectors;
		for(int i = 1; i < sectors; i++)
			index_array.push_back(i*sector_angle);
		index_array.push_back((sectors-1)*sector_angle + sector_angle/2);
	}
}

void floppy_image::find_index_hole(uint32_t pos, uint32_t &last, uint32_t &next) const
{
	auto nexti = std::lower_bound(index_array.begin(), index_array.end(), pos+1);
	next = nexti == index_array.end() ? 200000000 : *nexti;
	last = nexti == index_array.begin() ? 0 : *--nexti;
}

void floppy_image::get_maximal_geometry(int &_tracks, int &_heads) const noexcept
{
	_tracks = tracks;
	_heads = heads;
}

void floppy_image::get_actual_geometry(int &_tracks, int &_heads) const noexcept
{
	int maxt = (tracks-1)*4, maxh = heads-1;

	while(maxt >= 0) {
		for(int i=0; i<=maxh; i++)
			if(!track_array[maxt][i].cell_data.empty())
				goto track_done;
		maxt--;
	}
	track_done:
	if(maxt >= 0)
		while(maxh >= 0) {
			for(int i=0; i<=maxt; i++)
				if(!track_array[i][maxh].cell_data.empty())
					goto head_done;
			maxh--;
		}
	else
		maxh = -1;

	head_done:
	_tracks = (maxt+4)/4;
	_heads = maxh+1;
}

int floppy_image::get_resolution() const noexcept
{
	int mask = 0;
	for(int i=0; i<=(tracks-1)*4; i++)
		for(int j=0; j<heads; j++)
			if(!track_array[i][j].cell_data.empty())
				mask |= 1 << (i & 3);
	if(mask & 0xa)
		return 2;
	if(mask & 0x4)
		return 1;
	return 0;
}

bool floppy_image::track_is_formatted(int track, int head, int subtrack) const noexcept
{
	int idx = track*4 + subtrack;
	if(int(track_array.size()) <= idx)
		return false;
	if(int(track_array[idx].size()) <= head)
		return false;
	const auto &data = track_array[idx][head].cell_data;
	if(data.empty())
		return false;
	for(uint32_t mg : data)
		if((mg & floppy_image::MG_MASK) == floppy_image::MG_F)
			return true;
	return false;
}

const char *floppy_image::get_variant_name(uint32_t form_factor, uint32_t variant) noexcept
{
	switch(variant) {
	case SSSD:   return "Single side, single density";
	case SSSD10: return "Single side, single density, 10-sector";
	case SSSD16: return "Single side, single density, 16-sector";
	case SSSD32: return "Single side, single density, 32-sector";
	case SSDD:   return "Single side, double density";
	case SSDD10: return "Single side, double density, 10-sector";
	case SSDD16: return "Single side, double density, 16 hard sector";
	case SSDD32: return "Single side, double density, 32-sector";
	case SSQD:   return "Single side, quad density";
	case SSQD10: return "Single side, quad density, 10-sector";
	case SSQD16: return "Single side, quad density, 16 hard sector";
	case DSSD:   return "Double side, single density";
	case DSSD10: return "Double side, single density, 10-sector";
	case DSSD16: return "Double side, single density, 16-sector";
	case DSSD32: return "Double side, single density, 32-sector";
	case DSDD:   return "Double side, double density";
	case DSDD10: return "Double side, double density, 10-sector";
	case DSDD16: return "Double side, double density, 16 hard sector";
	case DSDD32: return "Double side, double density, 32-sector";
	case DSQD:   return "Double side, quad density";
	case DSQD10: return "Double side, quad density, 10-sector";
	case DSQD16: return "Double side, quad density, 16 hard sector";
	case DSHD:   return "Double side, high density";
	case DSED:   return "Double side, extended density";
	}
	return "Unknown";
}

bool floppy_image_format_t::has_variant(const std::vector<uint32_t> &variants, uint32_t variant) noexcept
{
	for(uint32_t v : variants)
		if(variant == v)
			return true;
	return false;
}

bool floppy_image_format_t::save(util::random_read_write &io, const std::vector<uint32_t> &, const floppy_image &) const
{
	return false;
}

bool floppy_image_format_t::extension_matches(const char *file_name) const
{
	const char *ext = strrchr(file_name, '.');
	if(!ext)
		return false;
	ext++;
	int elen = strlen(ext);
	const char *rext = extensions();
	for(;;) {
		const char *next_ext = strchr(rext, ',');
		int rlen = next_ext ? next_ext - rext : strlen(rext);
		if(rlen == elen && !memcmp(ext, rext, rlen))
			return true;
		if(next_ext)
			rext = next_ext +1;
		else
			break;
	}
	return false;
}

bool floppy_image_format_t::type_no_data(int type)
{
	return
		type == CRC_CCITT_START ||
		type == CRC_CCITT_FM_START ||
		type == CRC_AMIGA_START ||
		type == CRC_CBM_START ||
		type == CRC_MACHEAD_START ||
		type == CRC_FCS_START ||
		type == CRC_VICTOR_HDR_START ||
		type == CRC_VICTOR_DATA_START ||
		type == CRC_END ||
		type == SECTOR_LOOP_START ||
		type == SECTOR_LOOP_END ||
		type == END;
}

bool floppy_image_format_t::type_data_mfm(int type, int p1, const gen_crc_info *crcs)
{
	return
		type == MFM ||
		type == MFMBITS ||
		type == TRACK_ID ||
		type == HEAD_ID ||
		type == HEAD_ID_SWAP ||
		type == SECTOR_ID ||
		type == SIZE_ID ||
		type == OFFSET_ID_O ||
		type == OFFSET_ID_E ||
		type == OFFSET_ID_FM ||
		type == SECTOR_ID_O ||
		type == SECTOR_ID_E ||
		type == REMAIN_O ||
		type == REMAIN_E ||
		type == SECTOR_DATA ||
		type == SECTOR_DATA_O ||
		type == SECTOR_DATA_E ||
		(type == CRC && (crcs[p1].type == CRC_CCITT || crcs[p1].type == CRC_AMIGA));
}

void floppy_image_format_t::collect_crcs(const desc_e *desc, gen_crc_info *crcs)
{
	memset(crcs, 0, MAX_CRC_COUNT * sizeof(*crcs));
	for(int i=0; i != MAX_CRC_COUNT; i++)
		crcs[i].write = -1;

	for(int i=0; desc[i].type != END; i++)
		switch(desc[i].type) {
		case CRC_CCITT_START:
			crcs[desc[i].p1].type = CRC_CCITT;
			break;
		case CRC_CCITT_FM_START:
			crcs[desc[i].p1].type = CRC_CCITT_FM;
			break;
		case CRC_AMIGA_START:
			crcs[desc[i].p1].type = CRC_AMIGA;
			break;
		case CRC_CBM_START:
			crcs[desc[i].p1].type = CRC_CBM;
			break;
		case CRC_MACHEAD_START:
			crcs[desc[i].p1].type = CRC_MACHEAD;
			break;
		case CRC_FCS_START:
			crcs[desc[i].p1].type = CRC_FCS;
			break;
		case CRC_VICTOR_HDR_START:
			crcs[desc[i].p1].type = CRC_VICTOR_HDR;
			break;
		case CRC_VICTOR_DATA_START:
			crcs[desc[i].p1].type = CRC_VICTOR_DATA;
			break;
		}

	for(int i=0; desc[i].type != END; i++)
		if(desc[i].type == CRC) {
			int j;
			for(j = i+1; desc[j].type != END && type_no_data(desc[j].type); j++) {};
			crcs[desc[i].p1].fixup_mfm_clock = type_data_mfm(desc[j].type, desc[j].p1, crcs);
		}
}

int floppy_image_format_t::crc_cells_size(int type)
{
	switch(type) {
	case CRC_CCITT: return 32;
	case CRC_CCITT_FM: return 32;
	case CRC_AMIGA: return 64;
	case CRC_CBM: return 10;
	case CRC_MACHEAD: return 8;
	case CRC_FCS: return 20;
	case CRC_VICTOR_HDR: return 10;
	case CRC_VICTOR_DATA: return 20;
	default: return 0;
	}
}

bool floppy_image_format_t::bit_r(const std::vector<uint32_t> &buffer, int offset)
{
	return (buffer[offset] & floppy_image::MG_MASK) == MG_1;
}

uint32_t floppy_image_format_t::bitn_r(const std::vector<uint32_t> &buffer, int offset, int count)
{
	uint32_t r = 0;
	for(int i=0; i<count; i++)
		r = (r << 1) | (uint32_t) bit_r(buffer, offset+i);
	return r;
}

void floppy_image_format_t::bit_w(std::vector<uint32_t> &buffer, bool val, uint32_t size, int offset)
{
	buffer[offset] = (val ? MG_1 : MG_0) | size;
}

void floppy_image_format_t::bit_w(std::vector<uint32_t> &buffer, bool val, uint32_t size)
{
	buffer.push_back((val ? MG_1 : MG_0) | size);
}

void floppy_image_format_t::raw_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size)
{
	for(int i=n-1; i>=0; i--)
		bit_w(buffer, (val >> i) & 1, size);
}

void floppy_image_format_t::raw_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size, int offset)
{
	for(int i=n-1; i>=0; i--)
		bit_w(buffer, (val >> i) & 1, size, offset++);
}

void floppy_image_format_t::mfm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size)
{
	int prec = buffer.empty() ? 0 : bit_r(buffer, buffer.size()-1);
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, !(prec || bit), size);
		bit_w(buffer, bit, size);
		prec = bit;
	}
}

void floppy_image_format_t::mfm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size, int offset)
{
	int prec = offset ? bit_r(buffer, offset-1) : 0;
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, !(prec || bit), size, offset++);
		bit_w(buffer, bit,            size, offset++);
		prec = bit;
	}
}

void floppy_image_format_t::fm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size)
{
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, true, size);
		bit_w(buffer, bit,  size);
	}
}

void floppy_image_format_t::fm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size, int offset)
{
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, true, size, offset++);
		bit_w(buffer, bit,  size, offset++);
	}
}

void floppy_image_format_t::mfm_half_w(std::vector<uint32_t> &buffer, int start_bit, uint32_t val, uint32_t size)
{
	int prec = buffer.empty() ? 0 : bit_r(buffer, buffer.size()-1);
	for(int i=start_bit; i>=0; i-=2) {
		int bit = (val >> i) & 1;
		bit_w(buffer, !(prec || bit), size);
		bit_w(buffer, bit,            size);
		prec = bit;
	}
}

void floppy_image_format_t::gcr5_w(std::vector<uint32_t> &buffer, uint8_t val, uint32_t size)
{
	uint32_t e0 = gcr5fw_tb[val >> 4];
	uint32_t e1 = gcr5fw_tb[val & 0x0f];
	raw_w(buffer, 5, e0, size);
	raw_w(buffer, 5, e1, size);
}

void floppy_image_format_t::gcr5_w(std::vector<uint32_t> &buffer, uint8_t val, uint32_t size, int offset)
{
	uint32_t e0 = gcr5fw_tb[val >> 4];
	uint32_t e1 = gcr5fw_tb[val & 0x0f];
	raw_w(buffer, 5, e0, size, offset);
	raw_w(buffer, 5, e1, size, offset+5);
}

void floppy_image_format_t::_8n1_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size)
{
	bit_w(buffer, 0, size);
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, bit, size);
	}
	bit_w(buffer, 1, size);
}

void floppy_image_format_t::fixup_crc_amiga(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	uint16_t res = 0;
	int size = crc->end - crc->start;
	for(int i=1; i<size; i+=2)
		if(bit_r(buffer, crc->start + i))
			res = res ^ (0x8000 >> ((i >> 1) & 15));
	mfm_w(buffer, 16,   0, 1000, crc->write);
	mfm_w(buffer, 16, res, 1000, crc->write+32);
}

void floppy_image_format_t::fixup_crc_cbm(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	uint8_t v = 0;
	for(int o = crc->start; o < crc->end; o+=10) {
		v = v ^ (gcr5bw_tb[bitn_r(buffer, o, 5)] << 4);
		v = v ^ gcr5bw_tb[bitn_r(buffer, o+5, 5)];
	}
	gcr5_w(buffer, v, 1000, crc->write);
}

uint16_t floppy_image_format_t::calc_crc_ccitt(const std::vector<uint32_t> &buffer, int start, int end)
{
	uint32_t res = 0xffff;
	int size = end - start;
	for(int i=1; i<size; i+=2) {
		res <<= 1;
		if(bit_r(buffer, start + i))
			res ^= 0x10000;
		if(res & 0x10000)
			res ^= 0x11021;
	}
	return res;
}

void floppy_image_format_t::fixup_crc_ccitt(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	mfm_w(buffer, 16, calc_crc_ccitt(buffer, crc->start, crc->end), 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_ccitt_fm(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	fm_w(buffer, 16, calc_crc_ccitt(buffer, crc->start, crc->end), 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_machead(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	uint8_t v = 0;
	for(int o = crc->start; o < crc->end; o+=8)
		v = v ^ gcr6bw_tb[bitn_r(buffer, o, 8)];
	raw_w(buffer, 8, gcr6fw_tb[v], 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_fcs(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	// TODO
}

void floppy_image_format_t::fixup_crc_victor_header(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	uint8_t v = 0;
	for(int o = crc->start; o < crc->end; o+=10)
		v += ((gcr5bw_tb[bitn_r(buffer, o, 5)] << 4) | gcr5bw_tb[bitn_r(buffer, o+5, 5)]);
	gcr5_w(buffer, v, 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_victor_data(std::vector<uint32_t> &buffer, const gen_crc_info *crc)
{
	uint16_t v = 0;
	for(int o = crc->start; o < crc->end; o+=10)
		v += ((gcr5bw_tb[bitn_r(buffer, o, 5)] << 4) | gcr5bw_tb[bitn_r(buffer, o+5, 5)]);
	gcr5_w(buffer, v & 0xff, 1000, crc->write);
	gcr5_w(buffer, v >> 8, 1000, crc->write+10);
}

void floppy_image_format_t::fixup_crcs(std::vector<uint32_t> &buffer, gen_crc_info *crcs)
{
	for(int i=0; i != MAX_CRC_COUNT; i++)
		if(crcs[i].write != -1) {
			switch(crcs[i].type) {
			case CRC_AMIGA:         fixup_crc_amiga(buffer, crcs+i); break;
			case CRC_CBM:           fixup_crc_cbm(buffer, crcs+i); break;
			case CRC_CCITT:         fixup_crc_ccitt(buffer, crcs+i); break;
			case CRC_CCITT_FM:      fixup_crc_ccitt_fm(buffer, crcs+i); break;
			case CRC_MACHEAD:       fixup_crc_machead(buffer, crcs+i); break;
			case CRC_FCS:           fixup_crc_fcs(buffer, crcs+i); break;
			case CRC_VICTOR_HDR:    fixup_crc_victor_header(buffer, crcs+i); break;
			case CRC_VICTOR_DATA:   fixup_crc_victor_data(buffer, crcs+i); break;
			}
			if(crcs[i].fixup_mfm_clock) {
				int offset = crcs[i].write + crc_cells_size(crcs[i].type);
				bit_w(buffer, !((offset ? bit_r(buffer, offset-1) : false) || bit_r(buffer, offset+1)), 1000, offset);
			}
			crcs[i].write = -1;
		}
}

uint32_t floppy_image_format_t::gcr6_encode(uint8_t va, uint8_t vb, uint8_t vc)
{
	uint32_t r;
	r = gcr6fw_tb[((va >> 2) & 0x30) | ((vb >> 4) & 0x0c) | ((vc >> 6) & 0x03)] << 24;
	r |= gcr6fw_tb[va & 0x3f] << 16;
	r |= gcr6fw_tb[vb & 0x3f] << 8;
	r |= gcr6fw_tb[vc & 0x3f];
	return r;
}

void floppy_image_format_t::gcr6_decode(uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3, uint8_t &va, uint8_t &vb, uint8_t &vc)
{
	e0 = gcr6bw_tb[e0];
	e1 = gcr6bw_tb[e1];
	e2 = gcr6bw_tb[e2];
	e3 = gcr6bw_tb[e3];

	va = ((e0 << 2) & 0xc0) | e1;
	vb = ((e0 << 4) & 0xc0) | e2;
	vc = ((e0 << 6) & 0xc0) | e3;
}

uint16_t floppy_image_format_t::gcr4_encode(uint8_t va)
{
	return (va << 7) | va | 0xaaaa;
}

uint8_t floppy_image_format_t::gcr4_decode(uint8_t e0, uint8_t e1)
{
	return ((e0 << 1) & 0xaa) | (e1 & 0x55);
}


int floppy_image_format_t::calc_sector_index(int num, int interleave, int skew, int total_sectors, int track_head)
{
	int i = 0;
	int sec = 0;
	// use interleave
	while (i != num)
	{
		i++;
		i += interleave;
		i %= total_sectors;
		sec++;
		// This line prevents lock-ups of the emulator when the interleave is not appropriate
		if (sec > total_sectors)
			throw std::invalid_argument(util::string_format("Format error: interleave %d not appropriate for %d sectors per track", interleave, total_sectors));
	}
	// use skew param
	sec -= track_head * skew;
	sec %= total_sectors;
	if (sec < 0) sec += total_sectors;
	return sec;
}

void floppy_image_format_t::generate_track(const desc_e *desc, int track, int head, const desc_s *sect, int sect_count, int track_size, floppy_image &image)
{
	std::vector<uint32_t> buffer;

	gen_crc_info crcs[MAX_CRC_COUNT];
	collect_crcs(desc, crcs);

	int index = 0;
	int sector_loop_start = 0;
	int sector_idx = 0;
	int sector_cnt = 0;
	int sector_limit = 0;
	int sector_interleave = 0;
	int sector_skew = 0;

	while(desc[index].type != END) {
		switch(desc[index].type) {
		case FM:
			for(int i=0; i<desc[index].p2; i++)
				fm_w(buffer, 8, desc[index].p1);
			break;

		case MFM:
			for(int i=0; i<desc[index].p2; i++)
				mfm_w(buffer, 8, desc[index].p1);
			break;

		case MFMBITS:
			mfm_w(buffer, desc[index].p2, desc[index].p1);
			break;

		case GCR5:
			for(int i=0; i<desc[index].p2; i++)
				gcr5_w(buffer, desc[index].p1);
			break;

		case _8N1:
			for(int i=0; i<desc[index].p2; i++)
				_8n1_w(buffer, 8, desc[index].p1);
			break;

		case RAW:
			for(int i=0; i<desc[index].p2; i++)
				raw_w(buffer, 16, desc[index].p1);
			break;

		case RAWBYTE:
			for(int i=0; i<desc[index].p2; i++)
				raw_w(buffer, 8, desc[index].p1);
			break;

		case RAWBITS:
			raw_w(buffer, desc[index].p2, desc[index].p1);
			break;

		case SYNC_GCR5:
			for(int i=0; i<desc[index].p1; i++)
				raw_w(buffer, 10, 0xffff);
			break;

		case TRACK_ID:
			mfm_w(buffer, 8, track);
			break;

		case TRACK_ID_FM:
			fm_w(buffer, 8, track);
			break;

		case TRACK_ID_DOS2_GCR5:
			gcr5_w(buffer, 1 + (track >> 1) + (head * 35));
			break;

		case TRACK_ID_DOS25_GCR5:
			gcr5_w(buffer, 1 + track + (head * 77));
			break;

		case TRACK_ID_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[track & 0x3f]);
			break;

		case TRACK_ID_8N1:
			_8n1_w(buffer, 8, track);
			break;

		case TRACK_ID_VICTOR_GCR5:
			gcr5_w(buffer, track + (head * 0x80));
			break;

		case HEAD_ID:
			mfm_w(buffer, 8, head);
			break;

		case HEAD_ID_FM:
			fm_w(buffer, 8, head);
			break;

		case HEAD_ID_SWAP:
			mfm_w(buffer, 8, !head);
			break;

		case TRACK_HEAD_ID_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[(track & 0x40 ? 1 : 0) | (head ? 0x20 : 0)]);
			break;

		case SECTOR_ID:
			mfm_w(buffer, 8, sect[sector_idx].sector_id);
			break;

		case SECTOR_ID_FM:
			fm_w(buffer, 8, sect[sector_idx].sector_id);
			break;

		case SECTOR_ID_GCR5:
			gcr5_w(buffer, sect[sector_idx].sector_id);
			break;

		case SECTOR_ID_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[sect[sector_idx].sector_id]);
			break;

		case SECTOR_ID_8N1:
			_8n1_w(buffer, 8, sect[sector_idx].sector_id);
			break;

		case SIZE_ID: {
			int size = sect[sector_idx].size;
			int id;
			for(id = 0; size > 128; size >>=1, id++) {};
			mfm_w(buffer, 8, id);
			break;
		}

		case SIZE_ID_FM: {
			int size = sect[sector_idx].size;
			int id;
			for(id = 0; size > 128; size >>=1, id++) {};
			fm_w(buffer, 8, id);
			break;
		}

		case SECTOR_INFO_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[sect[sector_idx].sector_info]);
			break;

		case OFFSET_ID_O:
			mfm_half_w(buffer, 7, track*2+head);
			break;

		case OFFSET_ID_E:
			mfm_half_w(buffer, 6, track*2+head);
			break;

		case OFFSET_ID_FM:
			fm_w(buffer, 8, track*2+head);
			break;

		case OFFSET_ID:
			mfm_w(buffer, 8, track*2+head);
			break;

		case SECTOR_ID_O:
			mfm_half_w(buffer, 7, sector_idx);
			break;

		case SECTOR_ID_E:
			mfm_half_w(buffer, 6, sector_idx);
			break;

		case REMAIN_O:
			mfm_half_w(buffer, 7, desc[index].p1 - sector_idx);
			break;

		case REMAIN_E:
			mfm_half_w(buffer, 6, desc[index].p1 - sector_idx);
			break;

		case SECTOR_LOOP_START:
			fixup_crcs(buffer, crcs);
			sector_loop_start = index;
			sector_idx = desc[index].p1;
			sector_cnt = sector_idx;
			sector_limit = desc[index].p2 == -1 ? sector_idx+sect_count-1 : desc[index].p2;
			sector_idx = calc_sector_index(sector_cnt,sector_interleave,sector_skew,sector_limit+1,track*2 + head);
			break;

		case SECTOR_LOOP_END:
			fixup_crcs(buffer, crcs);
			if(sector_cnt < sector_limit) {
				sector_cnt++;
				sector_idx = calc_sector_index(sector_cnt,sector_interleave,sector_skew,sector_limit+1,track*2 + head);
				index = sector_loop_start;
			}
			break;

		case SECTOR_INTERLEAVE_SKEW:
			sector_interleave = desc[index].p1;
			sector_skew = desc[index].p2;
			break;

		case CRC_AMIGA_START:
		case CRC_CBM_START:
		case CRC_CCITT_START:
		case CRC_CCITT_FM_START:
		case CRC_MACHEAD_START:
		case CRC_FCS_START:
		case CRC_VICTOR_HDR_START:
		case CRC_VICTOR_DATA_START:
			crcs[desc[index].p1].start = buffer.size();
			break;

		case CRC_END:
			crcs[desc[index].p1].end = buffer.size();
			break;

		case CRC:
			crcs[desc[index].p1].write = buffer.size();
			buffer.resize(buffer.size() + crc_cells_size(crcs[desc[index].p1].type));
			break;

		case SECTOR_DATA: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				mfm_w(buffer, 8, csect->data[i]);
			break;
		}

		case SECTOR_DATA_FM: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				fm_w(buffer, 8, csect->data[i]);
			break;
		}

		case SECTOR_DATA_O: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				mfm_half_w(buffer, 7, csect->data[i]);
			break;
		}

		case SECTOR_DATA_E: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				mfm_half_w(buffer, 6, csect->data[i]);
			break;
		}

		case SECTOR_DATA_GCR5: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				gcr5_w(buffer, csect->data[i]);
			break;
		}

		case SECTOR_DATA_MAC: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			const uint8_t *data = csect->data;
			int size = csect->size;
			uint8_t ca = 0, cb = 0, cc = 0;
			for(int i=0; i < size; i+=3) {
				int dt = size-i;
				uint8_t va = data[i];
				uint8_t vb = dt > 1 ? data[i+1] : 0;
				uint8_t vc = dt > 2 ? data[i+2] : 0;

				cc = (cc << 1) | (cc >> 7);
				int suma = ca + va + (cc & 1);
				ca = suma;
				va = va ^ cc;
				int sumb = cb + vb + (suma >> 8);
				cb = sumb;
				vb = vb ^ ca;
				cc = cc + vc + (sumb >> 8);
				vc = vc ^ cb;

				int nb = dt > 2 ? 32 : dt > 1 ? 24 : 16;
				raw_w(buffer, nb, gcr6_encode(va, vb, vc) >> (32-nb));
			}
			raw_w(buffer, 32, gcr6_encode(ca, cb, cc));
			break;
		}

		case SECTOR_DATA_8N1: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				_8n1_w(buffer, 8, csect->data[i]);
			break;
		}

		case SECTOR_DATA_MX: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			uint16_t cksum = 0, data;
			for(int i=0; i < csect->size; i+=2)
			{
				data = csect->data[i+1];
				fm_w(buffer, 8, data);
				data = (data << 8) | csect->data[i];
				fm_w(buffer, 8, csect->data[i]);
				cksum += data;
			}
			fm_w(buffer, 16, cksum);
			break;
		}

		case SECTOR_DATA_DS9: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			uint8_t data;
			int cksum = 0;
			for(int i=0; i != csect->size; i++)
			{
				if (cksum > 255) { cksum++; cksum &= 255; }
				data = csect->data[i];
				mfm_w(buffer, 8, data);
				cksum += data;
			}
			cksum &= 255;
			mfm_w(buffer, 8, cksum);
			break;
		}

		default:
			printf("%d.%d.%d (%d) unhandled\n", desc[index].type, desc[index].p1, desc[index].p2, index);
			break;
		}
		index++;
	}

	if(int(buffer.size()) != track_size)
		throw std::invalid_argument(util::string_format("Wrong track size in generate_track, expected %d, got %d", track_size, buffer.size()));

	fixup_crcs(buffer, crcs);

	generate_track_from_levels(track, head, buffer, 0, image);
}

void floppy_image_format_t::normalize_times(std::vector<uint32_t> &buffer, uint32_t last_position)
{
	for(unsigned int i=0; i != buffer.size(); i++) {
		uint32_t time = buffer[i] & floppy_image::TIME_MASK;
		buffer[i] = (buffer[i] & floppy_image::MG_MASK) | (200000000ULL * time / last_position);
	}
}

void floppy_image_format_t::generate_track_from_bitstream(int track, int head, const uint8_t *trackbuf, int track_size, floppy_image &image, int subtrack, int splice)
{
	std::vector<uint32_t> &dest = image.get_buffer(track, head, subtrack);
	dest.clear();

	for(int i=0; i != track_size; i++)
		if(trackbuf[i >> 3] & (0x80 >> (i & 7)))
			dest.push_back(floppy_image::MG_F | (i*2+1));

	normalize_times(dest, track_size*2);

	if(splice >= 0 && splice < track_size) {
		int splpos = uint64_t(200000000) * splice / track_size;
		image.set_write_splice_position(track, head, splpos, subtrack);
	}
}

void floppy_image_format_t::generate_track_from_levels(int track, int head, const std::vector<uint32_t> &trackbuf, int splice_pos, floppy_image &image)
{
	// Retrieve the angular splice pos before messing with the data
	splice_pos = splice_pos % trackbuf.size();
	uint32_t splice_angular_pos = trackbuf[splice_pos] & floppy_image::TIME_MASK;

	std::vector<uint32_t> &dest = image.get_buffer(track, head);
	dest.clear();

	uint32_t total_time = 0;
	for(auto & elem : trackbuf) {
		uint32_t bit = elem & floppy_image::MG_MASK;
		uint32_t time = elem & floppy_image::TIME_MASK;
		if(bit == MG_1)
			dest.push_back(floppy_image::MG_F | (total_time + (time >> 1)));

		else if(bit != MG_0)
			dest.push_back(bit | total_time);

		total_time += time;
	}

	normalize_times(dest, total_time);
	image.set_write_splice_position(track, head, splice_angular_pos);
}

const uint8_t floppy_image_format_t::gcr5fw_tb[0x10] =
{
	0x0a, 0x0b, 0x12, 0x13, 0x0e, 0x0f, 0x16, 0x17,
	0x09, 0x19, 0x1a, 0x1b, 0x0d, 0x1d, 0x1e, 0x15
};

const uint8_t floppy_image_format_t::gcr5bw_tb[0x20] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x00, 0x01, 0x00, 0x0c, 0x04, 0x05,
	0x00, 0x00, 0x02, 0x03, 0x00, 0x0f, 0x06, 0x07,
	0x00, 0x09, 0x0a, 0x0b, 0x00, 0x0d, 0x0e, 0x00
};

const uint8_t floppy_image_format_t::gcr6fw_tb[0x40] =
{
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

const uint8_t floppy_image_format_t::gcr6bw_tb[0x100] =
{
	// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x03, 0x00, 0x04, 0x05, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0x00, 0x00, 0x00, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
	0x00, 0x00, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x00, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x1c, 0x1d, 0x1e,
	0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x20, 0x21, 0x00, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x2a, 0x2b, 0x00, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
	0x00, 0x00, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x00, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
};

//  Atari ST Fastcopy Pro layouts

#define SECTOR_42_HEADER(cid)   \
	{ CRC_CCITT_START, cid },   \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfe, 1 },         \
	{   TRACK_ID },             \
	{   HEAD_ID },              \
	{   MFM, 0x42, 1 },         \
	{   MFM, 0x02, 1 },         \
	{ CRC_END, cid },           \
	{ CRC, cid }

#define NORMAL_SECTOR(cid)      \
	{ CRC_CCITT_START, cid },   \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfe, 1 },         \
	{   TRACK_ID },             \
	{   HEAD_ID },              \
	{   SECTOR_ID },            \
	{   SIZE_ID },              \
	{ CRC_END, cid },           \
	{ CRC, cid },               \
	{ MFM, 0x4e, 22 },          \
	{ MFM, 0x00, 12 },          \
	{ CRC_CCITT_START, cid+1 }, \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfb, 1 },         \
	{   SECTOR_DATA, -1 },      \
	{ CRC_END, cid+1 },         \
	{ CRC, cid+1 }

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_9[] = {
	{ MFM, 0x4e, 501 },
	{ MFM, 0x00, 12 },

	SECTOR_42_HEADER(1),

	{ MFM, 0x4e, 22 },
	{ MFM, 0x00, 12 },

	{ SECTOR_LOOP_START, 0, 8 },
	NORMAL_SECTOR(2),
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	{ SECTOR_LOOP_END },

	SECTOR_42_HEADER(4),

	{ MFM, 0x4e, 157 },

	{ END }
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_0[] = {
	{ MFM, 0x4e, 46 },
	SECTOR_42_HEADER(1),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 9 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END }
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_1[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 9, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 8 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_2[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 8, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 7 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_3[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 7, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 6 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_4[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 6, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 5 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_5[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 5, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 4 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_6[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 4, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 3 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_7[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 3, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 2 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_8[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 2, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 1 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_9[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 1, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 0 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e *const floppy_image_format_t::atari_st_fcp_10[10] = {
	atari_st_fcp_10_0,
	atari_st_fcp_10_1,
	atari_st_fcp_10_2,
	atari_st_fcp_10_3,
	atari_st_fcp_10_4,
	atari_st_fcp_10_5,
	atari_st_fcp_10_6,
	atari_st_fcp_10_7,
	atari_st_fcp_10_8,
	atari_st_fcp_10_9,
};


const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_11[] = {
	{ MFM, 0x4e, 1 },
	{ SECTOR_INTERLEAVE_SKEW, 1, 1},
	{ SECTOR_LOOP_START,  0,  10 }, { MFM, 0x4e, 2 }, { MFM, 0x00, 2 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

#undef SECTOR_42_HEADER
#undef NORMAL_SECTOR

const floppy_image_format_t::desc_e *floppy_image_format_t::atari_st_fcp_get_desc(int track, int head, int head_count, int sect_count)
{
	switch(sect_count) {
	case 9:
		return atari_st_fcp_9;
	case 10:
		return atari_st_fcp_10[(track*head_count + head) % 10];
	case 11:
		return atari_st_fcp_11;
	}
	return nullptr;
}

//  Amiga layouts

const floppy_image_format_t::desc_e floppy_image_format_t::amiga_11[] = {
	{ SECTOR_LOOP_START, 0, 10 },
	{   MFM, 0x00, 2 },
	{   RAW, 0x4489, 2 },
	{   CRC_AMIGA_START, 1 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_O },
	{     SECTOR_ID_O },
	{     REMAIN_O, 11 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_E },
	{     SECTOR_ID_E },
	{     REMAIN_E, 11 },
	{     MFM, 0x00, 16 },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   CRC, 2 },
	{   CRC_AMIGA_START, 2 },
	{     SECTOR_DATA_O, -1 },
	{     SECTOR_DATA_E, -1 },
	{   CRC_END, 2 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x00, 266 },
	{ END }
};

const floppy_image_format_t::desc_e floppy_image_format_t::amiga_22[] = {
	{ SECTOR_LOOP_START, 0, 21 },
	{   MFM, 0x00, 2 },
	{   RAW, 0x4489, 2 },
	{   CRC_AMIGA_START, 1 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_O },
	{     SECTOR_ID_O },
	{     REMAIN_O, 11 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_E },
	{     SECTOR_ID_E },
	{     REMAIN_E, 11 },
	{     MFM, 0x00, 16 },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   CRC, 2 },
	{   CRC_AMIGA_START, 2 },
	{     SECTOR_DATA_O, -1 },
	{     SECTOR_DATA_E, -1 },
	{   CRC_END, 2 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x00, 532 },
	{ END }
};

std::vector<bool> floppy_image_format_t::generate_bitstream_from_track(int track, int head, int cell_size, const floppy_image &image, int subtrack, int *max_delta)
{
	std::vector<bool> trackbuf;
	const std::vector<uint32_t> &tbuf = image.get_buffer(track, head, subtrack);
	bool track_has_info = false;
	for(uint32_t mg : tbuf)
		if((mg & floppy_image::MG_MASK) == floppy_image::MG_F) {
			track_has_info = true;
			break;
		}

	if(!track_has_info) {
		// Unformatted track
		int track_size = 200000000/cell_size;
		trackbuf.resize(track_size, false);
		return trackbuf;
	}

	class pll {
	private:
		const std::vector<uint32_t> &tbuf;
		int cur_pos;
		int cur_entry;
		int period;
		int period_adjust_base;
		int min_period;
		int max_period;
		int phase_adjust;
		int freq_hist;
		bool next_is_first;

	public:
		int min_delta, max_delta;

		pll(const std::vector<uint32_t> &_tbuf, int cell_size) : tbuf(_tbuf) {
			period = cell_size;
			period_adjust_base = period * 0.05;

			min_period = int(cell_size*0.75);
			max_period = int(cell_size*1.25);
			phase_adjust = 0;
			freq_hist = 0;
			min_delta = 0;
			max_delta = 0;

			// Try to go back 16 flux changes from the end of the track, or at most at the start
			int flux_to_step = 16;
			cur_entry = tbuf.size()-1;
			while(cur_entry > 0 && flux_to_step) {
				if((tbuf[cur_entry] & floppy_image::MG_MASK) == floppy_image::MG_F)
					flux_to_step --;
				cur_entry--;
			}

			// Go back by half-a-period
			cur_pos = (tbuf[cur_entry] & floppy_image::TIME_MASK) - period/2;

			// Adjust the entry accordingly
			while(cur_entry > 0 && (cur_pos > (tbuf[cur_entry] & floppy_image::TIME_MASK)))
				cur_entry --;

			// Now go to the next flux change from there (the no-MG_F case has been handled earlier)
			while((tbuf[cur_entry] & floppy_image::MG_MASK) != floppy_image::MG_F)
				cur_entry ++;

			next_is_first = false;
		}

		std::pair<bool, bool> get() {
			bool bit, first;
			int edge = tbuf[cur_entry] & floppy_image::TIME_MASK;
			if(edge < cur_pos)
				edge += 200000000;
			int next = cur_pos + period + phase_adjust;

			if(edge >= next) {
				// No transition in the window means 0 and pll in free run mode
				bit = false;
				phase_adjust = 0;

			} else {
				// Transition in the window means 1, and the pll is adjusted
				bit = true;

				int delta = edge - (next - period/2);
				if(delta < min_delta)
					min_delta = delta;
				if(delta > max_delta)
					max_delta = delta;

				phase_adjust = 0.65*delta;

				if(delta < 0) {
					if(freq_hist < 0)
						freq_hist--;
					else
						freq_hist = -1;
				} else if(delta > 0) {
					if(freq_hist > 0)
						freq_hist++;
					else
						freq_hist = 1;
				} else
					freq_hist = 0;

				if(freq_hist) {
					int afh = freq_hist < 0 ? -freq_hist : freq_hist;
					if(afh > 1) {
						int aper = period_adjust_base*delta/period;
						if(!aper)
							aper = freq_hist < 0 ? -1 : 1;
						period += aper;

						if(period < min_period)
							period = min_period;
						else if(period > max_period)
							period = max_period;
					}
				}
			}

			first = next_is_first;
			next_is_first = false;

			cur_pos = next;
			if(cur_pos >= 200000000) {
				cur_pos -= 200000000;
				cur_entry = 0;

				if(cur_pos >= period/2)
					first = true;
				else
					next_is_first = true;
			}
			while(cur_entry < int(tbuf.size())-1 && (tbuf[cur_entry] & floppy_image::TIME_MASK) < cur_pos)
				cur_entry++;

			// Wrap around
			if(cur_entry == int(tbuf.size())-1 &&
			   (tbuf[cur_entry] & floppy_image::TIME_MASK) < cur_pos)
				cur_entry = 0;

			return std::make_pair(bit, first);
		}
	};

	pll cpll(tbuf, cell_size);

	for(;;) {
		auto r = cpll.get();
		if(r.second) {
			trackbuf.push_back(r.first);
			break;
		}
	}
	for(;;) {
		auto r = cpll.get();
		if(r.second)
			break;
		trackbuf.push_back(r.first);
	}

	if(max_delta) {
		*max_delta = -cpll.min_delta;
		if(*max_delta < cpll.max_delta)
			*max_delta = cpll.max_delta;
	}

	return trackbuf;
}

std::vector<uint8_t> floppy_image_format_t::generate_nibbles_from_bitstream(const std::vector<bool> &bitstream)
{
	std::vector<uint8_t> res;
	uint32_t pos = 0;
	while(pos < bitstream.size()) {
		while(pos < bitstream.size() && bitstream[pos] == 0)
			pos++;
		if(pos == bitstream.size()) {
			pos = 0;
			while(pos < bitstream.size() && bitstream[pos] == 0)
				pos++;
			if(pos == bitstream.size())
				return res;
			goto found;
		}
		pos += 8;
	}
	while(pos >= bitstream.size())
		pos -= bitstream.size();
	while(pos < bitstream.size() && bitstream[pos] == 0)
		pos++;
 found:
	for(;;) {
		uint8_t v = 0;
		for(uint32_t i=0; i != 8; i++) {
			if(bitstream[pos++])
				v |= 0x80 >> i;
			if(pos == bitstream.size())
				pos = 0;
		}
		res.push_back(v);
		if(pos < 8)
			return res;
		while(pos < bitstream.size() && bitstream[pos] == 0)
			pos++;
		if(pos == bitstream.size())
			return res;
	}
}

int floppy_image_format_t::sbit_rp(const std::vector<bool> &bitstream, uint32_t &pos)
{
	int res = bitstream[pos];
	pos ++;
	if(pos == bitstream.size())
		pos = 0;
	return res;
}

uint8_t floppy_image_format_t::sbyte_mfm_r(const std::vector<bool> &bitstream, uint32_t &pos)
{
	uint8_t res = 0;
	for(int i=0; i<8; i++) {
		sbit_rp(bitstream, pos);
		if(sbit_rp(bitstream, pos))
			res |= 0x80 >> i;
	}
	return res;
}

uint8_t floppy_image_format_t::sbyte_gcr5_r(const std::vector<bool> &bitstream, uint32_t &pos)
{
	uint16_t gcr = 0;
	for(int i=0; i<10; i++) {
		if(sbit_rp(bitstream, pos))
			gcr |= 0x200 >> i;
	}

	return (gcr5bw_tb[gcr >> 5] << 4) | gcr5bw_tb[gcr & 0x1f];
}

std::vector<std::vector<uint8_t>> floppy_image_format_t::extract_sectors_from_bitstream_mfm_pc(const std::vector<bool> &bitstream)
{
	std::vector<std::vector<uint8_t>> sectors;

	// Don't bother if it's just too small
	if(bitstream.size() < 100)
		return sectors;

	// Start by detecting all id and data blocks

	// If 100 is not enough, that track is too funky to be worth
	// bothering anyway

	uint32_t idblk[100], dblk[100];
	uint32_t idblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	uint16_t shift_reg = 0;
	for(uint32_t i=0; i<16; i++)
		if(bitstream[bitstream.size()-16+i])
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for
	// blocks
	for(uint32_t i=0; i<bitstream.size(); i++) {
		shift_reg = (shift_reg << 1) | bitstream[i];
		if(shift_reg == 0x4489) {
			uint16_t header;
			uint32_t pos = i+1;
			do {
				header = 0;
				for(int j=0; j<16; j++)
					if(sbit_rp(bitstream, pos))
						header |= 0x8000 >> j;
				// Accept strings of sync marks as long and they're not wrapping

				// Wrapping ones have already been take into account
				// thanks to the precharging
			} while(header == 0x4489 && pos > i);

			// fe, ff
			if(header == 0x5554 || header == 0x5555) {
				if(idblk_count < 100)
					idblk[idblk_count++] = pos;
				i = pos-1;
			}
			// f8, f9, fa, fb
			if(header == 0x554a || header == 0x5549 || header == 0x5544 || header == 0x5545) {
				if(dblk_count < 100)
					dblk[dblk_count++] = pos;
				i = pos-1;
			}
		}
	}

	// Then extract the sectors
	for(int i=0; i<idblk_count; i++) {
		uint32_t pos = idblk[i];
		[[maybe_unused]] uint8_t track = sbyte_mfm_r(bitstream, pos);
		[[maybe_unused]] uint8_t head = sbyte_mfm_r(bitstream, pos);
		uint8_t sector = sbyte_mfm_r(bitstream, pos);
		uint8_t size = sbyte_mfm_r(bitstream, pos);

		if(size >= 8)
			continue;
		int ssize = 128 << size;

		// Start of IDAM and DAM are supposed to be exactly 704 cells
		// apart in normal format or 1008 cells apart in perpendicular
		// format.  Of course the hardware is tolerant.  Accept +/-
		// 128 cells of shift.

		int d_index;
		for(d_index = 0; d_index < dblk_count; d_index++) {
			int delta = dblk[d_index] - idblk[i];
			if(delta >= 704-128 && delta <= 1008+128)
				break;
		}
		if(d_index == dblk_count)
			continue;

		pos = dblk[d_index];

		if(sectors.size() <= sector)
			sectors.resize(sector+1);
		auto &sdata = sectors[sector];
		sdata.resize(ssize);
		for(int j=0; j<ssize; j++)
			sdata[j] = sbyte_mfm_r(bitstream, pos);
	}

	return sectors;
}

void floppy_image_format_t::get_geometry_mfm_pc(const floppy_image &image, int cell_size, int &track_count, int &head_count, int &sector_count)
{
	image.get_actual_geometry(track_count, head_count);

	if(!track_count) {
		sector_count = 0;
		return;
	}

	// Extract an arbitrary track to get an idea of the number of
	// sectors

	// 20 was rarely used for protections, not near the start like
	// 0-10, not near the end like 70+, no special effects on sync
	// like 33

	auto buf = generate_bitstream_from_track(track_count > 20 ? 20 : 0, 0, cell_size, image);
	auto sectors = extract_sectors_from_bitstream_mfm_pc(buf);
	sector_count = sectors.size();
}


void floppy_image_format_t::get_track_data_mfm_pc(int track, int head, const floppy_image &image, int cell_size, int sector_size, int sector_count, uint8_t *sectdata)
{
	auto bitstream = generate_bitstream_from_track(track, head, cell_size, image);
	auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
	for(int sector=1; sector <= sector_count; sector++) {
		uint8_t *sd = sectdata + (sector-1)*sector_size;
		if(sector < sectors.size() && !sectors[sector].empty()) {
			unsigned int asize = sectors[sector].size();
			if(asize > sector_size)
				asize = sector_size;
			memcpy(sd, sectors[sector].data(), asize);
			if(asize < sector_size)
				memset(sd+asize, 0, sector_size-asize);
		} else
			memset(sd, 0, sector_size);
	}
}


std::vector<std::vector<uint8_t>> floppy_image_format_t::extract_sectors_from_bitstream_fm_pc(const std::vector<bool> &bitstream)
{
	std::vector<std::vector<uint8_t>> sectors;

	// Don't bother if it's just too small
	if(bitstream.size() < 100)
		return sectors;

	// Start by detecting all id and data blocks

	// If 100 is not enough, that track is too funky to be worth
	// bothering anyway

	uint32_t idblk[100], dblk[100];
	uint32_t idblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	uint16_t shift_reg = 0;
	for(int i=0; i<16; i++)
		if(bitstream[bitstream.size()-16+i])
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for
	// blocks
	// We scan for address marks only, as index marks are not mandatory,
	// and many formats actually do not use them

	for(uint32_t i=0; i<bitstream.size(); i++) {
		shift_reg = (shift_reg << 1) | bitstream[i];

		// fe
		if(shift_reg == 0xf57e) {       // address mark
			if(idblk_count < 100)
				idblk[idblk_count++] = i+1;
		}
		// f8, f9, fa, fb
		if(shift_reg == 0xf56a || shift_reg == 0xf56b ||
			shift_reg == 0xf56e || shift_reg == 0xf56f) {       // data mark
			if(dblk_count < 100)
				dblk[dblk_count++] = i+1;
		}
	}

	// Then extract the sectors
	for(uint32_t i=0; i<idblk_count; i++) {
		uint32_t pos = idblk[i];
		[[maybe_unused]] uint8_t track = sbyte_mfm_r(bitstream, pos);
		[[maybe_unused]] uint8_t head = sbyte_mfm_r(bitstream, pos);
		uint8_t sector = sbyte_mfm_r(bitstream, pos);
		uint8_t size = sbyte_mfm_r(bitstream, pos);
		if(size >= 8)
			continue;
		int ssize = 128 << size;

		// Start of IDAM and DAM are supposed to be exactly 384 cells
		// apart.  Of course the hardware is tolerant.  Accept +/- 128
		// cells of shift.

		int d_index;
		for(d_index = 0; d_index < dblk_count; d_index++) {
			int delta = dblk[d_index] - idblk[i];
			if(delta >= 384-128 && delta <= 384+128)
				break;
		}
		if(d_index == dblk_count)
			continue;

		pos = dblk[d_index];

		if(sectors.size() <= sector)
			sectors.resize(sector+1);
		auto &sdata = sectors[sector];
		sdata.resize(ssize);
		for(int j=0; j<ssize; j++)
			sdata[j] = sbyte_mfm_r(bitstream, pos);
	}

	return sectors;
}

void floppy_image_format_t::get_geometry_fm_pc(const floppy_image &image, int cell_size, int &track_count, int &head_count, int &sector_count)
{
	image.get_actual_geometry(track_count, head_count);

	if(!track_count) {
		sector_count = 0;
		return;
	}

	// Extract an arbitrary track to get an idea of the number of
	// sectors

	// 20 was rarely used for protections, not near the start like
	// 0-10, not near the end like 70+, no special effects on sync
	// like 33

	auto bitstream = generate_bitstream_from_track(track_count > 20 ? 20 : 0, 0, cell_size, image);
	auto sectors = extract_sectors_from_bitstream_fm_pc(bitstream);
	sector_count = sectors.size();
}


void floppy_image_format_t::get_track_data_fm_pc(int track, int head, const floppy_image &image, int cell_size, int sector_size, int sector_count, uint8_t *sectdata)
{
	auto bitstream = generate_bitstream_from_track(track, head, cell_size, image);
	auto sectors = extract_sectors_from_bitstream_fm_pc(bitstream);
	for(unsigned int sector=1; sector < sector_count; sector++) {
		uint8_t *sd = sectdata + (sector-1)*sector_size;
		if(sector < sectors.size() && !sectors[sector].empty()) {
			unsigned int asize = sectors[sector].size();
			if(asize > sector_size)
				asize = sector_size;
			memcpy(sd, sectors[sector].data(), asize);
			if(asize < sector_size)
				memset(sd+asize, 0, sector_size-asize);
		} else
			memset(sd, 0, sector_size);
	}
}

int floppy_image_format_t::calc_default_pc_gap3_size(uint32_t form_factor, int sector_size)
{
	return
		form_factor == floppy_image::FF_8 ? 25 :
		sector_size < 512 ?
		(form_factor == floppy_image::FF_35 ? 54 : 50) :
		(form_factor == floppy_image::FF_35 ? 84 : 80);
}

void floppy_image_format_t::build_wd_track_fm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2)
{
	build_pc_track_fm(track, head, image, cell_count, sector_count, sects, gap_3, -1, gap_1, gap_2);
}

void floppy_image_format_t::build_wd_track_mfm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2)
{
	build_pc_track_mfm(track, head, image, cell_count, sector_count, sects, gap_3, -1, gap_1, gap_2);
}

void floppy_image_format_t::build_pc_track_fm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a, int gap_1, int gap_2)
{
	std::vector<uint32_t> track_data;

	// gap 4a, IAM and gap 1
	if(gap_4a != -1) {
		for(int i=0; i<gap_4a; i++) fm_w(track_data, 8, 0xff);
		for(int i=0; i< 6;     i++) fm_w(track_data, 8, 0x00);
		raw_w(track_data, 16, 0xf77a);
	}
	for(int i=0; i<gap_1; i++) fm_w(track_data, 8, 0xff);

	int total_size = 0;
	for(int i=0; i<sector_count; i++)
		total_size += sects[i].actual_size;

	unsigned int etpos = track_data.size() + (sector_count*(6+5+2+gap_2+6+1+2) + total_size)*16;

	if(etpos > cell_count)
		throw std::invalid_argument(util::string_format("Incorrect layout on track %d head %d, expected_size=%d, current_size=%d", track, head, cell_count, etpos));

	if(etpos + gap_3*16*(sector_count-1) > cell_count)
		gap_3 = (cell_count - etpos) / 16 / (sector_count-1);

	// Build the track
	for(int i=0; i<sector_count; i++) {
		uint16_t crc;
		// sync and IDAM and gap 2
		for(int j=0; j< 6; j++) fm_w(track_data, 8, 0x00);

		unsigned int cpos = track_data.size();
		raw_w(track_data, 16, 0xf57e);
		fm_w (track_data, 8, sects[i].track);
		fm_w (track_data, 8, sects[i].head);
		fm_w (track_data, 8, sects[i].sector);
		fm_w (track_data, 8, sects[i].size);
		crc = calc_crc_ccitt(track_data, cpos, track_data.size());
		fm_w (track_data, 16, crc);
		for(int j=0; j<gap_2; j++) fm_w(track_data, 8, 0xff);

		if(!sects[i].data)
			for(int j=0; j<6+1+sects[i].actual_size+2+(i != sector_count-1 ? gap_3 : 0); j++) fm_w(track_data, 8, 0xff);

		else {
			// sync, DAM, data and gap 3
			for(int j=0; j< 6; j++) fm_w(track_data, 8, 0x00);
			cpos = track_data.size();
			raw_w(track_data, 16, sects[i].deleted ? 0xf56a : 0xf56f);
			for(int j=0; j<sects[i].actual_size; j++) fm_w(track_data, 8, sects[i].data[j]);
			crc = calc_crc_ccitt(track_data, cpos, track_data.size());
			if(sects[i].bad_crc)
				crc = 0xffff^crc;
			fm_w(track_data, 16, crc);
			if(i != sector_count-1)
				for(int j=0; j<gap_3; j++) fm_w(track_data, 8, 0xff);
		}
	}

	// Gap 4b

	while(int(track_data.size()) < cell_count-15) fm_w(track_data, 8, 0xff);
	raw_w(track_data, cell_count-int(track_data.size()), 0xffff >> (16+int(track_data.size())-cell_count));

	generate_track_from_levels(track, head, track_data, 0, image);
}

void floppy_image_format_t::build_pc_track_mfm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a, int gap_1, int gap_2)
{
	std::vector<uint32_t> track_data;

	// gap 4a, IAM and gap 1
	if(gap_4a != -1) {
		for(int i=0; i<gap_4a; i++) mfm_w(track_data, 8, 0x4e);
		for(int i=0; i<12;     i++) mfm_w(track_data, 8, 0x00);
		for(int i=0; i< 3;     i++) raw_w(track_data, 16, 0x5224);
		mfm_w(track_data, 8, 0xfc);
	}
	for(int i=0; i<gap_1; i++) mfm_w(track_data, 8, 0x4e);

	int total_size = 0;
	for(int i=0; i<sector_count; i++)
		total_size += sects[i].actual_size;

	int etpos = int(track_data.size()) + (sector_count*(12+3+5+2+gap_2+12+3+1+2) + total_size)*16;

	if(etpos > cell_count)
		throw std::invalid_argument(util::string_format("Incorrect layout on track %d head %d, expected_size=%d, current_size=%d", track, head, cell_count, etpos));

	if(etpos + gap_3*16*(sector_count-1) > cell_count)
		gap_3 = (cell_count - etpos) / 16 / (sector_count-1);

	// Build the track
	for(int i=0; i<sector_count; i++) {
		uint16_t crc;
		// sync and IDAM and gap 2
		for(int j=0; j<12; j++) mfm_w(track_data, 8, 0x00);
		unsigned int cpos = track_data.size();
		for(int j=0; j< 3; j++) raw_w(track_data, 16, 0x4489);
		mfm_w(track_data, 8, 0xfe);
		mfm_w(track_data, 8, sects[i].track);
		mfm_w(track_data, 8, sects[i].head);
		mfm_w(track_data, 8, sects[i].sector);
		mfm_w(track_data, 8, sects[i].size);
		crc = calc_crc_ccitt(track_data, cpos, track_data.size());
		mfm_w(track_data, 16, crc);
		for(int j=0; j<gap_2; j++) mfm_w(track_data, 8, 0x4e);

		if(!sects[i].data)
			for(int j=0; j<12+4+sects[i].actual_size+2+(i != sector_count-1 ? gap_3 : 0); j++) mfm_w(track_data, 8, 0x4e);

		else {
			// sync, DAM, data and gap 3
			for(int j=0; j<12; j++) mfm_w(track_data, 8, 0x00);
			cpos = track_data.size();
			for(int j=0; j< 3; j++) raw_w(track_data, 16, 0x4489);
			mfm_w(track_data, 8, sects[i].deleted ? 0xf8 : 0xfb);
			for(int j=0; j<sects[i].actual_size; j++) mfm_w(track_data, 8, sects[i].data[j]);
			crc = calc_crc_ccitt(track_data, cpos, track_data.size());
			if(sects[i].bad_crc)
				crc = 0xffff^crc;
			mfm_w(track_data, 16, crc);
			if(i != sector_count-1)
				for(int j=0; j<gap_3; j++) mfm_w(track_data, 8, 0x4e);
		}
	}

	// Gap 4b

	while(int(track_data.size()) < cell_count-15) mfm_w(track_data, 8, 0x4e);
	raw_w(track_data, cell_count-int(track_data.size()), 0x9254 >> (16+int(track_data.size())-cell_count));

	generate_track_from_levels(track, head, track_data, 0, image);
}

void floppy_image_format_t::build_mac_track_gcr(int track, int head, floppy_image &image, const desc_gcr_sector *sects)
{
	// 30318342 = 60.0 / 1.979e-6
	static const uint32_t cells_per_speed_zone[5] = {
		30318342 / 394,
		30318342 / 429,
		30318342 / 472,
		30318342 / 525,
		30318342 / 590
	};

	static const std::array<uint8_t, 12> notag{};

	uint32_t speed_zone = track/16;
	if(speed_zone > 4)
		speed_zone = 4;

	uint32_t sectors = 12 - speed_zone;
	uint32_t pregap = cells_per_speed_zone[speed_zone] - 6208 * sectors;

	std::vector<uint32_t> buffer;

	uint32_t prepregap = pregap % 48;
	if(prepregap >= 24) {
		raw_w(buffer, prepregap - 24, 0xff3fcf);
		raw_w(buffer, 24, 0xf3fcff);
	} else
		raw_w(buffer, prepregap, 0xf3fcff);

	for(uint32_t i = 0; i != pregap / 48; i++) {
		raw_w(buffer, 24, 0xff3fcf);
		raw_w(buffer, 24, 0xf3fcff);
	}

	for(uint32_t s = 0; s != sectors; s++) {
		for(uint32_t i=0; i != 8; i++) {
			raw_w(buffer, 24, 0xff3fcf);
			raw_w(buffer, 24, 0xf3fcff);
		}

		raw_w(buffer, 24, 0xd5aa96);
		raw_w(buffer, 8, gcr6fw_tb[sects[s].track & 0x3f]);
		raw_w(buffer, 8, gcr6fw_tb[sects[s].sector & 0x3f]);
		raw_w(buffer, 8, gcr6fw_tb[(sects[s].track & 0x40 ? 1 : 0) | (sects[s].head ? 0x20 : 0)]);
		raw_w(buffer, 8, gcr6fw_tb[sects[s].info & 0x3f]);
		uint8_t check = sects[s].track ^ sects[s].sector ^ ((sects[s].track & 0x40 ? 1 : 0) | (sects[s].head ? 0x20 : 0)) ^ sects[s].info;
		raw_w(buffer, 8, gcr6fw_tb[check & 0x3f]);
		raw_w(buffer, 24, 0xdeaaff);

		raw_w(buffer, 24, 0xff3fcf);
		raw_w(buffer, 24, 0xf3fcff);

		raw_w(buffer, 24, 0xd5aaad);
		raw_w(buffer, 8, gcr6fw_tb[sects[s].sector & 0x3f]);

		const uint8_t *data = sects[s].tag;
		if(!data)
			data = notag.data();

		uint8_t ca = 0, cb = 0, cc = 0;
		for(int i=0; i < 175; i ++) {
			if(i == 4)
				data = sects[s].data - 3*4;

			uint8_t va = data[3*i];
			uint8_t vb = data[3*i+1];
			uint8_t vc = i != 174 ? data[3*i+2] : 0;

			cc = (cc << 1) | (cc >> 7);
			uint16_t suma = ca + va + (cc & 1);
			ca = suma;
			va = va ^ cc;
			uint16_t sumb = cb + vb + (suma >> 8);
			cb = sumb;
			vb = vb ^ ca;
			if(i != 174)
				cc = cc + vc + (sumb >> 8);
			vc = vc ^ cb;

			uint32_t nb = i != 174 ? 32 : 24;
			raw_w(buffer, nb, gcr6_encode(va, vb, vc) >> (32-nb));
		}

		raw_w(buffer, 32, gcr6_encode(ca, cb, cc));
		raw_w(buffer, 32, 0xdeaaffff);
	}

	generate_track_from_levels(track, head, buffer, 0, image);
}

std::vector<std::vector<uint8_t>> floppy_image_format_t::extract_sectors_from_track_mac_gcr6(int head, int track, const floppy_image &image)
{
	// 200000000 / 60.0 * 1.979e-6 ~= 6.5967
	static const int cell_size_per_speed_zone[5] = {
		394 * 65967 / 10000,
		429 * 65967 / 10000,
		472 * 65967 / 10000,
		525 * 65967 / 10000,
		590 * 65967 / 10000
	};

	uint32_t speed_zone = track/16;
	if(speed_zone > 4)
		speed_zone = 4;

	uint32_t sectors = 12 - speed_zone;

	std::vector<std::vector<uint8_t>> sector_data(sectors);

	auto buf = generate_bitstream_from_track(track, head, cell_size_per_speed_zone[speed_zone], image);
	auto nib = generate_nibbles_from_bitstream(buf);

	if(nib.size() < 300)
		return sector_data;

	std::vector<uint32_t> hpos;

	uint32_t hstate = get_u16be(&nib[nib.size() - 2]);
	for(uint32_t pos = 0; pos != nib.size(); pos++) {
		hstate = ((hstate << 8) | nib[pos]) & 0xffffff;
		if(hstate == 0xd5aa96)
			hpos.push_back(pos == nib.size() - 1 ? 0 : pos+1);
	}

	for(uint32_t pos : hpos) {
		uint8_t h[7];

		for(auto &e : h) {
			e = nib[pos];
			pos ++;
			if(pos == nib.size())
				pos = 0;
		}

		uint8_t v2 = gcr6bw_tb[h[2]];
		uint8_t v3 = gcr6bw_tb[h[3]];
		uint8_t tr = gcr6bw_tb[h[0]] | (v2 & 1 ? 0x40 : 0x00);
		uint8_t se = gcr6bw_tb[h[1]];
		//                  uint8_t si = v2 & 0x20 ? 1 : 0;
		//                  uint8_t ds = v3 & 0x20 ? 1 : 0;
		//                  uint8_t fmt = v3 & 0x1f;
		uint8_t c1 = (tr^se^v2^v3) & 0x3f;
		uint8_t chk = gcr6bw_tb[h[4]];
		if(chk != c1 || se >= sectors || h[5] != 0xde || h[6] != 0xaa)
			continue;

		auto &sdata = sector_data[se];
		uint8_t ca = 0, cb = 0, cc = 0;

		uint32_t hstate = (nib[pos] << 8);
		pos ++;
		if(pos == nib.size())
			pos = 0;
		hstate |= nib[pos];
		pos ++;
		if(pos == nib.size())
			pos = 0;
		for(;;) {
			hstate = ((hstate << 8) | nib[pos]) & 0xffffff;
			pos ++;
			if(pos == nib.size())
				pos = 0;
			if(hstate == 0xd5aa96)
				goto no_data_field;
			if(hstate == 0xd5aaad)
				break;
		}
		pos ++; // skip the sector byte
		if(pos == nib.size())
			pos = 0;

		sdata.resize(512+12);
		for(int i=0; i < 175; i++) {
			uint8_t e0 = nib[pos++];
			if(pos == nib.size())
				pos = 0;
			uint8_t e1 = nib[pos++];
			if(pos == nib.size())
				pos = 0;
			uint8_t e2 = nib[pos++];
			if(pos == nib.size())
				pos = 0;
			uint8_t e3 = i < 174 ? nib[pos++] : 0x96;
			if(pos == nib.size())
				pos = 0;

			uint8_t va, vb, vc;
			gcr6_decode(e0, e1, e2, e3, va, vb, vc);
			cc = (cc << 1) | (cc >> 7);
			va = va ^ cc;
			uint16_t suma = ca + va + (cc & 1);
			ca = suma;
			vb = vb ^ ca;
			uint16_t sumb = cb + vb + (suma >> 8);
			cb = sumb;
			vc = vc ^ cb;

			sdata[3*i] = va;
			sdata[3*i+1] = vb;

			if(i != 174) {
				cc = cc + vc + (sumb >> 8);
				sdata[3*i+2] = vc;
			}
		}
		for(auto &e : h) {
			e = nib[pos];
			pos ++;
			if(pos == nib.size())
				pos = 0;
		}
		uint8_t va, vb, vc;
		gcr6_decode(h[0], h[1], h[2], h[3], va, vb, vc);
		if(va != ca || vb != cb || vc != cc || h[4] != 0xde || h[5] != 0xaa)
			sdata.clear();
	no_data_field:
		;
	}

	return sector_data;
}


std::vector<std::vector<uint8_t>> floppy_image_format_t::extract_sectors_from_bitstream_gcr5(const std::vector<bool> &bitstream, int head, int tracks)
{
	std::vector<std::vector<uint8_t>> sectors;

	// Don't bother if it's just too small
	if(bitstream.size() < 100)
		return sectors;

	// Start by detecting all id and data blocks
	uint32_t hblk[100]{}, dblk[100]{};
	uint32_t hblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	uint16_t shift_reg = 0;
	for(uint32_t i=0; i<16; i++)
		if(bitstream[bitstream.size()-16+i])
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for blocks
	bool sync = false;
	for(uint32_t i=0; i<bitstream.size(); i++) {
		int bit = bitstream[i];
		shift_reg = ((shift_reg << 1) | bit) & 0x3ff;

		if (sync && !bit) {
			uint8_t id = sbyte_gcr5_r(bitstream, i);

			switch (id) {
			case 0x08:
				if(hblk_count < 100)
					hblk[hblk_count++] = i-10;
				break;

			case 0x07:
				if(dblk_count < 100)
					dblk[dblk_count++] = i-10;
				break;
			}
		}

		sync = (shift_reg == 0x3ff);
	}

	// Then extract the sectors
	for(int i=0; i<hblk_count; i++) {
		uint32_t pos = hblk[i];
		[[maybe_unused]] uint8_t block_id = sbyte_gcr5_r(bitstream, pos);
		uint8_t crc = sbyte_gcr5_r(bitstream, pos);
		uint8_t sector = sbyte_gcr5_r(bitstream, pos);
		uint8_t track = sbyte_gcr5_r(bitstream, pos);
		uint8_t id2 = sbyte_gcr5_r(bitstream, pos);
		uint8_t id1 = sbyte_gcr5_r(bitstream, pos);

		if (crc ^ sector ^ track ^ id2 ^ id1) {
			// header crc mismatch
			continue;
		}

		pos = dblk[i];
		block_id = sbyte_gcr5_r(bitstream, pos);

		if (track > tracks) track -= tracks;
		if(sectors.size() <= sector)
			sectors.resize(sector+1);
		auto &sdata = sectors[sector];
		sdata.resize(256);
		uint8_t data_crc = 0;
		for(int j=0; j<256; j++) {
			uint8_t data = sbyte_gcr5_r(bitstream, pos);
			data_crc ^= data;
			sdata[j] = data;
		}
		data_crc ^= sbyte_gcr5_r(bitstream, pos);
		if (data_crc) {
			// data crc mismatch
			sdata.clear();
		}
	}

	return sectors;
}

std::vector<std::vector<uint8_t>> floppy_image_format_t::extract_sectors_from_bitstream_victor_gcr5(const std::vector<bool> &bitstream)
{
	std::vector<std::vector<uint8_t>> sectors;

	// Don't bother if it's just too small
	if(bitstream.size() < 100)
		return sectors;

	// Start by detecting all id and data blocks
	uint32_t hblk[100]{}, dblk[100]{};
	uint32_t hblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	uint16_t shift_reg = 0;
	for(uint32_t i=0; i<16; i++)
		if(bitstream[bitstream.size()-16+i])
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for blocks
	bool sync = false;
	for(uint32_t i=0; i<bitstream.size(); i++) {
		int bit = bitstream[i];
		shift_reg = ((shift_reg << 1) | bit) & 0x3ff;

		if (sync && !bit) {
			uint8_t id = sbyte_gcr5_r(bitstream, i);

			switch (id) {
			case 0x07:
				if(hblk_count < 100)
					hblk[hblk_count++] = i-10;
				break;

			case 0x08:
				if(dblk_count < 100)
					dblk[dblk_count++] = i-10;
				break;
			}
		}

		sync = (shift_reg == 0x3ff);
	}

	// Then extract the sectors
	for(int i=0; i<hblk_count; i++) {
		uint32_t pos = hblk[i];
		[[maybe_unused]] uint8_t block_id = sbyte_gcr5_r(bitstream, pos);
		[[maybe_unused]] uint8_t track = sbyte_gcr5_r(bitstream, pos);
		uint8_t sector = sbyte_gcr5_r(bitstream, pos);

		pos = dblk[i];
		block_id = sbyte_gcr5_r(bitstream, pos);

		if(sectors.size() <= sector)
			sectors.resize(sector+1);
		auto &sdata = sectors[sector];
		sdata.resize(512);

		for(int j=0; j<512; j++)
			sdata[j] = sbyte_gcr5_r(bitstream, pos);
	}

	return sectors;
}
