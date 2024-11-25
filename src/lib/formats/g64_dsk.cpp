// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/g64_dsk.cpp

    Commodore 1541/1571 GCR disk image format

    http://unusedino.de/ec64/technical/formats/g64.html

*********************************************************************/

#include "formats/g64_dsk.h"
#include "imageutl.h"

#include "ioprocs.h"
#include "multibyte.h"

#include "osdcore.h" // osd_printf_*


#define G64_FORMAT_HEADER   "GCR-1541"

g64_format::g64_format()
{
}

const uint32_t g64_format::c1541_cell_size[] =
{
	4000, // 16MHz/16/4
	3750, // 16MHz/15/4
	3500, // 16MHz/14/4
	3250  // 16MHz/13/4
};

int g64_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	char h[8];
	auto const [err, actual] = read_at(io, 0, h, 8);
	if (err || (8 != actual))
		return 0;

	if (!memcmp(h, G64_FORMAT_HEADER, 8))
		return FIFID_SIGN;

	return 0;
}

bool g64_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	auto const [err, img, actual] = read_at(io, 0, size);
	if (err || (actual != size))
		return false;

	if (img[POS_VERSION])
	{
		osd_printf_error("g64_format: Unsupported version %u\n", img[POS_VERSION]);
		return false;
	}

	int track_count = img[POS_TRACK_COUNT];
	int head = 0;

	for (int track = 0; track < track_count; track++)
	{
		int cylinder = track % TRACK_COUNT;

		if (track == TRACK_COUNT)
			head = 1;

		uint32_t tpos = POS_TRACK_OFFSET + (track * 4);
		uint32_t spos = tpos + (track_count * 4);
		uint32_t dpos = get_u32le(&img[tpos]);

		if (!dpos)
			continue;

		if (dpos > size)
		{
			osd_printf_error("g64_format: Track %u offset %06x out of bounds\n", track, dpos);
			return false;
		}

		uint32_t speed_zone = get_u32le(&img[spos]);

		if (speed_zone > 3)
		{
			osd_printf_error("g64_format: Unsupported variable speed zones on track %d\n", track);
			return false;
		}

		uint16_t track_bytes = get_u16le(&img[dpos]);
		int track_size = track_bytes * 8;

		LOG_FORMATS("head %u track %u offs %u size %u cell %ld\n", head, cylinder, dpos, track_bytes, 200000000L/track_size);

		generate_track_from_bitstream(cylinder, head, &img[dpos+2], track_size, image);
	}

	if (!head)
		image.set_variant(floppy_image::SSSD);
	else
		image.set_variant(floppy_image::DSSD);

	return true;
}

int g64_format::generate_bitstream(int track, int head, int speed_zone, std::vector<bool> &trackbuf, const floppy_image &image)
{
	int cell_size = c1541_cell_size[speed_zone];

	trackbuf = generate_bitstream_from_track(track, head, cell_size, image);

	int actual_cell_size = 200000000L/trackbuf.size();

	// allow a tolerance of +- 10 us (3990..4010 -> 4000)
	return ((actual_cell_size >= cell_size-10) && (actual_cell_size <= cell_size+10)) ? speed_zone : -1;
}

bool g64_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	uint8_t const zerofill[] = { 0x00, 0x00, 0x00, 0x00 };
	std::vector<uint8_t> const prefill(TRACK_LENGTH, 0xff);

	int tracks, heads;
	image.get_actual_geometry(tracks, heads);
	tracks = TRACK_COUNT * heads;

	// write header
	uint8_t header[] = { 'G', 'C', 'R', '-', '1', '5', '4', '1', 0x00, static_cast<uint8_t>(tracks), TRACK_LENGTH & 0xff, TRACK_LENGTH >> 8 };
	write_at(io, POS_SIGNATURE, header, sizeof(header)); // FIXME: check for errors

	// write tracks
	for (int head = 0; head < heads; head++) {
		int tracks_written = 0;

		std::vector<bool> trackbuf;

		for (int track = 0; track < TRACK_COUNT; track++) {
			uint32_t const tpos = POS_TRACK_OFFSET + (track * 4);
			uint32_t const spos = tpos + (tracks * 4);
			uint32_t const dpos = POS_TRACK_OFFSET + (tracks * 4 * 2) + (tracks_written * TRACK_LENGTH);

			write_at(io, tpos, zerofill, 4); // FIXME: check for errors
			write_at(io, spos, zerofill, 4); // FIXME: check for errors

			if (image.get_buffer(track, head).size() <= 1)
				continue;

			int track_size;
			int speed_zone;

			// figure out the cell size and speed zone from the track data
			if ((speed_zone = generate_bitstream(track, head, 3, trackbuf, image)) == -1)
				if ((speed_zone = generate_bitstream(track, head, 2, trackbuf, image)) == -1)
					if ((speed_zone = generate_bitstream(track, head, 1, trackbuf, image)) == -1)
						if ((speed_zone = generate_bitstream(track, head, 0, trackbuf, image)) == -1) {
							osd_printf_error("g64_format: Cannot determine speed zone for track %u\n", track);
							return false;
						}

			LOG_FORMATS("head %u track %u size %u cell %u\n", head, track, track_size, c1541_cell_size[speed_zone]);

			std::vector<uint8_t> packed((trackbuf.size() + 7) >> 3, 0);
			for(uint32_t i = 0; i != trackbuf.size(); i++)
				if(trackbuf[i])
					packed[i >> 3] |= 0x80 >> (i & 7);

			uint8_t track_offset[4];
			uint8_t speed_offset[4];
			uint8_t track_length[2];

			put_u32le(track_offset, dpos);
			put_u32le(speed_offset, speed_zone);
			put_u16le(track_length, packed.size());

			write_at(io, tpos, track_offset, 4); // FIXME: check for errors
			write_at(io, spos, speed_offset, 4); // FIXME: check for errors
			write_at(io, dpos, prefill.data(), TRACK_LENGTH); // FIXME: check for errors
			write_at(io, dpos, track_length, 2); // FIXME: check for errors
			write_at(io, dpos + 2, packed.data(), packed.size()); // FIXME: check for errors

			tracks_written++;
		}
	}

	return true;
}

const char *g64_format::name() const noexcept
{
	return "g64";
}

const char *g64_format::description() const noexcept
{
	return "Commodore 1541/1571 GCR disk image";
}

const char *g64_format::extensions() const noexcept
{
	return "g64,g41,g71";
}

const g64_format FLOPPY_G64_FORMAT;
