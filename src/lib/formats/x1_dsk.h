// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Sharp X1

    Disk image format

***************************************************************************/

#pragma once

#ifndef __X1_DSK_H__
#define __X1_DSK_H__

#include "wd177x_dsk.h"

class x1_format : public wd177x_format
{
public:
	x1_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_X1_FORMAT;

#endif // __X1_DSK_H__
