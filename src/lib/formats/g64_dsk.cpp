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

#include <algorithm>
#include <cstring>


#define G64_FORMAT_HEADER   "GCR-1541"
#define G71_FORMAT_HEADER   "GCR-1571"

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

//-------------------------------------------------
//  speed_zone - nominal speed zone of a half track,
//  after VICE disk_image_speed_map()
//-------------------------------------------------

int g64_format::speed_zone(int cylinder)
{
	int const track = (cylinder / 2) + 1;

	return (track < 31) + (track < 25) + (track < 18);
}


//-------------------------------------------------
//  raw_track_size - bytes one revolution holds at
//  a speed zone's nominal density, after VICE
//  disk_image_raw_track_size(): 6250/6666/7142/7692
//-------------------------------------------------

uint32_t g64_format::raw_track_size(int speed_zone)
{
	return 200000000L / (c1541_cell_size[speed_zone] * 8);
}


//-------------------------------------------------
//  generate_empty_track - lay down a track that the
//  image does not carry data for
//-------------------------------------------------

void g64_format::generate_empty_track(int cylinder, int head, uint8_t fill, floppy_image &image)
{
	// a fill of 0x00 has no flux reversals to lay down at all, and an image
	// track with no cell data is already exactly that
	if (!fill)
		return;

	uint32_t const track_bytes = raw_track_size(speed_zone(cylinder));
	std::vector<uint8_t> const trackbuf(track_bytes, fill);

	generate_track_from_bitstream(cylinder, head, trackbuf.data(), track_bytes * 8, image);
}


//-------------------------------------------------
//  is_empty_track - recognise the 0x55 filler laid
//  down for a half track the image has no data for
//-------------------------------------------------

bool g64_format::is_empty_track(const std::vector<bool> &trackbuf)
{
	if (trackbuf.empty())
		return true;

	// the filler alternates on every cell, which no real GCR track does -- a
	// sync mark alone is ten consecutive ones
	for (uint32_t i = 1; i < trackbuf.size(); i++)
		if (trackbuf[i] == trackbuf[i - 1])
			return false;

	return true;
}


int g64_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	char h[8];
	auto const [err, actual] = read_at(io, 0, h, 8);
	if (err || (8 != actual))
		return 0;

	if (!memcmp(h, G64_FORMAT_HEADER, 8) || !memcmp(h, G71_FORMAT_HEADER, 8))
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

	if (size < POS_TRACK_OFFSET)
	{
		osd_printf_error("g64_format: File too small for header\n");
		return false;
	}

	if (img[POS_VERSION])
	{
		osd_printf_error("g64_format: Unsupported version %u\n", img[POS_VERSION]);
		return false;
	}

	int image_tracks, image_heads;
	image.get_maximal_geometry(image_tracks, image_heads);

	// only the 1571 image carries a second side; a 1541 image is capped at one
	bool dual_sided = !memcmp(&img[POS_SIGNATURE], G71_FORMAT_HEADER, 8) && (image_heads > 1);
	int half_tracks_per_side = std::min<int>(TRACK_COUNT, image_tracks);
	int max_half_tracks = half_tracks_per_side * (dual_sided ? 2 : 1);

	int track_count = img[POS_TRACK_COUNT];

	if (track_count > max_half_tracks)
	{
		osd_printf_error("g64_format: Too many half tracks (%d)\n", track_count);
		return false;
	}

	uint16_t max_track_size = get_u16le(&img[POS_MAX_TRACK_SIZE]);

	int head = 0;

	for (int track = 0; track < max_half_tracks; track++)
	{
		int cylinder = track % half_tracks_per_side;

		if (track == half_tracks_per_side)
			head = 1;

		if (track >= track_count)
		{
			// past the end of the image: no flux at all, so the read amplifier
			// is left to find its own reversals in the noise floor
			generate_empty_track(cylinder, head, 0x00, image);
			continue;
		}

		uint32_t tpos = POS_TRACK_OFFSET + (track * 4);
		uint32_t spos = tpos + (track_count * 4);

		if ((uint64_t(spos) + 4) > size)
		{
			osd_printf_error("g64_format: Track %u offset table entry out of bounds\n", track);
			return false;
		}

		uint32_t dpos = get_u32le(&img[tpos]);

		if (!dpos)
		{
			// the image knows of this half track but carries no data for it:
			// steady flux that can never frame a sync mark, not silence
			generate_empty_track(cylinder, head, 0x00, image);
			continue;
		}

		if ((uint64_t(dpos) + 2) > size)
		{
			osd_printf_error("g64_format: Track %u offset %06x out of bounds\n", track, dpos);
			return false;
		}

		// a speed zone above 3 is an offset to a per-byte speed table; the
		// declared density is not used to time the cells either way, so just
		// note it rather than refusing the image
		uint32_t zone = get_u32le(&img[spos]);

		if (zone > 3)
			osd_printf_verbose("g64_format: Variable speed zones on track %d are ignored\n", track);

		uint16_t track_bytes = get_u16le(&img[dpos]);
		int track_size = track_bytes * 8;

		if (!track_bytes || (track_bytes > max_track_size))
		{
			osd_printf_error("g64_format: Track %u length %u is not supported\n", track, track_bytes);
			return false;
		}

		if ((uint64_t(dpos) + 2 + track_bytes) > size)
		{
			osd_printf_error("g64_format: Track %u data (%u bytes) out of bounds\n", track, track_bytes);
			return false;
		}

		LOG_FORMATS("head %u track %u offs %u size %u cell %ld\n", head, cylinder, dpos, track_bytes, 200000000L/track_size);

		generate_track_from_bitstream(cylinder, head, &img[dpos+2], track_size, image);
	}

	image.set_variant(dual_sided ? floppy_image::DSSD : floppy_image::SSSD);

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

int g64_format::encode_track(int track, int head, const floppy_image &image, std::vector<uint8_t> &packed, int &zone)
{
	if (image.get_buffer(track, head).size() <= 1)
		return 0;

	// half tracks the loader filled in for absent image data are not
	// content, so leave their offset table entries zeroed as found
	std::vector<bool> trackbuf;
	generate_bitstream(track, head, speed_zone(track), trackbuf, image);

	if (is_empty_track(trackbuf))
		return 0;

	// figure out the cell size and speed zone from the track data
	if ((zone = generate_bitstream(track, head, 3, trackbuf, image)) == -1)
		if ((zone = generate_bitstream(track, head, 2, trackbuf, image)) == -1)
			if ((zone = generate_bitstream(track, head, 1, trackbuf, image)) == -1)
				if ((zone = generate_bitstream(track, head, 0, trackbuf, image)) == -1)
					return -1;

	packed.assign((trackbuf.size() + 7) >> 3, 0);
	for (uint32_t i = 0; i != trackbuf.size(); i++)
		if (trackbuf[i])
			packed[i >> 3] |= 0x80 >> (i & 7);

	return 1;
}

bool g64_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	uint8_t const zerofill[] = { 0x00, 0x00, 0x00, 0x00 };

	auto write = [&io](uint32_t pos, void const *buf, std::size_t len) -> bool
	{
		auto const [err, actual] = write_at(io, pos, buf, len);
		if (err || (actual != len))
		{
			osd_printf_error("g64_format: Write error at offset %06x\n", pos);
			return false;
		}
		return true;
	};

	int tracks, heads;
	image.get_actual_geometry(tracks, heads);
	tracks = TRACK_COUNT * heads;

	// pass 1: find the largest encoded track before writing anything
	uint32_t max_track_size = TRACK_LENGTH;
	for (int head = 0; head < heads; head++) {
		for (int track = 0; track < TRACK_COUNT; track++) {
			std::vector<uint8_t> packed;
			int zone;
			if (encode_track(track, head, image, packed, zone) == 1)
				max_track_size = std::max<uint32_t>(max_track_size, packed.size());
		}
	}

	std::vector<uint8_t> const prefill(max_track_size, 0xff);

	// write header
	uint8_t header[12];
	std::memcpy(header, (heads == 2) ? G71_FORMAT_HEADER : G64_FORMAT_HEADER, 8);
	header[8] = 0x00;
	header[9] = static_cast<uint8_t>(tracks);
	header[10] = max_track_size & 0xff;
	header[11] = (max_track_size >> 8) & 0xff;
	if (!write(POS_SIGNATURE, header, sizeof(header)))
		return false;

	// pass 2: write tracks
	int tracks_written = 0;

	for (int head = 0; head < heads; head++) {
		for (int track = 0; track < TRACK_COUNT; track++) {
			uint32_t const tpos = POS_TRACK_OFFSET + ((head * TRACK_COUNT + track) * 4);
			uint32_t const spos = tpos + (tracks * 4);
			uint32_t const dpos = POS_TRACK_OFFSET + (tracks * 4 * 2) + (tracks_written * max_track_size);

			if (!write(tpos, zerofill, 4))
				return false;
			if (!write(spos, zerofill, 4))
				return false;

			std::vector<uint8_t> packed;
			int zone;
			int const result = encode_track(track, head, image, packed, zone);

			if (result == 0)
				continue;

			if (result == -1) {
				osd_printf_error("g64_format: Cannot determine speed zone for track %u\n", track);
				return false;
			}

			LOG_FORMATS("head %u track %u size %u cell %u\n", head, track, unsigned(packed.size()), c1541_cell_size[zone]);

			uint8_t track_offset[4];
			uint8_t speed_offset[4];
			uint8_t track_length[2];

			put_u32le(track_offset, dpos);
			put_u32le(speed_offset, zone);
			put_u16le(track_length, packed.size());

			if (!write(tpos, track_offset, 4))
				return false;
			if (!write(spos, speed_offset, 4))
				return false;
			if (!write(dpos, prefill.data(), max_track_size))
				return false;
			if (!write(dpos, track_length, 2))
				return false;
			if (!write(dpos + 2, packed.data(), packed.size()))
				return false;

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
