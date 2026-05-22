// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot PC/Xi

    Disk image format

    Notes:
    - The system has a WD2797 FDC, but the format is similar to the UPD765
    - Use /M0 to create single sided disks with 70 tracks
    - Use /M1 to create double sided disks with 80 tracks (default)

***************************************************************************/

#include "apricotpc_dsk.h"

apricotpc_format::apricotpc_format() : upd765_format(formats)
{
}

const char *apricotpc_format::name() const noexcept
{
	return "apricotpc";
}

const char *apricotpc_format::description() const noexcept
{
	return "ACT Apricot PC/Xi disk image";
}

const char *apricotpc_format::extensions() const noexcept
{
	return "img";
}

const apricotpc_format::format apricotpc_format::formats[] =
{
	{
		floppy_image::FF_35, floppy_image::SSDD, floppy_image::MFM,
		2000, 9, 70, 1, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{}
};

const apricotpc_format FLOPPY_APRICOTPC_FORMAT;
