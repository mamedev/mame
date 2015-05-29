// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*********************************************************************

    formats/vt_dsk.c

    VTech Laser/VZ disk images

*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "formats/vt_dsk.h"
#include "formats/basicdsk.h"

static FLOPPY_IDENTIFY(vz_identify)
{
	UINT64 size = floppy_image_size(floppy);
	*vote = ((size == 98560) || (size == 99200) || (size == 99184)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(vz_construct)
{
	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));

	if (params)
	{
		geometry.heads           = option_resolution_lookup_int(params, PARAM_HEADS);
		geometry.tracks          = option_resolution_lookup_int(params, PARAM_TRACKS);
		geometry.sectors         = option_resolution_lookup_int(params, PARAM_SECTORS);
		geometry.first_sector_id = option_resolution_lookup_int(params, PARAM_FIRST_SECTOR_ID);
		geometry.sector_length   = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);
	}
	else
	{
		geometry.heads           = 1;
		geometry.tracks          = 40;
		geometry.sectors         = 16;
		geometry.first_sector_id = 0;
		geometry.sector_length   = floppy_image_size(floppy)/geometry.tracks/geometry.sectors;
	}

	return basicdsk_construct(floppy, &geometry);
}


LEGACY_FLOPPY_OPTIONS_START(vz)
	LEGACY_FLOPPY_OPTION(vtech1, "dsk", "Laser/VZ disk image", vz_identify, vz_construct, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([16])
		SECTOR_LENGTH([154])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END
