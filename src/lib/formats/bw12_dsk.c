// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/bw12_dsk.c

    bw12 format

*********************************************************************/

#include <assert.h>

#include "formats/bw12_dsk.h"

bw12_format::bw12_format() : upd765_format(formats)
{
}

const char *bw12_format::name() const
{
	return "bw12";
}

const char *bw12_format::description() const
{
	return "Bondwell 12/14 disk image";
}

const char *bw12_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const bw12_format::format bw12_format::formats[] = {
	{ // 180KB BW 12
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, 0, {}, 80, 50, 12, 80
	},
	// format track mfm h=02 n=01 sc=12 gpl=0c d=e5
	{ // 360KB BW 12
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, 0, {}, 80, 50, 12, 80
	},
	{ // SVI-328
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 17, 40, 1, 256, {}, 0, {}, 80, 50, 22, 80
	},
	{ // SVI-328
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 17, 40, 2, 256, {}, 0, {}, 80, 50, 22, 80
	},
	{ // Kaypro II
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, 0, {}, 80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_BW12_FORMAT = &floppy_image_format_creator<bw12_format>;
