// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Sharp 2D

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_2D_DSK_H
#define MAME_FORMATS_2D_DSK_H

#pragma once

#include "wd177x_dsk.h"

class _2d_format : public wd177x_format
{
public:
	_2d_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const _2d_format FLOPPY_2D_FORMAT;

#endif // MAME_FORMATS_2D_DSK_H
