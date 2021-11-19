// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dim_dsk.c

    DIM disk images

*********************************************************************/

#include "dim_dsk.h"

#include "ioprocs.h"

#include <cstring>

dim_format::dim_format()
{
}

const char *dim_format::name() const
{
	return "dim";
}

const char *dim_format::description() const
{
	return "DIM disk image";
}

const char *dim_format::extensions() const
{
	return "dim";
}

int dim_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t h[16];

	size_t actual;
	io.read_at(0xab, h, 16, actual);

	if(strncmp((const char *)h, "DIFC HEADER", 11) == 0)
		return 100;

	return 0;
}

bool dim_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	size_t actual;
	int offset = 0x100;
	uint8_t h;
	uint8_t track_total = 77;
	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	io.read_at(0, &h, 1, actual);

	int spt, gap3, bps, size;
	switch(h) {
	case 0:
	default:
		spt = 8;
		gap3 = 0x74;
		size = 3;
		break;
	case 1:
		spt = 9;
		gap3 = 0x39;
		size = 3;
		break;
	case 3:
		spt = 9;
		gap3 = 0x39;
		size = 3;
		break;
	case 2:
		spt = 15;
		gap3 = 0x54;
		size = 2;
		break;
	case 9:
		spt = 18;
		gap3 = 0x54;
		size = 2;
		break;
	case 17:
		spt = 26;
		gap3 = 0x33;
		size = 1;
		break;
	}
	bps = 128 << size;

	for(int track=0; track < track_total; track++)
		for(int head=0; head < 2; head++) {
			desc_pc_sector sects[30];
			uint8_t sect_data[10000];
			int sdatapos = 0;
			for(int i=0; i<spt; i++) {
				sects[i].track       = track;
				sects[i].head        = head;
				if(h == 1)  // handle 2HS sector layout
				{
					if(i == 0 && track == 0)
						sects[i].sector      = i+1;
					else
						sects[i].sector      = i+10;
				}
				else
					sects[i].sector      = i+1;
				sects[i].size        = size;
				sects[i].actual_size = bps;
				sects[i].deleted     = false;
				sects[i].bad_crc     = false;
				sects[i].data        = &sect_data[sdatapos];
				io.read_at(offset, sects[i].data, bps, actual);
				offset += bps;
				sdatapos += bps;
			}

			build_pc_track_mfm(track, head, image, cell_count, spt, sects, gap3);
		}

	return true;
}


bool dim_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	return false;
}

bool dim_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_DIM_FORMAT = &floppy_image_format_creator<dim_format>;
