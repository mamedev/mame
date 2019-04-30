// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/asst128_dsk.h

    asst128 format

*********************************************************************/
#ifndef MAME_FORMATS_ASST128_DSK_H
#define MAME_FORMATS_ASST128_DSK_H

#pragma once

#include "upd765_dsk.h"

class asst128_format : public upd765_format {
public:
	asst128_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ASST128_FORMAT;

#endif // MAME_FORMATS_ASST128_DSK_H
