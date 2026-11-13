// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/mm_dsk.cpp

    mm format

*********************************************************************/

#include "formats/mm_dsk.h"

mm1_format::mm1_format() : upd765_format(formats)
{
}

const char *mm1_format::name() const noexcept
{
	return "mm1";
}

const char *mm1_format::description() const noexcept
{
	return "Nokia MikroMikko 1 disk image";
}

const char *mm1_format::extensions() const noexcept
{
	return "img";
}

const mm1_format::format mm1_format::formats[] = {
    {
		// 320KB
        floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
        2000, // 2us, 300rpm
        8, 80, 1,
        512, {},
        -1, { 1,4,7,2,5,8,3,6 },
        80, 50, 22, 80
    },
    {
		// 640KB
        floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
        2000, // 2us, 300rpm
        8, 80, 2,
        512, {},
        -1, { 1,4,7,2,5,8,3,6 },
        80, 50, 22, 80
    },
    {}
};

const mm1_format FLOPPY_MM1_FORMAT;
