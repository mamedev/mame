// license:BSD-3-Clause
// copyright-holders:tim lindner, 68bit
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

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual const wd177x_format::format &get_track_format(const format &f, int head, int track) override;

private:
	static const format formats[];
	static const format formats_track0[];
};

extern const floppy_format_type FLOPPY_OS9_FORMAT;

#endif // MAME_FORMATS_OS9_DSK_H
