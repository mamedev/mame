// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_FORMATS_S900_DSK_H
#define MAME_FORMATS_S900_DSK_H

#pragma once

#include "flopimg.h"
#include "upd765_dsk.h"

/**************************************************************************/

class s900_format : public upd765_format
{
public:
	s900_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const s900_format FLOPPY_S900_FORMAT;

#endif // MAME_FORMATS_S900_DSK_H
