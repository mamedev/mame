// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*********************************************************************

    formats/pc98_dsk.h

    PC disk images

*********************************************************************/
#ifndef MAME_FORMATS_PC98_DSK_H
#define MAME_FORMATS_PC98_DSK_H

#pragma once

#include "flopimg.h"
#include "upd765_dsk.h"

/**************************************************************************/


class pc98_format : public upd765_format
{
public:
	pc98_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const pc98_format FLOPPY_PC98_FORMAT;

#endif // MAME_FORMATS_PC98_DSK_H
