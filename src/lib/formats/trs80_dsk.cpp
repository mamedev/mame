// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    TRS-80

    JV1 disk image format

    Used by Jeff Vavasour's TRS-80 Emulators

    TODO:
    - Gap sizes unverified

***************************************************************************/

#include "trs80_dsk.h"

trs80_format::trs80_format() : wd177x_format(formats)
{
}

const char *trs80_format::name() const
{
	return "trs80";
}

const char *trs80_format::description() const
{
	return "TRS-80 JV1 disk image";
}

const char *trs80_format::extensions() const
{
	return "dsk";
}

int trs80_format::get_track_dam_fm(const format &f, int head, int track)
{
	return (track == 17 && head == 0) ? FM_DDAM : FM_DAM;
}

const trs80_format::format trs80_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{}
};

const floppy_format_type FLOPPY_TRS80_FORMAT = &floppy_image_format_creator<trs80_format>;
