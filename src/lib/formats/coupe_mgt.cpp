// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe MGT Disk Image

***************************************************************************/

#include "coupe_mgt.h"

coupe_mgt_format::coupe_mgt_format() : wd177x_format(formats)
{
}

const char *coupe_mgt_format::name() const noexcept
{
	return "coupe_mgt";
}

const char *coupe_mgt_format::description() const noexcept
{
	return "SAM Coupe MGT disk image";
}

const char *coupe_mgt_format::extensions() const noexcept
{
	return "mgt,dsk,cpm";
}

const coupe_mgt_format::format coupe_mgt_format::formats[] =
{
	{   //  720k 3.5 inch double sided double density (usually CP/M)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 60, 22, 24
	},
	{   //  800k 3.5 inch double sided double density (standard)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 60, 22, 24
	},
	{}
};

const coupe_mgt_format FLOPPY_COUPE_MGT_FORMAT;
