// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss
/***************************************************************************

	Brother LW-30 Disk image format
	see https://github.com/BartmanAbyss/brother-diskconv for disk conversion tool

***************************************************************************/
#ifndef MAME_FORMATS_LW30_DSK_H
#define MAME_FORMATS_LW30_DSK_H

#pragma once

#include "flopimg.h"

class lw30_format : public floppy_image_format_t {
public:
	int identify(util::random_read& io, uint32_t form_factor, const std::vector<uint32_t>& variants) const override;
	bool load(util::random_read& io, uint32_t form_factor, const std::vector<uint32_t>& variants, floppy_image* image) const override;
	const char* name() const override;
	const char* description() const override;
	const char* extensions() const override;
	bool supports_save() const override;
};

extern const lw30_format FLOPPY_LW30_FORMAT;

#endif // MAME_FORMATS_LW30_DSK_H
