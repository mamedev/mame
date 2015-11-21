// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc800_dsk.c

    Luxor ABC 830/832/834/838 disk image formats

*********************************************************************/

#include <assert.h>

#include "formats/abc800_dsk.h"

abc800_format::abc800_format() : wd177x_format(formats)
{
}

const char *abc800_format::name() const
{
	return "abc800";
}

const char *abc800_format::description() const
{
	return "Luxor ABC 830/832/834/838 disk image";
}

const char *abc800_format::extensions() const
{
	return "dsk";
}

const abc800_format::format abc800_format::formats[] = {
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
		4000, 16, 40, 1, 128, {}, 1, {}, 28, 11, 27
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
		2000, 16, 40, 1, 256, {}, 1, {}, 55, 22, 54
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

	{   //  640K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, 1, {}, 55, 22, 54
	},

	// track description
	// 55x4e 12x00 3xf5 fe 00 01 01 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 02 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 03 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 04 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 05 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 06 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 07 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 08 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 09 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 0a 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 0b 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 0c 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 0d 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 0e 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 0f 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 10 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 11 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 12 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 13 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 14 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 15 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 16 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 17 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 18 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 19 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 54x4e 12x00 3xf5 fe 00 01 1a 01 f7 22x4e 12x00 3xf5 fb 256xe5 f7
	// 2828x4e

	{   //  1M 8 inch double density
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		2000, 26, 77, 2, 256, {}, 1, {}, 55, 22, 54
	},

	{}
};

const floppy_format_type FLOPPY_ABC800_FORMAT = &floppy_image_format_creator<abc800_format>;
