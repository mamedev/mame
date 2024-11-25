// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_FORMATS_IPF_DSK_H
#define MAME_FORMATS_IPF_DSK_H

#pragma once

#include "flopimg.h"

#include <vector>

class ipf_format : public floppy_image_format_t
{
public:
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

private:
	struct ipf_decode;
};

extern const ipf_format FLOPPY_IPF_FORMAT;

#endif // MAME_FORMATS_IPF_DSK_H
