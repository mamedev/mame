// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/dip_dsk.h

    PC98DIP disk images

*********************************************************************/
#ifndef MAME_FORMATS_DIP_DSK_H
#define MAME_FORMATS_DIP_DSK_H

#pragma once

#include "flopimg.h"


class dip_format : public floppy_image_format_t
{
public:
	dip_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const dip_format FLOPPY_DIP_FORMAT;

#endif // MAME_FORMATS_DIP_DSK_H
