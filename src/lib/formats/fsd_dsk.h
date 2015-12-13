// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro

    Disk image formats

***************************************************************************/

#pragma once

#ifndef __FSD_DSK_H__
#define __FSD_DSK_H__

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

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
};

extern const floppy_format_type FLOPPY_FSD_FORMAT;

#endif // __FSD_DSK_H__
