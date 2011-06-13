/*********************************************************************

    formats/atari_dsk.c

    Atari disk images

*********************************************************************/

#include "formats/atari_dsk.h"

static FLOPPY_IDENTIFY( atari_dsk_identify )
{
	*vote = 100;
	return FLOPPY_ERROR_SUCCESS;
}


static FLOPPY_CONSTRUCT( atari_dsk_construct )
{
	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_OPTIONS_START( atari_only )
	FLOPPY_OPTION(
		atari_dsk,
		"atr,dsk,xfd",
		"Atari floppy disk image",
		atari_dsk_identify,
		atari_dsk_construct,
		NULL,
		NULL
	)
FLOPPY_OPTIONS_END0
