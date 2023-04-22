// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/*********************************************************************

    formats/hp300_dsk.h

    HP9000/300 disk format

*********************************************************************/
#ifndef MAME_FORMATS_HP300_DSK_H
#define MAME_FORMATS_HP300_DSK_H

#pragma once

#include "wd177x_dsk.h"

class hp300_format : public wd177x_format
{
public:
	hp300_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const hp300_format FLOPPY_HP300_FORMAT;

#endif // MAME_FORMATS_HP300_DSK_H
