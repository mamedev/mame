// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Sinclair ZX Spectrum

    Opus Discovery disk image formats

***************************************************************************/

#include "opd_dsk.h"


opd_format::opd_format() : wd177x_format(formats)
{
}

const char *opd_format::name() const
{
	return "opd";
}

const char *opd_format::description() const
{
	return "Opus Discovery disk image";
}

const char *opd_format::extensions() const
{
	return "opd,opu";
}

int opd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int const type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_SIZE;

	return 0;
}

int opd_format::get_image_offset(const format &f, int head, int track) const
{
	return (f.track_count * head + track) * compute_track_size(f);
}

const opd_format::format opd_format::formats[] =
{
	{ // 180k 40 track single sided double density - gaps unverified
		floppy_image::FF_35, floppy_image::SSSD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, 0, {}, 36, 22, 27
	},
	{ // 360k 40 track double sided double density - gaps unverified
		floppy_image::FF_35, floppy_image::DSSD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, 0, {}, 36, 22, 27
	},
	{}
};


const opd_format FLOPPY_OPD_FORMAT;
