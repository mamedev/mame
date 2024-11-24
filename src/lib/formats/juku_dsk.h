// license: BSD-3-Clause
// copyright-holders: Dirk Best, Märt Põder
/***************************************************************************

    Juku E5101/E5104

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_JUKU_DSK_H
#define MAME_FORMATS_JUKU_DSK_H

#pragma once

#include "wd177x_dsk.h"

class juku_format : public wd177x_format
{
public:
	juku_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const juku_format FLOPPY_JUKU_FORMAT;

#endif // MAME_FORMATS_JUKU_DSK_H
