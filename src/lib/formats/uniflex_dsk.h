// license:BSD-3-Clause
// copyright-holders:68bit
/*
 * uniflex_dsk.h
 */
#ifndef MAME_FORMATS_UNIFLEX_DSK_H
#define MAME_FORMATS_UNIFLEX_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

class uniflex_format : public wd177x_format
{
public:
	uniflex_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual int find_size(io_generic *io, uint32_t form_factor) override;
	void build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_UNIFLEX_FORMAT;

#endif // MAME_FORMATS_UNIFLEX_DSK_H
