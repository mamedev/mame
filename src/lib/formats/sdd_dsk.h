// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/sdd_dsk.h

    Speccy-DOS SDD disk images

*********************************************************************/
#ifndef MAME_FORMATS_SDD_DSK_H
#define MAME_FORMATS_SDD_DSK_H

#pragma once

#include "wd177x_dsk.h"

class sdd_format : public wd177x_format {
public:
	sdd_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
	virtual int get_image_offset(const format &f, int head, int track) const override;
};

extern const sdd_format FLOPPY_SDD_FORMAT;

#endif // MAME_FORMATS_SDD_DSK_H
