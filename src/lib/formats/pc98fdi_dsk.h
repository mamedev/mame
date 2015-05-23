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

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_PC98FDI_FORMAT;

#endif /* PC98FDI_DSK_H */
