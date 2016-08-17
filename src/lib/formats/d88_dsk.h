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

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_D88_FORMAT;

#endif /* D88_DSK_H */
