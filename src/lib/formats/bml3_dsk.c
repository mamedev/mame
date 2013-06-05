/*********************************************************************

    formats/bml3_dsk.c

    BML3 disk images

*********************************************************************/

#include <string.h>

#include "bml3_dsk.h"
#include "basicdsk.h"

static FLOPPY_IDENTIFY(bml3_dsk_identify)
{
	*vote = 100;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(bml3_dsk_construct)
{
	struct basicdsk_geometry geometry;

	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 2;
	geometry.first_sector_id = 1;
	geometry.sector_length = 256;
	geometry.tracks = 40;
	geometry.sectors = 16;
	return basicdsk_construct(floppy, &geometry);
}



/* ----------------------------------------------------------------------- */


LEGACY_FLOPPY_OPTIONS_START( bml3 )
	LEGACY_FLOPPY_OPTION( bml3_dsk, "bm3",		"BML3 floppy disk image",	bml3_dsk_identify, bml3_dsk_construct, NULL,
			HEADS([2])
			TRACKS([40])
			SECTORS([16])
			SECTOR_LENGTH([256])
			FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END
