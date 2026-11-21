// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/nes_dsk.c

    NES disk images

*********************************************************************/

#include "formats/nes_dsk.h"
#include "formats/basicdsk.h"

#include <cstring>



/*****************************************************************************
 NES floppy core functions
*****************************************************************************/


/*
 The fwNES .fds disk image (NESdev "FDS file format"): an optional 16 byte header ("FDS" $1A, the number of sides, zero
 padding) and 65500 bytes of block data per side, without the gaps and CRCs of the real disk. The size of the image gives
 the number of sides, up to what the drive takes (nes_disksys_device::MAX_SIDES); the header's own count is not used.
 */
static constexpr uint64_t FDS_SIDE_SIZE = 65500;
static constexpr uint64_t FDS_HEADER_SIZE = 16;
static constexpr uint64_t FDS_MAX_SIDES = 16;

/*
 The Quick Disk .qd image (NESdev "FDS file format" and "FDS disk format"): the same blocks, but every block is followed by
 its 2 byte CRC, there is no header and a side takes up to 65536 bytes. The drive works on the .fds layout, so
 nes_disksys_device::load_disk drops the CRCs when it loads one.
 */
static constexpr uint64_t QD_SIDE_SIZE = 65536;


static FLOPPY_IDENTIFY( nes_dsk_identify )
{
	uint8_t header[4];

	*vote = 0;

	/* get first 4 bytes */
	floppy_image_read(floppy, &header, 0, sizeof(header));

	/* first check the size of the image */
	uint64_t const size = floppy_image_size(floppy);

	if (size && !(size % FDS_SIDE_SIZE) && ((size / FDS_SIDE_SIZE) <= FDS_MAX_SIDES))
	{
		/* the image has no header, hence let's trust the extension and load the file */
		*vote = 100;
	}
	else if ((size > FDS_HEADER_SIZE) && !((size - FDS_HEADER_SIZE) % FDS_SIDE_SIZE) && (((size - FDS_HEADER_SIZE) / FDS_SIDE_SIZE) <= FDS_MAX_SIDES))
	{
		/* the image has an header, hence check the first sector for the magic string */
		if (!memcmp(header, "FDS\x1a", 4))
			*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}


static FLOPPY_IDENTIFY( nes_qd_identify )
{
	*vote = 0;

	uint64_t const size = floppy_image_size(floppy);

	if (size && !(size % QD_SIDE_SIZE) && ((size / QD_SIDE_SIZE) <= FDS_MAX_SIDES))
		*vote = 100;

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
		nullptr,
		nullptr
	)
	LEGACY_FLOPPY_OPTION(
		qd_dsk,
		"qd",
		"NES Quick Disk image",
		nes_qd_identify,
		nes_dsk_construct,
		nullptr,
		nullptr
	)
LEGACY_FLOPPY_OPTIONS_END0
