// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ccvf_dsk.cpp

    Compucolor Virtual Floppy Disk Image format

*********************************************************************/

#include "formats/ccvf_dsk.h"

#include "coretmpl.h" // BIT
#include "ioprocs.h"
#include "strformat.h"

#include <algorithm>
#include <cstring>
#include <sstream>


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
	std::vector<uint8_t> bytes(TRACK_BYTES * f.track_count);

	int start = 0, end = 0;
	std::string line;
	uint32_t byteoffs = 0;
	char hex[3] = {0};

	do {
		end = ccvf.find_first_of(10, start);
		line.assign(ccvf.substr(start, end - start));
		if (line.length() >= 64 && !line.starts_with("Compucolor Virtual Floppy Disk Image") && !line.starts_with("Label") && !line.starts_with("Track")) {
			for (int byte = 0; byte < 32; byte++) {
				if (byteoffs == bytes.size()) break;
				hex[0]=line[byte * 2];
				hex[1]=line[(byte * 2) + 1];
				bytes[byteoffs++] = strtol(hex, nullptr, 16);
			}
		}
		start = end + 1;
	} while (start > 0 && end != -1);

	for (int track = 0; track < f.track_count; track++) {
		const uint8_t *levels = &bytes[track * TRACK_BYTES];
		std::vector<uint32_t> buffer;

		for (int cell = 0; cell < TRACK_CELLS; cell++) {
			int const prev = (cell + TRACK_CELLS - 1) % TRACK_CELLS;
			bool const transition = util::BIT(levels[cell >> 3], cell & 7) != util::BIT(levels[prev >> 3], prev & 7);
			bit_w(buffer, transition, f.cell_size);
		}

		generate_track_from_levels(track, 0, buffer, 0, image);
	}

	image.set_variant(f.variant);

	return true;
}

bool ccvf_format::supports_save() const noexcept
{
	return true;
}

bool ccvf_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	const format &f = formats[0];

	std::ostringstream ccvf;
	ccvf << "Compucolor Virtual Floppy Disk Image\r\n";

	for (int track = 0; track < f.track_count; track++) {
		std::vector<bool> transitions(TRACK_CELLS, false);

		for (uint32_t entry : image.get_buffer(track, 0)) {
			if ((entry & floppy_image::MG_MASK) == floppy_image::MG_F) {
				uint64_t const cell = uint64_t(entry & floppy_image::TIME_MASK) * TRACK_CELLS / 200'000'000;
				transitions[cell % TRACK_CELLS].flip();
			}
		}

		uint8_t bytes[TRACK_BYTES] = {};

		// flux reversals carry no polarity, so recover it the way the drive's read latch does: a run
		// longer than any space in an 8N1 frame is the idle mark
		if (std::find(transitions.begin(), transitions.end(), true) != transitions.end()) {
			bool level = true;
			int run = IDLE_CELLS;

			for (int i = 0; i < TRACK_CELLS * 2; i++) {
				int const cell = i % TRACK_CELLS;

				if (transitions[cell]) {
					level = !level;
					run = 0;
				} else if (++run >= IDLE_CELLS) {
					level = true;
				}

				if (level && i >= TRACK_CELLS)
					bytes[cell >> 3] |= 1 << (cell & 7);
			}
		}

		util::stream_format(ccvf, "Track %d\r\n", track);

		for (int i = 0; i < TRACK_BYTES; i++) {
			util::stream_format(ccvf, "%02X", bytes[i]);

			if ((i & 31) == 31)
				ccvf << "\r\n";
		}
	}

	std::string const text = std::move(ccvf).str();
	auto const [err, actual] = write_at(io, 0, text.data(), text.length());
	return !err && (actual == text.length());
}

const ccvf_format FLOPPY_CCVF_FORMAT;
