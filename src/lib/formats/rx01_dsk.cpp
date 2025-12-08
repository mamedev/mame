// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/rx01_dsk.cpp

    DEC RX01 disk images

*********************************************************************/

#include "formats/rx01_dsk.h"

#include <cassert>
#include <cstring>


static FLOPPY_IDENTIFY(rx01_identify)
{
	*vote = (floppy_image_size(floppy) == (77 * 26 * 128)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(rx01_construct)
{
	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 1;
	geometry.first_sector_id = 1;
	geometry.sector_length = 128;
	geometry.tracks = 77;
	geometry.sectors = 26;
	return basicdsk_construct(floppy, &geometry);
}

LEGACY_FLOPPY_OPTIONS_START( rx01 )
	LEGACY_FLOPPY_OPTION( rx01_dsk, "img", "RX01 floppy disk image", rx01_identify, rx01_construct, nullptr, nullptr)
LEGACY_FLOPPY_OPTIONS_END
