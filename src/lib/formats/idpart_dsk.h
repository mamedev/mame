// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/idpart_dsk.h

    Iskra Delta Partner format

*********************************************************************/
#ifndef MAME_FORMATS_IDPART_DSK_H
#define MAME_FORMATS_IDPART_DSK_H

#pragma once

#include "upd765_dsk.h"

class idpart_format : public upd765_format
{
public:
	idpart_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const idpart_format FLOPPY_IDPART_FORMAT;

#endif // MAME_FORMATS_IDPART_DSK_H
