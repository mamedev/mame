// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_FORMATS_ADAM_DSK_H
#define MAME_FORMATS_ADAM_DSK_H

#pragma once

#include "flopimg.h"

class adam_format : public floppy_image_format_t
{
public:
	adam_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;
};

extern const adam_format FLOPPY_ADAM_FORMAT;

#endif // MAME_FORMATS_ADAM_DSK_H
