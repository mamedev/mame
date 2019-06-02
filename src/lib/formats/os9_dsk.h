// license:BSD-3-Clause
// copyright-holders:tim lindner
/*********************************************************************

    formats/os9_dsk.h

    OS-9 disk images

*********************************************************************/
#ifndef MAME_FORMATS_OS9_DSK_H
#define MAME_FORMATS_OS9_DSK_H

#pragma once

#include "wd177x_dsk.h"

class os9_format : public wd177x_format {
public:
	os9_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual int find_size(io_generic *io, uint32_t form_factor) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_OS9_FORMAT;

#endif // MAME_FORMATS_OS9_DSK_H
