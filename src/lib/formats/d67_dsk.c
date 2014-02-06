// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d67_dsk.c

    Commodore 2040 sector disk image format

*********************************************************************/

#include "emu.h"
#include "formats/d67_dsk.h"

d67_format::d67_format() : d64_format(file_formats)
{
}

const char *d67_format::name() const
{
	return "d67";
}

const char *d67_format::description() const
{
	return "Commodore 2040 disk image";
}

const char *d67_format::extensions() const
{
	return "d67";
}

const d67_format::format d67_format::file_formats[] = {
	{ // d67, dos 1, 35 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 690, 35, 1, 256, 9, 8
	},
	{}
};

const int d67_format::d67_sectors_per_track[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, //  1-17
	20, 20, 20, 20, 20, 20, 20,                                         // 18-24
	18, 18, 18, 18, 18, 18,                                             // 25-30
	17, 17, 17, 17, 17,                                                 // 31-35
	17, 17, 17, 17, 17,                                                 // 36-40
	17, 17                                                              // 41-42
};

int d67_format::get_sectors_per_track(const format &f, int track)
{
	return d67_sectors_per_track[track];
}

const floppy_format_type FLOPPY_D67_FORMAT = &floppy_image_format_creator<d67_format>;
