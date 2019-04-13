// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/basicdsk.h

    Floppy format code for basic disks

*********************************************************************/
#ifndef MAME_FORMATS_BASICDSK_H
#define MAME_FORMATS_BASICDSK_H

#pragma once

#include "flopimg.h"

struct basicdsk_geometry
{
	int heads;
	int tracks;
	int sectors;
	int first_sector_id;
	int interleave;
	int sector_map[256];
	uint32_t sector_length;
	uint64_t offset;

	int (*translate_sector)(floppy_image_legacy *floppy, int sector);
	uint64_t (*translate_offset)(floppy_image_legacy *floppy, const struct basicdsk_geometry *geom, int track, int head, int sector);
	uint64_t (*get_ddam)(floppy_image_legacy *floppy, const struct basicdsk_geometry *geom, int track, int head, int sector);
};

floperr_t basicdsk_construct(floppy_image_legacy *floppy, const struct basicdsk_geometry *geometry);

FLOPPY_IDENTIFY(basicdsk_identify_default);
FLOPPY_CONSTRUCT(basicdsk_construct_default);

#endif // MAME_FORMATS_BASICDSK_H
