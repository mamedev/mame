// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/*********************************************************************

Micropolis VGI disk image format

The Micropolis disk format was used in Vector Graphic machines.

*********************************************************************/
#ifndef MAME_FORMATS_VGI_DSK_H
#define MAME_FORMATS_VGI_DSK_H

#pragma once

#include "flopimg.h"

class micropolis_vgi_format : public floppy_image_format_t
{
public:
	micropolis_vgi_format();

	int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	const char *name() const noexcept override { return "vgi"; }
	const char *description() const noexcept override { return "Micropolis VGI disk image"; }
	const char *extensions() const noexcept override { return "vgi"; }
	bool supports_save() const noexcept override { return true; }
};

extern const micropolis_vgi_format FLOPPY_VGI_FORMAT;

#endif // MAME_FORMATS_VGI_DSK_H
