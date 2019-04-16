// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    formats/excali64_dsk.h

    Excalibur 64 disk image format

*********************************************************************/
#ifndef MAME_FORMATS_EXCALI64_DSK_H
#define MAME_FORMATS_EXCALI64_DSK_H

#pragma once

#include "wd177x_dsk.h"

class excali64_format : public wd177x_format
{
public:
	excali64_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_EXCALI64_FORMAT;

#endif // MAME_FORMATS_EXCALI64_DSK_H
