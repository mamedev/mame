// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/tiki100_dsk.c

    TIKI 100 disk image format

*********************************************************************/

#include "emu.h"
#include "formats/tiki100_dsk.h"

tiki100_format::tiki100_format() : wd177x_format(formats)
{
}

const char *tiki100_format::name() const
{
	return "tiki100";
}

const char *tiki100_format::description() const
{
	return "TIKI 100 disk image";
}

const char *tiki100_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
// double sided disks have t0s0,t0s1,t1s0,t1s1... format
const tiki100_format::format tiki100_format::formats[] = {
	{   //  90K 5 1/4 inch single density single sided
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 18, 40, 1, 128, {}, 1, {}, 40, 11, 10
	},
	{   //  200K 5 1/4 inch double density single sided
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{   //  400K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{   //  800K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{}
};

const floppy_format_type FLOPPY_TIKI100_FORMAT = &floppy_image_format_creator<tiki100_format>;
