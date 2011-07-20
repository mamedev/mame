/*********************************************************************

    formats/ami_dsk.c

    Amiga disk images

*********************************************************************/


#include <string.h>

#include "formats/ami_dsk.h"
#include "formats/basicdsk.h"



/*****************************************************************************
 Amiga floppy core functions
*****************************************************************************/


static FLOPPY_IDENTIFY( amiga_dsk_identify )
{
	UINT64 size;

	*vote = 100;

	/* first check the size of the image */
	size = floppy_image_size(floppy);
	if ((size != 901120) && (size != 1802240))
		*vote = 0;

	return FLOPPY_ERROR_SUCCESS;
}


static FLOPPY_CONSTRUCT( amiga_dsk_construct )
{
	struct basicdsk_geometry geometry;

	/* setup geometry with standard values */
	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 2;
	geometry.tracks = 80;
	geometry.first_sector_id = 0;
	geometry.sector_length = 512;

	if (params)
	{
		/* create */
		geometry.sectors = option_resolution_lookup_int(params, PARAM_SECTORS);
	}
	else
	{
		/* open */
		UINT64 size = floppy_image_size(floppy);
		geometry.sectors = size/512/80/2;
		if (geometry.sectors != 11 && geometry.sectors != 22)
			return FLOPPY_ERROR_INVALIDIMAGE;
	}

	return basicdsk_construct(floppy, &geometry);
}



/*****************************************************************************
 Amiga floppy options
*****************************************************************************/


LEGACY_FLOPPY_OPTIONS_START( amiga )
	LEGACY_FLOPPY_OPTION(
		ami_dsk,
		"adf",
		"Amiga floppy disk image",
		amiga_dsk_identify,
		amiga_dsk_construct,
		NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([11]/22)
	)
LEGACY_FLOPPY_OPTIONS_END

LEGACY_FLOPPY_OPTIONS_START( amiga_only )
	LEGACY_FLOPPY_OPTION(
		ami_dsk,
		"adf",
		"Amiga floppy disk image",
		amiga_dsk_identify,
		amiga_dsk_construct,
		NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([11]/22)
	)
LEGACY_FLOPPY_OPTIONS_END0
