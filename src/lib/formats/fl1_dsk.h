// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/fl1_dsk.h

    FloppyOne DOS disk images

*********************************************************************/
#ifndef MAME_FORMATS_FL1_DSK_H
#define MAME_FORMATS_FL1_DSK_H

#pragma once

#include "wd177x_dsk.h"

class fl1_format : public wd177x_format {
public:
	fl1_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
	virtual int get_image_offset(const format &f, int head, int track) const override;
};

extern const fl1_format FLOPPY_FL1_FORMAT;

#endif // MAME_FORMATS_FL1_DSK_H
