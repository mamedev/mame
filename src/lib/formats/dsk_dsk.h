// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dsk_dsk.h

    CPC DSK disk images

*********************************************************************/

#ifndef CPCDSK_DSK_H
#define CPCDSK_DSK_H

#include "flopimg.h"

class dsk_format : public floppy_image_format_t
{
public:
	dsk_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_DSK_FORMAT;

#endif /* CPCDSK_DSK_H */
