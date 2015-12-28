// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/dip_dsk.h

    PC98DIP disk images

*********************************************************************/

#ifndef DIP_DSK_H
#define DIP_DSK_H

#include "flopimg.h"


class dip_format : public floppy_image_format_t
{
public:
	dip_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_DIP_FORMAT;

#endif /* DIP_DSK_H */
