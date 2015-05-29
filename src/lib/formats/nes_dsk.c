// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/nes_dsk.c

    NES disk images

*********************************************************************/


#include <string.h>
#include <assert.h>

#include "formats/nes_dsk.h"
#include "formats/basicdsk.h"



/*****************************************************************************
 NES floppy core functions
*****************************************************************************/


static FLOPPY_IDENTIFY( nes_dsk_identify )
{
	UINT64 size;
	UINT8 header[3];

	*vote = 0;

	/* get first 3 bytes */
	floppy_image_read(floppy, &header, 0, sizeof(header));

	/* first check the size of the image */
	size = floppy_image_size(floppy);

	if ((size == 65516) || (size == 131016) || (size == 262016))
	{
		/* the image has an header, hence check the first sector for the magic string */
		if (!memcmp(header, "FDS", 3))
			*vote = 100;
	}

	if ((size == 65500) || (size == 131000) || (size == 262000))
	{
		/* the image has no header, hence let's trust the extension and load the file */
		*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}


static FLOPPY_CONSTRUCT( nes_dsk_construct )
{
	return FLOPPY_ERROR_SUCCESS;
}


/*****************************************************************************
 NES floppy options
*****************************************************************************/

LEGACY_FLOPPY_OPTIONS_START( nes_only )
	LEGACY_FLOPPY_OPTION(
		fds_dsk,
		"fds",
		"NES floppy disk image",
		nes_dsk_identify,
		nes_dsk_construct,
		NULL,
		NULL
	)
LEGACY_FLOPPY_OPTIONS_END0
