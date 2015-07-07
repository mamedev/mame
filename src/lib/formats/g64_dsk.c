// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/g64_dsk.c

    Commodore 1541/1571 GCR disk image format

    http://unusedino.de/ec64/technical/formats/g64.html

*********************************************************************/

#include "emu.h" // emu_fatalerror
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
	io_generic_read(io, &img[0], 0, size);

	if (img[POS_VERSION]) {
		throw emu_fatalerror("g64_format: Unsupported version %u", img[POS_VERSION]);
	}

	int track_count = img[POS_TRACK_COUNT];
	int head = 0;

	for (int track = 0; track < track_count; track++)
	{
		int cylinder = track % TRACK_COUNT;

		if (track == TRACK_COUNT)
			head = 1;

		UINT32 tpos = POS_TRACK_OFFSET + (track * 4);
		UINT32 spos = tpos + (track_count * 4);
		UINT32 dpos = pick_integer_le(&img[0], tpos, 4);

		if (!dpos)
			continue;

		if (dpos > size)
			throw emu_fatalerror("g64_format: Track %u offset %06x out of bounds", track, dpos);

		UINT32 speed_zone = pick_integer_le(&img[0], spos, 4);

		if (speed_zone > 3)
			throw emu_fatalerror("g64_format: Unsupported variable speed zones on track %d", track);

		UINT16 track_bytes = pick_integer_le(&img[0], dpos, 2);
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
	int tracks, heads;
	image->get_actual_geometry(tracks, heads);
	tracks = TRACK_COUNT * heads;

	// write header
	UINT8 header[] = { 'G', 'C', 'R', '-', '1', '5', '4', '1', 0x00, tracks, TRACK_LENGTH & 0xff, TRACK_LENGTH >> 8 };
	io_generic_write(io, header, POS_SIGNATURE, sizeof(header));

	// write tracks
	for (int head = 0; head < heads; head++) {
		int tracks_written = 0;

		dynamic_buffer trackbuf(TRACK_LENGTH-2);

		for (int track = 0; track < TRACK_COUNT; track++) {
			UINT32 tpos = POS_TRACK_OFFSET + (track * 4);
			UINT32 spos = tpos + (tracks * 4);
			UINT32 dpos = POS_TRACK_OFFSET + (tracks * 4 * 2) + (tracks_written * TRACK_LENGTH);

			io_generic_write_filler(io, 0x00, tpos, 4);
			io_generic_write_filler(io, 0x00, spos, 4);

			if (image->get_buffer(track, head).size() <= 1)
				continue;

			int track_size;
			int speed_zone;

			// figure out the cell size and speed zone from the track data
			if ((speed_zone = generate_bitstream(track, head, 3, &trackbuf[0], track_size, image)) == -1)
				if ((speed_zone = generate_bitstream(track, head, 2, &trackbuf[0], track_size, image)) == -1)
					if ((speed_zone = generate_bitstream(track, head, 1, &trackbuf[0], track_size, image)) == -1)
						if ((speed_zone = generate_bitstream(track, head, 0, &trackbuf[0], track_size, image)) == -1)
							throw emu_fatalerror("g64_format: Cannot determine speed zone for track %u", track);

			LOG_FORMATS("head %u track %u size %u cell %u\n", head, track, track_size, c1541_cell_size[speed_zone]);

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
			io_generic_write(io, &trackbuf[0], dpos + 2, track_size);

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
