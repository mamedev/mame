// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Michael R. Furman
/*
 * cp68_dsk.h
 *
 *  Created on: 02/08/2022
 */
#ifndef MAME_FORMATS_CP68_DSK_H
#define MAME_FORMATS_CP68_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

class cp68_format : public wd177x_format
{
public:
	cp68_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual const wd177x_format::format &get_track_format(const format &f, int head, int track) const override;
};

extern const cp68_format FLOPPY_CP68_FORMAT;

#endif // MAME_FORMATS_CP68_DSK_H
