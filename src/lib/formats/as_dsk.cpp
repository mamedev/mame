// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Applesauce solved output formats

#include "as_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <string.h>


namespace {

template <typename T>
uint32_t crc32r(T &&data, uint32_t size)
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

template <typename T>
uint32_t find_tag(T &&data, size_t size, uint32_t tag)
{
	uint32_t offset = 12;
	do {
		if(get_u32le(&data[offset]) == tag)
			return offset + 8;
		offset += get_u32le(&data[offset+4]) + 8;
	} while(offset < (size - 8));
	return 0;
}

} // anonymous namespace


as_format::as_format() : floppy_image_format_t()
{
}


bool as_format::load_bitstream_track(const uint8_t *img, floppy_image &image, int head, int track, int subtrack, uint8_t idx, uint32_t off_trks, bool may_be_short, bool set_variant)
{
	uint32_t trks_off = off_trks + (idx * 8);

	uint32_t track_size = get_u32le(&img[trks_off + 4]);
	if (track_size == 0)
		return false;

	uint32_t boff = (uint32_t)get_u16le(&img[trks_off + 0]) * 512;

	// With 5.25 floppies the end-of-track may be missing
	// if unformatted.  Accept track length down to 95% of
	// 51090, otherwise pad it

	bool short_track = may_be_short && track_size < 48535;

	if(short_track) {
		std::vector<uint8_t> buffer(6387, 0);
		memcpy(buffer.data(), &img[boff], (track_size + 7) / 8);
		generate_track_from_bitstream(track, head, buffer.data(), 51090, image, subtrack, 0xffff);

	} else
		generate_track_from_bitstream(track, head, &img[boff], track_size, image, subtrack, 0xffff);

	if(set_variant)
		image.set_variant(get_u32le(&img[trks_off + 4]) >= 90000 ? floppy_image::DSHD : floppy_image::DSDD);
	return true;
}

void as_format::load_flux_track(const uint8_t *img, floppy_image &image, int head, int track, int subtrack, uint8_t fidx, uint32_t off_trks)
{
	uint32_t trks_off = off_trks + (fidx * 8);
	uint32_t boff = (uint32_t)get_u16le(&img[trks_off + 0]) * 512;
	uint32_t track_size = get_u32le(&img[trks_off + 4]);

	uint32_t total_ticks = 0;
	for(uint32_t i=0; i != track_size; i++)
		total_ticks += img[boff+i];

	// There is always a pulse at index, and it's
	// the last one in the stream
	std::vector<uint32_t> &buf = image.get_buffer(track, head, subtrack);
	buf.push_back(floppy_image::MG_F | 0);
	uint32_t cpos = 0;
	for(uint32_t i=0; i != track_size; i++) {
		uint8_t step = img[boff+i];
		cpos += step;
		if(step != 0xff && i != track_size-1)
			buf.push_back(floppy_image::MG_F | uint64_t(cpos)*200000000/total_ticks);
	}
}


as_format::tdata as_format::analyze_for_save(const floppy_image &image, int head, int track, int subtrack, int speed_zone)
{
	// 200000000 / 60.0 * 1.979e-6 ~= 6.5967
	static const int cell_size_per_speed_zone[7] = {
		394 * 65967 / 10000,
		429 * 65967 / 10000,
		472 * 65967 / 10000,
		525 * 65967 / 10000,
		590 * 65967 / 10000,

		3915,
		1000
	};

	static const int ticks_per_speed_zone[7] = {
		60*8000000 / 394,
		60*8000000 / 429,
		60*8000000 / 472,
		60*8000000 / 525,
		60*8000000 / 590,

		1333333,
		1600000
	};

	tdata result;

	if(!image.track_is_formatted(track, head, subtrack))
		return result;

	// Generate a bitstream to get the data and whether the phase is clean
	int cell_size = cell_size_per_speed_zone[speed_zone];
	int max_delta;
	std::vector<bool> bitstream = generate_bitstream_from_track(track, head, cell_size, image, subtrack, &max_delta);

	// Bitstreams encodable as non-flux have a max_delta as 10% or less, otherwise it's 40% or more.  Use 20% as the limit
	if(max_delta <= cell_size/5) {
		result.track_size = bitstream.size();
		result.data.resize((bitstream.size()+7)/8, 0);
		for(unsigned j=0; j != bitstream.size(); j++)
			if(bitstream[j])
				result.data[j >> 3] |= 0x80 >> (j & 7);
		return result;
	}

	result.flux = true;

	const std::vector<uint32_t> &tbuf = image.get_buffer(track, head, subtrack);
	uint32_t first_edge = 0, last_edge = 0;
	for(uint32_t fp : tbuf)
		if((fp & floppy_image::MG_MASK) == floppy_image::MG_F) {
			first_edge = fp & floppy_image::TIME_MASK;
			break;
		}
	for(auto i = tbuf.rbegin(); i != tbuf.rend(); ++i)
		if((*i & floppy_image::MG_MASK) == floppy_image::MG_F) {
			last_edge = *i & floppy_image::TIME_MASK;
			break;
		}

	int dt = last_edge - 200000000;
	if((-dt) < first_edge)
		dt = first_edge;

	if(dt < -10000 || dt > 10000)
		dt = 0;

	uint32_t cur_tick = 0;
	uint64_t ticks = ticks_per_speed_zone[speed_zone];
	for(uint32_t fp : tbuf)
		if((fp & floppy_image::MG_MASK) == floppy_image::MG_F) {
			uint32_t next_tick = ((fp & floppy_image::TIME_MASK) - dt) * ticks / 200000000;
			uint32_t cdt = next_tick - cur_tick;
			if(cdt) {
				while(cdt >= 255) {
					result.data.push_back(255);
					cdt -= 255;
				}
				result.data.push_back(cdt);
			}
			cur_tick = next_tick;
		}

	uint32_t cdt = ticks - cur_tick;
	if(cdt) {
		while(cdt >= 255) {
			result.data.push_back(255);
			cdt -= 255;
		}
		result.data.push_back(cdt);
	}

	result.track_size = result.data.size();
	return result;
}

std::pair<int, int> as_format::count_blocks(const std::vector<tdata> &tracks)
{
	int max_blocks = 0;
	int total_blocks = 0;
	for(const auto &t : tracks) {
		int blocks = (t.data.size() + 511) / 512;
		total_blocks += blocks;
		if(max_blocks < blocks)
			max_blocks = blocks;
	}
	return std::make_pair(total_blocks, max_blocks);
}

bool as_format::test_flux(const std::vector<tdata> &tracks)
{
	for(const auto &t : tracks)
		if(t.flux)
			return true;
	return false;
}

void as_format::save_tracks(std::vector<uint8_t> &data, const std::vector<tdata> &tracks, uint32_t total_blocks, bool has_flux)
{
	put_u32le(&data[80], 0x50414d54);   // TMAP
	put_u32le(&data[84], 160);          // size

	uint32_t fstart = 1536 + total_blocks*512;
	if(has_flux) {
		put_u32le(&data[fstart], 0x58554c46);
		put_u32le(&data[fstart+4], 160);
		fstart += 8;
	}

	memset(data.data()+88, 0xff, 160);
	if(has_flux)
		memset(data.data()+fstart, 0xff, 160);

	uint8_t tcount = 0;
	for(int i=0; i != 160 ; i++) {
		if(!tracks[i].data.empty()) {
			if(!tracks[i].flux)
				data[88+i] = tcount;
			else
				data[fstart+i] = tcount;
			tcount++;
		}
	}

	put_u32le(&data[248], 0x534b5254); // TRKS
	put_u32le(&data[252], 1280 + total_blocks*512);   // size

	uint8_t tid = 0;
	uint16_t tb = 3;
	for(int i=0; i != 160 ; i++)
		if(!tracks[i].data.empty()) {
			int size = tracks[i].data.size();
			int blocks = (size + 511) / 512;
			memcpy(data.data() + tb*512, tracks[i].data.data(), size);
			put_u16le(&data[256 + tid*8], tb);
			put_u16le(&data[256 + tid*8 + 2], blocks);
			put_u32le(&data[256 + tid*8 + 4], tracks[i].track_size);
			tb += blocks;
			tid ++;
		}

	put_u32le(&data[8], crc32r(&data[12], data.size() - 12));
}



woz_format::woz_format() : as_format()
{
}

const char *woz_format::name() const noexcept
{
	return "woz";
}

const char *woz_format::description() const noexcept
{
	return "Apple II WOZ Image";
}

const char *woz_format::extensions() const noexcept
{
	return "woz";
}

bool woz_format::supports_save() const noexcept
{
	return true;
}

const uint8_t woz_format::signature[8] = { 0x57, 0x4f, 0x5a, 0x31, 0xff, 0x0a, 0x0d, 0x0a };
const uint8_t woz_format::signature2[8] = { 0x57, 0x4f, 0x5a, 0x32, 0xff, 0x0a, 0x0d, 0x0a };

int woz_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[8];
	auto const [err, actual] = read_at(io, 0, header, 8);
	if(err || (8 != actual)) return 0;
	if(!memcmp(header, signature, 8)) return FIFID_SIGN;
	if(!memcmp(header, signature2, 8)) return FIFID_SIGN;
	return 0;
}

bool woz_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t image_size;
	if(io.length(image_size))
		return false;
	auto const [err, img, actual] = read_at(io, 0, image_size);
	if(err || (actual != image_size))
		return false;

	// Check signature
	if((memcmp(&img[0], signature, 8)) && (memcmp(&img[0], signature2, 8)))
		return false;

	uint32_t woz_vers = 1;
	if(!memcmp(&img[0], signature2, 8)) woz_vers = 2;

	// Check integrity
	uint32_t crc = crc32r(&img[12], image_size - 12);
	if(crc != get_u32le(&img[8]))
		return false;

	uint32_t off_info = find_tag(img, image_size, 0x4f464e49);
	uint32_t off_tmap = find_tag(img, image_size, 0x50414d54);
	uint32_t off_trks = find_tag(img, image_size, 0x534b5254);
//  uint32_t off_writ = find_tag(img, image_size, 0x54495257);

	if(!off_info || !off_tmap || !off_trks)
		return false;

	uint32_t info_vers = img[off_info + 0];
	if(info_vers < 1 || info_vers > 3)
		return false;

	uint16_t off_flux = (info_vers < 3) ? 0 : get_u16le(&img[off_info + 46]);
	uint16_t flux_size = (info_vers < 3) ? 0 : get_u16le(&img[off_info + 48]);

	if(!flux_size)
		off_flux = 0;

	bool is_35 = img[off_info + 1] == 2;

	if((form_factor == floppy_image::FF_35 && !is_35) || (form_factor == floppy_image::FF_525 && is_35))
		return false;

	unsigned int limit = is_35 ? 160 : 141;

	if(is_35)
		image.set_form_variant(floppy_image::FF_35, floppy_image::SSDD);
	else
		image.set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	if(woz_vers == 1) {
		for (unsigned int trkid = 0; trkid != limit; trkid++) {
			int head = is_35 && trkid >= 80 ? 1 : 0;
			int track = is_35 ? trkid % 80 : trkid / 4;
			int subtrack = is_35 ? 0 : trkid & 3;

			uint8_t idx = img[off_tmap + trkid];
			if(idx != 0xff) {
				uint32_t boff = off_trks + 6656*idx;
				if (get_u16le(&img[boff + 6648]) == 0)
					return false;
				generate_track_from_bitstream(track, head, &img[boff], get_u16le(&img[boff + 6648]), image, subtrack, get_u16le(&img[boff + 6650]));
				if(is_35 && !track && head)
					image.set_variant(floppy_image::DSDD);
			}
		}
	} else if(woz_vers == 2) {
		for (unsigned int trkid = 0; trkid != limit; trkid++) {
			int head = is_35 && trkid & 1 ? 1 : 0;
			int track = is_35 ? trkid >> 1 : trkid / 4;
			int subtrack = is_35 ? 0 : trkid & 3;

			uint8_t idx = img[off_tmap + trkid];
			uint8_t fidx = off_flux ? img[off_flux*512 + 8 + trkid] : 0xff;

			if(fidx != 0xff) {
				load_flux_track(&img[0], image, head, track, subtrack, fidx, off_trks);

			} else if(idx != 0xff) {
				if(!load_bitstream_track(&img[0], image, head, track, subtrack, idx, off_trks, !is_35, is_35 && !track && head))
					return false;
			}
		}
	}
	else return false;

	return true;
}


bool woz_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	std::vector<tdata> tracks(160);
	bool twosided = false;

	if(image.get_form_factor() == floppy_image::FF_525) {
		for(unsigned int i=0; i != 141; i++)
			tracks[i] = analyze_for_save(image, 0, i >> 2, i & 3, 5);

	} else if(image.get_variant() == floppy_image::DSHD) {
		for(unsigned int i=0; i != 160; i++) {
			tracks[i] = analyze_for_save(image, i & 1, i >> 1, 0, 6);
			if((i & 1) && tracks[i].track_size)
				twosided = true;
		}

	} else {
		for(unsigned int i=0; i != 160; i++) {
			tracks[i] = analyze_for_save(image, i & 1, i >> 1, 0, i / (2*16));
			if((i & 1) && tracks[i].track_size)
				twosided = true;
		}
	}

	auto [total_blocks, max_blocks] = count_blocks(tracks);
	bool has_flux = test_flux(tracks);

	std::vector<uint8_t> data(1536 + total_blocks*512 + (has_flux ? 512 : 0), 0);

	memcpy(&data[0], signature2, 8);

	put_u32le(&data[12], 0x4f464e49);   // INFO
	put_u32le(&data[16], 60);           // size
	data[20] = 3;                       // chunk version
	data[21] = image.get_form_factor() == floppy_image::FF_525 ? 1 : 2;
	data[22] = 0;                       // not write protected
	data[23] = 1;                       // synchronized, since our internal format is
	data[24] = 1;                       // weak bits are generated, not stored
	data[25] = 'M';
	data[26] = 'A';
	data[27] = 'M';
	data[28] = 'E';
	memset(&data[29], ' ', 32-4);
	data[57] = twosided ? 2 : 1;
	data[58] = 0;                       // boot sector unknown
	data[59] = image.get_form_factor() == floppy_image::FF_525 ? 32 : image.get_variant() == floppy_image::DSHD ? 8 : 16;
	put_u16le(&data[60], 0);            // compatibility unknown
	put_u16le(&data[62], 0);            // needed RAM unknown
	put_u16le(&data[64], max_blocks);
	put_u16le(&data[66], has_flux ? total_blocks+3 : 0);
	put_u16le(&data[68], max_blocks);

	save_tracks(data, tracks, total_blocks, has_flux);

	/*auto const [err, actual] =*/ write_at(io, 0, data.data(), data.size()); // FIXME: check for errors
	return true;
}

const woz_format FLOPPY_WOZ_FORMAT;


moof_format::moof_format() : as_format()
{
}

const char *moof_format::name() const noexcept
{
	return "moof";
}

const char *moof_format::description() const noexcept
{
	return "Macintosh MOOF Image";
}

const char *moof_format::extensions() const noexcept
{
	return "moof";
}

bool moof_format::supports_save() const noexcept
{
	return true;
}

const uint8_t moof_format::signature[8] = { 0x4d, 0x4f, 0x4f, 0x46, 0xff, 0x0a, 0x0d, 0x0a };

int moof_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[8];
	auto const [err, actual] = read_at(io, 0, header, 8);
	if(err || (8 != actual)) return 0;
	if(!memcmp(header, signature, 8)) return FIFID_SIGN;
	return 0;
}

bool moof_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t image_size;
	if(io.length(image_size))
		return false;
	auto const [err, img, actual] = read_at(io, 0, image_size);
	if(err || (actual != image_size))
		return false;

	// Check signature
	if(memcmp(&img[0], signature, 8))
		return false;

	// Check integrity
	uint32_t crc = crc32r(&img[12], image_size - 12);
	if(crc != get_u32le(&img[8]))
		return false;

	uint32_t off_info = find_tag(img, image_size, 0x4f464e49);
	uint32_t off_tmap = find_tag(img, image_size, 0x50414d54);
	uint32_t off_trks = find_tag(img, image_size, 0x534b5254);

	if(!off_info || !off_tmap || !off_trks)
		return false;

	uint32_t info_vers = img[off_info + 0];
	if(info_vers != 1)
		return false;

	uint16_t off_flux = get_u16le(&img[off_info + 40]);
	uint16_t flux_size = get_u16le(&img[off_info + 42]);

	if(!flux_size)
		off_flux = 0;

	switch(img[off_info + 1]) {
	case 1:
		image.set_form_variant(floppy_image::FF_35, floppy_image::SSDD);
		break;
	case 2:
		image.set_form_variant(floppy_image::FF_35, floppy_image::DSDD);
		break;
	case 3:
		image.set_form_variant(floppy_image::FF_35, floppy_image::DSHD);
		break;
	default:
		return false;
	}

	for (unsigned int trkid = 0; trkid != 160; trkid++) {
		int head = trkid & 1;
		int track = trkid >> 1;

		uint8_t idx = img[off_tmap + trkid];
		uint8_t fidx = off_flux ? img[off_flux*512 + 8 + trkid] : 0xff;

		if(fidx != 0xff) {
			load_flux_track(&img[0], image, head, track, 0, fidx, off_trks);

		} else if(idx != 0xff) {
			if(!load_bitstream_track(&img[0], image, head, track, 0, idx, off_trks, false, false))
				return false;
		}
	}

	return true;
}

bool moof_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	std::vector<tdata> tracks(160);
	bool twosided = false;
	bool is_hd = false;

	if(image.get_variant() == floppy_image::DSHD) {
		twosided = true;
		is_hd = true;
		for(unsigned int i=0; i != 160; i++)
			tracks[i] = analyze_for_save(image, i & 1, i >> 1, 0, 6);

	} else {
		for(unsigned int i=0; i != 160; i++) {
			tracks[i] = analyze_for_save(image, i & 1, i >> 1, 0, i / (2*16));
			if((i & 1) && tracks[i].track_size)
				twosided = true;
		}
	}

	auto [total_blocks, max_blocks] = count_blocks(tracks);
	bool has_flux = test_flux(tracks);

	std::vector<uint8_t> data(1536 + total_blocks*512 + (has_flux ? 512 : 0), 0);

	memcpy(&data[0], signature, 8);

	put_u32le(&data[12], 0x4f464e49);   // INFO
	put_u32le(&data[16], 60);           // size
	data[20] = 1;                       // chunk version
	data[21] = is_hd ? 3 : twosided ? 2 : 1; // variant
	data[22] = 0;                       // not write protected
	data[23] = 1;                       // synchronized, since our internal format is
	data[24] = is_hd ? 8 : 16;          // optimal timing
	data[25] = 'M';
	data[26] = 'A';
	data[27] = 'M';
	data[28] = 'E';
	memset(&data[29], ' ', 32-4);
	data[57] = 0;                       // pad
	put_u16le(&data[58], max_blocks);
	put_u16le(&data[60], has_flux ? total_blocks+3 : 0);
	put_u16le(&data[62], max_blocks);

	save_tracks(data, tracks, total_blocks, has_flux);

	/*auto const [err, actual] =*/ write_at(io, 0, data.data(), data.size()); // FIXME: check for errors
	return true;
}

const moof_format FLOPPY_MOOF_FORMAT;
