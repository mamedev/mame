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

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_FDD_FORMAT;

#endif // MAME_FORMATS_FDD_DSK_H
