// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*********************************************************************

    formats/ds2717_dsk.cpp

    Consul 2717 DS 2717 disk images

    The DS 2717 uses single-sided single-density 8" discs: 77 tracks
    of 26 128-byte FM sectors.  Some dumps carry a 78th track read past
    the end of the disc; it is ignored, and not saved.

*********************************************************************/

#include "formats/ds2717_dsk.h"

#include "ioprocs.h"

ds2717_format::ds2717_format() : upd765_format(formats)
{
}

const char *ds2717_format::name() const noexcept
{
	return "ds2717";
}

const char *ds2717_format::description() const noexcept
{
	return "Consul 2717 DS 2717 disk image";
}

const char *ds2717_format::extensions() const noexcept
{
	return "img";
}

const ds2717_format::format ds2717_format::formats[] = {
	{
		floppy_image::FF_8, floppy_image::SSSD, floppy_image::FM,
		2400,
		26, 77, 1,
		128, {},
		1, {},
		40, 26, 11, 27
	},
	{}
};

int ds2717_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if ((size == 77 * TRACK_SIZE) || (size == 78 * TRACK_SIZE))
		return FIFID_SIZE;

	return 0;
}

bool ds2717_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	if (!identify(io, form_factor, variants))
		return false;

	format const &f = formats[0];
	int img_tracks, img_heads;
	image.get_maximal_geometry(img_tracks, img_heads);
	if (f.track_count > img_tracks)
		return false;

	int current_size, end_gap_index;
	desc_e *const desc = get_desc_fm(f, current_size, end_gap_index);

	int const total_size = 200'000'000 / f.cell_size;
	int const remaining_size = total_size - current_size;
	desc[end_gap_index].p2 = remaining_size / 16;
	desc[end_gap_index + 1].p2 = remaining_size & 15;
	desc[end_gap_index + 1].p1 >>= 16 - (remaining_size & 15);

	uint8_t sectdata[TRACK_SIZE];
	desc_s sectors[26];
	for (int track = 0; track < f.track_count; track++)
	{
		build_sector_description(f, sectdata, sectors, track, 0);
		auto const [err, actual] = read_at(io, track * TRACK_SIZE, sectdata, TRACK_SIZE);
		if (err || (actual != TRACK_SIZE))
			return false;
		generate_track(desc, track, 0, sectors, f.sector_count, total_size, image);
	}

	image.set_form_variant(f.form_factor, f.variant);
	return true;
}

const ds2717_format FLOPPY_DS2717_FORMAT;
