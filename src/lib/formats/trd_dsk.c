/*********************************************************************

    formats/trd_dsk.c

    TRD disk images

*********************************************************************/

#include <string.h>

#include "trd_dsk.h"
#include "basicdsk.h"


static FLOPPY_IDENTIFY(trd_dsk_identify)
{
	*vote = 100;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(trd_dsk_construct)
{
	struct basicdsk_geometry geometry;
	UINT8 data[1];
	int heads;
	int cylinders;

	floppy_image_read( floppy, data, 0x8e3 , 1 );

	/* guess geometry of disk */
	heads =  data[0] & 0x08 ? 1 : 2;
	cylinders = data[0] & 0x01 ? 40 : 80;

	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = heads;
	geometry.first_sector_id = 1;
	geometry.sector_length = 256;
	geometry.tracks = cylinders;
	geometry.sectors = 16;
	return basicdsk_construct(floppy, &geometry);
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( trd )
	LEGACY_FLOPPY_OPTION( trd_dsk, "trd",       "TRD floppy disk image",    trd_dsk_identify, trd_dsk_construct, NULL, NULL)
LEGACY_FLOPPY_OPTIONS_END
