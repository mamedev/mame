// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Sharp 2D

    Disk image format

***************************************************************************/

#pragma once

#ifndef __2D_DSK_H__
#define __2D_DSK_H__

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

extern const floppy_format_type FLOPPY_2D_FORMAT;

#endif // __2D_DSK_H__
