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
	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_POLY_CPM_FORMAT;

#endif // MAME_FORMATS_POLY_DSK_H
