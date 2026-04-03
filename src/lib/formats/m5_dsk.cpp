// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/m5_dsk.cpp

    sord m5 format

*********************************************************************/

#include "formats/m5_dsk.h"

m5_format::m5_format() : upd765_format(formats)
{
}

const char *m5_format::name() const noexcept
{
	return "m5";
}

const char *m5_format::description() const noexcept
{
	return "Sord M5 disk image";
}

const char *m5_format::extensions() const noexcept
{
	return "dsk";
}

// Unverified gap sizes
const m5_format::format m5_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, // 2us, 300rpm
		18, 40, 2,
		256, {},
		1, {},
		80, 50, 22, 80
	},
	{}
};

const m5_format FLOPPY_M5_FORMAT;


fd5_format::fd5_format() : upd765_format(formats)
{
}

const char* fd5_format::name() const noexcept
{
	return "fd5";
}

const char* fd5_format::description() const noexcept
{
	return "Sord M5 FD-5 disk image";
}

const char* fd5_format::extensions() const noexcept
{
	return "fd5,img";
}

const fd5_format::format fd5_format::formats[] = {
	// SINGLE-SIDED (SS) – 160 kB
	{
		floppy_image::FF_3,		 // 3" floppy
		floppy_image::SSDD,      // single side, double density
		floppy_image::MFM,
		2000,                    // 2 µs (cca 250 kbps in 300 rpm)
		40,                      // tracks
		1,                       // heads
		16,                      // sectors per track
		256,                     // bytes per sector
		{},                      // default IDs (1..16)
		1,                       // first sector id
		{},                      // use sequential IDs
		80, 50, 22, 80           // gap1..4 (stejné jako m5_format)
	},

	//  DOUBLE-SIDED (DS) – 320 kB
	{
		floppy_image::FF_3,
		floppy_image::DSDD,      // double side, double density
		floppy_image::MFM,
		2000,
		40,                      // tracks (cylindry)
		2,                       // heads
		16,                      // sectors per track
		256,
		{},
		1,
		{},
		80, 50, 22, 80
	},

	{}
};

const fd5_format FLOPPY_FD5_FORMAT;
