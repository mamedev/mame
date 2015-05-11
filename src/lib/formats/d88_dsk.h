// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/d88_dsk.h

    D88 disk images

*********************************************************************/

#ifndef D88_DSK_H
#define D88_DSK_H

#include "flopimg.h"


class d88_format : public floppy_image_format_t
{
public:
	d88_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_D88_FORMAT;

#endif /* D88_DSK_H */
