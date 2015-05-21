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

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_NFD_FORMAT;

#endif /* NFD_DSK_H */
