// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Vector 06

    Disk image format

***************************************************************************/

#pragma once

#ifndef __VECTOR06_DSK_H__
#define __VECTOR06_DSK_H__

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

extern const floppy_format_type FLOPPY_VECTOR06_FORMAT;

#endif // __VECTOR06_DSK_H__
