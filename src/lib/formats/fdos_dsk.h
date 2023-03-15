// license:BSD-3-Clause
// copyright-holders:Michael R. Furman
/*
 * fdos_dsk.h
 *
 *  Created on: 24/08/2022
 */
#ifndef MAME_FORMATS_FDOS_DSK_H
#define MAME_FORMATS_FDOS_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

class fdos_format : public wd177x_format
{
public:
	fdos_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
};

extern const fdos_format FLOPPY_FDOS_FORMAT;

#endif // MAME_FORMATS_FDOS_DSK_H
