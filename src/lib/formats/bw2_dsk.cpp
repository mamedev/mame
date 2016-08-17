// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/bw2_dsk.c

    bw2 format

*********************************************************************/

#include <assert.h>

#include "formats/bw2_dsk.h"

bw2_format::bw2_format() : upd765_format(formats)
{
}

const char *bw2_format::name() const
{
	return "bw2";
}

const char *bw2_format::description() const
{
	return "Bondwell 2 disk image";
}

const char *bw2_format::extensions() const
{
	return "dsk";
}

const bw2_format::format bw2_format::formats[] = {
	{   // 340K 3 1/2 inch double density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 17, 80, 1, 256, {}, 0, {}, 80, 20, 22, 14
	},
	{   // 360K 3 1/2 inch double density
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, 0, {}, 80, 20, 22, 14
	},
	{}
};

const floppy_format_type FLOPPY_BW2_FORMAT = &floppy_image_format_creator<bw2_format>;
