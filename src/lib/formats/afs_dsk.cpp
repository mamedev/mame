// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn FileStore

    Disk image formats

***************************************************************************/

#include "afs_dsk.h"


afs_format::afs_format() : wd177x_format(formats)
{
}

const char *afs_format::name() const
{
	return "afs";
}

const char *afs_format::description() const
{
	return "Acorn FileStore disk image";
}

const char *afs_format::extensions() const
{
	return "adl,img";
}

int afs_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return 50;

	return 0;
}

int afs_format::get_image_offset(const format &f, int head, int track)
{
	if (f.sector_base_id == -1)
		return (track * f.head_count + head) * compute_track_size(f);
	else
		return (f.track_count * head + track) * compute_track_size(f);
}

const afs_format::format afs_format::formats[] =
{
	{ // 640K 3 1/2 inch 80 track double sided double density (interleaved) - gaps unverified
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }, 60, 22, 43
	},
	{}
};


const floppy_format_type FLOPPY_AFS_FORMAT = &floppy_image_format_creator<afs_format>;
