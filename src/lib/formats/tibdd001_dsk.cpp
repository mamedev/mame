// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/tibdd001_dsk.cpp

    TIB Disc Drive DD-001 disk images

*********************************************************************/

#include "formats/tibdd001_dsk.h"

tib_dd_001_format::tib_dd_001_format() : upd765_format(formats)
{
}

const char *tib_dd_001_format::name() const
{
	return "tibdd001";
}

const char *tib_dd_001_format::description() const
{
	return "TIB Disc Drive DD-001 floppy disk image";
}

const char *tib_dd_001_format::extensions() const
{
	return "img";
}

const tib_dd_001_format::format tib_dd_001_format::formats[] = {
	{   /*  720K 3 1/2 inch double density */
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{}
};

const tib_dd_001_format FLOPPY_TIB_DD_001_FORMAT;
