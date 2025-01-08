// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/*********************************************************************

    formats/hp300_dsk.cpp

    HP 9000/300 disk format

*********************************************************************/

#include "formats/hp300_dsk.h"

hp300_format::hp300_format() : wd177x_format(formats)
{
}

const char *hp300_format::name() const noexcept
{
	return "hp300";
}

const char *hp300_format::description() const noexcept
{
	return "HP 9000/300 disk image";
}

const char *hp300_format::extensions() const noexcept
{
	return "img";
}

const hp300_format::format hp300_format::formats[] = {
	{ floppy_image::FF_35,  floppy_image::SSDD, floppy_image::MFM, 2000,  16, 66, 1,  256,  {}, 0, {}, 32,  22,  46 }, // FORMAT 4
	// HP 9121S, 9121D or 9133A
	{ floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM, 2000,  16, 77, 2,  256,  {}, 1, {}, 50,  22,  54 }, // FORMAT 0,1
	{ floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM, 2000,   9, 77, 2,  512,  {}, 1, {}, 50,  22,  89 }, // FORMAT 2
	{ floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM, 2000,   5, 77, 2, 1024,  {}, 1, {}, 50,  22, 108 }, // FORMAT 3
	{ floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM, 2000,   9, 80, 2,  512,  {}, 1, {}, 146, 22,  81 }, // FORMAT 16
	// HP9122C/D, 9123D, 9133D/H/L or 9153A/B
	{ floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM, 2000,  32, 77, 2,  256,  {}, 1, {}, 50,  22,  59 }, // FORMAT 0,1,4
	{ floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM, 2000,  18, 77, 2,  512,  {}, 1, {}, 50,  22,  97 }, // FORMAT 2
	{ floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM, 2000,  10, 77, 2, 1024,  {}, 1, {}, 50,  22, 123 }, // FORMAT 3
	{ floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM, 2000,  18, 80, 2,  512,  {}, 1, {}, 146, 22,  98 }, // FORMAT 16
	{}
};

const hp300_format FLOPPY_HP300_FORMAT;

