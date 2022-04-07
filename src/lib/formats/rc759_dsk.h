// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline

    Disk image format

***************************************************************************/

#ifndef MAME_FORMATS_RC759_DSK_H
#define MAME_FORMATS_RC759_DSK_H

#pragma once

#include "wd177x_dsk.h"

class rc759_format : public wd177x_format
{
public:
	rc759_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const rc759_format FLOPPY_RC759_FORMAT;

#endif // MAME_FORMATS_RC759_DSK_H
