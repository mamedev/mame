// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Wren Executive

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_WREN_DSK_H
#define MAME_FORMATS_WREN_DSK_H

#pragma once

#include "wd177x_dsk.h"


class wren_format : public wd177x_format
{
public:
	wren_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


extern const wren_format FLOPPY_WREN_FORMAT;

#endif // MAME_FORMATS_WREN_DSK_H
