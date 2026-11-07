// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    msx_dsk.h

    MSX disk images

*********************************************************************/
#ifndef MAME_FORMATS_MSX_DSK_H
#define MAME_FORMATS_MSX_DSK_H

#pragma once

#include "upd765_dsk.h"

class msx_format: public upd765_format
{
public:
	msx_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;

private:
	static const format formats[];
};

extern const msx_format FLOPPY_MSX_FORMAT;

#endif // MAME_FORMATS_MSX_DSK_H
