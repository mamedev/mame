// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/tibdd001_dsk.h

    TIB Disc Drive DD-001 disk images

*********************************************************************/
#ifndef MAME_FORMATS_TIB_DD_001_DSK_H
#define MAME_FORMATS_TIB_DD_001_DSK_H

#pragma once

#include "flopimg.h"
#include "upd765_dsk.h"

class tib_dd_001_format : public upd765_format
{
public:
	tib_dd_001_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const tib_dd_001_format FLOPPY_TIB_DD_001_FORMAT;

#endif // MAME_FORMATS_TIB_DD_001_DSK_H
