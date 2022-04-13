// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    formats/agat840k_hle_dsk.cpp

    Agat 840KB floppies -- high level simulation (sector-level images)

    http://agatcomp.ru/Reading/docs/es5323.txt
    https://github.com/sintech/AGAT/blob/master/docs/agat-840k-format.txt
    http://www.torlus.com/floppy/forum/viewtopic.php?f=19&t=1385

************************************************************************/

#include "agat840k_hle_dsk.h"
#include "imageutl.h"

#include <cstring>


static FLOPPY_IDENTIFY(agat840k_hle_dsk_identify)
{
	switch (floppy_image_size(floppy))
	{
	case 860160:
		*vote = 100;
		break;

	case 860164:
	case 860288:
		*vote = 99;
		break;

	default:
		*vote = 0;
		break;
	}

	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(agat840k_hle_dsk_construct)
{
	struct basicdsk_geometry geometry;

	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 2;
	geometry.first_sector_id = 0;
	geometry.sector_length = 256;
	geometry.tracks = 80;
	geometry.sectors = 21;

	return basicdsk_construct(floppy, &geometry);
}

LEGACY_FLOPPY_OPTIONS_START(agat840k_hle)
	LEGACY_FLOPPY_OPTION(agat840k_hle_dsk, "ds9,dsk,raw", "Agat 840K DSK image",
		agat840k_hle_dsk_identify, agat840k_hle_dsk_construct, nullptr, nullptr)
LEGACY_FLOPPY_OPTIONS_END

