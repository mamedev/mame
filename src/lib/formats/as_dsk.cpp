// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Applesauce solved output formats

#include "as_dsk.h"
#include "ioprocs.h"

#include <string.h>

as_format::as_format() : floppy_image_format_t()
{
}

uint32_t as_format::find_tag(const std::vector<uint8_t> &data, uint32_t tag)
{
	uint32_t offset = 12;
	do {
		if(r32(data, offset) == tag)
			return offset + 8;
		offset += r32(data, offset+4) + 8;
	} while(offset < data.size() - 8);
	return 0;
}

uint32_t as_format::r32(const std::vector<uint8_t> &data, uint32_t offset)
{
	return data[offset] | (data[offset+1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24);
}

uint16_t as_format::r16(const std::vector<uint8_t> &data, uint32_t offset)
{
	return data[offset] | (data[offset+1] << 8);
}

void as_format::w32(std::vector<uint8_t> &data, int offset, uint32_t value)
{
	data[offset] = value;
	data[offset+1] = value >> 8;
	data[offset+2] = value >> 16;
	data[offset+3] = value >> 24;
}

void as_format::w16(std::vector<uint8_t> &data, int offset, uint16_t value)
{
	data[offset] = value;
	data[offset+1] = value >> 8;
}

uint8_t as_format::r8(const std::vector<uint8_t> &data, uint32_t offset)
{
	return data[offset];
}

uint32_t as_format::crc32r(const uint8_t *data, uint32_t size)
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

bool as_format::load_bitstream_track(const std::vector<uint8_t> &img, floppy_image *image, int head, int track, int subtrack, uint8_t idx, uint32_t off_trks, bool may_be_short, bool set_variant)
{
	uint32_t trks_off = off_trks + (idx * 8);
	
	uint32_t track_size = r32(img, trks_off + 4);
	if (track_size == 0)
		return false;

	uint32_t boff = (uint32_t)r16(img, trks_off + 0) * 512;

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
		image->set_variant(r32(img, trks_off + 4) >= 90000 ? floppy_image::DSHD : floppy_image::DSDD);
	return true;
}

void as_format::load_flux_track(const std::vector<uint8_t> &img, floppy_image *image, int head, int track, int subtrack, uint8_t fidx, uint32_t off_trks)
{
	uint32_t trks_off = off_trks + (fidx * 8);
	uint32_t boff = (uint32_t)r16(img, trks_off + 0) * 512;
	uint32_t track_size = r32(img, trks_off + 4);

	uint32_t total_ticks = 0;
	for(uint32_t i=0; i != track_size; i++)
		total_ticks += img[boff+i];

	// There is always a pulse at index, and it's
	// the last one in the stream
	std::vector<uint32_t> &buf = image->get_buffer(track, head, subtrack);
	buf.push_back(floppy_image::MG_F | 0);
	uint32_t cpos = 0;
	for(uint32_t i=0; i != track_size; i++) {
		uint8_t step = img[boff+i];
		cpos += step;
		if(step != 0xff && i != track_size-1)
			buf.push_back(floppy_image::MG_F | uint64_t(cpos)*200000000/total_ticks);
	}
}



woz_format::woz_format() : as_format()
{
}

const char *woz_format::name() const
{
	return "woz";
}

const char *woz_format::description() const
{
	return "Apple II WOZ Image";
}

const char *woz_format::extensions() const
{
	return "woz";
}

bool woz_format::supports_save() const
{
	return true;
}

const uint8_t woz_format::signature[8] = { 0x57, 0x4f, 0x5a, 0x31, 0xff, 0x0a, 0x0d, 0x0a };
const uint8_t woz_format::signature2[8] = { 0x57, 0x4f, 0x5a, 0x32, 0xff, 0x0a, 0x0d, 0x0a };

int woz_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[8];
	size_t actual;
	io.read_at(0, header, 8, actual);
	if (!memcmp(header, signature, 8)) return FIFID_SIGN;
	if (!memcmp(header, signature2, 8)) return FIFID_SIGN;
	return 0;
}

bool woz_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t image_size;
	if(io.length(image_size))
		return false;
	std::vector<uint8_t> img(image_size);
	size_t actual;
	io.read_at(0, &img[0], img.size(), actual);

	// Check signature
	if((memcmp(&img[0], signature, 8)) && (memcmp(&img[0], signature2, 8)))
		return false;

	uint32_t woz_vers = 1;
	if(!memcmp(&img[0], signature2, 8)) woz_vers = 2;

	// Check integrity
	uint32_t crc = crc32r(&img[12], img.size() - 12);
	if(crc != r32(img, 8))
		return false;

	uint32_t off_info = find_tag(img, 0x4f464e49);
	uint32_t off_tmap = find_tag(img, 0x50414d54);
	uint32_t off_trks = find_tag(img, 0x534b5254);
//  uint32_t off_writ = find_tag(img, 0x54495257);

	if(!off_info || !off_tmap || !off_trks)
		return false;

	uint32_t info_vers = r8(img, off_info + 0);
	if(info_vers < 1 || info_vers > 3)
		return false;

	uint16_t off_flux = info_vers < 3 ? 0 : r16(img, off_info + 46);
	uint16_t flux_size = info_vers < 3 ? 0 : r16(img, off_info + 48);

	if(!flux_size)
		off_flux = 0;

	bool is_35 = r8(img, off_info + 1) == 2;

	if((form_factor == floppy_image::FF_35 && !is_35) || (form_factor == floppy_image::FF_525 && is_35))
		return false;

	unsigned int limit = is_35 ? 160 : 141;

	if(is_35)
		image->set_form_variant(floppy_image::FF_35, floppy_image::SSDD);
	else
		image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	if (woz_vers == 1) {
		for (unsigned int trkid = 0; trkid != limit; trkid++) {
			int head = is_35 && trkid >= 80 ? 1 : 0;
			int track = is_35 ? trkid % 80 : trkid / 4;
			int subtrack = is_35 ? 0 : trkid & 3;

			uint8_t idx = r8(img, off_tmap + trkid);
			if(idx != 0xff) {
				uint32_t boff = off_trks + 6656*idx;
				if (r16(img, boff + 6648) == 0)
					return false;
				generate_track_from_bitstream(track, head, &img[boff], r16(img, boff + 6648), image, subtrack, r16(img, boff + 6650));
				if(is_35 && !track && head)
					image->set_variant(floppy_image::DSDD);
			}
		}
	} else if (woz_vers == 2) {
		for (unsigned int trkid = 0; trkid != limit; trkid++) {
			int head = is_35 && trkid & 1 ? 1 : 0;
			int track = is_35 ? trkid >> 1 : trkid / 4;
			int subtrack = is_35 ? 0 : trkid & 3;

			uint8_t idx = r8(img, off_tmap + trkid);
			uint8_t fidx = off_flux ? r8(img, off_flux*512 + 8 + trkid) : 0xff;

			if(fidx != 0xff)
				load_flux_track(img, image, head, track, subtrack, fidx, off_trks);

			else if(idx != 0xff) {
				if(!load_bitstream_track(img, image, head, track, subtrack, idx, off_trks, !is_35, is_35 && !track && head))
					return false;
			}
		}
	}
	else return false;

	return true;
}

bool woz_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	std::vector<std::vector<bool>> tracks(160);
	bool twosided = false;

	if(image->get_form_factor() == floppy_image::FF_525) {
		for(unsigned int i=0; i != 141; i++)
			if(image->track_is_formatted(i >> 2, 0, i & 3))
				tracks[i] = generate_bitstream_from_track(i >> 2, 0, 3915, image, i & 3);

	} else if(image->get_variant() == floppy_image::DSHD) {
		for(unsigned int i=0; i != 160; i++)
			if(image->track_is_formatted(i >> 1, i & 1)) {
				tracks[i] = generate_bitstream_from_track(i >> 1, i & 1, 1000, image);
				if(i & 1)
					twosided = true;
			}

	} else {
		// 200000000 / 60.0 * 1.979e-6 ~= 6.5967
		static const int cell_size_per_speed_zone[5] = {
			394 * 65967 / 10000,
			429 * 65967 / 10000,
			472 * 65967 / 10000,
			525 * 65967 / 10000,
			590 * 65967 / 10000
		};

		for(unsigned int i=0; i != 160; i++)
			if(image->track_is_formatted(i >> 1, i & 1)) {
				tracks[i] = generate_bitstream_from_track(i >> 1, i & 1, cell_size_per_speed_zone[i / (2*16)], image);
				if(i & 1)
					twosided = true;
			}
	}

	int max_blocks = 0;
	int total_blocks = 0;
	for(const auto &t : tracks) {
		int blocks = (t.size() + 4095) / 4096;
		total_blocks += blocks;
		if(max_blocks < blocks)
			max_blocks = blocks;
	}

	std::vector<uint8_t> data(1536 + total_blocks*512, 0);

	memcpy(&data[0], signature2, 8);

	w32(data, 12, 0x4F464E49);  // INFO
	w32(data, 16, 60);          // size
	data[20] = 2;               // chunk version
	data[21] = image->get_form_factor() == floppy_image::FF_525 ? 1 : 2;
	data[22] = 0;               // not write protected
	data[23] = 1;               // synchronized, since our internal format is
	data[24] = 1;               // weak bits are generated, not stored
	data[25] = 'M';
	data[26] = 'A';
	data[27] = 'M';
	data[28] = 'E';
	memset(&data[29], ' ', 32-4);
	data[57] = twosided ? 2 : 1;
	data[58] = 0;               // boot sector unknown
	data[59] = image->get_form_factor() == floppy_image::FF_525 ? 32 : image->get_variant() == floppy_image::DSHD ? 8 : 16;
	w16(data, 60, 0);           // compatibility unknown
	w16(data, 62, 0);           // needed ram unknown
	w16(data, 64, max_blocks);
	w32(data, 80, 0x50414D54);  // TMAP
	w32(data, 84, 160);         // size

	uint8_t tcount = 0;
	for(int i=0; i != 160 ; i++)
		data[88 + i] = tracks[i].empty() ? 0xff : tcount++;

	w32(data, 248, 0x534B5254); // TRKS
	w32(data, 252, 1280 + total_blocks*512);   // size

	uint8_t tid = 0;
	uint16_t tb = 3;
	for(int i=0; i != 160 ; i++)
		if(!tracks[i].empty()) {
			int blocks = (tracks[i].size() + 4095) / 4096;
			w16(data, 256 + tid*8, tb);
			w16(data, 256 + tid*8 + 2, blocks);
			w32(data, 256 + tid*8 + 4, tracks[i].size());
			tb += blocks;
			tid ++;
		}

	tb = 3;
	for(int i=0; i != 160 ; i++)
		if(!tracks[i].empty()) {
			int off = tb * 512;
			int size = tracks[i].size();
			for(int j=0; j != size; j++)
				if(tracks[i][j])
					data[off + (j >> 3)] |= 0x80 >> (j & 7);
			tb += (size + 4095) / 4096;
		}

	w32(data, 8, crc32r(&data[12], data.size() - 12));

	size_t actual;
	io.write_at(0, data.data(), data.size(), actual);
	return true;
}

const woz_format FLOPPY_WOZ_FORMAT;


moof_format::moof_format() : as_format()
{
}

const char *moof_format::name() const
{
	return "moof";
}

const char *moof_format::description() const
{
	return "Macintosh MOOF Image";
}

const char *moof_format::extensions() const
{
	return "moof";
}

bool moof_format::supports_save() const
{
	return false;
}

const uint8_t moof_format::signature[8] = { 0x4d, 0x4f, 0x4f, 0x46, 0xff, 0x0a, 0x0d, 0x0a };

int moof_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[8];
	size_t actual;
	io.read_at(0, header, 8, actual);
	if (!memcmp(header, signature, 8)) return FIFID_SIGN;
	return 0;
}

bool moof_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t image_size;
	if(io.length(image_size))
		return false;
	std::vector<uint8_t> img(image_size);
	size_t actual;
	io.read_at(0, &img[0], img.size(), actual);

	// Check signature
	if(memcmp(&img[0], signature, 8))
		return false;

	// Check integrity
	uint32_t crc = crc32r(&img[12], img.size() - 12);
	if(crc != r32(img, 8))
		return false;

	uint32_t off_info = find_tag(img, 0x4f464e49);
	uint32_t off_tmap = find_tag(img, 0x50414d54);
	uint32_t off_trks = find_tag(img, 0x534b5254);

	if(!off_info || !off_tmap || !off_trks)
		return false;

	uint32_t info_vers = r8(img, off_info + 0);
	if(info_vers != 1)
		return false;

	uint16_t off_flux = r16(img, off_info + 40);
	uint16_t flux_size = r16(img, off_info + 42);

	if(!flux_size)
		off_flux = 0;

	switch(r8(img, off_info + 1)) {
	case 1:
		image->set_form_variant(floppy_image::FF_35, floppy_image::SSDD);
		break;
	case 2:
		image->set_form_variant(floppy_image::FF_35, floppy_image::DSDD);
		break;
	case 3:
		image->set_form_variant(floppy_image::FF_35, floppy_image::DSHD);
		break;
	default:
		return false;
	}

	for (unsigned int trkid = 0; trkid != 160; trkid++) {
		int head = trkid & 1;
		int track = trkid >> 1;

		uint8_t idx = r8(img, off_tmap + trkid);
		uint8_t fidx = off_flux ? r8(img, off_flux*512 + 8 + trkid) : 0xff;

		if(fidx != 0xff)
			load_flux_track(img, image, track, head, 0, fidx, off_trks);

		else if(idx != 0xff) {
			if(!load_bitstream_track(img, image, head, track, 0, idx, off_trks, false, false))
				return false;
		}
	}

	return true;
}

const moof_format FLOPPY_MOOF_FORMAT;
