// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    BBC Micro

    Disk image format

***************************************************************************/

#include "bbc_dsk.h"

bbc_format::bbc_format() : wd177x_format(formats)
{
}

const char *bbc_format::name() const
{
	return "bbc";
}

const char *bbc_format::description() const
{
	return "BBC Micro disk image";
}

const char *bbc_format::extensions() const
{
	return "bbc,img,ssd,dsd";
}

const bbc_format::format bbc_format::formats[] =
{
	{   // 100k single sided single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 16, 11, 19
	},
	{   // 200k double sided single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, 0, {}, 16, 11, 19
	},
	{   // 200k single sided double density
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, 0, {}, 16, 11, 19
	},
	{   // 400k double sided double density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, 0, {}, 16, 11, 19
	},
	{}
};

const floppy_format_type FLOPPY_BBC_FORMAT = &floppy_image_format_creator<bbc_format>;
