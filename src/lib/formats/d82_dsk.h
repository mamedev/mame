// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d82_dsk.h

    Commodore 8250/SFD-1001 sector disk image format

*********************************************************************/
#ifndef MAME_FORMATS_D82_DSK_H
#define MAME_FORMATS_D82_DSK_H

#pragma once

#include "d80_dsk.h"

class d82_format : public d80_format
{
public:
	d82_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	static const format file_formats[];
};

extern const d82_format FLOPPY_D82_FORMAT;

#endif // MAME_FORMATS_D82_DSK_H
