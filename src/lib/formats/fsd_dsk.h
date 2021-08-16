// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro

    Disk image formats

***************************************************************************/
#ifndef MAME_FORMATS_FSD_DSK_H
#define MAME_FORMATS_FSD_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(fsd);

/**************************************************************************/



class fsd_format : public floppy_image_format_t
{
public:
	fsd_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
};

extern const floppy_format_type FLOPPY_FSD_FORMAT;

#endif // MAME_FORMATS_FSD_DSK_H
