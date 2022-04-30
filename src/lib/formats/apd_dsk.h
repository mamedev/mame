// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    formats/apd_dsk.h

    Archimedes Protected Disk Image format

*********************************************************************/
#ifndef MAME_FORMATS_APD_DSK_H
#define MAME_FORMATS_APD_DSK_H

#pragma once

#include "flopimg.h"

class apd_format : public floppy_image_format_t
{
public:
	apd_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool supports_save() const override;
};

extern const apd_format FLOPPY_APD_FORMAT;

#endif // MAME_FORMATS_APD_DSK_H
