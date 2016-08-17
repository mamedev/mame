// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ql_dsk.c

    Sinclair QL disk image formats

*********************************************************************/

#include <assert.h>

#include "formats/ql_dsk.h"

ql_format::ql_format() : wd177x_format(formats)
{
}

const char *ql_format::name() const
{
	return "ql";
}

const char *ql_format::description() const
{
	return "Sinclair QL disk image";
}

const char *ql_format::extensions() const
{
	return "dsk,img";
}

// unverified gaps
const ql_format::format ql_format::formats[] = {
	{   // QDOS 800KB
		floppy_image::FF_35, floppy_image::SSSD, floppy_image::MFM,
		2000, 5, 80, 2, 1024, {}, 1, {}, 80, 22, 24
	},
	{   // 720KB DSDD
		floppy_image::FF_35, floppy_image::SSSD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 80, 22, 24
	},
	{   // 1.44MB DSHD
		floppy_image::FF_35, floppy_image::SSSD, floppy_image::MFM,
		2000, 18, 80, 2, 512, {}, 1, {}, 80, 22, 24
	},
	{   // 2.88MB DSED
		floppy_image::FF_35, floppy_image::SSSD, floppy_image::MFM,
		2000, 40, 80, 2, 512, {}, 1, {}, 80, 41, 24
	},
	{}
};

const floppy_format_type FLOPPY_QL_FORMAT = &floppy_image_format_creator<ql_format>;
