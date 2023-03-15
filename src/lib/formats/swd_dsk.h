// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    formats/swd_dsk.h

    Swift Disc disk images

*********************************************************************/
#ifndef MAME_FORMATS_SWD_DSK_H
#define MAME_FORMATS_SWD_DSK_H

#pragma once

#include "wd177x_dsk.h"

class swd_format : public wd177x_format {
public:
	swd_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
	virtual int get_image_offset(const format &f, int head, int track) const override;
};

extern const swd_format FLOPPY_SWD_FORMAT;

#endif // MAME_FORMATS_SWD_DSK_H
