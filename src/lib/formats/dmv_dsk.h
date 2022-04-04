// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dmv_dsk.h

    NCR Decision Mate V format

*********************************************************************/
#ifndef MAME_FORMATS_DMV_DSK_H
#define MAME_FORMATS_DMV_DSK_H

#pragma once

#include "upd765_dsk.h"

class dmv_format : public upd765_format
{
public:
	dmv_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const dmv_format FLOPPY_DMV_FORMAT;

#endif // MAME_FORMATS_DMV_DSK_H
