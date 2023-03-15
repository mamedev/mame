// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/m5_dsk.h

    sord m5 format

*********************************************************************/
#ifndef MAME_FORMATS_M5_DSK_H
#define MAME_FORMATS_M5_DSK_H

#pragma once

#include "upd765_dsk.h"

class m5_format : public upd765_format {
public:
	m5_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const m5_format FLOPPY_M5_FORMAT;

#endif // MAME_FORMATS_M5_DSK_H
