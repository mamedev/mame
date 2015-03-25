// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/naslite_dsk.c

    NASLite 1.72MB with funky interleaving format

*********************************************************************/

#include <assert.h>

#include "formats/naslite_dsk.h"

naslite_format::naslite_format() : upd765_format(formats)
{
}

const char *naslite_format::name() const
{
	return "NASLite";
}

const char *naslite_format::description() const
{
	return "NASLite disk image";
}

const char *naslite_format::extensions() const
{
	return "img";
}

const naslite_format::format naslite_format::formats[] = {
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 21, 82, 2, 512, {}, -1, { 0x01, 0x0c, 0x02, 0x0d, 0x03, 0x0e, 0x04, 0x0f, 0x05, 0x10, 0x06, 0x11, 0x07, 0x12, 0x08, 0x13, 0x09, 0x14, 0x0a, 0x15, 0x0b }, 80, 50, 22, 0xc
	},
	{}
};

void naslite_format::build_sector_description(const format &f, UINT8 *sectdata, desc_s *sectors, int track, int head) const
{
	for(int i=0; i<f.sector_count; i++) {
		int cur_offset = 0;
		for(int j=0; j<f.sector_count; j++)
			if(f.per_sector_id[j] < f.per_sector_id[i])
				cur_offset += f.sector_base_size ? f.sector_base_size : f.per_sector_size[j];
		sectors[i].data = sectdata + cur_offset;
		sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
		sectors[i].sector_id = f.per_sector_id[(i + (track * 0x0a) + (head * 0x11)) % f.sector_count];
	}
}

const floppy_format_type FLOPPY_NASLITE_FORMAT = &floppy_image_format_creator<naslite_format>;
