// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/d40_dsk.h

    Didaktik D40/D80 disk images

*********************************************************************/
#ifndef MAME_FORMATS_D40_DSK_H
#define MAME_FORMATS_D40_DSK_H

#pragma once

#include "wd177x_dsk.h"

class d40_format : public wd177x_format {
public:
	d40_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_D40D80_FORMAT;

#endif // MAME_FORMATS_D40_DSK_H
