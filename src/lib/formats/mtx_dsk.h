// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Memotech MTX

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_MTX_DSK_H
#define MAME_FORMATS_MTX_DSK_H

#pragma once

#include "wd177x_dsk.h"

class mtx_format : public wd177x_format
{
public:
	mtx_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const mtx_format FLOPPY_MTX_FORMAT;

#endif // MAME_FORMATS_MTX_DSK_H
