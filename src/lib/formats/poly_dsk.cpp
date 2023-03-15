// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Poly CP/M Disk image format

    FLEX-POLYSYS image formats are handled in flex_dsk.

***************************************************************************/

#include "poly_dsk.h"

#include "ioprocs.h"

#include <cstring>


poly_cpm_format::poly_cpm_format()
{
}

const char *poly_cpm_format::name() const
{
	return "cpm";
}

const char *poly_cpm_format::description() const
{
	return "Poly CP/M disk image";
}

const char *poly_cpm_format::extensions() const
{
	return "cpm";
}

bool poly_cpm_format::supports_save() const
{
	return true;
}

int poly_cpm_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	// check for valid sizes
	if (size == 630784 || size == 622592 || size == 256256)
	{
		// check for Poly CP/M boot sector
		uint8_t boot[16];
		size_t actual;
		io.read_at(0, boot, 16, actual);
		if (memcmp(boot, "\x86\xc3\xb7\x00\x00\x8e\x10\xc0\xbf\x00\x01\xbf\xe0\x60\x00\x00", 16) == 0)
		{
			return FIFID_SIZE|FIFID_SIGN;
		}
	}

	return 0;
}

bool poly_cpm_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if (io.length(size) || io.seek(0, SEEK_SET))
		return false;

	int total_tracks, spt, bps, head_num;
	switch (size)
	{
	case 622592:
		// CP/M 3" disk DSSD
		total_tracks = 76;
		spt = 8;
		bps = 512;
		head_num = 2;
		break;
	case 256256:
		// CP/M 8" disk SDSD
		total_tracks = 77;
		spt = 26;
		bps = 128;
		head_num = 1;
		break;
	default:
		// CP/M 8" disk DSSD
		total_tracks = 77;
		spt = 8;
		bps = 512;
		head_num = 2;
		break;
	}

	int const cell_count = (form_factor == floppy_image::FF_525) ? 50000 : 100000;

	for (int track = 0; track < total_tracks; track++)
		for (int head = 0; head < head_num; head++)
		{
			desc_pc_sector sects[80];
			uint8_t sect_data[20000];
			int sdatapos = 0;
			for (int i = 0; i<spt; i++)
			{
				sects[i].track = track;
				sects[i].head = head;
				if (head == 0)
					sects[i].sector = i + 1;
				else
					sects[i].sector = i + 1 + spt;
				sects[i].actual_size = bps;
				sects[i].size = bps >> 8;
				sects[i].deleted = false;
				sects[i].bad_crc = false;
				sects[i].data = &sect_data[sdatapos];
				size_t actual;
				io.read(sects[i].data, bps, actual);
				sdatapos += bps;
			}
			// gap sizes unverified
			build_wd_track_fm(track, head, image, cell_count, spt, sects, 24, 16, 11);
		}
	return true;
}

const poly_cpm_format FLOPPY_POLY_CPM_FORMAT;
