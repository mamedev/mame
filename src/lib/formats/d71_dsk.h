// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d71_dsk.h

    Commodore 1571 sector disk image format

*********************************************************************/
#ifndef MAME_FORMATS_D71_DSK_H
#define MAME_FORMATS_D71_DSK_H

#pragma once

#include "d64_dsk.h"

class d71_format : public d64_format
{
public:
	d71_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	static const format formats[];
};

extern const d71_format FLOPPY_D71_FORMAT;

#endif // MAME_FORMATS_D71_DSK_H
