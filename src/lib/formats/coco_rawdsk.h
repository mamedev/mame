// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    CoCo Raw Disk

***************************************************************************/

#ifndef MAME_FORMATS_COCO_RAWDSK_H
#define MAME_FORMATS_COCO_RAWDSK_H

#pragma once

#include "wd177x_dsk.h"


class coco_rawdsk_format : public wd177x_format
{
public:
	coco_rawdsk_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const coco_rawdsk_format FLOPPY_COCO_RAWDSK_FORMAT;

#endif // MAME_FORMATS_COCO_RAWDSK_H
