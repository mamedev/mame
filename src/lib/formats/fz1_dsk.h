// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_FORMATS_FZ1_DSK_H
#define MAME_FORMATS_FZ1_DSK_H

#pragma once

#include "flopimg.h"
#include "upd765_dsk.h"

/**************************************************************************/

class fz1_format : public upd765_format
{
public:
	fz1_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const fz1_format FLOPPY_FZ1_FORMAT;

#endif // MAME_FORMATS_FZ1_DSK_H
