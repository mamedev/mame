// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Poly Disk image formats

***************************************************************************/
#ifndef MAME_FORMATS_POLY_DSK_H
#define MAME_FORMATS_POLY_DSK_H

#pragma once

#include "flopimg.h"

class poly_cpm_format : public floppy_image_format_t
{
public:
	poly_cpm_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool supports_save() const override;
};

extern const poly_cpm_format FLOPPY_POLY_CPM_FORMAT;

#endif // MAME_FORMATS_POLY_DSK_H
