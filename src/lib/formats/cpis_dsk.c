/*********************************************************************

    formats/cpis_dsk.c

    Telenova Compis disk images

*********************************************************************/

#include <string.h>

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
