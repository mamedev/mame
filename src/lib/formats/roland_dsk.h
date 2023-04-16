// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_FORMATS_ROLAND_DSK_H
#define MAME_FORMATS_ROLAND_DSK_H

#pragma once

#include "wd177x_dsk.h"

class roland_sdisk_format : public wd177x_format
{
public:
	roland_sdisk_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const roland_sdisk_format FLOPPY_ROLAND_SDISK_FORMAT;

#endif // MAME_FORMATS_ROLAND_DSK_H
