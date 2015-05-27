// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    BBC Micro

    Disk image format

***************************************************************************/

#pragma once

#ifndef __BBC_DSK_H__
#define __BBC_DSK_H__

#include "wd177x_dsk.h"

class bbc_format : public wd177x_format
{
public:
	bbc_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_BBC_FORMAT;

#endif // __BBC_DSK_H__
