// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/vtech1_dsk.c

    VTech1 disk images

*********************************************************************/

#include <assert.h>

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
		HEADS([1])
		TRACKS([40])
		SECTORS([16])
		SECTOR_LENGTH([154])
		FIRST_SECTOR_ID([0])
	)
LEGACY_FLOPPY_OPTIONS_END0
