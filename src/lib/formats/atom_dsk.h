// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_FORMATS_ATOM_DSK_H
#define MAME_FORMATS_ATOM_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

class atom_format : public wd177x_format
{
public:
	atom_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ATOM_FORMAT;

#endif // MAME_FORMATS_ATOM_DSK_H
