// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/oric_dsk.h

    Oric disk images

*********************************************************************/

#ifndef ORIC_DSK_H
#define ORIC_DSK_H

#include "flopimg.h"

class oric_dsk_format : public floppy_image_format_t
{
public:
	oric_dsk_format();
	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_ORIC_DSK_FORMAT;

#endif /* ORIC_DSK_H */
