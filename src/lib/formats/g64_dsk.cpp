// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/g64_dsk.c

    Commodore 1541/1571 GCR disk image format

    http://unusedino.de/ec64/technical/formats/g64.html

*********************************************************************/

#include "formats/g64_dsk.h"


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

int g64_format::identify(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	char h[8];

	io_generic_read(io, h, 0, 8);
	if (!memcmp(h, G64_FORMAT_HEADER, 8))
		return 100;

	return 0;
}

bool g64_format::load(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint64_t size = io_generic_size(io);
	std::vector<uint8_t> img(size);
	io_generic_read(io, &img[0], 0, size);

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
		uint32_t dpos = pick_integer_le(&img[0], tpos, 4);

		if (!dpos)
			continue;

		if (dpos > size)
		{
			osd_printf_error("g64_format: Track %u offset %06x out of bounds\n", track, dpos);
			return false;
		}

		uint32_t speed_zone = pick_integer_le(&img[0], spos, 4);

		if (speed_zone > 3)
		{
			osd_printf_error("g64_format: Unsupported variable speed zones on track %d\n", track);
			return false;
		}

		uint16_t track_bytes = pick_integer_le(&img[0], dpos, 2);
		int track_size = track_bytes * 8;

		LOG_FORMATS("head %u track %u offs %u size %u cell %ld\n", head, cylinder, dpos, track_bytes, 200000000L/track_size);

		generate_track_from_bitstream(cylinder, head, &img[dpos+2], track_size, image);
	}

	if (!head)
		image->set_variant(floppy_image::SSSD);
	else
		image->set_variant(floppy_image::DSSD);

	return true;
}

int g64_format::generate_bitstream(int track, int head, int speed_zone, std::vector<bool> &trackbuf, floppy_image *image)
{
	int cell_size = c1541_cell_size[speed_zone];

	trackbuf = generate_bitstream_from_track(track, head, cell_size, image);

	int actual_cell_size = 200000000L/trackbuf.size();

	// allow a tolerance of +- 10 us (3990..4010 -> 4000)
	return ((actual_cell_size >= cell_size-10) && (actual_cell_size <= cell_size+10)) ? speed_zone : -1;
}

bool g64_format::save(io_generic *io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	int tracks, heads;
	image->get_actual_geometry(tracks, heads);
	tracks = TRACK_COUNT * heads;

	// write header
	uint8_t header[] = { 'G', 'C', 'R', '-', '1', '5', '4', '1', 0x00, static_cast<uint8_t>(tracks), TRACK_LENGTH & 0xff, TRACK_LENGTH >> 8 };
	io_generic_write(io, header, POS_SIGNATURE, sizeof(header));

	// write tracks
	for (int head = 0; head < heads; head++) {
		int tracks_written = 0;

		std::vector<bool> trackbuf;

		for (int track = 0; track < TRACK_COUNT; track++) {
			uint32_t tpos = POS_TRACK_OFFSET + (track * 4);
			uint32_t spos = tpos + (tracks * 4);
			uint32_t dpos = POS_TRACK_OFFSET + (tracks * 4 * 2) + (tracks_written * TRACK_LENGTH);

			io_generic_write_filler(io, 0x00, tpos, 4);
			io_generic_write_filler(io, 0x00, spos, 4);

			if (image->get_buffer(track, head).size() <= 1)
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

			place_integer_le(track_offset, 0, 4, dpos);
			place_integer_le(speed_offset, 0, 4, speed_zone);
			place_integer_le(track_length, 0, 2, packed.size());

			io_generic_write(io, track_offset, tpos, 4);
			io_generic_write(io, speed_offset, spos, 4);
			io_generic_write_filler(io, 0xff, dpos, TRACK_LENGTH);
			io_generic_write(io, track_length, dpos, 2);
			io_generic_write(io, packed.data(), dpos + 2, packed.size());

			tracks_written++;
		}
	}

	return true;
}

const char *g64_format::name() const
{
	return "g64";
}

const char *g64_format::description() const
{
	return "Commodore 1541/1571 GCR disk image";
}

const char *g64_format::extensions() const
{
	return "g64,g41,g71";
}

const floppy_format_type FLOPPY_G64_FORMAT = &floppy_image_format_creator<g64_format>;
