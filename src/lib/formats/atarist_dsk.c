// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/atarist_dsk.c

    Atari ST disk images

*********************************************************************/

#include <assert.h>

#include "formats/atarist_dsk.h"
#include "formats/basicdsk.h"

/*

    TODO:

    - MSA format
    - STT format
    - DIM format

*/

/***************************************************************************
    CONSTANTS / MACROS
***************************************************************************/

#define LOG 0

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    FLOPPY_IDENTIFY( atarist_st_identify )
-------------------------------------------------*/

static FLOPPY_IDENTIFY( atarist_st_identify )
{
	*vote = 100;

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_CONSTRUCT( atarist_st_construct )
-------------------------------------------------*/

static FLOPPY_CONSTRUCT( atarist_st_construct )
{
	int heads = 0;
	int tracks = 0;
	int sectors = 0;
	UINT8 bootsector[512];

	floppy_image_read(floppy, bootsector, 0, 512);
	sectors = bootsector[0x18];
	heads = bootsector[0x1a];
	tracks = (bootsector[0x13] | (bootsector[0x14] << 8)) / sectors / heads;

	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));

	geometry.heads = heads;
	geometry.first_sector_id = 1;
	geometry.sector_length = 512;
	geometry.tracks = tracks;
	geometry.sectors = sectors;

	if (LOG) LOG_FORMATS("ST Heads %d Tracks %d Sectors %d\n", heads, tracks, sectors);

	return basicdsk_construct(floppy, &geometry);
}

/*-------------------------------------------------
    FLOPPY_CONSTRUCT(atarist_dsk_construct)
-------------------------------------------------*/

LEGACY_FLOPPY_OPTIONS_START( atarist )
	LEGACY_FLOPPY_OPTION( atarist, "st", "Atari ST floppy disk image", atarist_st_identify, atarist_st_construct, NULL, NULL )
/*  LEGACY_FLOPPY_OPTION( atarist, "stt", "Atari ST floppy disk image", atarist_stt_identify, atarist_stt_construct, NULL, NULL )
    LEGACY_FLOPPY_OPTION( atarist, "msa", "Atari ST floppy disk image", atarist_msa_identify, atarist_msa_construct, NULL, NULL )
    LEGACY_FLOPPY_OPTION( atarist, "dim", "Atari ST floppy disk image", atarist_dim_identify, atarist_dim_construct, NULL, NULL )*/
LEGACY_FLOPPY_OPTIONS_END
