// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/tim011_dsk.h

    TIM 011 format

*********************************************************************/
#ifndef MAME_FORMATS_TIM011_DSK_H
#define MAME_FORMATS_TIM011_DSK_H

#pragma once

#include "upd765_dsk.h"

class tim011_format : public upd765_format
{
public:
	tim011_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const tim011_format FLOPPY_TIM011_FORMAT;

#endif // MAME_FORMATS_TIM011_DSK_H
