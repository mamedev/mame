// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/fdd_dsk.h

    PC98 FDD disk images

*********************************************************************/
#ifndef MAME_FORMATS_FDD_DSK_H
#define MAME_FORMATS_FDD_DSK_H

#pragma once

#include "flopimg.h"


class fdd_format : public floppy_image_format_t
{
public:
	fdd_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
};

extern const fdd_format FLOPPY_FDD_FORMAT;

#endif // MAME_FORMATS_FDD_DSK_H
