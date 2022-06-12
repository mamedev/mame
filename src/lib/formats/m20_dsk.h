// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/m20_dsk.c

    Olivetti M20 floppy-disk images

*********************************************************************/
#ifndef MAME_FORMATS_M20_DSK_H
#define MAME_FORMATS_M20_DSK_H

#pragma once

#include "flopimg.h"

class m20_format : public floppy_image_format_t {
public:
	m20_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const m20_format FLOPPY_M20_FORMAT;

#endif // MAME_FORMATS_M20_DSK_H
