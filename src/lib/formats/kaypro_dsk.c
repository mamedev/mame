// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Robbbert
/*********************************************************************

    formats/kaypro_dsk.c

    Kaypro disk image format

    There is no inter-sector info on these disks. It is simply a
    dump of the 512 bytes from each sector and track in order.
    It is just like a headerless quickload.
    No idea about how to calculate the gaps. Used the ones recommended
    in the wd177x_dsk notes, works for most disks. Please note that
    these disks usually have DSK extension, but that conflicts with
    the CPCEMU DSK format. You need to rename your Kaypro DSK disks
    to KAY extension.

*********************************************************************/

#include "emu.h"
#include "formats/kaypro_dsk.h"

kayproii_format::kayproii_format() : wd177x_format(formats)
{
}

const char *kayproii_format::name() const
{
	return "kaypro";
}

const char *kayproii_format::description() const
{
	return "Kaypro disk image";
}

const char *kayproii_format::extensions() const
{
	return "kay";
}

// gap info is a total guess
const kayproii_format::format kayproii_format::formats[] = {
	{   /*  191K 13cm double density single sided */
		floppy_image::FF_525,  floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, 0, {}, 80, 22, 24
	},
	{}
};

kaypro2x_format::kaypro2x_format() : wd177x_format(formats)
{
}

const char *kaypro2x_format::name() const
{
	return "kaypro";
}

const char *kaypro2x_format::description() const
{
	return "Kaypro disk image";
}

const char *kaypro2x_format::extensions() const
{
	return "kay";
}

// gap info is a total guess
const kaypro2x_format::format kaypro2x_format::formats[] = {
	{   /*  382K 13cm double density double sided */
		floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, 0, {}, 80, 22, 24
	},
	{}
};

const floppy_format_type FLOPPY_KAYPROII_FORMAT = &floppy_image_format_creator<kayproii_format>;
const floppy_format_type FLOPPY_KAYPRO2X_FORMAT = &floppy_image_format_creator<kaypro2x_format>;
