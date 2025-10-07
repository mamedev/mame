// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ccvf_dsk.cpp

    Compucolor Virtual Floppy Disk Image format

*********************************************************************/

#include "formats/ccvf_dsk.h"

#include "coretmpl.h" // BIT
#include "ioprocs.h"


ccvf_format::ccvf_format()
{
	formats = file_formats;
}

ccvf_format::ccvf_format(const format *_formats)
{
	formats = _formats;
}

const char *ccvf_format::name() const noexcept
{
	return "ccvf";
}

const char *ccvf_format::description() const noexcept
{
	return "Compucolor Virtual Floppy Disk Image";
}

const char *ccvf_format::extensions() const noexcept
{
	return "ccvf";
}

const ccvf_format::format ccvf_format::file_formats[] = {
	{
		floppy_image::FF_525, floppy_image::SSSD,
		(int) ((1./(9600*8))*1000000000), 10, 41, 1, 128, {}, 0, { 0,5,1,6,2,7,3,8,4,9 }
	},
	{}
};

int ccvf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	char h[36];
	auto const [err, actual] = read_at(io, 0, h, 36);
	if (err || (36 != actual))
		return 0;

	if (!memcmp(h, "Compucolor Virtual Floppy Disk Image", 36))
		return FIFID_SIGN;

	return 0;
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

bool ccvf_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	const format &f = formats[0];

	uint64_t size;
	if (io.length(size))
		return false;

	auto [err, img, actual] = read_at(io, 0, size);
	if (err || (actual != size))
		return false;

	std::string ccvf = std::string((const char *)&img[0], size);
	std::vector<uint8_t> bytes(78720);

	int start = 0, end = 0;
	std::string line;
	uint32_t byteoffs = 0;
	char hex[3] = {0};

	do {
		end = ccvf.find_first_of(10, start);
		line.assign(ccvf.substr(start, end));
		if (line.find("Compucolor Virtual Floppy Disk Image") != std::string::npos && line.find("Label") != std::string::npos && line.find("Track") != std::string::npos) {
			for (int byte = 0; byte < 32; byte++) {
				if (byteoffs==78720) break;
				hex[0]=line[byte * 2];
				hex[1]=line[(byte * 2) + 1];
				bytes[byteoffs++] = strtol(hex, nullptr, 16);
			}
		}
		start = end + 1;
	} while (start > 0 && end != -1);

	uint64_t pos = 0;
	int total_size = 200000000/f.cell_size;

	for(int track=0; track < f.track_count; track++) {
		std::vector<uint32_t> buffer;
		int offset = 0;

		for (int i=0; i<1920 && pos<size; i++, pos++) {
			for (int bit=0; bit<8; bit++) {
				bit_w(buffer, util::BIT(bytes[pos], bit), f.cell_size);
			}
		}

		if (offset < total_size) {
			// pad the remainder of the track with sync
			int count = total_size-buffer.size();
			for (int i=0; i<count;i++) {
				bit_w(buffer, (track > 0) ? 1 : 0, f.cell_size);
			}
		}

		generate_track_from_levels(track, 0, buffer, 0, image);
	}

	image.set_variant(f.variant);

	return true;
}

const ccvf_format FLOPPY_CCVF_FORMAT;
