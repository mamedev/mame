// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc800i_dsk.cpp

    Luxor ABC 830 interleaved disk image formats

*********************************************************************/

#include "abc800i_dsk.h"

#include "ioprocs.h"


abc800i_format::abc800i_format() : wd177x_format(formats)
{
}

const char *abc800i_format::name() const noexcept
{
	return "abc800i";
}

const char *abc800i_format::description() const noexcept
{
	return "Luxor ABC 830 interleaved disk image";
}

const char *abc800i_format::extensions() const noexcept
{
	return "dsk";
}

const abc800i_format::format abc800i_format::formats[] = {
	// track description
	// 28xff 6x00 fe 2x00 01 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 02 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 03 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 04 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 05 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 06 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 07 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 08 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 09 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 0a 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 0b 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 0c 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 0d 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 0e 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 0f 00 f7 11xff 6x00 fb 128xe5 f7
	// 27xff 6x00 fe 2x00 10 00 f7 11xff 6x00 fb 128xe5 f7
	// 117xff

	{   //  80K 5 1/4 inch single density single sided
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 16, 40, 1, 128, {}, -1, { 1,2,11,12,5,6,15,16,9,10,3,4,13,14,7,8 }, 28, 11, 27
	},

	// track description
	// 55x4e 12x00 3xf5 fe 2x00 01 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 02 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 03 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 04 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 05 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 06 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 07 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 08 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 09 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0a 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0b 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0c 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0d 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0e 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 0f 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 2x00 10 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 298x4e

	{   //  160K 5 1/4 inch double density single sided
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, -1, { 1,8,15,6,13,4,11,2,9,16,7,14,5,12,3,10 }, 55, 22, 54
	},

	{}
};

const abc800i_format FLOPPY_ABC800I_FORMAT;

int abc800i_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[1];
	auto const [err, actual] = read_at(io, 0x810, h, 1);

	// start of directory
	if (!err && (actual == 1) && (h[0] == 0x03))
		return FIFID_SIGN;

	return 0;
}

void abc800i_format::build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const
{
	if(f.sector_base_id == -1) {
		for(int i=0; i<f.sector_count; i++) {
			int cur_offset = 0;
			for(int j=0; j<f.sector_count; j++)
				if(f.per_sector_id[j] < f.per_sector_id[i])
					cur_offset += f.sector_base_size ? f.sector_base_size : f.per_sector_size[j];
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			sectors[i].sector_id = i + f.per_sector_id[0];
		}
	} else {
		int cur_offset = 0;
		for(int i=0; i<f.sector_count; i++) {
			sectors[i].data = sectdata + cur_offset;
			sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
			cur_offset += sectors[i].size;
			sectors[i].sector_id = i + f.sector_base_id;
		}
	}
}
