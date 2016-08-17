// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Spectravideo SVI-318/328

    Disk image format

***************************************************************************/

#pragma once

#ifndef __SVI_DSK_H__
#define __SVI_DSK_H__

#include "flopimg.h"

class svi_format : public floppy_image_format_t
{
public:
	svi_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_SVI_FORMAT;

#endif // __SVI_DSK_H__
