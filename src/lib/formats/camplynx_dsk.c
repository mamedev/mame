// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    formats/camplynx_dsk.c

    Camputers Lynx disk image format

    There is no inter-sector info on these disks. It is simply a
    dump of the 512 bytes from each sector and track in order.

    Extension is LDF as used by the Pale emulator

    The disk is formatted with 512 bytes per sector, 10 sectors,
    6040 bytes per track. 200KB disks are single sided 40 tracks.
    800KB disks are double sided 80 tracks.

    The numbers below are guesswork since there's no documentation.
    Currently not working.

*********************************************************************/

#include <assert.h>

#include "formats/camplynx_dsk.h"

camplynx_format::camplynx_format() : wd177x_format(formats)
{
}

const char *camplynx_format::name() const
{
	return "camplynx";
}

const char *camplynx_format::description() const
{
	return "Camputers Lynx disk image";
}

const char *camplynx_format::extensions() const
{
	return "ldf";
}

const camplynx_format::format camplynx_format::formats[] = {
	{   /*  200K 13cm double density single sided */
		floppy_image::FF_525,  floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, 1, {}, 100, 22, 30 // guesswork to stop it crashing
	},
	{   /*  800K 13cm quad density double sided */
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 100, 22, 30 // guesswork to stop it crashing
	},
	{}
};

const floppy_format_type FLOPPY_CAMPLYNX_FORMAT = &floppy_image_format_creator<camplynx_format>;
