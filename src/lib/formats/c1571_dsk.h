// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/c1571_dsk.h

    Commodore 1571 native MFM disk image formats (read via the WD1770)

*********************************************************************/
#ifndef MAME_FORMATS_C1571_DSK_H
#define MAME_FORMATS_C1571_DSK_H

#pragma once

#include "wd177x_dsk.h"

class c1571_format : public wd177x_format
{
public:
	c1571_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const c1571_format FLOPPY_C1571_FORMAT;

#endif // MAME_FORMATS_C1571_DSK_H
