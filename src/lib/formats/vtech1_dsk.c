/*********************************************************************

    formats/vtech1_dsk.c

    VTech1 disk images

*********************************************************************/

#include "formats/vtech1_dsk.h"

static FLOPPY_IDENTIFY( vtech1_dsk_identify )
{
	*vote = 100;
	return FLOPPY_ERROR_SUCCESS;
}


static FLOPPY_CONSTRUCT( vtech1_dsk_construct )
{
	return FLOPPY_ERROR_SUCCESS;
}

LEGACY_FLOPPY_OPTIONS_START( vtech1_only )
	LEGACY_FLOPPY_OPTION(
		vtech1_dsk,
		"dsk",
		"Laser floppy disk image",
		vtech1_dsk_identify,
		vtech1_dsk_construct,
		NULL,
		NULL
	)
LEGACY_FLOPPY_OPTIONS_END0
