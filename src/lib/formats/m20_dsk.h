// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/m20_dsk.c

    Olivetti M20 floppy-disk images

*********************************************************************/

#ifndef M20_DSK_H
#define M20_DSK_H

#include "flopimg.h"

class m20_format : public floppy_image_format_t {
public:
	m20_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_M20_FORMAT;

#endif /* M20_DSK_H */
