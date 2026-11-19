// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/clipper_dsk.h

    PDC Clipper 3" disk image format

*********************************************************************/
#ifndef MAME_FORMATS_CLIPPER_DSK_H
#define MAME_FORMATS_CLIPPER_DSK_H

#pragma once

#include "upd765_dsk.h"

class clipper_format : public upd765_format
{
public:
	clipper_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const clipper_format FLOPPY_CLIPPER_FORMAT;

#endif // MAME_FORMATS_CLIPPER_DSK_H
