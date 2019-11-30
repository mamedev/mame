// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/kc85_dsk.h

    kc85 format

*********************************************************************/
#ifndef MAME_FORMATS_KC85_DSK_H
#define MAME_FORMATS_KC85_DSK_H

#pragma once

#include "upd765_dsk.h"

class kc85_format : public upd765_format {
public:
	kc85_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_KC85_FORMAT;

#endif // MAME_FORMATS_KC85_DSK_H
