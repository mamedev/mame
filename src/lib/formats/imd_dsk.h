// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/imd_dsk.h

    IMD disk images

*********************************************************************/

#ifndef IMD_DSK_H
#define IMD_DSK_H

#include "flopimg.h"


class imd_format : public floppy_image_format_t
{
public:
	imd_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	void fixnum(char *start, char *end) const;
};

extern const floppy_format_type FLOPPY_IMD_FORMAT;

#endif /* IMD_DSK_H */
