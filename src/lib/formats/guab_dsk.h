// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    JPM Give us a Break

    Disk image format

***************************************************************************/

#pragma once

#ifndef __GUAB_DSK_H__
#define __GUAB_DSK_H__

#include "wd177x_dsk.h"

class guab_format : public wd177x_format
{
public:
	guab_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_GUAB_FORMAT;

#endif // __GUAB_DSK_H__
