// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/g64_dsk.c

    Commodore 1541/1571 GCR disk image format

    http://unusedino.de/ec64/technical/formats/g64.html

*********************************************************************/

#include "emu.h"
#include "formats/g64_dsk.h"

#define G64_FORMAT_HEADER   "GCR-1541"

g64_format::g64_format()
{
}

const UINT32 g64_format::c1541_cell_size[] =
{
	4000, // 16MHz/16/4
	3750, // 16MHz/15/4
	3500, // 16MHz/14/4
	3250  // 16MHz/13/4
};

int g64_format::identify(io_generic *io, UINT32 form_factor)
{
	char h[8];

	io_generic_read(io, h, 0, 8);
	if (!memcmp(h, G64_FORMAT_HEADER, 8)) {
		return 100;
	}
	return 0;
}

bool g64_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT64 size = io_generic_size(io);
	dynamic_buffer img(size);
	io_generic_read(io, img, 0, size);

	if (img[VERSION]) {
		throw emu_fatalerror("g64_format: Unsupported version %u", img[VERSION]);
	}

	int track_count = img[TRACK_COUNT];
	int head = 0;

	for (int track = 0; track < track_count; track++)
	{
		offs_t track_offset = pick_integer_le(img, TRACK_OFFSET + (track * 4), 4);

		if (!track_offset)
			continue;

		if (track_offset > size)
			throw emu_fatalerror("g64_format: Track %u offset %06x out of bounds", track, track_offset);

		offs_t speed_zone = pick_integer_le(img, SPEED_ZONE + (track * 4), 4);

		if (speed_zone > 3)
			throw emu_fatalerror("g64_format: Unsupported variable speed zones on track %d", track);

		UINT16 track_bytes = pick_integer_le(img, track_offset, 2);
		int track_size = track_bytes * 8;

		LOG_FORMATS("track %u size %u cell %ld\n", track, track_size, 200000000L/track_size);

		generate_track_from_bitstream(track, head, &img[track_offset+2], track_size, image);
	}

	image->set_variant(floppy_image::SSSD);

	return true;
}

int g64_format::generate_bitstream(int track, int head, int speed_zone, UINT8 *trackbuf, int &track_size, floppy_image *image)
{
	int cell_size = c1541_cell_size[speed_zone];

	generate_bitstream_from_track(track, head, cell_size, trackbuf, track_size, image);

	int actual_cell_size = 200000000L/track_size;

	// allow a tolerance of +- 10 us (3990..4010 -> 4000)
	return ((actual_cell_size >= cell_size-10) && (actual_cell_size <= cell_size+10)) ? speed_zone : -1;
}

bool g64_format::save(io_generic *io, floppy_image *image)
{
	UINT8 header[] = { 'G', 'C', 'R', '-', '1', '5', '4', '1', 0x00, 0x54, TRACK_LENGTH & 0xff, TRACK_LENGTH >> 8 };

	io_generic_write(io, header, SIGNATURE, sizeof(header));

	int head = 0;
	int tracks_written = 0;

	dynamic_buffer trackbuf(TRACK_LENGTH-2);
	for (int track = 0; track < 84; track++) {
		offs_t tpos = TRACK_OFFSET + track * 4;
		offs_t spos = SPEED_ZONE + track * 4;
		offs_t dpos = TRACK_DATA + tracks_written * TRACK_LENGTH;

		io_generic_write_filler(io, 0x00, tpos, 4);
		io_generic_write_filler(io, 0x00, spos, 4);

		if (image->get_track_size(track, head) <= 1)
			continue;

		int track_size;
		int speed_zone;

		// figure out the cell size and speed zone from the track data
		if ((speed_zone = generate_bitstream(track, head, 3, trackbuf, track_size, image)) == -1)
			if ((speed_zone = generate_bitstream(track, head, 2, trackbuf, track_size, image)) == -1)
				if ((speed_zone = generate_bitstream(track, head, 1, trackbuf, track_size, image)) == -1)
					if ((speed_zone = generate_bitstream(track, head, 0, trackbuf, track_size, image)) == -1)
						throw emu_fatalerror("g64_format: Cannot determine speed zone for track %u", track);

		LOG_FORMATS("track %u size %u cell %u\n", track, track_size, c1541_cell_size[speed_zone]);

		UINT8 track_offset[4];
		UINT8 speed_offset[4];
		UINT8 track_length[2];

		place_integer_le(track_offset, 0, 4, dpos);
		place_integer_le(speed_offset, 0, 4, speed_zone);
		place_integer_le(track_length, 0, 2, track_size/8);

		io_generic_write(io, track_offset, tpos, 4);
		io_generic_write(io, speed_offset, spos, 4);
		io_generic_write_filler(io, 0xff, dpos, TRACK_LENGTH);
		io_generic_write(io, track_length, dpos, 2);
		io_generic_write(io, trackbuf, dpos + 2, track_size);

		tracks_written++;
	}

	return true;
}

const char *g64_format::name() const
{
	return "g64";
}

const char *g64_format::description() const
{
	return "Commodore 1541 GCR disk image";
}

const char *g64_format::extensions() const
{
	return "g64,g41";
}

const floppy_format_type FLOPPY_G64_FORMAT = &floppy_image_format_creator<g64_format>;
