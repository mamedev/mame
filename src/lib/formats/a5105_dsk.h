// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/a5105_dsk.h

    a5105 format

*********************************************************************/
#ifndef MAME_FORMATS_A5105_DSK_H
#define MAME_FORMATS_A5105_DSK_H

#pragma once

#include "upd765_dsk.h"

class a5105_format : public upd765_format
{
public:
	a5105_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const a5105_format FLOPPY_A5105_FORMAT;

#endif // MAME_FORMATS_A5105_DSK_H
