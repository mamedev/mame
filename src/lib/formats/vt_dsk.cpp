// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/vt_dsk.cpp

    VTech disk image formats

*********************************************************************/

#include "formats/vt_dsk.h"

#include "ioprocs.h"


// Zero = |    9187     |
// One  = | 2237 | 6950 |
// 0.5us ~= 143

void vtech_common_format::wbit(std::vector<uint32_t> &buffer, uint32_t &pos, bool bit)
{
	if(bit) {
		pos += 2237;
		buffer.push_back(pos | floppy_image::MG_F);
		pos += 6950;
	} else
		pos += 9187;
}

void vtech_common_format::wbyte(std::vector<uint32_t> &buffer, uint32_t &pos, uint8_t byte)
{
	for(int i = 7; i >= 0; i--)
		wbit(buffer, pos, (byte >> i) & 1);
}

void vtech_common_format::image_to_flux(const std::vector<uint8_t> &bdata, floppy_image *image)
{
	static const uint8_t sector_map[16] = {
		0x0, 0xb, 0x6, 0x1, 0xc, 0x7, 0x2, 0xd, 0x8, 0x3, 0xe, 0x9, 0x4, 0xf, 0xa, 0x5
	};

	for(int track = 0; track != 40; track ++) {
		uint32_t pos = 0;
		std::vector<uint32_t> &buffer = image->get_buffer(track, 0);
		buffer.clear();
		image->set_write_splice_position(track, 0, 0);
		// One window of pad at the start to avoid problems with the write splice
		wbit(buffer, pos, 0);

		for(int sector = 0; sector != 16; sector ++) {
			uint8_t sid = sector_map[sector];
			for(int i=0; i != 7; i++)
				wbyte(buffer, pos, 0x80);
			wbyte(buffer, pos, 0x00);
			wbyte(buffer, pos, 0xfe);
			wbyte(buffer, pos, 0xe7);
			wbyte(buffer, pos, 0x18);
			wbyte(buffer, pos, 0xc3);
			wbyte(buffer, pos, track);
			wbyte(buffer, pos, sid);
			wbyte(buffer, pos, track+sid);
			for(int i=0; i != 5; i++)
				wbyte(buffer, pos, 0x80);
			wbyte(buffer, pos, 0x00);
			wbyte(buffer, pos, 0xc3);
			wbyte(buffer, pos, 0x18);
			wbyte(buffer, pos, 0xe7);
			wbyte(buffer, pos, 0xfe);
			uint16_t chk = 0;
			const uint8_t *src = bdata.data() + 16*128*track + 128*sid;
			for(int i=0; i != 128; i++) {
				chk += src[i];
				wbyte(buffer, pos, src[i]);
			}
			wbyte(buffer, pos, chk);
			wbyte(buffer, pos, chk >> 8);
		}
		// Rest is just not formatted
		buffer.push_back(pos | floppy_image::MG_N);
	}
}

std::vector<uint8_t> vtech_common_format::flux_to_image(floppy_image *image)
{
	std::vector<uint8_t> bdata(16*256*40, 0);

	for(int track = 0; track != 40; track++) {
		auto buffer = image->get_buffer(track, 0);
		int sz = buffer.size();
		if(sz < 128)
			continue;

		std::vector<bool> bitstream;
		int cpos = 0;
		while((buffer[cpos] & floppy_image::MG_MASK) != floppy_image::MG_F) {
			cpos++;
			if(cpos == sz) {
				cpos = -1;
				break;
			}
		}
		if(cpos == -1)
			continue;
		for(;;) {
			int npos = cpos;
			for(;;) {
				npos ++;
				if(npos == sz)
					npos = 0;
				if((buffer[npos] & floppy_image::MG_MASK) == floppy_image::MG_F)
					break;
			}
			int dt = (buffer[npos] & floppy_image::TIME_MASK) - (buffer[cpos] & floppy_image::TIME_MASK);
			if(dt < 0)
				cpos += 200000000;
			bitstream.push_back(dt < 9187 - 143);
			if(npos <= cpos)
				break;
			cpos = npos;
		}
		int mode = 0;
		int pos = 0;
		int count = 0;
		bool looped = false;
		uint8_t *dest = nullptr;
		[[maybe_unused]] uint16_t checksum = 0;
		uint64_t buf = 0;
		sz = bitstream.size();
		if(sz < 128)
			continue;

		for(int i=0; i != 63; i++)
			buf = (buf << 1) | bitstream[sz-64+i];
		for(;;) {
			buf = (buf << 1) | bitstream[pos];
			count ++;
			switch(mode) {
			case 0: // idle
				if(buf == 0x80808000fee718c3)
					mode = 1;
				count = 0;
				break;

			case 1: // sector header
				if(count == 24) {
					uint8_t trk = buf >> 16;
					uint8_t sector = buf >> 8;
					uint8_t chk = buf;
					if(chk != sector + trk) {
						mode = 0;
						break;
					}
					checksum = 0;
					dest = bdata.data() + 128 * 16 * trk + 128 * (sector & 0xf);
					mode = 2;
				}
				break;

			case 2: // look for sector data
				if(buf == 0x80808000fee718c3)
					mode = 1;
				else if(buf == 0x80808000c318e7fe)
					mode = 3;
				count = 0;
				break;

			case 3: // sector data
				if(count <= 128*8 && !(count & 7)) {
					uint8_t byte = buf;
					checksum += byte;
					*dest++ = byte;
				} else if(count == 128*8+16) {
					//                  uint16_t disk_checksum = buf;
					//                  printf("sector checksum %04x %04x\n", checksum, disk_checksum);
					mode = 0;
				}
				break;
			}
			if(mode == 0 && looped)
				break;
			pos++;
			if(pos == sz) {
				pos = 0;
				looped = true;
			}
		}
	}

	return bdata;
}


const char *vtech_bin_format::name() const
{
	return "vtech_bin";
}

const char *vtech_bin_format::description() const
{
	return "VTech sector disk image";
}

const char *vtech_bin_format::extensions() const
{
	return "bin";
}

const char *vtech_dsk_format::name() const
{
	return "vtech_dsk";
}

const char *vtech_dsk_format::description() const
{
	return "VTech dsk image";
}

const char *vtech_dsk_format::extensions() const
{
	return "dsk";
}

int vtech_bin_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return 0;

	if(size == 40*16*256)
		return FIFID_SIZE;

	return 0;
}

int vtech_dsk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return 0;

	if(size < 256)
		return 0;

	std::vector<uint8_t> bdata(size);
	size_t actual;
	io.read_at(0, bdata.data(), size, actual);

	// Structurally validate the presence of sector headers and data
	int count_sh = 0, count_sd = 0;
	uint64_t buf = 0;
	for(uint8_t b : bdata) {
		buf = (buf << 8) | b;
		if(buf == 0x80808000fee718c3)
			count_sh++;
		else if(buf == 0x80808000c318e7fe)
			count_sd++;
	}

	return count_sh >= 30*16 && count_sd >= 30*16 ? FIFID_STRUCT : 0;
}

bool vtech_bin_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if(io.length(size) || (size != 40*16*256))
		return false;

	std::vector<uint8_t> bdata(size);
	size_t actual;
	io.read_at(0, bdata.data(), size, actual);

	image_to_flux(bdata, image);
	image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);
	return true;
}

bool vtech_dsk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if(io.length(size))
		return false;
	std::vector<uint8_t> bdata(size);
	size_t actual;
	io.read_at(0, bdata.data(), size, actual);

	std::vector<uint8_t> bdatax(128*16*40, 0);

	int mode = 0;
	int count = 0;
	[[maybe_unused]] uint16_t checksum = 0;
	uint64_t buf = 0;
	uint8_t *dest = nullptr;

	for(uint8_t b : bdata) {
		buf = (buf << 8) | b;
		count ++;
		switch(mode) {
		case 0: // idle
			if(buf == 0x80808000fee718c3)
				mode = 1;
			count = 0;
			break;

		case 1: // sector header
			if(count == 3) {
				uint8_t trk = buf >> 16;
				uint8_t sector = buf >> 8;
				uint8_t chk = buf;
				if(chk != sector + trk) {
					mode = 0;
					break;
				}
				dest = bdatax.data() + 128*16*trk + sector*128;
				checksum = 0;
				mode = 2;
			}
			break;

		case 2: // look for sector data
			if(buf == 0x80808000fee718c3)
				mode = 1;
			else if(buf == 0x80808000c318e7fe)
				mode = 3;
			count = 0;
			break;

		case 3: // sector data
			if(count <= 128) {
				uint8_t byte = buf;
				checksum += byte;
				*dest++ = byte;

			} else if(count == 128+2) {
				uint16_t disk_checksum = buf;
				disk_checksum = (disk_checksum << 8) | (disk_checksum >> 8);
				mode = 0;
			}
			break;
		}
	}

	image_to_flux(bdatax, image);
	image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);
	return true;
}

bool vtech_bin_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	int tracks, heads;
	image->get_maximal_geometry(tracks, heads);
	if(tracks < 40)
		return false;

	auto bdata = flux_to_image(image);
	size_t actual;
	io.write_at(0, bdata.data(), bdata.size(), actual);
	return true;
}

bool vtech_dsk_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	int tracks, heads;
	image->get_maximal_geometry(tracks, heads);
	if(tracks < 40)
		return false;

	auto bdata = flux_to_image(image);
	std::vector<uint8_t> bdatax(0x9b*16*40);

	// Format is essentially an idealized version of what's written on the disk

	static const uint8_t sector_map[16] = {
		0x0, 0xb, 0x6, 0x1, 0xc, 0x7, 0x2, 0xd, 0x8, 0x3, 0xe, 0x9, 0x4, 0xf, 0xa, 0x5
	};

	int pos = 0;
	for(int track = 0; track != 40; track ++) {
		for(int sector = 0; sector != 16; sector ++) {
			uint8_t sid = sector_map[sector];
			for(int i=0; i != 7; i++)
				bdatax[pos++] = 0x80;
			bdatax[pos++] = 0x00;
			bdatax[pos++] = 0xfe;
			bdatax[pos++] = 0xe7;
			bdatax[pos++] = 0x18;
			bdatax[pos++] = 0xc3;
			bdatax[pos++] = track;
			bdatax[pos++] = sid;
			bdatax[pos++] = track+sid;
			for(int i=0; i != 5; i++)
				bdatax[pos++] = 0x80;
			bdatax[pos++] = 0x00;
			bdatax[pos++] = 0xc3;
			bdatax[pos++] = 0x18;
			bdatax[pos++] = 0xe7;
			bdatax[pos++] = 0xfe;
			uint16_t chk = 0;
			const uint8_t *src = bdata.data() + 16*128*track + 128*sid;
			for(int i=0; i != 128; i++) {
				chk += src[i];
				bdatax[pos++] = src[i];
			}
			bdatax[pos++] = chk;
			bdatax[pos++] = chk >> 8;
		}
	}

	size_t actual;
	io.write_at(0, bdatax.data(), bdatax.size(), actual);
	return true;
}


const vtech_bin_format FLOPPY_VTECH_BIN_FORMAT;
const vtech_dsk_format FLOPPY_VTECH_DSK_FORMAT;
