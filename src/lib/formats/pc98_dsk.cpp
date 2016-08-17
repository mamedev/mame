// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*********************************************************************

    formats/pc98_dsk.c

    PC-98 disk images

*********************************************************************/

#include <string.h>
#include <assert.h>

#include "formats/pc98_dsk.h"


/* ----------------------------------------------------------------------- */

pc98_format::pc98_format() : upd765_format(formats)
{
}

const char *pc98_format::name() const
{
	return "pc98";
}

const char *pc98_format::description() const
{
	return "PC-98 floppy disk image";
}

const char *pc98_format::extensions() const
{
	return "dsk,ima,img,ufi,360,hdm";
}

const pc98_format::format pc98_format::formats[] = {
	{   /*  160K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  8, 40, 1, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  320K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  8, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  180K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000,  9, 40, 1, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  360K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  400K 5 1/4 inch double density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  720K 5 1/4 inch quad density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /* 1200K 5 1/4 inch high density */
		floppy_image::FF_525, floppy_image::DSHD, floppy_image::MFM,
		1200, 15, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{   /*  720K 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /* 1200K 3 1/2 inch high density (japanese variant) - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1200, 15, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{   /* 1440K 3 1/2 inch high density */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 18, 80, 2, 512, {}, 1, {}, 80, 50, 22, 108
	},
	{   /* 2880K 3 1/2 inch extended density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSED, floppy_image::MFM,
		500, 36, 80, 2, 512, {}, 1, {}, 80, 50, 41, 80
	},
	{
		floppy_image::FF_525,  floppy_image::DSHD, floppy_image::MFM,
		1200, 8, 77, 2, 1024, {}, 1, {}, 80, 50, 22, 84
	},
	{}
};


const floppy_format_type FLOPPY_PC98_FORMAT = &floppy_image_format_creator<pc98_format>;
