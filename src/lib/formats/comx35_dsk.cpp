// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/comx35_dsk.c

    COMX-35 disk image format

*********************************************************************/

/*

    TODO:

    - implement 70 track image detection (first byte of file = 0x01 -> single sided)
    - format double sided, single track density

        :exp:fd:wd1770: track description
        40xff 6x00
        fe 4x00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 07 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0e 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 05 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0c 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 03 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0a 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 01 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 08 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0f 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 06 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0d 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 04 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0b 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 02 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 09 00 f7 17x00 fb 128x00 f7 3476x00

*/

#include <assert.h>

#include "formats/comx35_dsk.h"

comx35_format::comx35_format() : wd177x_format(formats)
{
}

const char *comx35_format::name() const
{
	return "comx35";
}

const char *comx35_format::description() const
{
	return "COMX-35 disk image";
}

const char *comx35_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
const comx35_format::format comx35_format::formats[] = {
	{   //  70K 5 1/4 inch single density, single sided, 35 tracks
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 16, 35, 1, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	{   //  140K 5 1/4 inch single density, double sided, 35 tracks
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 16, 35, 2, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	/*{   //  140K 5 1/4 inch single density, double track density, single sided, 70 tracks
	    floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
	    4000, 16, 70, 1, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},*/
	{}
};

const floppy_format_type FLOPPY_COMX35_FORMAT = &floppy_image_format_creator<comx35_format>;
