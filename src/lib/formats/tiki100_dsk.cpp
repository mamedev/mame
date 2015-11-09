// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/tiki100_dsk.c

    TIKI 100 disk image format

*********************************************************************/

#include <assert.h>

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

const tiki100_format::format tiki100_format::formats[] = {
	// track description
	// 20xff 6x00 fe 2x00 01 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 0a 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 06 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 0f 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 0b 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 02 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 10 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 07 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 03 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 0c 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 08 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 11 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 0d 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 04 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 12 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 09 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 05 00 f7 11xff 6x00 fb 128xe5 f7
	// 7xff 6x00 fe 2x00 0e 00 f7 11xff 6x00 fb 128xe5 f7
	// 89xff
	{   //  90K 5 1/4 inch single density single sided
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 18, 40, 1, 128, {}, -1, { 1,10,6,15,11,2,16,7,3,12,8,17,13,4,12,9,5,14 }, 20, 11, 7
	},

	// track description
	// 20x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 09 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 0a 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 275x4e
	{   //  200K 5 1/4 inch double density single sided
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 20, 22, 24
	},

	{   //  360K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 40, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 20, 22, 24
	},

	// track description
	// 20x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 09 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 0a 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 275x4e
	{   //  400K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 20, 22, 24
	},

	// track description
	// 20x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 09 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 24x4e 12x00 3xf5 fe 2x00 0a 02 f7 22x4e 12x00 3xf5 fb 512xe5 f7
	// 275x4e
	{   //  800K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 20, 22, 24
	},

	{}
};

const floppy_format_type FLOPPY_TIKI100_FORMAT = &floppy_image_format_creator<tiki100_format>;
