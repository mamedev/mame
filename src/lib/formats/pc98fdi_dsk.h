// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/pc98fdi_dsk.h

    PC98FDI disk images

*********************************************************************/

#ifndef PC98FDI_DSK_H
#define PC98FDI_DSK_H

#include "flopimg.h"


class pc98fdi_format : public floppy_image_format_t
{
public:
	pc98fdi_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_PC98FDI_FORMAT;

#endif /* PC98FDI_DSK_H */
