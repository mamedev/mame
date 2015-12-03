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

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_DSK_FORMAT;

#endif /* CPCDSK_DSK_H */
