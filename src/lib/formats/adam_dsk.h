// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/adam_dsk.h

    Coleco Adam disk image format

*********************************************************************/
#ifndef MAME_FORMATS_ADAM_DSK_H
#define MAME_FORMATS_ADAM_DSK_H

#pragma once

#include "wd177x_dsk.h"

class adam_format : public wd177x_format
{
public:
	adam_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int get_image_offset(const format &f, int head, int track) const override;

private:
	static const format formats[];
};

extern const adam_format FLOPPY_ADAM_FORMAT;

#endif // MAME_FORMATS_ADAM_DSK_H
