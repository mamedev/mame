// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/ppg_dsk.c

    PPG Waveterm formats

*********************************************************************/

#include "formats/ppg_dsk.h"

ppg_format::ppg_format() : wd177x_format(formats)
{
}

const char *ppg_format::name() const
{
	return "ppg";
}

const char *ppg_format::description() const
{
	return "PPG Waveterm disk image";
}

const char *ppg_format::extensions() const
{
	return "wta";
}

const ppg_format::format ppg_format::formats[] = {
	// .dsk formats used by flexemu.  gaps unverified.
	{ floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  10, 35, 1, 256,  {}, 1, {},  32, 22, 31 },
	{ floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  10, 40, 1, 256,  {}, 1, {},  32, 22, 31 },
	{ floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 35, 2, 256,  {}, 1, {},  32, 22, 31 },
	{ floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 40, 2, 256,  {}, 1, {},  32, 22, 31 },
	{ floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  20, 35, 2, 256,  {}, 1, {},  32,  2,  2 },
	{ floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  20, 40, 2, 256,  {}, 1, {},  32,  2,  2 },

	// .wta format used by PPG Waveterm A transfer software.  gaps unverified.
	{ floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  16, 77, 2, 256,  {}, 1, {},  32, 22, 31 },
	{}
};

void ppg_format::build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const
{
	int cur_offset = 0;

	for (int i=0; i<f.sector_count; i++) {
		sectors[i].data = sectdata + cur_offset;
		sectors[i].size = f.sector_base_size ? f.sector_base_size : f.per_sector_size[i];
		cur_offset += sectors[i].size;
		sectors[i].sector_id = i + f.sector_base_id + (head * f.sector_count);
	}
}

const floppy_format_type FLOPPY_PPG_FORMAT = &floppy_image_format_creator<ppg_format>;
