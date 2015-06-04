// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie

    Disk image format

***************************************************************************/

#include "cgenie_dsk.h"

cgenie_format::cgenie_format() : wd177x_format(formats)
{
}

const char *cgenie_format::name() const
{
	return "cgenie";
}

const char *cgenie_format::description() const
{
	return "Colour Genie disk image";
}

const char *cgenie_format::extensions() const
{
	return "dsk";
}

int cgenie_format::get_track_dam_fm(const format &f, int head, int track)
{
	return (track == f.track_count/2) ? FM_DDAM : FM_DAM;
}

int cgenie_format::get_track_dam_mfm(const format &f, int head, int track)
{
	return (track == f.track_count/2) ? MFM_DDAM : MFM_DAM;
}

const cgenie_format::format cgenie_format::formats[] =
{
	{   //  102k 5 1/4 inch single density single sided (Type A)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 41, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{   //  184k 5 1/4 inch double density single sided (Type C)
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 42, 1, 256, {}, 0, {}, 21, 22, 16
	},
	{   //  204k 5 1/4 inch single density single sided (Type I)
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
		4000, 10, 81, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{   //  368k 5 1/4 inch double density single sided (Type K)
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 18, 82, 1, 256, {}, 0, {}, 21, 22, 16
	},
	{   //  204k 5 1/4 inch single density double sided (Type B)
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 41, 2, 256, {}, 0, {}, 14, 11, 12
	},
	{   //  368k 5 1/4 inch double density double sided (Type D)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 42, 2, 256, {}, 0, {}, 21, 22, 16
	},
	{   //  408k 5 1/4 inch single density double sided (Type J)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::FM,
		4000, 10, 81, 2, 256, {}, 0, {}, 14, 11, 12
	},
	{   //  736k 5 1/4 inch double density double sided (Type L)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 18, 82, 2, 256, {}, 0, {}, 21, 22, 16
	},
	{}
};

const floppy_format_type FLOPPY_CGENIE_FORMAT = &floppy_image_format_creator<cgenie_format>;
