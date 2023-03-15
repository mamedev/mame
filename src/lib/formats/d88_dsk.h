// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/d88_dsk.h

    D88 disk images

*********************************************************************/
#ifndef MAME_FORMATS_D88_DSK_H
#define MAME_FORMATS_D88_DSK_H

#pragma once

#include "flopimg.h"


class d88_format : public floppy_image_format_t
{
public:
	d88_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const d88_format FLOPPY_D88_FORMAT;

#endif // MAME_FORMATS_D88_DSK_H
