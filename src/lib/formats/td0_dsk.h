// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef TD0_DSK_H_
#define TD0_DSK_H_

#include "flopimg.h"


class td0_format : public floppy_image_format_t
{
public:
	td0_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_TD0_FORMAT;

#endif /* TD0_DSK_H_ */
