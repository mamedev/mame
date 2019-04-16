// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/nanos_dsk.h

    nanos format

*********************************************************************/
#ifndef MAME_FORMATS_NANOS_DSK_H
#define MAME_FORMATS_NANOS_DSK_H

#pragma once

#include "upd765_dsk.h"

class nanos_format : public upd765_format {
public:
	nanos_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_NANOS_FORMAT;

#endif // MAME_FORMATS_NANOS_DSK_H
