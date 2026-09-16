// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/bk0010_dsk.h

*********************************************************************/
#ifndef MAME_FORMATS_BK0010_DSK_H
#define MAME_FORMATS_BK0010_DSK_H

#pragma once

#include "wd177x_dsk.h"

class bk0010_format : public wd177x_format
{
public:
	bk0010_format();

	virtual const char *name() const noexcept override { return "bk0010"; }
	virtual const char *description() const noexcept override { return "BK-0010 disk image"; }
	virtual const char *extensions() const noexcept override { return "img,bkd"; }

private:
	static const format formats[];
};

extern const bk0010_format FLOPPY_BK0010_FORMAT;

#endif // MAME_FORMATS_BK0010_DSK_H
