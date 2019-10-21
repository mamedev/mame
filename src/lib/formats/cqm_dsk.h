// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/cqm_dsk.h

    CopyQM disk images

*********************************************************************/
#ifndef MAME_FORMATS_CQM_DSK_H
#define MAME_FORMATS_CQM_DSK_H

#pragma once

#include "flopimg.h"

class cqm_format : public floppy_image_format_t
{
public:
	cqm_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_CQM_FORMAT;

#endif // MAME_FORMATS_CQM_DSK_H
