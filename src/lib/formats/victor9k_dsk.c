// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

	formats/victor9k_dsk.c

	Victor 9000 sector disk image format

*********************************************************************/

/*

	Sector format
	-------------

	Header sync
	Sector header (header ID, track ID, sector ID, and checksum)
	Gap 1
	Data Sync
	Data field (data sync, data ID, data bytes, and checksum)
	Gap 2

	Track format
	------------

	ZONE        LOWER HEAD  UPPER HEAD  SECTORS     ROTATIONAL   RPM
	NUMBER      TRACKS      TRACKS      PER TRACK   PERIOD (MS)

	0           0-3         unused      19          237.9        252
	1           4-15        0-7         18          224.5        267
	2           16-26       8-18        17          212.2        283
	3           27-37       19-29       16          199.9        300
	4           38-48       30-40       15          187.6        320
	5           49-59       41-51       14          175.3        342
	6           60-70       52-62       13          163.0        368
	7           71-79       63-74       12          149.6        401
	8           unused      75-79       11          144.0        417

*/

#include "emu.h"
#include "formats/victor9k_dsk.h"

victor9k_format::victor9k_format()
{
}

const char *victor9k_format::name() const
{
	return "victor9k";
}

const char *victor9k_format::description() const
{
	return "Victor 9000 disk image";
}

const char *victor9k_format::extensions() const
{
	return "img";
}

int victor9k_format::identify(io_generic *io, UINT32 form_factor)
{
	return 0;
}

bool victor9k_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	return false;
}

bool victor9k_format::supports_save() const
{
	return false;
}

const victor9k_format::format victor9k_format::formats[] = {
	{ //
		floppy_image::FF_525, floppy_image::SSDD, 80, 1, 512
	},
	{ //
		floppy_image::FF_525, floppy_image::DSDD, 80, 2, 512
	},
	{}
};

const UINT32 victor9k_format::cell_size[] =
{
	1789, 1896, 2009, 2130, 2272, 2428, 2613, 2847, 2961
};

const int victor9k_format::sectors_per_track[2][80] =
{
	{
		19, 19, 19, 19,
		18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		12, 12, 12, 12, 12, 12, 12, 12, 12
	},
	{
		18, 18, 18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
		11, 11, 11, 11, 11
	}
};

const int victor9k_format::speed_zone[2][80] =
{
	{
		0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7
	},
	{
		1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8
	}
};

const floppy_format_type FLOPPY_VICTOR_9000_FORMAT = &floppy_image_format_creator<victor9k_format>;
