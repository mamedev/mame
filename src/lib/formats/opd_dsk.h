// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Sinclair ZX Spectrum

    Opus Discovery disk image formats

***************************************************************************/
#ifndef MAME_FORMATS_OPD_DSK_H
#define MAME_FORMATS_OPD_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"


class opd_format : public wd177x_format
{
public:
	opd_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const opd_format FLOPPY_OPD_FORMAT;

#endif // MAME_FORMATS_OPD_DSK_H
