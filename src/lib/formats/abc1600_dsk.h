// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc1600_dsk.h

    Luxor ABC 1600 disk image formats

*********************************************************************/
#ifndef MAME_FORMATS_ABC1600_DSK_H
#define MAME_FORMATS_ABC1600_DSK_H

#pragma once

#include "wd177x_dsk.h"

class abc1600_format : public wd177x_format
{
public:
	abc1600_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	virtual int get_image_offset(const format &f, int head, int track) override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ABC1600_FORMAT;

#endif // MAME_FORMATS_ABC1600_DSK_H
