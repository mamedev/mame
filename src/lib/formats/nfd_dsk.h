// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/nfd_dsk.h

    PC98 NFD disk images

*********************************************************************/

#ifndef NFD_DSK_H
#define NFD_DSK_H

#include "flopimg.h"


class nfd_format : public floppy_image_format_t
{
public:
	nfd_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_NFD_FORMAT;

#endif /* NFD_DSK_H */
