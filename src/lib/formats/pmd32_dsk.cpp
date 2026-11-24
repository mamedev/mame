// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*********************************************************************

    formats/pmd32_dsk.cpp

    Tesla PMD 32 disk images

    The PMD 32 formats 40 cylinders x 2 sides x 9 x 512-byte MFM
    sectors (gap 3 = 0x50 when formatting).  Its control program
    addresses them as 80 logical tracks of 36 128-byte sectors, so a
    P32 image in logical order is a plain sector dump.

*********************************************************************/

#include "formats/pmd32_dsk.h"

#include "ioprocs.h"

#include <algorithm>
#include <iterator>
#include <vector>

pmd32_format::pmd32_format() : upd765_format(formats)
{
}

const char *pmd32_format::name() const noexcept
{
	return "pmd32";
}

const char *pmd32_format::description() const noexcept
{
	return "Tesla PMD 32 disk image";
}

const char *pmd32_format::extensions() const noexcept
{
	return "p32";
}

const pmd32_format::format pmd32_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,
		9, 40, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const pmd32_format FLOPPY_PMD32_FORMAT;


const pmd32_c2717_format::format pmd32_c2717_format::c2717_formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,
		9, 40, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,
		9, 80, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

pmd32_c2717_format::pmd32_c2717_format() : upd765_format(c2717_formats)
{
}

const char *pmd32_c2717_format::name() const noexcept
{
	return "pmd32_c2717";
}

const char *pmd32_c2717_format::description() const noexcept
{
	return "Consul 2717 PMD 32 disk image";
}

const char *pmd32_c2717_format::extensions() const noexcept
{
	return "img,dz8";
}

int pmd32_c2717_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size) || !size || (size % LOGICAL_TRACK_SIZE) || (size > 160 * LOGICAL_TRACK_SIZE))
		return 0;

	return FIFID_SIZE;
}

bool pmd32_c2717_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size) || !size || (size % LOGICAL_TRACK_SIZE) || (size > 160 * LOGICAL_TRACK_SIZE))
		return false;

	// logical track t is cylinder t / 2, head t & 1
	unsigned const tracks = size / LOGICAL_TRACK_SIZE;
	format const &f = c2717_formats[(tracks > 80) ? 1 : 0];
	int img_tracks, img_heads;
	image.get_maximal_geometry(img_tracks, img_heads);
	if ((f.track_count > img_tracks) || (f.head_count > img_heads))
		return false;

	int current_size, end_gap_index;
	desc_e *const desc = get_desc_mfm(f, current_size, end_gap_index);

	int const total_size = 200'000'000 / f.cell_size;
	int const remaining_size = total_size - current_size;
	desc[end_gap_index].p2 = remaining_size / 16;
	desc[end_gap_index + 1].p2 = remaining_size & 15;
	desc[end_gap_index + 1].p1 >>= 16 - (remaining_size & 15);

	uint8_t sectdata[9 * 512];
	desc_s sectors[9];
	for (int track = 0; track < f.track_count; track++)
	{
		for (int head = 0; head < f.head_count; head++)
		{
			build_sector_description(f, sectdata, sectors, track, head);
			std::fill(std::begin(sectdata), std::end(sectdata), 0xe5);
			unsigned const logical = track * 2 + head;
			if (logical < tracks)
			{
				auto const [err, actual] = read_at(io, logical * LOGICAL_TRACK_SIZE, sectdata, LOGICAL_TRACK_SIZE);
				if (err || (actual != LOGICAL_TRACK_SIZE))
					return false;
			}
			generate_track(desc, track, head, sectors, f.sector_count, total_size, image);
		}
	}

	image.set_form_variant(f.form_factor, f.variant);
	return true;
}

bool pmd32_c2717_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	// a full 8" side or DZ8 disc, plus any later track holding data
	int img_tracks, img_heads;
	image.get_actual_geometry(img_tracks, img_heads);
	bool const dz8 = (image.get_variant() == floppy_image::DSQD) || (img_tracks > 40);
	format const &f = c2717_formats[dz8 ? 1 : 0];
	unsigned const disc_tracks = f.track_count * f.head_count;
	std::vector<uint8_t> data(disc_tracks * LOGICAL_TRACK_SIZE);
	unsigned tracks = dz8 ? DZ8_TRACKS : SIDE_8INCH_TRACKS;

	uint8_t sectdata[9 * 512];
	desc_s sectors[9];
	for (unsigned logical = 0; logical < disc_tracks; logical++)
	{
		build_sector_description(f, sectdata, sectors, logical / 2, logical & 1);
		extract_sectors(image, f, sectors, logical / 2, logical & 1);
		std::copy_n(sectdata, LOGICAL_TRACK_SIZE, &data[logical * LOGICAL_TRACK_SIZE]);
		if (std::any_of(sectdata, sectdata + LOGICAL_TRACK_SIZE, [] (uint8_t b) { return b != 0xe5; }))
			tracks = std::max(tracks, logical + 1);
	}

	auto const [err, actual] = write_at(io, 0, data.data(), tracks * LOGICAL_TRACK_SIZE);
	return !err && (actual == tracks * LOGICAL_TRACK_SIZE);
}

const pmd32_c2717_format FLOPPY_PMD32_C2717_FORMAT;
