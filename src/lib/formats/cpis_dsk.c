// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/cpis_dsk.c

    Telenova Compis disk images

*********************************************************************/

#include <string.h>
#include <assert.h>

#include "formats/cpis_dsk.h"
#include "formats/basicdsk.h"


static int compis_get_tracks_and_sectors(floppy_image_legacy *floppy, int *tracks, int *sectors)
{
	switch(floppy_image_size(floppy)) {
	case 0x50000:   /* 320 KB */
		*tracks = 40;
		*sectors = 8;
		break;

	case 0x5a000:   /* 360 KB */
		*tracks = 40;
		*sectors = 9;
		break;

	case 0xa0000:   /* 640 KB */
		*tracks = 80;
		*sectors = 8;
		break;

	case 0xb4000:   /* 720 KB */
		*tracks = 80;
		*sectors = 9;
		break;

	case 0x12c000:  /* 1200 KB */
		*tracks = 80;
		*sectors = 15;
		break;

	default:
		return 0;
	}
	return 1;
}



static FLOPPY_IDENTIFY(compis_dsk_identify)
{
	int dummy;
	*vote = compis_get_tracks_and_sectors(floppy, &dummy, &dummy) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(compis_dsk_construct)
{
	struct basicdsk_geometry geometry;

	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 1;
	geometry.first_sector_id = 1;
	geometry.sector_length = 512;

	if (params)
	{
		/* create */
		geometry.tracks = option_resolution_lookup_int(params, PARAM_TRACKS);
		geometry.sectors = option_resolution_lookup_int(params, PARAM_SECTORS);
	}
	else
	{
		/* open */
		if (!compis_get_tracks_and_sectors(floppy, &geometry.tracks, &geometry.sectors))
			return FLOPPY_ERROR_INVALIDIMAGE;
	}

	return basicdsk_construct(floppy, &geometry);
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( compis )
	LEGACY_FLOPPY_OPTION( compis_dsk, "dsk",        "Compis floppy disk image", compis_dsk_identify, compis_dsk_construct, NULL,
		TRACKS(40/[80])
		SECTORS(8/[9]/15))
LEGACY_FLOPPY_OPTIONS_END



/*********************************************************************

    formats/cpis_dsk.c

    cpis format

*********************************************************************/

#include "formats/cpis_dsk.h"

cpis_format::cpis_format() : upd765_format(formats)
{
}

const char *cpis_format::name() const
{
	return "cpis";
}

const char *cpis_format::description() const
{
	return "COMPIS disk image";
}

const char *cpis_format::extensions() const
{
	return "dsk,img";
}

// Unverified gap sizes
const cpis_format::format cpis_format::formats[] = {
	{   /*  320K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  8, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  360K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 40, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  640K 5 1/4 inch quad density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  8, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  720K 5 1/4 inch quad density - gaps unverified */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /* 1200K 5 1/4 inch quad density */
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		1200, 15, 80, 2, 512, {}, 1, {}, 80, 50, 22, 84
	},
	{}
};

const floppy_format_type FLOPPY_CPIS_FORMAT = &floppy_image_format_creator<cpis_format>;
