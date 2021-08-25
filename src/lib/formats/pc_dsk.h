// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/pc_dsk.h

    PC disk images

*********************************************************************/
#ifndef MAME_FORMATS_PC_DSK_H
#define MAME_FORMATS_PC_DSK_H

#pragma once

#include "flopimg.h"
#include "upd765_dsk.h"

class pc_format : public upd765_format
{
public:
	pc_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_PC_FORMAT;

#endif // MAME_FORMATS_PC_DSK_H
