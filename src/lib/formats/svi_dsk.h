// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Spectravideo SVI-318/328

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_SVI_DSK_H
#define MAME_FORMATS_SVI_DSK_H

#pragma once

#include "flopimg.h"

class svi_format : public floppy_image_format_t
{
public:
	svi_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_SVI_FORMAT;

#endif // MAME_FORMATS_SVI_DSK_H
