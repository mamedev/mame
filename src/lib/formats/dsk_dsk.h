// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dsk_dsk.h

    CPC DSK disk images

*********************************************************************/
#ifndef MAME_FORMATS_DSK_DSK_H
#define MAME_FORMATS_DSK_DSK_H

#pragma once

#include "flopimg.h"

class dsk_format : public floppy_image_format_t
{
public:
	dsk_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_DSK_FORMAT;

#endif // MAME_FORMATS_DSK_DSK_H
