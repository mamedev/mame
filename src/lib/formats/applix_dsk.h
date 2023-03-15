// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    formats/applix_dsk.h

    Applix disk image format

*********************************************************************/
#ifndef MAME_FORMATS_APPLIX_DSK_H
#define MAME_FORMATS_APPLIX_DSK_H

#pragma once

#include "wd177x_dsk.h"

class applix_format : public wd177x_format
{
public:
	applix_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const applix_format FLOPPY_APPLIX_FORMAT;

#endif // MAME_FORMATS_APPLIX_DSK_H
