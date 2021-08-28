// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    formats/jfd_dsk.h

    JASPP Floppy Disk image format

*********************************************************************/
#ifndef MAME_FORMATS_JFD_DSK_H
#define MAME_FORMATS_JFD_DSK_H

#pragma once

#include "flopimg.h"

class jfd_format : public floppy_image_format_t
{
public:
	jfd_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_JFD_FORMAT;

#endif // MAME_FORMATS_JFD_DSK_H
