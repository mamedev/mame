// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/dip_dsk.cpp

    PC98 DIP disk images

    0x100 header, followed by track data

    TODO:
    - Investigate header structure
    - can this format be used to support different disc types?

*********************************************************************/

#include "dip_dsk.h"

#include "ioprocs.h"


dip_format::dip_format()
{
}

const char *dip_format::name() const noexcept
{
	return "dip";
}

const char *dip_format::description() const noexcept
{
	return "DIP disk image";
}

const char *dip_format::extensions() const noexcept
{
	return "dip";
}

int dip_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == 0x134000 + 0x100)
		return FIFID_SIZE;

	return 0;
}

bool dip_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	int heads, tracks, spt, bps;

	//For the moment we only support this disk structure...
	//2 sides, 77 tracks, 8 sectors/track, 1024 bytes/sector = 1261568 bytes (360rpm)
	heads = 2;
	tracks = 77;
	spt = 8;
	bps = 1024;

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	int ssize;
	for (ssize = 0; (128 << ssize) < bps; ssize++) {};

	desc_pc_sector sects[256];
	uint8_t sect_data[65536];

	for (int track = 0; track < tracks; track++)
		for (int head = 0; head < heads; head++)
		{
			/*auto const [err, actual] =*/ read_at(io, 0x100 + bps * spt * (track * heads + head), sect_data, bps * spt); // FIXME: check for errors and premature EOF

			for (int i = 0; i < spt; i++)
			{
				sects[i].track       = track;
				sects[i].head        = head;
				sects[i].sector      = i + 1;
				sects[i].size        = ssize;
				sects[i].actual_size = bps;
				sects[i].deleted     = false;
				sects[i].bad_crc     = false;
				sects[i].data        = sect_data + i * bps;
			}

			build_pc_track_mfm(track, head, image, cell_count, spt, sects, calc_default_pc_gap3_size(form_factor, bps));
		}

	return true;
}

const dip_format FLOPPY_DIP_FORMAT;
