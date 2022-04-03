// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Vector 06

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_VECTOR06_DSK_H
#define MAME_FORMATS_VECTOR06_DSK_H

#pragma once

#include "wd177x_dsk.h"

class vector06_format : public wd177x_format
{
public:
	vector06_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const vector06_format FLOPPY_VECTOR06_FORMAT;

#endif // MAME_FORMATS_VECTOR06_DSK_H
