// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/nfd_dsk.h

    PC98 NFD disk images

*********************************************************************/
#ifndef MAME_FORMATS_NFD_DSK_H
#define MAME_FORMATS_NFD_DSK_H

#pragma once

#include "flopimg.h"


class nfd_format : public floppy_image_format_t
{
public:
	nfd_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_NFD_FORMAT;

#endif // MAME_FORMATS_NFD_DSK_H
