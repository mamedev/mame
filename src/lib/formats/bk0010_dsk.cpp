// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/bk0010_dsk.cpp

    Floppies used by BK (BY: device), DVK (MY: device) and UKNC
    (MZ: device)

*********************************************************************/

#include "formats/bk0010_dsk.h"

bk0010_format::bk0010_format() : wd177x_format(formats)
{
}

// gap sizes taken from BKBTL emulator
const bk0010_format::format bk0010_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		10, 80, 1,
		512, {},
		1, {},
		42, 22, 36
	},
	{
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, // 2us, 300rpm
		10, 80, 2,
		512, {},
		1, {},
		42, 22, 36
	},
	{}
};

const bk0010_format FLOPPY_BK0010_FORMAT;
